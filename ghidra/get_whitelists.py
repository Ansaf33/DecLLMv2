"""
Return a whitelist of function names for a single source file.
"""
from typing import Dict, List
from pathlib import Path
import os
import re
import yaml

# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)
    



def get_single_file_whitelist(file_name: str) -> Dict[str, List[str]]:
  # defensive: ensure file exists
  if not os.path.isfile(file_name):
    return {}

  try:
    with open(file_name, "r", encoding="utf-8", errors="ignore") as f:
      src = f.read()
  except Exception:
    return {}

  src_nocomments = re.sub(r"/\*.*?\*/", "", src, flags=re.DOTALL)
  src_nocomments = re.sub(r"//.*?$", "", src_nocomments, flags=re.MULTILINE)

  func_pattern = re.compile(
    r"^\s*[a-zA-Z_][\w\s\*]*?\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*(?:\{)",
    flags=re.MULTILINE,
  )

  func_names = []

  # Exclude common C keywords and control constructs which might be
  # accidentally matched as identifiers in pathological cases.
  C_KEYWORDS = {
    'if', 'else', 'while', 'for', 'switch', 'case', 'break', 'continue',
    'return', 'goto', 'do', 'sizeof', 'struct', 'union', 'enum', 'static',
    'inline', 'const', 'volatile', 'extern', 'register', 'default', 'typedef',
    'auto', 'signed', 'unsigned', 'short', 'long', 'int', 'char', 'float', 'double',
    'void', 'bool'
  }

  for m in func_pattern.finditer(src_nocomments):
    name = m.group(1)
    if name in C_KEYWORDS:
      continue
    if name not in func_names:
      func_names.append(name)

  header_pattern = re.compile(r"^\s*[a-zA-Z_][\w\s\*]*?\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*$", flags=re.MULTILINE)
  for m in header_pattern.finditer(src_nocomments):
    name = m.group(1)
    # position after the match
    endpos = m.end()
    lookahead = src_nocomments[endpos:endpos + 200]
    if name in C_KEYWORDS:
      continue
    if "{" in lookahead and name not in func_names:
      func_names.append(name)

  # basename for the key
  base = os.path.basename(file_name)
  return {base: func_names}


def get_directory_whitelist(dir_path: str) -> Dict[str, List[str]]:
  """Return a whitelist of function names for all source files in a directory.

  Args:
    dir_path: Path to a directory containing C/C++ source files.

  Returns:
    A dictionary mapping base filenames to lists of function names.
  """
  if not os.path.isdir(dir_path):
    return {}

  whitelist: Dict[str, List[str]] = {}
  for entry in os.listdir(dir_path):
    full_path = os.path.join(dir_path, entry)
    if os.path.isfile(full_path) and entry.endswith(('.c', '.cpp', '.cc', '.cxx')):
      file_whitelist = get_single_file_whitelist(full_path)
      whitelist.update(file_whitelist)

  return whitelist

def get_cgc_whitelist_functions(compiled_file: str) -> Dict[str, List[str]]:
  """Return the whitelist of function names for a given CGC compiled file.

  Args:
    compiled_file: Name of the compiled CGC challenge file.
  Returns:
    A dictionary mapping base filenames to lists of function names.
  """
  directory_path = os.path.join(config["paths"]["cgc_corpus_root"], compiled_file, "src")
  print(directory_path)
  return get_directory_whitelist(directory_path)

def get_coreutils_whitelist_functions(compiled_file: str) -> Dict[str, List[str]]:
  """Return the whitelist of function names for a given coreutils compiled file.

  Args:
    compiled_file: Name of the compiled coreutils file.
  Returns:
    A dictionary mapping base filenames to lists of function names.
  """
  file_path = os.path.join(config["paths"]["coreutils_corpus_root"], "src", compiled_file + ".c")
  return get_single_file_whitelist(file_path)
