"""Parser for test/build failure logs (LLM-friendly formatter).

This module provides two simple functions:
- parse_test_failure(log: str) -> dict
- format_for_llm(parsed: dict) -> str

The parser is deliberately conservative and focuses on high-signal items:
- test name (if the log begins with a "Test ... failed" line)
- linker undefined references (handles ld's two-line object+undefined form)
- make failure lines

The formatted output is a compact text block suitable to include in an LLM
prompt or issue report.
"""
from __future__ import annotations

import re
from typing import List, Dict, Any


def _lines(log: str) -> List[str]:
    return [l.rstrip() for l in log.splitlines()]


def parse_test_failure(log: str) -> Dict[str, Any]:
    """Parse a test/build failure log into structured findings.

    Returns a dict with keys: test_name, make_errors, linker_undefined (list), raw_lines
    Each linker entry: {symbol, object, source, context}
    """
    lines = _lines(log)
    findings: Dict[str, Any] = {}

    # Test name: "Test <name> failed" or "Test script <name> failed"
    test_name = None
    if lines:
        m = re.match(r"Test(?: script)?\s+([^\s:]+)\s+failed", lines[0], re.IGNORECASE)
        if m:
            test_name = m.group(1)
    findings["test_name"] = test_name
    findings["raw_lines"] = lines

    # Make failure lines
    findings["make_errors"] = [l for l in lines if re.search(r"make\[?\d*\]?: \*\*\*|make: \*\*\*", l)]

    # Linker undefined references detection
    undefined_re = re.compile(r"undefined reference to\s*(?:[`\"']?)(?P<symbol>[^`'\"\s]+)(?:[`\"']?)", re.IGNORECASE)
    obj_re = re.compile(r"(?:/usr/bin/ld: )?(?P<obj>[\w\-./]+\.o)\b")
    src_re = re.compile(r"(?P<src>[\w\-./]+\.(?:c|cpp|cc|cxx)):")

    linker: List[Dict[str, Any]] = []
    for i, line in enumerate(lines):
        # direct undefined on the same line
        m = undefined_re.search(line)
        if m:
            symbol = m.group("symbol")
            ctx_start = max(0, i - 4)
            ctx = lines[ctx_start: i + 3]
            obj = None
            src = None
            for c in reversed(lines[ctx_start:i+1]):
                mo = obj_re.search(c)
                if mo:
                    obj = mo.group("obj")
                    break
            for c in lines[ctx_start:i+3]:
                ms = src_re.search(c)
                if ms:
                    src = ms.group("src")
                    break
            linker.append({"symbol": symbol, "object": obj, "source": src, "context": [l for l in ctx if l.strip()]})
            continue

        # object line then undefined on next line
        if i + 1 < len(lines):
            next_line = lines[i+1]
            mo = obj_re.search(line)
            m2 = undefined_re.search(next_line)
            if mo and m2:
                obj = mo.group("obj")
                symbol = m2.group("symbol")
                src = None
                for c in lines[i: i + 4]:
                    ms = src_re.search(c)
                    if ms:
                        src = ms.group("src")
                        break
                ctx = lines[max(0, i - 2): i + 4]
                linker.append({"symbol": symbol, "object": obj, "source": src, "context": [l for l in ctx if l.strip()]})

    findings["linker_undefined"] = linker
    findings["summary"] = {"make_errors": len(findings["make_errors"]), "linker_undefined": len(linker)}
    return findings


def format_for_llm(parsed: Dict[str, Any]) -> str:
    """Format the parsed findings into compact text for LLM prompts."""
    lines: List[str] = []
    tn = parsed.get("test_name")
    if tn:
        lines.append(f"Test: {tn}")
    lines.append("Summary:")
    lines.append(f" - make failures: {parsed.get('summary', {}).get('make_errors',0)}")
    lines.append(f" - undefined linker symbols: {parsed.get('summary', {}).get('linker_undefined',0)}")
    lines.append("")

    if parsed.get("linker_undefined"):
        lines.append("Undefined linker symbols (examples):")
        for e in parsed["linker_undefined"]:
            lines.append(f" - {e['symbol']} (object={e.get('object')}, source={e.get('source')})")
            if e.get("context"):
                ctx = " | ".join(e["context"])[:400]
                lines.append(f"   context: {ctx}")
        lines.append("")

    if parsed.get("make_errors"):
        lines.append("Make failure lines:")
        for m in parsed["make_errors"]:
            lines.append(f" - {m}")
        lines.append("")

    lines.append("Probable causes and suggestions:")
    lines.append(" 1) Missing library or object file in the link line. Search the tree for the symbol (grep -R 'symbol' .) to find which file/library defines it.")
    lines.append(" 2) The symbol may belong to an optional feature (e.g., SMACK). Adjust configure flags or install the missing dependency.")
    lines.append(" 3) Update Makefile/linker flags to include the library or add the defining .o to the link rule.")
    lines.append("")
    lines.append("Action checklist:")
    lines.append(" - [ ] locate symbol definitions in source tree")
    lines.append(" - [ ] identify library (-l) or .o missing from link line")
    lines.append(" - [ ] re-run make in the failing directory and confirm the fix")

    return "\n".join(lines)


if __name__ == "__main__":
    # Quick demo using the example provided in the prompt
    demo = """
Test gnu-zero-uids.sh failed.
/usr/bin/ld: src/id.o: in function `usage':
id.c:(.text+0xf0): undefined reference to `emit_ancillary_info'
/usr/bin/ld: src/id.o: in function `main':
id.c:(.text+0x180): undefined reference to `is_smack_enabled'
/usr/bin/ld: id.c:(.text+0x515): undefined reference to `smack_new_label_from_self'
collect2: error: ld returned 1 exit status
make[2]: *** [Makefile:12050: src/id] Error 1
make[1]: *** [Makefile:24195: all-recursive] Error 1
make: *** [Makefile:9488: all] Error 2
"""
    parsed = parse_test_failure(demo)
    print(format_for_llm(parsed))
