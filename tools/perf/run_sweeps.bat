@echo off
setlocal enabledelayedexpansion
set ROOT=%~dp0\..\..
set APP=%ROOT%\build\win-msvc\engine_app.exe
for %%N in (1000 10000 50000 100000) do (
  set OUT=%ROOT%\artifacts\perf\%DATE:~10,4%%DATE:~4,2%%DATE:~7,2%
  if not exist "!OUT!" mkdir "!OUT!"
  "%APP%" --spawn %%N --frames 200 --fixed-dt 0.016 --profile on > "!OUT!\sweep_%%N.csv"
)
endlocal


