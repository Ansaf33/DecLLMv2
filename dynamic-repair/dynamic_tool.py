import os
import subprocess
from pathlib import Path
from typing import List, Optional, Dict, Tuple
import yaml
import shutil
from pathlib import Path
from ..utils.compile import Compiler
from ..utils.clean_errors import ErrorNormalizer
from ..utils.coreutils_parser import format_for_llm, parse_test_failure
from ..utils.llm_interface import create_llm_interface
from ..static_repair.static_tool_v2 import run_repair_loop_for_file


# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)
    
corpus_root = Path(config["paths"]["coreutils_corpus_root"])

      
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
    
# wait for this process to finish before proceeding
def change_name(dir: str, old_name: str, new_name: str) -> None:
    """
    Change the name of a file in a given directory from old_name to new_name.
    """
    old_path = Path(dir) / old_name
    new_path = Path(dir) / new_name
    print(f"Renaming {old_name} to {new_name}...")
    shutil.move(old_path, new_path)
    os.sync()
    
    
def re_execute(c_filename: str) -> None:
    """
    Re-execute any necessary commands after file name changes.
    """
    command = ["make", "src/" + Path(c_filename).stem]
    try:
      proc = subprocess.run(command, cwd=corpus_root, capture_output=True, text=True, check=True)
      if proc.returncode == 0:
        print(f"Restored original file {c_filename} compiled successfully.")
      else:
        print(f"Restored original file {c_filename} failed to compile.")
    except subprocess.CalledProcessError as e:
      print(f"Restored original file {c_filename} failed to compile due to exception: {str(e)}")

    
    pass



def get_output_error_prompt(error_log: str,c_code: str) -> str:
    """
    Generate a prompt for the LLM based on the compiler error log.
    """
    error_prompt_template = config["prompts"]["error_prompt"]
    prompt = f"{error_prompt_template}\n\nExecution Error Log:\n```\n{error_log}\n```"
    prompt += f"\n\nC Code:\n```c\n{c_code}\n```"
    return prompt

def dynamic_repair_test(c_file_path: str, test_case_path: str, max_iterations: int):
    """
    Run the dynamic repair loop for a given C file and its associated test case.
    """
    llm_interface = create_llm_interface(
        provider=config["llm"]["gemini_provider"],
        model_name=config["llm"]["gemini_model_name"],
        api_key=config["llm"]["gemini_api_key"]
    )
    repaired = False
    c_code = c_file_path.read_text()
    c_file_name = Path(c_file_path).stem
    test_name = test_case_path.name
    make_command = ["make", "src/" + c_file_name]
    run_test_command = ["make", "check-TESTS", f"TESTS=tests/{c_file_name}/{test_name}"]

    for iteration in range(max_iterations):
        print(f"Dynamic Repair Iteration {iteration + 1} for file {c_file_path.name}...")
        try:
            proc = subprocess.run(run_test_command, cwd = corpus_root, capture_output=True, text=True, check=True)
            if proc.returncode == 0:
                print(f"Test case {test_name} passed for file {c_file_path.name}.")
                repaired = True
                break
        except subprocess.CalledProcessError as e:
            proc = e
            
            
        # If the test fails, capture the error log
        message = proc.stdout + "\n" + proc.stderr
        print(f"Test case {test_name} failed for file {c_file_path.name} for iteration {iteration + 1}.")
        error_prompt = get_output_error_prompt(format_for_llm(message),c_code)
        
        # Generate optimized code using the LLM interface
        optimized_code = llm_interface.generate(error_prompt)
        
        # run static repair loop
        repaired, static_optimized_code = run_repair_loop_for_file(c_file_path, 3)
        
        if not repaired:
            print(f"Static repair failed for file {c_file_path.name} during dynamic repair iteration {iteration + 1}.")
            return repaired, optimized_code
        
        # replace code in file with llm-optimized code
        c_code = static_optimized_code
        
        # recompile the C file just to ensure no compilation errors
        try:
            proc = subprocess.run(make_command, cwd=corpus_root, capture_output=True, text=True, check=True)
            if proc.returncode != 0:
                print(f"Recompilation of {c_file_path.name} failed after optimization.")
                return repaired, c_code
            
        except subprocess.CalledProcessError as e:
            print(f"Recompilation of {c_file_path.name} failed due to exception: {str(e)}")
            return repaired, c_code
            
    return repaired, c_code  



