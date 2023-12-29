base_path="$(dirname "$0")/../";
app_name="main";
output_path="./dist/main";
source_file_path="./src/main.c";

pushd $base_path > /dev/null;
mkdir -p $(dirname $output_path);
flags="-Wall -Wpedantic -Wextra -Wshadow -Wstrict-prototypes -Wvla -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-sign-conversion";
includes="-I./src/opus -I./include";
gcc-12 $includes $flags $source_file_path -o $output_path -g;
popd > /dev/null;