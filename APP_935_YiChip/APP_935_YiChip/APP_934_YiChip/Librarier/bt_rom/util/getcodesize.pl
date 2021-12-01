#输入output文件夹地址

$dir_output= "$ARGV[0]";
$membreg=$dir_output."/memmap.format";#"./memmap.format";
$flashfile1=$dir_output."/flash_add_coef.dat";#"./flash_add_coef.dat";
$flashfile2=$dir_output."/flash.dat";#"./flash.dat";

@svnveraddr = ("aa\n","55\n","09\n","00\n","00\n","00\n");

open(Mem, "<$membreg") or die "can't open $membreg: $!";
@memlines = <Mem>;
$memfilelength = @memlines;
#print "$memfilelength\n";
for($i=0;$i<$memfilelength;$i+=1)
{
		@string = split(' ', $memlines[$i]);

		if( $string[1] eq "mem_public_code_svn_version_code")
		{
			$svnveraddr[5] = substr($string[0],2,2)."\n";
			$svnveraddr[4] = substr($string[0],4,2)."\n";
			#print "addr: $svnveraddr[4]$svnveraddr[5]";
		}
}
close(Mem)|| die "无法关闭文件";


if(!open(Flashaddcoef,"+<$flashfile1"))#or die "can't open $flashaddcoef: $!";
{
		#print "***don't have coef,use flash.dat****";
		$addcoef=1;
		open(Flashaddcoef,"+<$flashfile2")
}
@flashlines = <Flashaddcoef>;
$flashlength = @flashlines;
#print "@flashlines";
if($addcoef==1)
{
		print "*******no coef file,flash.dat Code Size: $flashlength*********\n";
}
else
{
		print "*******flash_add_coef.dat Code Size: $flashlength*********\n";
}

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
			$index = $count+$regLen+6;
			#****************write svn version************************	
			$temptrans=($flashlength&0xff);
			$temptransHex = sprintf "%02x",$temptrans;
			$flashlines[$index++] = "$temptransHex\n";
			$temptrans=($flashlength&0xff00)>>8;
			$temptransHex = sprintf "%02x",$temptrans;
			$flashlines[$index++] = "$temptransHex\n";
			$temptrans=($flashlength&0xff0000)>>16;
			$temptransHex = sprintf "%02x",$temptrans;
			$flashlines[$index++] = "$temptransHex\n";
		}
	}
	#print "$count\n";
	print Flashaddcoef $flashlines[$count];
}
close(Flashaddcoef);