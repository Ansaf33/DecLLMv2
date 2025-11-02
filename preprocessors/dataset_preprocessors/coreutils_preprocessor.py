"""
Preprocessor to compile GNU coreutils source files into executables.
"""

import os
from pathlib import Path
from typing import Optional, Dict, Tuple, List
import yaml

from ..utils.compile import Compiler, OptimizationLevel
from ..utils.logger import setup_logger


# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent.parent / "config.yaml"
with open(CONFIG_PATH, "r") as f:
	config = yaml.safe_load(f)

DEFAULT_COREUTILS_ROOT = Path(config["paths"]["coreutils_corpus_root"])
DEFAULT_OUTPUT_DIR = Path(config["paths"]["coreutils_compiled_dir"])


logger = setup_logger("coreutils_preprocessor", None)

def compile_coreutils(
	corpus_root: Optional[str] = None,
	output_root: Optional[str] = None,
	opt_level: OptimizationLevel = OptimizationLevel.O0,
	gcc_path: str = "gcc",
	gpp_path: str = "g++",
) -> Dict[str, Tuple[bool, str, str]]:
  
  if corpus_root is None:
    corpus_root_path = DEFAULT_COREUTILS_ROOT
  else:
    corpus_root_path = Path(corpus_root)

  if output_root is None:
    out_root = DEFAULT_OUTPUT_DIR
  else:
    out_root = Path(output_root)

  src_dir = corpus_root_path / "src"
  if not src_dir.exists():
    raise FileNotFoundError(f"coreutils src directory not found: {src_dir}")

  compiler = Compiler(gcc_path=gcc_path, gpp_path=gpp_path)
  
  includes = []
  config_path = corpus_root_path / "lib" / "config.h"
  if config_path.exists():
    includes.append(str(config_path.parent))


  # iterate through each c file
  results: Dict[str, Tuple[bool, str, str]] = {}
  
  for c_file in src_dir.rglob("*.c"):
    relative_path = c_file.relative_to(src_dir)
    output_path = out_root / relative_path.stem
    
    if not output_path.parent.exists():
      output_path.parent.mkdir(parents=True, exist_ok=True)
        
    status, message = compiler.compile_source(
        source_file_path=str(c_file),
        output_file_path=str(output_path),
        is_cpp=False,
        opt=opt_level,
        extra_flags=None,
        include_dirs=includes,
        library_dirs=None,
        libraries=None,
        
    )
    
    results[str(relative_path)] = (status, message, str(output_path))
    
  
  
  return results


def main():
  opt = OptimizationLevel[config["compiler"]["optimization"]]
  results = compile_coreutils(
      corpus_root=str(DEFAULT_COREUTILS_ROOT),
      output_root=str(DEFAULT_OUTPUT_DIR),
      opt_level=opt,
      gcc_path=config["compiler"]["gcc_path"],
      gpp_path=config["compiler"]["gpp_path"],
  )
  success_count = sum(1 for success, _, _ in results.values() if success)
  total_count = len(results)
  logger.info(f"Compiled {success_count}/{total_count} coreutils source files successfully.")
  
  
  
  
if __name__ == "__main__":
    main()
  
  
    
  
  
  
  
