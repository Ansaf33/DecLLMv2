import os
import subprocess
from pathlib import Path
from typing import List, Optional, Dict, Tuple
import yaml
from pathlib import Path
from ..utils.compile import Compiler
from ..utils.clean_errors import ErrorNormalizer
from ..utils.coreutils_parser import format_for_llm, parse_test_failure
from ..utils.llm_interface import create_llm_interface
from ..static_repair.repair_tool import get_optimized_code

c = Compiler()

# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)

def get_cgc_test_case(challenge_path: str) -> Optional[Path]:
    """
    Given a CGC challenge path, return the path to its test case directory
    """
    challenge_name = Path(challenge_path).name
    corpus_root = Path(config["paths"]["cgc_corpus_root"])
    challenge_dir = corpus_root / challenge_name
    poller_path = challenge_dir / "poller"
    pov_path = challenge_dir / "pov"
    
    if poller_path.exists() and pov_path.exists():
        return poller_path,pov_path
    else:
        return None
      
def get_coreutils_test_case(utility_path: str) -> Optional[Path]:
    """
    Given a coreutils utility path, return the path to its test case directory
    """
    utility_name = Path(utility_path).name
    corpus_root = Path(config["paths"]["coreutils_corpus_root"])
    test_dir = corpus_root / "tests" / utility_name
    
    if test_dir.exists():
        return test_dir
    else:
        return None

def get_output_error_prompt(error_log: str,c_code: str) -> str:
    """
    Generate a prompt for the LLM based on the compiler error log.
    """
    error_prompt_template = config["prompts"]["error_prompt"]
    prompt = f"{error_prompt_template}\n\nExecution Error Log:\n```\n{error_log}\n```"
    prompt += f"\n\nC Code:\n```c\n{c_code}\n```"
    return prompt


def restore_original_executable(utility_name: str):
    '''
    Restore the original executable from the compiled directory to the corpus src directory.
    '''
    #utility name without o
    utility_n = utility_name
    if utility_name.endswith(".o"):
        utility_n = utility_name[:-2]
        
    exec_dir = Path(config["paths"]["coreutils_corpus_root"]) / "src"
    src = Path(config["paths"]["coreutils_compiled_dir"]) / utility_n
    dest = exec_dir / utility_name
    command = f"cp {src} {dest}"
    proc = subprocess.run(command, shell=True)
    if proc.returncode == 0:
        print(f"Restored original executable for {utility_name}")
    else:
        print(f"Failed to restore original executable for {utility_name}")



def execute_test_scripts(test_case_dir: Path, src_file_path: Path, max_iterations: int) -> Tuple[int,int]:
    '''
    Execute the test scripts in the given test case directory.
    '''
    os.chdir(Path(config["paths"]["coreutils_corpus_root"]))
    llm_interface = create_llm_interface(
                provider=config["llm"]["gemini_provider"],
                model_name=config["llm"]["gemini_model_name"],
                api_key=config["llm"]["gemini_api_key"]
            )
    

    # iterate through each .sh file in the test_case_dir
    total_tests = 0
    total_passed = 0
    name = src_file_path.stem
    for test_script in test_case_dir.glob("*.sh"):
        total_tests += 1
        command = ['make', 'check-TESTS', f'TESTS=tests/{name}/{test_script.stem}']
        
        # Run the test command and capture stdout/stderr for logging and LLM feedback
        try:
            proc = subprocess.run(command, shell=True, capture_output=True, text=True)
            result = proc.returncode
            
            if result == 0:
                total_passed += 1
                print(f"Test {test_script.name} passed.")
                continue 
                        
            # iterative dynamic repair loop
            print(f"Test {test_script.name} failed. Initiating dynamic repair.")
            for i in range(max_iterations):
                print(f"Dynamic repair iteration {i+1} for test name {test_script.name} for file {src_file_path.name}.")
                error_log = proc.stderr

                # create the prompt
                c_code = src_file_path.read_text()
                error = format_for_llm(parse_test_failure(error_log))
                print(f"Parsed error for LLM:\n{error}")
                prompt = get_output_error_prompt(error_log,c_code)
                optimized_code = llm_interface.generate(prompt)

                # pass through static repair
                print("Passed optimized code to static repair module.")
                repaired, optimized_c_code = get_optimized_code(optimized_code, 3)
                print("Received optimized code from static repair module.")
                
                if not repaired:
                    print(f"LLM suggested code failed to compile.")
                    break
                
                # write the optimized code to the src file path
                with open(src_file_path, "w") as f:
                    f.write(optimized_c_code)
                    
                # recompile, give permission to executables and re-execute
                status, message = c.compile_source(
                    source_file_path=src_file_path,
                    output_file_path=Path(config["paths"]["coreutils_corpus_root"])/"src"/name
                    
                )
                status, message = c.compile_source(
                    source_file_path=src_file_path,
                    output_file_path=Path(config["paths"]["coreutils_corpus_root"])/"src"/(name + ".o")
                )
                if not status:
                    print(f"Recompiled code failed to compile: {message}")
                    break
                                
                # re-execute the test script
                proc = subprocess.run(command, shell=True, capture_output=True, text=True)
                result = proc.returncode
                if result == 0:
                    total_passed += 1
                    print(f"Test {test_script.name} passed after {i+1} iterations.")
                    break
                else:
                    print(f"Test {test_script.name} still failing after {i+1} iterations.")
                
        except Exception:
            # If logging fails, continue but keep behavior
            result = -1

    return total_passed, total_tests

  
