gcc -o TermCalc main.c
if ($LASTEXITCODE -ne 0) { exit }
Invoke-Expression "./TermCalc.exe $args"