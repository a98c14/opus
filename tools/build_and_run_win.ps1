Push-Location $PSScriptRoot/../
.\tools\build_win.ps1; 
if ($?) { 
    Push-Location .\dist\
    Write-Host "[BUILD] launching main.exe";
    .\main.exe
    Pop-Location
}