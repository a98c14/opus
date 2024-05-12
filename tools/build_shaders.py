"""
Examples:
python .\\build_shaders.py ---src "$HOME\source\github\opus\src\shaders" --out=".\\test_out.h"
"""
import os
import argparse
from pprint import pprint

shader_line_end = "\\n\"\n"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Opus Font Header Generator")
    parser.add_argument("--src", type=str, help="Shader folder path", required=True)
    parser.add_argument("--out", type=str, help="Output path for the header file", required=True)
    args = parser.parse_args()

    if os.path.exists(args.out):
        os.remove(args.out)
        
    shader_files = os.listdir(args.src)
    
    shader_map = {}
    for file in shader_files:
        name, ext = file.split('.')
        name_files = shader_map.get(name, [])
        name_files.append(file)
        shader_map[name] = name_files
    
    with open(args.out, 'w+') as out:
        # Write Header
        
        out.write("// clang-format off\n")
        out.write("#pragma once\n")
        out.write("#include \"base.h\"\n")
        out.write("\n")
                
        for shader_name in shader_map:
            print(f"building shader: {shader_name}")
            out.write(f"/** {shader_name} shader */\n")
            
            for shader_file in shader_map[shader_name]:
                path = os.path.join(args.src, shader_file)
                _, ext = shader_file.split('.')
                
                c_variable_name = f"d_shader_opengl_{shader_name}_{ext}"
                out.write(f"read_only global String {c_variable_name} = string_comp(\n")
                with open(path) as f:
                    shader_contents = f.readlines()
                    for line in shader_contents:
                        line.replace("\n", "")
                        out.write(f"\t\"{line.strip()}{shader_line_end}")
                out.write(f");\n\n")
                
                
        
        
        
    
        
    