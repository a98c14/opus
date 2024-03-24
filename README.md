## Naming Conventions
- `snake_case` for variables and functions
- `PascalCase` for types (except for primitive types like uint64, float32 etc.)
- All functions and types are prefixed with abbreviated name of their layer except for the base layer.
- Global variables should generally used my the layer that defined it and not from outside. They are also prefixed with the same abbreviation.




## Commands
### Build & Run
```ps1
.\tools\build_win.ps1; if($?) { .\dist\main.exe }
```

### Building GLAD
```ps1
# run at workspace root
mkdir ./temp/;
pushd ./temp/;
cl /LD ../glad/src/glad.c -I../glad/include;
popd;
```
Move the `.obj` file to `lib` folder and move the `.dll` to `dist` folder

### Creating font texture
```ps1
msdf-atlas-gen.exe -font  <PATH_TO_TTF_FILE> -fontname <NAME_OF_THE_FONT> -json  <OUTPUT_DATA_FILE_PATH> -size 64 -type sdf -format 'png' -imageout <OUTPUT_TEXTURE_FILE_PATH> -pxrange 16  
```
Example
```
msdf-atlas-gen.exe -font .\open_sans.ttf -fontname open_sans -json open_sans -size 64 -type sdf -format 'png' -imageout .\open_sans.png -pxrange 16
```

## Notes
Defining `STB_IMAGE_IMPLEMENTATION` causes clangd to crash for some reason so it is defined during compilation (with `/D` flag) and not in the code. 
