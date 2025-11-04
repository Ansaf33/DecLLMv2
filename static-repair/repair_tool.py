import yaml
from pathlib import Path



# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)



def get_initial_prompt(c_code: str) -> str:
    """
    Generate the initial prompt for the repair tool given C code of the particular function.
    """
    initial_prompt = config["prompts"]["system_prompt"]
    prompt = f"{initial_prompt}\n\n```c\n{c_code}\n```"
    return prompt
  


def repair_loop(json_path: str, max_iterations: int = 3) -> str:
    """
    Main repair loop that attempts to fix compilation errors using LLM.
    Args:
        json_path: Path to the JSON file containing decompiled code.
        max_iterations: Maximum number of repair iterations.
    Returns:
        Path to the repaired C code file.
        
    Steps:
    1. For each C file in the JSON object
        i. Generate initial prompt with c-file code
        ii. For up to max_iterations:
            - Send prompt to LLM and get response
            - Compile the modified code
            - If compilation succeeds, break
            - If compilation fails, extract errors and update prompt
    """
    # create temporary directory to store C files
    with tempfile.TemporaryDirectory(prefix="repair_tool_") as temp_dir:
        temp_dir_path = Path(temp_dir)
        
        '''
        JSON Object Structure
        {
          "file": "challenge_name",
          "decompiled_code": {
            "file1.c": [
              {
                "func_name": "function1",
                "decompiled_code": "C code of function1"
              },
              ...
            ],
            "file2.c": [
              ...
            ],
            ...
          }
        }
        '''
        
        
        

        
        
        
        
        
    
    
    
 
    
    
    
    
    
    

    
    
    
