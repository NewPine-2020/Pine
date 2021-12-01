

rem Get the version name from directory name
set current_path="%cd%"
cd ..\
for /f "delims=" %%i in ("%cd%") do set version="%%~ni"
cd %current_path%

@echo ################################################################
@echo Version		:%version%
@echo ################################################################

set opposite_src_path="..\"
set adjust_path="ModuleDemo\YC1171\bt_demo\CM0\Prj\Output"
set firmware="flash.bin"
set production="flash.dat"

set release_file="%version%_Update.bin"
set production_file="%version%_Production.lod"

set cur_path=%cd%
if not exist %opposite_src_path% goto ExitBat
cd %opposite_src_path%
set src_path=%cd%

if not exist "%src_path%\%adjust_path%" goto ExitBat

cd "%cur_path%\"
copy "%src_path%\%adjust_path%\%firmware%" ".\app.bin"
copy "%src_path%\%adjust_path%\%production%" "%production_file%"

wscript.exe "VBScript\FileCombiner.vbs" "%release_file%" 

del app.bin

gen_update_file.exe 100

@echo ################################################################
@echo Generated successful!
@echo ################################################################

@echo Close window in 2.4 seconds
ping /n 3 127.1 > ping.txt
del ping.txt
exit


:ExitBat
pause
exit