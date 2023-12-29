Push-Location $PSScriptRoot\..\
Get-ChildItem -Path .\dist\* -Include opus_log_*.txt | Remove-Item
Pop-Location