def dynamic_repair_tool(static_repaired_dir: str) -> Tuple[int,int]:
    
    # get the test case directory based on the static repaired directory and create dynamic output directory
    name = Path(static_repaired_dir).name
    name_o = name + ".o"
    
    exec_dir = Path(config["paths"]["coreutils_corpus_root"]) / "src"
    exec_path = exec_dir / name
    exec_path_o = exec_dir / name_o
    
    total_tests = 0
    total_passed = 0
    
    if "coreutils" in static_repaired_dir:
        test_case_dir = get_coreutils_test_case(static_repaired_dir)
        dynamic_output_dir = Path(config["paths"]["coreutils_dynamic_repair_path"]) / name
    
    # create directory if not exists
    if not dynamic_output_dir.exists():
        dynamic_output_dir.mkdir(parents=True, exist_ok=True)
    
    # iterate through each file in the project
    for c_file in Path(static_repaired_dir).glob("*.c"):
        
        # copy C file to dynamic_output_dir
        dynamic_c_file_path = dynamic_output_dir / c_file.name
        with open(c_file, "r") as src_file:
            with open(dynamic_c_file_path, "w") as dest_file:
                dest_file.write(src_file.read())
                
                
        # compile the C file and place the binary inside corpus
        c = Compiler()
        print(f"Compiling {dynamic_c_file_path} and placing binary in {exec_path}")
        status, message = c.compile_source(
            source_file_path=dynamic_c_file_path,
            output_file_path=exec_path
        )
        # won't happen since already verified
        if not status:
            print(f"Compilation failed for {c_file.name}: {message}")
            continue
        # compile the C file and place the binary inside corpus, provide executable permissions
        print(f"Compiling {dynamic_c_file_path} and placing binary in {exec_path_o}")
        status, message = c.compile_source(
            source_file_path=dynamic_c_file_path,
            output_file_path=exec_path_o
        )
        # won't happen since already verified
        if not status:
            print(f"Compilation failed for {c_file.name}: {message}")
            continue
    
        
        # run the test scripts
        passed, tests = execute_test_scripts(test_case_dir, dynamic_c_file_path, 3)
        print(f"Test results for {c_file.name}: {passed}/{tests} test cases passed.")
        total_passed += passed
        total_tests += tests
        
    
        
        # copy the original executable from src to dest
        restore_original_executable(name)
        restore_original_executable(name_o)
    
    return total_passed, total_tests


def main():
    # Example usage
    static_repaired_dir = "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/DecLLMv2/output/static_repair_output/coreutils/id"
    passed, total = dynamic_repair_tool(static_repaired_dir)
    print(f"Dynamic Repair Results: {passed}/{total} test cases passed.")
 

    
    
if __name__ == "__main__":
    main()
    
    