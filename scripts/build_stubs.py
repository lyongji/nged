import subprocess
import sys
import shutil
import os
import re

def install_stubgen():
    try:
        import pybind11_stubgen
    except ImportError:
        print("Installing pybind11-stubgen...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", "pybind11-stubgen"])

def build_stubs():
    print("Generating stubs for nged...")
    # Generate stubs into a temporary directory
    output_dir = "stubs"
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    
    # Run pybind11-stubgen
    cmd = [
        "pybind11-stubgen", 
        "nged", 
        "--output-dir", output_dir,
        "--ignore-all-errors", # suppress errors about unrepresentable defaults
        "--root-suffix", "" # cleaner output for packages
    ]
    
    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError as e:
        print(f"Error running pybind11-stubgen: {e}")
        return

    # Verify output
    stub_pkg = os.path.join(output_dir, "nged")
    if not os.path.exists(stub_pkg):
        print("Stub generation failed or produced unexpected output structure.")
        return

    print(f"Stubs generated in {output_dir}/nged")
    
    # Merge ngpy stubs into a single file
    ngpy_stub_dir = os.path.join(stub_pkg, "ngpy")
    if os.path.exists(ngpy_stub_dir) and os.path.isdir(ngpy_stub_dir):
        merge_stubs(ngpy_stub_dir, os.path.join(stub_pkg, "ngpy.pyi"))
        # Clean up directory after merging
        # shutil.rmtree(ngpy_stub_dir) 

def merge_stubs(stub_dir, output_file):
    print(f"Merging stubs from {stub_dir} into {output_file}...")
    
    init_file = os.path.join(stub_dir, "__init__.pyi")
    if not os.path.exists(init_file):
        print(f"Error: {init_file} not found")
        return

    with open(init_file, 'r') as f:
        lines = f.readlines()

    # Filter imports of submodules that we are going to merge
    submodules = [f[:-4] for f in os.listdir(stub_dir) if f.endswith(".pyi") and f != "__init__.pyi"]
    
    merged_lines = []
    # headers
    merged_lines.append("from __future__ import annotations\n")
    merged_lines.append("import typing\n")
    merged_lines.append("import nged.ngpy\n") # self-reference often used in stubs
    
    # Process __init__.pyi content
    skip_imports = [f"from . import {m}" for m in submodules]
    
    for line in lines:
        if line.startswith("from __future__") or line.startswith("import typing"):
            continue # handled
        if any(line.strip().startswith(s) for s in skip_imports):
            continue
        merged_lines.append(line)

    merged_lines.append("\n")

    # Process submodules
    for submodule in submodules:
        sub_file = os.path.join(stub_dir, f"{submodule}.pyi")
        print(f"  Merging submodule {submodule}...")
        
        with open(sub_file, 'r') as f:
            sub_lines = f.readlines()
            
        merged_lines.append(f"class {submodule}:\n")
        
        # Indent content
        for line in sub_lines:
            if line.startswith("from __future__") or line.startswith("import typing") or line.startswith("import nged.ngpy"):
                continue
            if line.strip() == "":
                continue
                
            # Naive staticmethod conversion for top-level functions in submodule
            # This logic is fragile but works for simple pybind11 output
            # We assume top-level defs are methods of the module
            if line.startswith("def "):
                merged_lines.append("    @staticmethod\n")
            
            merged_lines.append("    " + line)
        
        merged_lines.append("\n")

    with open(output_file, 'w') as f:
        f.writelines(merged_lines)
    print(f"Merged stub created at {output_file}")
    
    # Copy to source tree
    source_dest = os.path.join("nged", "ngpy.pyi")
    shutil.copy(output_file, source_dest)
    print(f"Copied stubs to {source_dest}")

if __name__ == "__main__":
    install_stubgen()
    build_stubs()