def dynamic_repair_file(c_file_path: str, test_case_dir: str) -> Optional[str]:
    """
    Run the repair loop for a given C file and its associated test case directory.
    """
    c_code = c_file_path.read_text()
    repaired = False
    tests_passed = 0
    total_scripts = 0
    name = Path(c_file_path).stem
    
    # final output directory = dynamic_repair/ls/ls.c
    final_code_path = Path(config["paths"]["coreutils_dynamic_repair_path"]) / name / f"{name}.c"
    if not final_code_path.parent.exists():
        final_code_path.parent.mkdir(parents=True, exist_ok=True)
    
    # compile c_file_path using the make command
    command = ["make", "src/" + c_file_path.stem]
    try:
        proc = subprocess.run(command, cwd=corpus_root, capture_output=True, text=True, check=True)
        if proc.returncode != 0:
            print(f"Initial compilation of {c_file_path} failed.")
            return 0,0
    except subprocess.CalledProcessError as e:
        print(f"Initial compilation of {c_file_path} failed due to exception: {str(e)}")
        return 0,0
    
    # iterate through each test_case in test_case_dir (beginning with .sh)
    test_case = test_case_dir
    for test_script in test_case.glob("*.sh"):
        total_scripts += 1
        print(f"Running test case {test_script} for file {c_file_path.name}...")
        status, optimized_code = dynamic_repair_test(c_file_path, test_script, max_iterations)
        if status:
            tests_passed += 1
            print(f"File {c_file_path.name} repaired successfully using test case {test_script.name}.")
            c_code = optimized_code
            continue
        else:
            print(f"Test case {test_script.name} did not repair the file {c_file_path.name}.")
        # write optimized_code back to c_file_path
        c_file_path.write_text(optimized_code)
        
    # if repaired, write to final path
    final_code_path.write_text(c_code)
    return tests_passed, total_scripts


def initialize_dynamic_repair(static_repair_path: str):
    """
    Initialize the dynamic repair process by iterating through static repair outputs.
    """
    total_files = 0
    successfully_repaired = 0
    partially_repaired = 0
    
    # iterate through each c file in static_repair_path
    
    static_repair_dir = Path(static_repair_path)
    for c_file in static_repair_dir.glob("*.c"):
        total_files += 1
        
        # ensure test cases are present
        name = c_file.stem
        test_case_dir = get_coreutils_test_case(name)
        if test_case_dir is None:
            print(f"No test case found for file {c_file.name}, skipping dynamic repair.")
            continue
        
        # change name to temporary and add c file to src/
        change_name(corpus_root / "src", c_file.name, f"{c_file.stem}_temp.c")
        with open(corpus_root / "src" / c_file.name, 'w', encoding='utf-8') as f:
            with open(c_file, 'r', encoding='utf-8') as original_f:
                f.write(original_f.read())   
        print(f"Copied {c_file} to {corpus_root / 'src' / c_file.name}")
        
        
        tests_passed, total_scripts = dynamic_repair_file(c_file, test_case_dir)
        if tests_passed == total_scripts and total_scripts > 0:
            successfully_repaired += 1
            print(f"File {c_file.name} repaired successfully for all test cases.")
        elif tests_passed > 0:
            partially_repaired += 1
            print(f"File {c_file.name} partially repaired ({tests_passed}/{total_scripts} test cases passed).")
        else:
            print(f"File {c_file.name} could not be repaired for all test cases.")
        
        
        # restore original state
        os.remove(corpus_root / "src" / c_file.name)
        change_name(corpus_root / "src", f"{c_file.stem}_temp.c", c_file.name)
        os.sync()
        re_execute(c_file.name)
        
    return partially_repaired, successfully_repaired, total_files
    
    
    
def main():
    total = 0
    success = 0
    partial = 0
    
    static_repair_path = config["paths"]["coreutils_static_repair_path"]
    for static_repair_dir in Path(static_repair_path).glob("*"):
        partially_repaired, successfully_repaired, total_files = initialize_dynamic_repair(str(static_repair_dir))
        total += total_files
        success += successfully_repaired
        partial += partially_repaired
        print(f"Dynamic Repair Summary for {static_repair_dir.name}:")
        print(f"Total files processed: {total_files}")
        print(f"Successfully repaired files: {successfully_repaired}")
        print(f"Partially repaired files: {partially_repaired}")

        
    # Summary
    print("Overall Dynamic Repair Summary:")
    print(f"Total files processed: {total}")
    print(f"Successfully repaired files: {success}")
    print(f"Partially repaired files: {partial}")
    # Rates
    print(f"Overall Success Rate: {success/total*100:.2f}%")
    print(f"Overall Partial Repair Rate: {partial/total*100:.2f}%")
    
    
    
if __name__ == "__main__":
    main()