#!/usr/bin/env python3
"""
Code generator for the fault detection system.

Reads:
  - common/messages/*.json       → generated/cpp/<Name>.hpp
                                   generated/python/<name>.py
  - common/fault_ids.json        → generated/cpp/FaultIds.hpp
                                   generated/python/fault_ids.py

Run from repo root:
  python codegen/codegen.py
"""
import json
import sys
from pathlib import Path

from jinja2 import Environment, FileSystemLoader

REPO_ROOT     = Path(__file__).resolve().parent.parent
MESSAGES_DIR  = REPO_ROOT / "common" / "messages"
FAULT_IDS_FILE = REPO_ROOT / "common" / "fault_ids.json"
TEMPLATES_DIR = REPO_ROOT / "codegen" / "templates"
GEN_CPP_DIR   = REPO_ROOT / "generated" / "cpp"
GEN_PY_DIR    = REPO_ROOT / "generated" / "python"


# ---------------------------------------------------------------------------
# Type mapping helpers
# ---------------------------------------------------------------------------

CPP_TYPES = {"string": "std::string", "float": "float", "bool": "bool"}
PY_TYPES  = {"string": "str",         "float": "float", "bool": "bool"}
PY_DEFS   = {"string": '""',          "float": "0.0",   "bool": "False"}


def cpp_type(t: str) -> str:
    if t not in CPP_TYPES:
        raise ValueError(f"Unknown type: {t!r}. Valid: {list(CPP_TYPES)}")
    return CPP_TYPES[t]


def py_type(t: str) -> str:
    if t not in PY_TYPES:
        raise ValueError(f"Unknown type: {t!r}. Valid: {list(PY_TYPES)}")
    return PY_TYPES[t]


def py_default(t: str) -> str:
    if t not in PY_DEFS:
        raise ValueError(f"Unknown type: {t!r}. Valid: {list(PY_DEFS)}")
    return PY_DEFS[t]


def pascal_to_snake(name: str) -> str:
    """TemperatureControlUnitCommand → temperature_control_unit_command"""
    import re
    s1 = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", s1).lower()


# ---------------------------------------------------------------------------
# Jinja2 environment
# ---------------------------------------------------------------------------

def make_env() -> Environment:
    import json as _json
    env = Environment(
        loader=FileSystemLoader(str(TEMPLATES_DIR)),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
    )
    env.globals["cpp_type"]    = cpp_type
    env.globals["py_type"]     = py_type
    env.globals["py_default"]  = py_default

    def tojson_filter(value):
        return _json.dumps(value)
    env.filters["tojson"] = tojson_filter
    return env


# ---------------------------------------------------------------------------
# Pass 1 — message types
# ---------------------------------------------------------------------------

def generate_messages(env: Environment) -> None:
    hpp_tmpl = env.get_template("message.hpp.jinja2")
    py_tmpl  = env.get_template("message.py.jinja2")

    for msg_file in sorted(MESSAGES_DIR.glob("*.json")):
        with open(msg_file) as f:
            spec = json.load(f)

        name   = spec["message_name"]
        fields = spec["fields"]

        # Validate types
        for field in fields:
            cpp_type(field["type"])

        # C++ header
        cpp_out = GEN_CPP_DIR / f"{name}.hpp"
        cpp_out.write_text(hpp_tmpl.render(message_name=name, fields=fields))
        print(f"  [cpp] {cpp_out.relative_to(REPO_ROOT)}")

        # Python module
        py_out = GEN_PY_DIR / f"{pascal_to_snake(name)}.py"
        py_out.write_text(py_tmpl.render(message_name=name, fields=fields))
        print(f"  [py]  {py_out.relative_to(REPO_ROOT)}")


# ---------------------------------------------------------------------------
# Pass 2 — fault ID registry
# ---------------------------------------------------------------------------

def generate_fault_ids(env: Environment) -> None:
    with open(FAULT_IDS_FILE) as f:
        spec = json.load(f)

    fault_ids = spec["fault_ids"]

    hpp_tmpl = env.get_template("fault_ids.hpp.jinja2")
    py_tmpl  = env.get_template("fault_ids.py.jinja2")

    cpp_out = GEN_CPP_DIR / "FaultIds.hpp"
    cpp_out.write_text(hpp_tmpl.render(fault_ids=fault_ids))
    print(f"  [cpp] {cpp_out.relative_to(REPO_ROOT)}")

    py_out = GEN_PY_DIR / "fault_ids.py"
    py_out.write_text(py_tmpl.render(fault_ids=fault_ids))
    print(f"  [py]  {py_out.relative_to(REPO_ROOT)}")


# ---------------------------------------------------------------------------
# Python package init
# ---------------------------------------------------------------------------

def write_py_init() -> None:
    init = GEN_PY_DIR / "__init__.py"
    if not init.exists():
        init.write_text("# auto-generated package\n")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    GEN_CPP_DIR.mkdir(parents=True, exist_ok=True)
    GEN_PY_DIR.mkdir(parents=True, exist_ok=True)

    env = make_env()

    print("Codegen: generating message types...")
    generate_messages(env)

    print("Codegen: generating fault ID registry...")
    generate_fault_ids(env)

    write_py_init()
    print("Codegen: done.")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
