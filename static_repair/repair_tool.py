import yaml
from pathlib import Path
from ..utils.reassembler import Reassembler
from ..utils.llm_interface import create_llm_interface, clean_llm_output
from ..utils.compile import Compiler,OptimizationLevel
from ..utils.clean_errors import ErrorNormalizer
import re
import shutil
import tempfile

c = Compiler()

# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)
    
CGC_STATIC_REPAIR_PATH = config["paths"]["cgc_static_repair_path"]
COREUTILS_STATIC_REPAIR_PATH = config["paths"]["coreutils_static_repair_path"]
    

def get_initial_prompt(c_code: str) -> str:
    """
    Generate the initial prompt for the repair tool given C code of the particular function.
    """
    initial_prompt = config["prompts"]["system_prompt"]
    prompt = f"{initial_prompt}\n\n```c\n{c_code}\n```"
    return prompt
  
def get_optimized_code(original_c_code: str, max_iterations: int) -> str:
    """
    Generate optimized C code using LLM for the given original C code file.
    """
    final_c_code = original_c_code
    
    llm_interface = create_llm_interface(
        provider=config["llm"]["gemini_provider"],
        model_name=config["llm"]["gemini_model_name"],
        api_key=config["llm"]["gemini_api_key"]
    )
    
    # work in temporary directory
    with tempfile.TemporaryDirectory() as temp_dir:
      temp_c_path = Path(temp_dir) / "temp.c"
      
      # copy original file to temp
      with open(temp_c_path, 'w', encoding='utf-8') as f:
        f.write(original_c_code)
        
      # check if original code compiles
      status, message = c.compile_source(
        source_file_path=str(temp_c_path),
        output_file_path=str(Path(temp_dir) / "temp"),
        is_cpp=False,
        opt=OptimizationLevel.O0,
        extra_flags=[],
      )
      if status:
        print(f"Original code compiles successfully. No optimization needed.")
        final_c_code = original_c_code
        repaired = True
        return repaired, final_c_code
      
        
      # send initial prompt to llm and acquire response
      prompt = get_initial_prompt(original_c_code)
      print("Sending initial prompt to LLM...")
      optimized_code = clean_llm_output(llm_interface.generate(prompt))
      
      # replace code in file with llm-optimized code
      with open(temp_c_path, 'w', encoding='utf-8') as f:
        f.write(optimized_code)
        
      # check whether it compiles
      status, message = c.compile_source(
        source_file_path=str(temp_c_path),
        output_file_path=str(Path(temp_dir) / "temp"),
        is_cpp=False,
        opt=OptimizationLevel.O0,
        extra_flags=[],
        include_dirs=[],
        library_dirs=[],
        libraries=[]
      )
      repaired = status
      
      if repaired:
        print(f"File compiled successfully after initial LLM optimization.")
        final_c_code = optimized_code
        
        return repaired, final_c_code
      
      else:
        print(f"File failed to compile after initial LLM optimization.")
        
      #  -- if compilation fails, enter repair loop --
      for iteration in range(max_iterations):
        error_normalizer = ErrorNormalizer()
        compilation_errors = error_normalizer.format_for_llm(message)
        error_prompt = config["prompts"]["compilation_error"]
        repaired_code = clean_llm_output(llm_interface.generate(error_prompt))
        
        # replace code in file with llm-repaired code
        with open(temp_c_path, 'w', encoding='utf-8') as f:
          f.write(repaired_code)
        # check whether it compiles
        status, message = c.compile_source(
          source_file_path=str(temp_c_path),
          output_file_path=str(Path(temp_dir) / "temp")
        )
        if status:
          total_repaired += 1
          final_c_code = repaired_code
          print(f"File compiled successfully after iteration {iteration+1}.")
          repaired = True
          break
        else:
          optimized_code = repaired_code
          # print errors
          #print(f"Errors:\n{message}")
          print(f"File failed to compile after iteration {iteration+1}.")
      
    return repaired, final_c_code
      
        


