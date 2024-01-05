"""
Examples:
python .\opus\tools\generate_prefabs.py --data "$HOME\source\github\enginefire\src\components.h"
"""
import sys
import os
import argparse
import re

def log_info(message):
    print(f"[info] {message}")

def parse_sprite_name(filename):
    # 'basic/base_robot_4.png' -> 'BASE_ROBOT_4'
    return filename.split('/')[-1].split('.')[0].upper()

def parse_animation_name(filename):
    # 'basic/base_robot_4.png' -> 'BASE_ROBOT'
    return "_".join(parse_sprite_name(filename).split('_')[:-1]).upper()

def camel_to_snake(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Opus Component Header Generator")
    parser.add_argument("--data", type=str, help="Component header file", required=True)
    args = parser.parse_args()
    
    with  open(args.data, 'r') as component_file:
        lines = component_file.readlines()
        
        
        for line in lines:
            match = re.match(r';', line)
            if(match is not None):
                print(line)
    
            
    print(f"[info] generated header successfully")

