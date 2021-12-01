::@echo off
cls
echo start download ROM code...
cd ..\Output

e pu
e fs 0
e pu
::e 8097   040307020506  
:: enable flash vcc
e 809a 3f
:: set flash io port
e 8090 060502
::e 8094 040307
e 8094 070304
e fe 0 
e pu
e fa
e 8043 00
::e fw 3 a5
e fp flash.dat 1000
e fw 4 a5
e fw 0 031000
e k
::pause


echo download ROM code OK
goto:eof
































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

