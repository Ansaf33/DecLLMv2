import yaml
from pathlib import Path
from ..utils.reassembler import Reassembler
from ..utils.llm_interface import create_llm_interface, clean_llm_output
from ..utils.compile import Compiler,OptimizationLevel
from ..utils.clean_errors import ErrorNormalizer
import re
import shutil
import tempfile
import os
from typing import Tuple, List, Dict


import subprocess
c = Compiler()

# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)
    
corpus_root = Path(config["paths"]["coreutils_corpus_root"])
    
CGC_STATIC_REPAIR_PATH = config["paths"]["cgc_static_repair_path"]
COREUTILS_STATIC_REPAIR_PATH = config["paths"]["coreutils_static_repair_path"]
    

def get_initial_prompt(c_code: str) -> str:
    """
    Generate the initial prompt for the repair tool given C code of the particular function.
    """
    initial_prompt = config["prompts"]["system_prompt"]
    prompt = f"{initial_prompt}\n\n```c\n{c_code}\n```"
    return prompt
  
      
# wait for this process to finish before proceeding
def change_name(dir: str, old_name: str, new_name: str) -> None:
    """
    Change the name of a file in a given directory from old_name to new_name.
    Args:
        dir: Directory path
        old_name: Old file name
        new_name: New file name
    """
    old_path = Path(dir) / old_name
    new_path = Path(dir) / new_name
    print(f"Renaming {old_path} to {new_path}")
    shutil.move(old_path, new_path)
    os.sync()
    
    
    

def run_repair_loop_for_file(c_file_path: str, max_iterations: int = 3) -> Tuple[bool, str]:
  """
  Run the repair loop for a single C file.
  Args:
      c_filename: Name of the C file to repair
      max_iterations: Maximum number of repair iterations
  Returns:
      repaired: Whether the file was successfully repaired
      optimized_code: The final optimized C code
  """
  repaired = False
  
  llm_interface = create_llm_interface(
    provider=config["llm"]["gemini_provider"],
    model_name=config["llm"]["gemini_model_name"],
    api_key=config["llm"]["gemini_api_key"]
    )
  
  c_filename = Path(c_file_path).name
  
  with open(c_file_path, 'r', encoding='utf-8') as f:
    c_code = f.read()
  optimized_code = c_code
    
  # send initial prompt to llm and acquire response
  print("Sending initial prompt to LLM...")
  prompt = get_initial_prompt(c_code)
  optimized_code = clean_llm_output(llm_interface.generate(prompt))

  # replace code in file with llm-optimized code
  with open(c_file_path, 'w', encoding='utf-8') as f:
    f.write(optimized_code)
    
  # check whether it compiles
  message = ""
  try:
    proc = subprocess.run(
      ["make", "src/" + Path(c_filename).stem],
      cwd=corpus_root,
      capture_output=True,
      text=True,
      timeout=30
    )
    message = proc.stderr
    if proc.returncode == 0:
      print(f"File {c_filename} compiled successfully after initial LLM optimization.")
      repaired = True
      return repaired, optimized_code
  except subprocess.CalledProcessError as e:
    print(f"File {c_filename} failed to compile after initial LLM optimization due to exception: {str(e)}")

  #  -- if compilation fails, enter repair loop --
  for iteration in range(max_iterations):
    
    error_normalizer = ErrorNormalizer()
    compilation_errors = error_normalizer.format_for_llm(message)
    print(f"Compilation errors for LLM:\n{compilation_errors}\n")
    error_prompt = config["prompts"]["compilation_error"]
    error_prompt += f"\n\nCompilation Errors:\n{compilation_errors}\n\n```c\n{optimized_code}\n```"
    optimized_code = clean_llm_output(llm_interface.generate(error_prompt))
    
    # replace code in file with llm-repaired code
    with open(c_file_path, 'w', encoding='utf-8') as f:
      f.write(optimized_code)
      
    # check whether it compiles
    try:
      proc = subprocess.run(
        ["make", "src/" + Path(c_filename).stem],
        cwd=corpus_root,
        capture_output=True,
        text=True,
        timeout=30
      )
      message = proc.stderr
      if proc.returncode == 0:
        print(f"File {c_filename} compiled successfully after iteration {iteration+1}.")
        repaired = True
        return repaired, optimized_code
    except subprocess.CalledProcessError as e:
      print(f"File {c_filename} failed to compile after iteration {iteration+1} due to exception: {str(e)}")
      
      
  return repaired, optimized_code  
    
