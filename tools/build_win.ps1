$app_name = "main";
$output_path = ".\dist";
$source_file_path = ".\examples\sprite\main.c";

$compile_args = @(
    "/O2",
    "/Zi",
    "/nologo",
    "/Fd:""$output_path\$app_name.pdb""",
    "/Fo:""$output_path""\",
    "/I"".\include""",
    "/I"".\src""",
    "/DSTB_IMAGE_IMPLEMENTATION",
    ".\lib\glfw3dll.lib",
    ".\lib\glfw3.lib",
    ".\lib\gl.obj",
    "opengl32.lib");

$link_args = @(
    "/debug:full",
    "/out:""$output_path\$app_name.exe""",
    "/pdb:""$output_path\$app_name.pdb""");

Push-Location $PSScriptRoot/../
New-Item -ItemType Directory -Force -Path $output_path;
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
cl $compile_args $source_file_path /link $link_args;
Write-Host "[BUILD] time: $(($stopwatch.ElapsedMilliseconds / 1000).ToString())s"
Pop-Location