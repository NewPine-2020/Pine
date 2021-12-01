@echo on
cls

set ROOT_PATH=..\..
set RESPIN_PATH=%ROOT_PATH%\respin
set OBJ_PATH=.\Objects
set now_path=%~dp0
call:getstrlocation %now_path% ModuleDemo location
call:get_fix_len_str  %now_path% location soft_path
set lib_path=%soft_path%Librarier
set rom_path=%lib_path%\bt_rom
set util=%rom_path%\util
@echo on
echo start compile cm0 bin
set OBJ_PATH=.\Objects
perl %OBJ_PATH%\tool\hex2rom.pl 0 80000  32 %OBJ_PATH%\yc11xx.hex  %OBJ_PATH%\output\out.rom
copy %OBJ_PATH%\output\out.rom ..\Output

echo compile cm0 bin ok
@echo off
copy %RESPIN_PATH%\ramcode.rom ..\Output\ramcode.rom
copy %RESPIN_PATH%\sched.rom   ..\Output\sched.rom
copy %RESPIN_PATH%\memmap.format   ..\Output\memmap.format
copy %RESPIN_PATH%\zcode.rom   ..\Output\zcode.rom 
cd ..\Output
::%util%/geneep  -n -z 
%util%/geneep -f -n -z

del zcode.rom 	/s
del ramcode.rom 	/s
del memmap.format 	/s
del sched.rom 	/s

perl %util%\flash2bin.pl

echo compile full bin ok

:end
goto:eof

::------------------------------------------
::---------------func:get_fix_len_str-------
::------------------------------------------
:get_fix_len_str
setlocal
set "getstr=%NULL%"
set str=%1
set /a len=%2
set num=0
:get_fix_len_str_loop
set getstr=%getstr%%str:~0,1%
set /a num+=1
set str=%str:~1%
if %num% lss %len% goto get_fix_len_str_loop
(endlocal
 set %3=%getstr%
)
goto:eof
::---------------end:func-------------------



::------------------------------------------
::---------------func:getstrlocation-------
::------------------------------------------
:getstrlocation
setlocal
set "bstr=%1"
set "cstr=%2"
set %3=0
set "num=0"
set "len_bstr=0"
set "len_cstr=0"

call:getstrlen %bstr% len_bstr
call:getstrlen %cstr% len_cstr

:getstrlocation_next
set /a len_remain=len_bstr-num
if %len_remain% lss %len_cstr% (
echo "cann't get str location"
set /a num =0
goto getstrlocation_last
)

call:get_fix_len_str %bstr%  len_cstr compar_str

if not %compar_str%==%cstr% (
set /a num+=1
set "bstr=%bstr:~1%"
goto getstrlocation_next
)

:getstrlocation_last
(endlocal
	set %3=%num%
)
goto:eof
::---------------end:func-------------------




::------------------------------------------
::---------------func:getstrlen-------------
::------------------------------------------
:getstrlen
setlocal
set str=%1
set %2=0
set /a "num=0"

:getstrlen_next
if "%str%"=="" goto getstrlen_last

if not "%str%"=="" (
set /a num+=1 
set "str=%str:~1%"
goto getstrlen_next
)

:getstrlen_last
(endlocal
	set /a "%2=%num%"
)
goto:eof
::---------------end:func-------------------




::------------------------------------------
::---------------func:getcharlocation-------
::------------------------------------------
:getcharlocation
setlocal
set "str=%1"
set "ch=%2"
set %3=0
set /a "num=0"

:getcharnext
if "%str%"=="" ( 
set /a num=0
goto getcharlast
)

if not "%str%"=="" (
set /a num+=1
if "%str:~0,1%"=="%ch%" goto getcharlast
set "str=%str:~1%"
goto getcharnext
)
:getcharlast
(endlocal
set %3=%num%
)
goto:eof 
::---------------end:func-------------------