def initialize_repair_loop(json_path: str) -> str:
  """
  Initialize the repair loop by reassembling the project from JSON.
  Args:
      json_path: Path to the JSON file containing decompiled code.
  Returns:
      Path to the temporary output directory containing C files.
  """
  # temporary directory to store C files
  r = Reassembler()
  output_dir, file_mapping = r.reassemble_project(json_path)
  
  # create actual output directory
  if "cgc-challenge-corpus" in json_path:
    project_path = CGC_STATIC_REPAIR_PATH
  elif "coreutils" in json_path:
    project_path = COREUTILS_STATIC_REPAIR_PATH
  
  real_output_dir = Path(project_path) / Path(output_dir).name.split("__")[0]
  real_output_dir.mkdir(parents=True, exist_ok=True)
  
  for c_filename, c_file_path in file_mapping.items():
    repaired = False
    final_code_path = real_output_dir / c_filename
    
    # since we are handling coreutils, change name in corpus from x.c to x_temp.c
    # wait for it to complete
    change_name(corpus_root / "src", c_filename, f"{Path(c_filename).stem}_temp.c")
    
    
    # create a new c file and write c_file_path content into it
  
    with open(corpus_root / "src" / c_filename, 'w', encoding='utf-8') as f:
      with open(c_file_path, 'r', encoding='utf-8') as original_f:
        f.write(original_f.read())
      
    print(f"Copied {c_file_path} to {corpus_root / 'src' / c_filename}")
    
    # check if it compiles
    command = ["make", "src/" + Path(c_filename).stem]
    try:
      proc = subprocess.run(command, cwd=corpus_root, capture_output=True, text=True, check=True)
      if proc.returncode == 0:
        print(f"Ghidra File {c_filename} compiled successfully on initial attempt.")
        # copy the optimized code to real output dir
        shutil.copy(corpus_root / "src" / c_filename, final_code_path)
        break          
    except subprocess.CalledProcessError as e:
      print(f"Ghidra File {c_filename} failed to compile on initial attempt.")
      # Run the repair loop
      max_iterations = 5
      repaired, optimized_code = run_repair_loop_for_file(corpus_root / "src" / c_filename, max_iterations)
      if repaired:
        # write the optimized code to real output dir
        with open(final_code_path, 'w', encoding='utf-8') as f:
          f.write(optimized_code)
        print(f"File {c_filename} repaired and written to output directory.")
      else:
        print(f"File {c_filename} could not be repaired after {max_iterations} iterations. Restoring original code.")
        # copy the original ghidra code to real output dir
        shutil.copy(c_file_path, final_code_path)   

       
    # delete temporary files and restore original
    print("Cleaning up temporary files...")
    os.remove(corpus_root / "src" / c_filename)
    change_name(corpus_root / "src", f"{Path(c_filename).stem}_temp.c", c_filename)
  
    
  r.cleanup_temp_directory(output_dir)
  
  
  
        
def main():
  # status
  json_path = "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/DecLLMv2/data/coreutils/decompiled/chcon_decompiled.json"
  output_dir = initialize_repair_loop(json_path)
  print(f"Reassembled C files are in: {output_dir}")
      
      
      
if __name__ == "__main__":
    main()
      
    
      
      
      
      
    
    
    
    
        
        
        

        
        
        
        
        
    
    
    
 
    
    
    
    
    
    

    
    
    
