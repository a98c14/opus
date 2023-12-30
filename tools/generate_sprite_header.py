"""
Examples:
python .\generate_sprite_header.py --data "$HOME\\source\\github\\greenrock\\assets\\_dist\\environment.json" "$HOME\\source\\github\\greenrock\\assets\\_dist\\creature.json" --out=".\\test_out.c"
"""
import json
import os
import argparse

def log_info(message):
    print(f"[info] {message}")

def parse_sprite_name(filename):
    # 'basic/base_robot_4.png' -> 'BASE_ROBOT_4'
    return filename.split('/')[-1].split('.')[0].upper()

def parse_animation_name(filename):
    # 'basic/base_robot_4.png' -> 'BASE_ROBOT'
    return "_".join(parse_sprite_name(filename).split('_')[:-1]).upper()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Opus Sprite Header Generator")
    parser.add_argument("--data", nargs='+', type=str, help="List of texture packer output data files", required=True)
    parser.add_argument("--out", type=str, help="Output path", required=True)
    parser.add_argument("--excluded-animations", nargs='+', type=str, help="List of animation prefixes to be excluded from generation", required=False)
    args = parser.parse_args()

    exclude = [] if args.excluded_animations is None else args.excluded_animations
    texture_exclude = list(filter(lambda x: x.startswith('TEXTURE'), exclude))
    animation_exclude = list(filter(lambda x: x.startswith('ANIMATION'), exclude))
    
    raw_texture_data_dict = {}
    for path in args.data:
        with open(path, 'r') as data_file:
            data = json.load(data_file)
            texture_name, _ = data['meta']['image'].split('.')
            texture_name = texture_name.upper()
            w, h = data['meta']['size']['w'], data['meta']['size']['h']
            raw_texture_data_dict[texture_name] = {}
            raw_texture_data_dict[texture_name]['metadata'] = {'w': w, 'h': h, 'name': texture_name}
            raw_texture_data_dict[texture_name]['frames'] = data['frames']
            
    texture_data = {}
    for texture in raw_texture_data_dict:
        texture_fullname = f"TEXTURE_{texture}"
        frames = raw_texture_data_dict[texture]['frames']
        
        texture_data[texture] = {}
        # parse animation
        animations = {}
        if not any(texture_fullname.startswith(s) for s in texture_exclude):
            for frame in frames:
                animation_name = parse_animation_name(frame['filename'])
                animation_fullname = f"ANIMATION_{texture}_{animation_name}"
                if any(animation_fullname.startswith(s) for s in animation_exclude): 
                    continue
                animations[animation_name] = animations.get(animation_name, 0) + 1
                
        texture_data[texture]['animations'] = animations

        # parse sprites
        sprites = {}
        for frame in frames:
            sprite_name = parse_sprite_name(frame['filename'])
            sprites[sprite_name] = {
                "frame": frame['frame'],
                "source_size": frame['sourceSize'],
                "sprite_source_size": frame['spriteSourceSize'],
                "pivot": frame['pivot']
            }
        texture_data[texture]['sprites'] = sprites

        log_info(f"texture: {texture}, w: {w}, h: {h}")
        
    animation_count = sum(len(v['animations']) for v in texture_data.values())
    sprite_count = sum(len(v['sprites']) for v in texture_data.values())
    log_info(f"found {animation_count} animation{'s' if animation_count > 1 else ''}, {sprite_count} sprite{'s' if sprite_count > 1 else ''}")
            
        
    if os.path.exists(args.out):
        os.remove(args.out)

    with open(args.out, 'w') as output_file:
        # write header
        
        output_file.write("// clang-format off\n")
        output_file.writelines(["/**\n", " * IMPORTANT:\n", " * THIS FILE IS AUTO GENERATED\n", " * DO NOT EDIT THIS FILE BY HAND\n", "*/\n"])
        output_file.write("#pragma once\n\n")
        output_file.write("#include <core/defines.h>\n")
        output_file.write("#include <core/math.h>\n")
        output_file.write("#include <gfx/sprite.h>\n")
        output_file.write("#include <engine/text.h>\n")
        
        # write textures
        output_file.write("\ntypedef enum\n{\n")
        output_file.write("\tTEXTURE_UNDEFINED = 0,\n")
        for texture in texture_data:
            output_file.write(f"\tTEXTURE_{texture},\n")
        output_file.write("\tTEXTURE_COUNT\n")
        output_file.write("} TEXTURE_INDEX;\n")
    
        # write animations enum
        output_file.write("\ntypedef enum\n{\n")
        output_file.write("\tANIMATION_UNDEFINED = 0,\n")
        for texture in texture_data:
            animation_prefix = f"ANIMATION_{texture}"
            animations = texture_data[texture]['animations']
            for animation in animations:
                output_file.write(f"\t{animation_prefix}_{animation},\n")
        output_file.write("\tANIMATION_COUNT\n")
        output_file.write("} ANIMATION_INDEX;\n")
    
        # write sprites enum
        output_file.write("\ntypedef enum\n{\n")
        output_file.write("\tSPRITE_UNDEFINED = 0,\n")
        for texture in texture_data:
            sprite_prefix = f"SPRITE_{texture}"
            sprites = texture_data[texture]['sprites']
            for sprite in sprites:
                output_file.write(f"\t{sprite_prefix}_{sprite},\n")
        output_file.write("\tSPRITE_COUNT\n")
        output_file.write("} SPRITE_INDEX;\n")
        
        # write texture indices        
        output_file.write(f"\ninternal const TEXTURE_INDEX texture_index_map[{sprite_count+1}] = {{\n")
        output_file.write("\tTEXTURE_UNDEFINED,\n")
        for texture in texture_data:
            sprites = texture_data[texture]['sprites']
            for _ in sprites:
                output_file.write(f"\tTEXTURE_{texture},\n")
        output_file.write("};\n")
        
        # write animations array
        output_file.write(f"\ninternal const Animation Animations[{animation_count+1}] = {{\n")
        output_file.write("\t{0},\n")
        for texture in texture_data:
            animations = texture_data[texture]['animations']
            sprite_prefix = f"SPRITE_{texture}"
            for animation in animations:
                frame_count = animations[animation]
                output_file.write(f"\t{{ {sprite_prefix}_{animation}_0, {sprite_prefix}_{animation}_{frame_count - 1} + 1 }},\n")
        output_file.write("};\n")
        
        # write sprites array
        output_file.write(f"\ninternal const Sprite Sprites[{sprite_count+1}] = {{\n")
        output_file.write("\t{0},\n")
        for texture in texture_data:
            sprites = texture_data[texture]['sprites']
            for sprite in sprites:
                frame = sprites[sprite]['frame']
                source_size = sprites[sprite]['source_size']
                sprite_source_size = sprites[sprite]['sprite_source_size']
                pivot = sprites[sprite]['pivot']
                output_file.write("\t{ ")
                output_file.write(f".rect = {{ .x = {frame['x']:4}, .y = {frame['y']:4}, .w = {frame['w']:4}, .h = {frame['h']:4} }}, ")
                output_file.write(f".size = {{ .x = {sprite_source_size['x']:4}, .y = {sprite_source_size['y']:4}, .w = {sprite_source_size['w']:4}, .h = {sprite_source_size['h']:4} }}, ")
                output_file.write(f".pivot = {{ .x = {pivot['x']:6.3f}, .y = {pivot['y']:6.3f} }},")
                output_file.write(f".source_size = {{ .x = {source_size['w']:4}, .y = {source_size['h']:4} }}")
                output_file.write("},\n")
        output_file.write("};\n")

    print(f"[info] generated header successfully at '{os.path.abspath(args.out)}'")

