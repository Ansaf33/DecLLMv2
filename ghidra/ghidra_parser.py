#!/usr/bin/env python3
#!/usr/bin/env python3
"""
Parse a decompiled C source file and emit enriched JSON per the requested schema.
"""

from __future__ import annotations

import argparse
import json
import re
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import List, Dict, Tuple

CONTROL_KEYWORDS = {"if", "for", "while", "switch", "else", "do", "case", "return"}

FUNC_HEADER_RE = re.compile(
    r"(?m)^[ \t]*(?P<header>(?:[A-Za-z_][\w\*\s\[\]]*?\s)+(?P<name>[A-Za-z_]\w*)\s*\([^;{)]*\))\s*\{"
)

INCLUDE_RE = re.compile(r'(?m)^\s*#\s*include\s+["<]([^">]+)[">]')
TYPEDEF_RE = re.compile(r'(?m)^\s*typedef[\s\S]*?;')
STRUCT_RE = re.compile(r'(?ms)struct\s+[A-Za-z_][\w]*\s*\{.*?\};')
GLOBAL_VAR_RE = re.compile(r'(?m)^\s*(?:extern\s+)?[A-Za-z_][\w\s\*\[\]]+\s+([A-Za-z_]\w+)\s*(?:=\s*[^;]+)?;')
PROTOTYPE_RE = re.compile(r'(?m)^\s*(?:extern\s+)?[^;\n]+?\([^;\n]*\)\s*;')
CALL_RE = re.compile(r'\b([A-Za-z_]\w*)\s*\(')


def extract_braced_block(text: str, start_idx: int) -> Tuple[str, int]:
    i = start_idx
    n = len(text)
    depth = 0
    buf = []
    while i < n:
        ch = text[i]
        buf.append(ch)
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                return "".join(buf), i + 1
        i += 1
    return "".join(buf), n


def find_functions(text: str) -> List[Tuple[str, str, int, int]]:
    results = []
    for m in FUNC_HEADER_RE.finditer(text):
        name = m.group('name')
        if name in CONTROL_KEYWORDS:
            continue
        header = m.group('header').strip()
        brace_idx = m.end() - 1
        body, endpos = extract_braced_block(text, brace_idx)
        decompiled = header + " " + body
        results.append((name, decompiled, m.start(), endpos))
    return results


def extract_header(text: str, first_func_start: int) -> str:
    return text[:first_func_start].strip()


def extract_includes(header_text: str) -> List[str]:
    return list(dict.fromkeys(m.group(1) for m in INCLUDE_RE.finditer(header_text)))


def extract_types(header_text: str) -> List[str]:
    items = []
    items.extend(t.strip() for t in TYPEDEF_RE.findall(header_text))
    items.extend(m.group(0).strip() for m in STRUCT_RE.finditer(header_text))
    return list(dict.fromkeys(items))


def extract_globals(header_text: str) -> List[str]:
    names = []
    for m in GLOBAL_VAR_RE.finditer(header_text):
        line = m.group(0)
        if '(' in line:
            continue
        names.append(m.group(1))
    return list(dict.fromkeys(names))


def extract_prototypes(header_text: str) -> List[str]:
    return list(dict.fromkeys(p.strip() for p in PROTOTYPE_RE.findall(header_text)))


def extract_called_functions(code: str) -> List[str]:
    names = []
    for m in CALL_RE.finditer(code):
        nm = m.group(1)
        if nm in CONTROL_KEYWORDS:
            continue
        names.append(nm)
    return list(dict.fromkeys(names))


def guess_defined_filename(cpath: Path) -> str:
    return cpath.name


def guess_binary_name(cpath: Path) -> str:
    for p in cpath.resolve().parents:
        if re.match(r'^[A-Z]+_\d{5}$', p.name):
            return p.name
    return ""


def build_metadata(cpath: Path) -> Dict:
    return {
        "binary_name": cpath.stem,
        "opt_level": "O0",
        "compiler": "gcc",
        "arch": "x86_64",
        "source_path": str(cpath),
        "decompiler": "Ghidra",
        "timestamp": datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    }


def parse_and_enrich(cpath: Path) -> Dict:
    text = cpath.read_text(encoding='utf-8', errors='ignore')
    funcs = find_functions(text)

    if funcs:
        first_start = funcs[0][2]
    else:
        first_start = len(text)

    header_text = extract_header(text, first_start)
    includes = extract_includes(header_text)
    types = extract_types(header_text)
    globals_all = extract_globals(header_text)
    prototypes = extract_prototypes(header_text)

    # prepare call graph
    called_map = {}
    for name, code, s, e in funcs:
        called_map[name] = extract_called_functions(code)

    callers_map = defaultdict(list)
    for caller, callees in called_map.items():
        for callee in callees:
            if callee in called_map:
                callers_map[callee].append(caller)

    defined_in_file = guess_defined_filename(cpath)
    metadata_common = build_metadata(cpath)

    functions_out = []
    for name, code, s, e in funcs:
        signature = code.split('{', 1)[0].strip() if '{' in code else code.strip()
        called = called_map.get(name, [])
        callers = callers_map.get(name, [])
        externs = [c for c in called if c not in called_map]
        globals_used = [g for g in globals_all if re.search(r'\b' + re.escape(g) + r'\b', code)]

        functions_out.append({
            "func_name": name,
            "func_c_signature": signature,
            "decompiled_code": code.strip(),
            "globals": globals_used,
            "externs": externs,
            "types": types,
            "includes": includes,
            "dependencies": {
                "called_functions": called,
                "calling_functions": callers,
                "defined_in_file": defined_in_file,
            }
        })

    return {"header": header_text, "functions": functions_out, "metadata": metadata_common}
