::删除Keil编译产生的一些垃圾文件

echo off

echo ///////////////////////////////////////
echo 清理编译文件?
echo //////////////////////////////////////////

pause

del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*.o /s
del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*.d /s
del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*.crf /s
del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*.lst /s
del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*.htm /s
del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*.lnp /s
del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*.iex /s
del ModuleDemo\YC1171\bt_demo\CM0\Prj\MDK\Objects\*._2i /s

exit