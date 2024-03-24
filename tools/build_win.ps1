# --- Parse Commands --------------------------------------
$total_stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
$commands = @{};
foreach ($arg in $args) {
    $parts = $arg -split "="
    $commands[$parts[0]] = if ($parts.Length -gt 1) { $parts[1] } else { 1 }
}

$root_directory = "$PSScriptRoot\.."
if ($null -eq $commands["output"]) {
    $commands["output"] = "$root_directory\dist\main.exe"
}
else {
    $commands["output"] = "$root_directory\$($commands["output"])"
}
$output_directory = ([io.fileinfo]$($commands["output"])).DirectoryName
$output_basename = ([io.fileinfo]$($commands["output"])).BaseName



# --- Set Compile Flags -----------------------------------
if (-not $commands["release"]) { $commands["debug"] = 1; }

if ($commands["run"] -and $null -eq $commands["version"]) {
    $commands["version"] = "v0.0.0"
}
elseif ($null -eq $commands["version"]) {
    $commands["version"] = "$(git describe --tags --abbrev=0)-$(git describe --always)"
}

$compile_args = @();
if ($commands["release"]) {
    Write-Host "[BUILD] release mode" 
    $compile_args += "/O2";
    $compile_args += "/DBUILD_DEBUG=0"
}

if ($commands["debug"]) {
    Write-Host "[BUILD] debug mode"; 
    $compile_args += "/Z7";
    $compile_args += "/Od";
    $compile_args += "/DBUILD_DEBUG=1"
    $commands["version"] = "$($commands["version"])-debug"
}

if ($commands["profile"]) {
    $compile_args += "/DPROFILE_SUPERLUMINAL=1"
}

# --- Common Compile Flags ---------------------------------
$compile_args += "/Fo$output_directory\$output_basename.obj"
$compile_args += "/nologo"
$compile_args += "/I.\include"
$compile_args += "/I.\src"
$compile_args += "/DSTB_IMAGE_IMPLEMENTATION"
$compile_args += "/DVERSION_NUMBER=\""$($commands["version"])\"""

# --- Libs -------------------------------------------------
$compile_args += ".\lib\soloud_x64_d.lib";
$compile_args += ".\lib\freetype.lib";
$compile_args += ".\lib\glfw3dll.lib";
$compile_args += ".\lib\glfw3.lib";
$compile_args += ".\lib\gl.obj";
$compile_args += "opengl32.lib";

# --- Set Link Flags --------------------------------------
$link_args = @();
$link_args += "/out:$output_directory\$output_basename.exe"
if ($commands["debug"]) {
    $link_args += "/pdb:$output_directory\$output_basename.pdb"
}

# --- Set Build Target ------------------------------------
$source_file_path = ""
if ($commands["trail_rendering"]) { $source_file_path = "$root_directory\examples\trail_rendering\main.c" }
elseif ($commands["shape_rendering"]) { $source_file_path = "$root_directory\examples\shape_rendering\main.c" }

New-Item -ItemType Directory -Force -Path $output_directory | Out-Null;
Push-Location $root_directory
$compile_stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
cl $compile_args $source_file_path /link $link_args;
Write-Host "[BUILD] target: $source_file_path";
Write-Host "[BUILD] time compilation: $(($compile_stopwatch.ElapsedMilliseconds / 1000.0).ToString("0.00"))s, total: $(($total_stopwatch.ElapsedMilliseconds / 1000.0).ToString("0.00"))s"
Pop-Location
Push-Location $output_directory | Out-Null
if ($commands["run"]) { Invoke-Expression "$output_directory\$output_basename.exe" }
Pop-Location;

