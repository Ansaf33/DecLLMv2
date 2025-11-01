"""
Preprocessor to compile CGC Challenge Corpus samples into single executables.
"""

import os
from pathlib import Path
from typing import List, Optional, Dict, Tuple
from ..utils.compile import Compiler, OptimizationLevel
from ..utils.logger import setup_logger
import yaml
from pathlib import Path

# Config.yaml paths
CONFIG_PATH = Path(__file__).resolve().parent.parent.parent / "config.yaml"
print(f"Loading config from: {CONFIG_PATH}")

with open(CONFIG_PATH, "r") as f:
    config = yaml.safe_load(f)

DEFAULT_CORPUS_ROOT = config["paths"]["cgc_corpus_root"]
DEFAULT_OUTPUT_DIR = config["paths"]["cgc_output_dir"]
DEFAULT_LIBCGC_PATH = config["paths"]["cgc_libcgc_path"]


# Keep a module-level compiler instance for any callers that want it.
compiler = Compiler()

def find_challenge_dirs(corpus_root: Path) -> List[Path]:
    """
    Return immediate child directories under corpus_root that look like challenges.
    Args -> 
    corpus_root: List of Paths
    Returns -> 
    List of Paths
    """
    if not corpus_root.exists():
        raise FileNotFoundError(f"CGC corpus root not found: {corpus_root}")

    dirs = [p for p in corpus_root.iterdir() if p.is_dir()]
    return sorted(dirs)


def compile_cgc_corpus(
    corpus_root: Optional[str] = None,
    output_root: Optional[str] = None,
    opt_level: OptimizationLevel = OptimizationLevel.O0,
    gcc_path: str = "gcc",
    gpp_path: str = "g++",
) -> Dict[str, Tuple[bool, str, str]]:
    """
    Compile each CGC challenge into a single binary.
    Args ->
    corpus_root: Path to cgc-challenge-corpus root
    output_root: Where to store compiled binaries
    opt_level: Optimization level
    gcc_path: Path to gcc
    gpp_path: Path to g++
    
    Returns ->
    A mapping: challenge_name -> (success, message, output_path)
    """
    logger = setup_logger("cgc_preprocessor", None)

    if corpus_root is None:
        corpus_root_path = DEFAULT_CORPUS_ROOT
    else:
        corpus_root_path = Path(corpus_root)

    if output_root is None:
        out_root = DEFAULT_OUTPUT_DIR
    else:
        out_root = Path(output_root)

    libcgc_path = DEFAULT_LIBCGC_PATH
    logger.info(f"Using libcgc at: {libcgc_path}")

    challenges = find_challenge_dirs(corpus_root_path)
    logger.info(f"Found {len(challenges)} challenges in {corpus_root_path}")

    compiler = Compiler(gcc_path=gcc_path, gpp_path=gpp_path)

    results: Dict[str, Tuple[bool, str, str]] = {}

    for chal_dir in challenges:
        chal_name = chal_dir.name
        src_dir = chal_dir / "src"
        lib_dir = chal_dir / "lib"
        include_dir = chal_dir / "include"

        # Collect source files
        if not src_dir.exists():
            logger.warning(f"Skipping {chal_name}: no src directory at {src_dir}")
            results[chal_name] = (False, f"no src directory", "")
            continue

        src_files = sorted([p for p in src_dir.glob("*.c")])
        if not src_files:
            logger.warning(f"Skipping {chal_name}: no .c files in {src_dir}")
            results[chal_name] = (False, f"no .c sources", "")
            continue

        lib_files = []
        if lib_dir.exists():
            lib_files = sorted([p for p in lib_dir.glob("*.c")])

        include_dirs: List[str] = []
        if libcgc_path:
            include_dirs.append(str(libcgc_path))
        if include_dir.exists():
            include_dirs.append(str(include_dir))
        if lib_dir.exists():
            include_dirs.append(str(lib_dir))

        # Output path per challenge
        out_path = out_root / chal_name

        # Build extra flags for CGC: 32-bit, no stdlib/startfiles, include libcgc asm
        extra_flags: List[str] = [
            "-m32",
            "-nostdlib",
            "-fno-builtin",
            "-nostartfiles",
            "-fcommon",
        ]

        if libcgc_path:
            # libcgc provides assembly files that need to be linked/assembled
            asm_lib = Path(libcgc_path) / "libcgc.s"
            asm_maths = Path(libcgc_path) / "maths.s"
            if asm_lib.exists():
                extra_flags.append(str(asm_lib))
            if asm_maths.exists():
                extra_flags.append(str(asm_maths))

        # Choose a main file; use the first source file as entry point
        main_file = str(src_files[0])
        # Add remaining src files and lib files to extra flags so they are compiled/linked
        for s in src_files[1:]:
            extra_flags.append(str(s))
        for l in lib_files:
            extra_flags.append(str(l))

        logger.info(f"Compiling challenge {chal_name}: {len(src_files)} src files, {len(lib_files)} lib files")

        success, message = compiler.compile_source(
            source_file_path=main_file,
            output_file_path=str(out_path),
            opt=opt_level,
            is_cpp=False,
            include_dirs=include_dirs if include_dirs else None,
            extra_flags=extra_flags,
        )

        results[chal_name] = (success, message, str(out_path) if success else "")

    return results


def main():
    import argparse
    
    parser = argparse.ArgumentParser(description="Compile CGC corpus into single binaries")
    parser.add_argument("--corpus-root", default=DEFAULT_CORPUS_ROOT, help="Path to cgc-challenge-corpus root")
    parser.add_argument("--output-root", default=DEFAULT_OUTPUT_DIR, help="Where to store compiled binaries")
    parser.add_argument("--opt-level", choices=[l.name for l in OptimizationLevel], default=OptimizationLevel.O0.name)
    parser.add_argument("--gcc", default="gcc", help="Path to gcc")
    parser.add_argument("--gpp", default="g++", help="Path to g++")

    args = parser.parse_args()
    opt_enum = OptimizationLevel[args.opt_level]

    results = compile_cgc_corpus(
        corpus_root=args.corpus_root,
        output_root=args.output_root,
        opt_level=opt_enum,
        gcc_path=args.gcc,
        gpp_path=args.gpp,
    )

    succ = sum(1 for r in results.values() if r[0])
    total = len(results)
    print(f"Compiled {succ}/{total} challenges. See logs for details.")


if __name__ == "__main__":
    main()