def repair_loop(json_path: str, max_iterations: int = 3) -> str:
    """
    Main repair loop that attempts to fix compilation errors using LLM.
    Args:
        json_path: Path to the JSON file containing decompiled code.
        max_iterations: Maximum number of repair iterations.
    Returns:
        Path to the repaired C code file.
    """
    # create temporary directory to store C files
    r = Reassembler()
    output_dir, file_mapping = r.reassemble_project(json_path)
    print(output_dir)
    
    # create actual output directory
    if "cgc-challenge-corpus" in json_path:
      project_path = CGC_STATIC_REPAIR_PATH
    elif "coreutils" in json_path:
      project_path = COREUTILS_STATIC_REPAIR_PATH
      
    # dir name is prefix whose end is __
    real_output_dir = Path(project_path) / Path(output_dir).name.split("__")[0]
   
    real_output_dir.mkdir(parents=True, exist_ok=True)
    
    #stats
    total_files = 0
    total_repaired = 0
    
  
    # iterate over each file in the project
    for c_filename, c_file_path in file_mapping.items():
        
      with open(c_file_path, 'r', encoding='utf-8') as f:
        c_code = f.read()
        
      # store origin c_code
      original_c_code = c_code
      final_c_code = c_code
      
      # check if original code compiles
      status, message = c.compile_source(
        source_file_path=str(c_file_path),
        output_file_path=str(Path(output_dir) / Path(c_filename).stem),
        is_cpp=False,
        opt=OptimizationLevel.O0,
        extra_flags=[],
        include_dirs=[],
        library_dirs=[],
        libraries=[]
      )
      if not status:
        #print(f"Errors:\n{message}")
        print(f"Ghidra File {c_filename} failed to compile.")
      else:
        final_c_code = c_code
        continue
        
      total_files += 1
        
      # send initial prompt to llm and acquire response
      llm_interface = create_llm_interface(
        provider=config["llm"]["gemini_provider"],
        model_name=config["llm"]["gemini_model_name"],
        api_key=config["llm"]["gemini_api_key"]
        )
      print("Sending initial prompt to LLM...")
      prompt = get_initial_prompt(c_code)
      optimized_code = clean_llm_output(llm_interface.generate(prompt))

      # replace code in file with llm-optimized code
      with open(c_file_path, 'w', encoding='utf-8') as f:
        f.write(optimized_code)
        
      # check whether it compiles
      status, message = c.compile_source(
        source_file_path=str(c_file_path),
        output_file_path=str(Path(output_dir) / Path(c_filename).stem),
        is_cpp=False,
        opt=OptimizationLevel.O0,
        extra_flags=[],
        include_dirs=[],
        library_dirs=[],
        libraries=[]
      )
      repaired = status
      
      if repaired:
        total_repaired += 1
        print(f"File {c_filename} compiled successfully after initial LLM optimization.")
        final_c_code = optimized_code
        # finally, write the (possibly repaired) code to the real output directory
        final_code_path = real_output_dir / c_filename
        with open(final_code_path, 'w', encoding='utf-8') as f:
          f.write(final_c_code)
        continue
      
      else:
        #print(f"Errors:\n{message}")
        print(f"File {c_filename} failed to compile after initial LLM optimization.")
        
      #  -- if compilation fails, enter repair loop --
      for iteration in range(max_iterations):
        error_normalizer = ErrorNormalizer()
        compilation_errors = error_normalizer.format_for_llm(message)
        error_prompt = config["prompts"]["compilation_error"]
        repaired_code = clean_llm_output(llm_interface.generate(prompt))
        # replace code in file with llm-repaired code
        with open(c_file_path, 'w', encoding='utf-8') as f:
          f.write(repaired_code)
        # check whether it compiles
        status, message = c.compile_source(
          source_file_path=str(c_file_path),
          output_file_path=str(Path(output_dir) / Path(c_filename).stem)
        )
        if status:
          total_repaired += 1
          final_c_code = repaired_code
          print(f"File {c_filename} compiled successfully after iteration {iteration+1}.")
          repaired = True
          break
        else:
          optimized_code = repaired_code
          # print errors
          #print(f"Errors:\n{message}")
          print(f"File {c_filename} failed to compile after iteration {iteration+1}.")
        
      # restore original code if still not repaired   
      if not repaired:
        print(f"File {c_filename} could not be repaired after {max_iterations} iterations. Restoring original code.")
        with open(c_file_path, 'w', encoding='utf-8') as f:
          f.write(original_c_code)
      
          
      # finally, write the (possibly repaired) code to the real output directory
      final_code_path = real_output_dir / c_filename
      with open(final_code_path, 'w', encoding='utf-8') as f:
        f.write(final_c_code)
        
    r.cleanup_temp_directory(output_dir)
        
    return total_repaired, total_files
        
    
        
      
def main():
  # status
  total_repaired = 0
  total_files = 0
  json_dir = config["paths"]["cgc_decompiled_dir"]
  
  # iterate through every object and repair
  for json_file in Path(json_dir).glob("*.json"):
    print(f"Starting repair for {json_file}...")
    repaired, files = repair_loop(str(json_file), max_iterations=3)
    total_repaired += repaired
    total_files += files
 
  # print compilation rate
  print(f"Total files repaired: {total_repaired}/{total_files} ({(total_repaired/total_files)*100:.2f}%)")
  

  
      
      
      
if __name__ == "__main__":
    main()
      
    
      
      
      
      
    
    
    
    
        
        
        

        
        
        
        
        
    
    
    
 
    
    
    
    
    
    

    
    
    
