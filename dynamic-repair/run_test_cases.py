import os
from pathlib import Path
from typing import List, Optional, Dict, Tuple
import yaml
from pathlib import Path

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
      
      
