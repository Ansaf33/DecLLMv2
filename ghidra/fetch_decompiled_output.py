import os
import subprocess
import tempfile
from pathlib import Path
from typing import List, Optional, Tuple, Dict
from enum import Enum
import argparse
import shutil
import yaml
from .ghidra import *
from .get_whitelists import *
from .ghidra_parser import *
import json

# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)
          
def main():
  compiled_dir_path = config["paths"]["coreutils_compiled_dir"]
  decompiled_dir_path = config["paths"]["coreutils_decompiled_dir"]

  # iterate through every file in compiled_dir_path
  ghidra = Ghidra()
  for file in os.listdir(compiled_dir_path):
    
    #output json file
    json_file = os.path.join(decompiled_dir_path, f"{file}_decompiled.json")
    
    compiled_file = os.path.join(compiled_dir_path, file)
    status, data = ghidra.convert_executable_to_ghidra(compiled_file, decompiled_dir_path)
    if status:
      # type = Dict[str,List[Dict]] -> c_file, functions: List[Dict]
      filtered_content = {}
      # get whitelists
      whitelisted_functions = get_coreutils_whitelist_functions(file)
      '''
      only those functions in whitelisted_functions must be present in the filtered_content
      whitelisted
      { c_file : [func1, func2, ... ], c_file2: [ ... ] }
      data
      { functions : [ { func_name: func_name, code: func_code }, ... ] }, { ... } ]
      '''
      for function in data["functions"]:
        func_name = function["func_name"]
        # check if present in whitelist of any key
        for c_file, func_list in whitelisted_functions.items():
          if func_name in func_list:
            if c_file not in filtered_content:
              filtered_content[c_file] = []
            filtered_content[c_file].append(function)
            break
            
      # write to json file
      with open(json_file, "w") as jf:
        json.dump({
          "file": file,
          "decompiled_code": filtered_content,
        }, jf, indent=4)
      print(f"Decompiled output written to: {json_file}")
    else:
      print(f"Decompilation failed for {compiled_file}: {data}")
      
    
  
    


if __name__=="__main__":
    main()     