"""
Examples:
python .\opus\tools\generate_component_template.py --data "$HOME\source\github\enginefire\src\components.h" --out-template "$HOME\source\github\enginefire\docs\component_template.txt"
"""
import yaml
import os
import argparse
import re

def log_info(message):
    print(f"[info] {message}")

def camel_to_snake(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Opus Component Header Generator")
    parser.add_argument("--data", type=str, help="Component header file", required=True)
    parser.add_argument("--out-template", type=str, help="Template file path", required=True)
    args = parser.parse_args()

    lines = []
    with  open(args.data, 'r') as component_file:
        lines = component_file.readlines()

    components = []
    tag_components = []
    current_struct = None
    for line in lines:
        # check for struct start
        if current_struct is None:
            match = re.match(r"typedef struct\s*", line)
            if match is not None:
                current_struct = {}
                current_struct['name'] = 'UNKNOWN'
                current_struct['fields'] = []
                continue
            
            match = re.match(r"typedef TagComponent\s*(?P<struct_name>\w+);\s*$", line)
            if match is not None:
                tag_components.append({'name': match.groups()[0], 'fields': []})
                continue
            
        struct_end_match = re.match(r"}\s+(?P<struct_name>\w+);\s*$", line)
        if struct_end_match is not None:
            current_struct['name'] = struct_end_match.groups()[0]
            components.append(current_struct)
            current_struct = None
            continue

        struct_field_match = re.match(r"\s*(?P<type>\w+)\s*(?P<name>\w+);\s*$", line)
        if(struct_field_match is not None):
            current_struct['fields'].append({'type': struct_field_match.groups()[0], 'name': struct_field_match.groups()[1]})

    if os.path.exists(args.out_template):
        os.remove(args.out_template)

    with open(args.out_template, 'w') as template_file:
        template_file.write("# tag_component\n")
        for component in tag_components:
            component_name = camel_to_snake(component['name'])
            component_name = "_".join(component_name.split('_')[:-1])
            template_file.write(f"[{component_name}] : {component['name']}")
            for field in component['fields']:
                template_file.write(f"\n\t({field['name']} : {field['type']})")
            template_file.write("\n")
            
        if len(tag_components) > 0:
            template_file.write("\n")
            
        template_file.write("# component\n")
        for component in components:
            component_name = camel_to_snake(component['name'])
            component_name = "_".join(component_name.split('_')[:-1])
            template_file.write(f"[{component_name}] : {component['name']}")
            for field in component['fields']:
                template_file.write(f"\n\t({field['name']} : {field['type']})")
            template_file.write("\n")


    print(f"[info] generated template successfully at {args.out_template}")

