import os
import subprocess
import tempfile
from pathlib import Path
from typing import List, Optional, Tuple, Dict
from enum import Enum
import argparse
import shutil
import yaml

# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)

# import compiler from the utils

class Ghidra:
    def __init__(self, ghidra_path: str = config["paths"]["ghidra_path"], post_script: str = config["paths"]["postscript_path"]):
      self.ghidra_path = ghidra_path
      self.post_script = post_script

    def convert_executable_to_ghidra(
      self,
      executable_path: str,
      output_dir: str) -> List[Tuple[bool, str]]:
      """
      Convert an executable binary to Ghidra decompiled pseudo C code.
      Args:
          executable_path: Path to the executable binary
          output_dir: Directory to save the decompiled output
      Returns:
          List of tuples (success, output_path or error_message)
      """
      print(f"Starting Ghidra decompilation for {executable_path}")
      executable_path = Path(executable_path)
      results = []
      
      print("Here!")

      if not executable_path.exists():
        msg = f"Executable not found: {executable_path}"
        results.append((False, msg))
        return results
      
      print("There!")
        
      # if output directory exists, delete it to avoid conflicts
      output_path = Path(output_dir)
      output_path.mkdir(parents=True, exist_ok=True)
      output_file = output_path / f"{executable_path.stem}_decompiled.c"
      command = [
          self.ghidra_path,
          output_dir,
          "tmp_ghidra_proj",
          "-import", executable_path,
          "-postScript", self.post_script, output_path,
          "-deleteProject",
      ]
      
      try:
        subprocess.run(command,check=True,capture_output=True)
        print(f"Ghidra decompilation succeeded for {executable_path.name}")
      except Exception as e:
        print(f"Ghidra decompilation failed: {e}")
        

      # Print the decompiled output
      with open(output_file,'w') as f:
        f.write(f"// Decompiled output for {executable_path.name}\n")

      return results
          
def main():
  compiled_dir_path = "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/DecLLMv2/data/cgc-challenge-corpus/compiled/"
  decompiled_dir_path = "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/DecLLMv2/data/cgc-challenge-corpus/decompiled/"

  compiled_file_path = compiled_dir_path + "CADET_00001"

  ghidra = Ghidra()
  ghidra.convert_executable_to_ghidra(
      compiled_file_path,
      decompiled_dir_path
  )


if __name__=="__main__":
    main()     