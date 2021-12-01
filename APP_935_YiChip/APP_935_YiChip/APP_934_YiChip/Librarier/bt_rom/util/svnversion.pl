
#输入output文件夹地址

$dir_output= "$ARGV[0]";
$membreg=$dir_output."/memmap.format";
$flashaddcoef=$dir_output."/flash.dat";
#$membreg="<C:/Users/Administrator/Desktop/YiChipTWS/TWSnew/ModuleDemo/24_TWS/bt_demo/BT/output/memmap.format";
#$flashaddcoef="+<C:/Users/Administrator/Desktop/YiChipTWS/TWSnew/ModuleDemo/24_TWS/bt_demo/BT/output/flash2.dat";
@svnveraddr = ("aa\n","55\n","06\n","00\n","00\n","00\n");



#**************************get time**********************************************
use POSIX qw(strftime);
#$datestring = strftime "%m-%d-%H-%M", localtime;
$datestring[0] =  strftime "%m", localtime;
$datestring[1] =  strftime "%d", localtime;
$datestring[2] =  strftime "%H", localtime;
$datestring[3] =  strftime "%M", localtime;
print "TIMER:$datestring[0]-$datestring[1] $datestring[2]:$datestring[3]\n";
#************************get svn version***********************************************
$svninfo=`svn info -r COMMITTED .\\..\\..\\..\\..\\..\\`;

#print "\$svninfo=$svninfo\n";

if($svninfo=~/Revision:/)
{
	#print "find version ok\n";
	$'=~ /(\d+)/;                                                #'
	$svnvernum=$1;
	print "******get svnversion is: $svnvernum*********\n";
}
else
{
	print "/************************ERROR****************************/\n";
	print "find version faild,please check svn commmand<svn info>\n";
}

#********************get mem addr*******************************
open(Mem, "<$membreg") or die "can't open $membreg: $!";
@memlines = <Mem>;
$memfilelength = @memlines;
#print "$memfilelength\n";
for($i=0;$i<$memfilelength;$i+=1)
{
		@string = split(' ', $memlines[$i]);
		#print "@string\n";
		if( $string[1] eq "mem_public_code_svn_version_code")
		{
			#print "find mem_public_code_svn_version_code ok\n";
			$svnveraddr[5] = substr($string[0],2,2)."\n";
			$svnveraddr[4] = substr($string[0],4,2)."\n";
			#print "$svnveraddr[4]$svnveraddr[5]";
		}
		#elsif( $string[1] eq "mem_public_code_time_info") 
		#{
			#print "find mem_public_code_time_info ok\n";
		#	$timeraddr[0] = substr($string[0],2,2)."\n";
		#	$timeraddr[1] = substr($string[0],4,2)."\n";
		#	print "$timeraddr[0]$timeraddr[1]\n";
		#}
}
close(Mem)|| die "无法关闭文件";
#***********************change flash****************************************
open(Flashaddcoef,"+<$flashaddcoef") or die "can't open $flashaddcoef: $!";
@flashlines = <Flashaddcoef>;
$flashlength = @flashlines;
#print "@flashlines";
$regLen = @svnveraddr; 
#print "$addcoeflength\n";

seek (Flashaddcoef, 0, 0);
for($count=0;$count<$flashlength;$count++)
{
	if ($searchRes == 0)
	{
		for($countIn=0; $countIn<$regLen; $countIn++)
		{
				if ($countIn == 2)
				{
					next;
				}
				if ($flashlines[$count+$countIn] ne $svnveraddr[$countIn])
				{
					last;
				}
		}
		if ($countIn == $regLen)
		{
			#print "find .dat mem location ok\n";	
			$searchRes = 1;
			#print "count:$count  regLen:$regLen\n";
			$index = $count+$regLen;
			#****************write svn version************************	
			$temptrans=($svnvernum&0xff);
			$temptransHex = sprintf "%02x",$temptrans;
			#print "$temptransHex\n";
			$flashlines[$index++] = "$temptransHex\n";
			$temptrans=($svnvernum&0xff00)>>8;
			$temptransHex = sprintf "%02x",$temptrans;
			#print "$temptransHex\n";
			$flashlines[$index++] = "$temptransHex\n";
			#*********************write time*************************************
			$flashlines[$index++] = "$datestring[0]\n";
			$flashlines[$index++] = "$datestring[1]\n";
			$flashlines[$index++] = "$datestring[2]\n";
			$flashlines[$index++] = "$datestring[3]\n";
		}
	}
	#print "$count\n";
	print Flashaddcoef $flashlines[$count];
}
close(Flashaddcoef);

#print "@flashlines";
#open(Flashaddcoef, "+>".$flashaddcoef) or die "can't open $flashaddcoef: $!";
#print Flashaddcoef "@flashlines";
#close(Flashaddcoef);

	

