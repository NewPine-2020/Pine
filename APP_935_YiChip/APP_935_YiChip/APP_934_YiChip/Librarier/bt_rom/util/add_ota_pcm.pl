
#第1个参数是输入ota_flash文件
#第2个参数是输入flash文件
#第3个参数是输入pcm文件
#第4个参数是输出目录

use strict;

if(@ARGV < 4) {
	printf "less  input \n";
	exit;
}
else{ 	
	open (FILE_OTA_FALSH,"$ARGV[0]") or die  "Can't open  file $ARGV[0] : $!"; 
	open(FILE_FLASH,"$ARGV[1]")		or die "Can't open  file $ARGV[1] : $!"; 
	open(FILE_PCM,"$ARGV[2]")		or die "Can't open  file $ARGV[2] : $!"; 
	open(FILE_OUT,">$ARGV[3]"."/flash_all.dat")		or die "Can't open  $ARGV[3] : $!"; 

	my	@lines = <FILE_OTA_FALSH>;
	my	$length = @lines;
	my $i =0;
	my $j=0;

	print FILE_OUT "00\n60\n03\n";
	for($i=3;$i<0x1000;$i+=1){
		print FILE_OUT "ff\n";
	}

	for($j=0; $j<$length;$j+=1){
		print FILE_OUT $lines[$j];
		$i+=1;
	}
########################
	for(; $i<0x6000;$i+=1){
		print FILE_OUT "ff\n";
	}

	@lines = <FILE_FLASH>;
	$length = @lines;
	for($j=0; $j<$length;$j+=1){
		print FILE_OUT $lines[$j];
		$i+=1;
	}		
	
######################
	for(; $i<0x70000;$i+=1){
		print FILE_OUT "ff\n";
	}

	@lines = <FILE_PCM>;
	$length = @lines;
	for($j=0; $j<$length;$j+=1){
		print FILE_OUT $lines[$j];
		$i+=1;
	}
#	print  "flash_all.dat generated\n";

	close(FILE_OTA_FALSH);
	close(FILE_FLASH);
	close(FILE_PCM);
	close(FILE_OUT);
	
	open(FILE_OUT,"$ARGV[3]"."/flash_all.dat")	;
	my @flashArray = <FILE_OUT>;
	my $flashLen = @flashArray; 
	printf "$ARGV[3]/flash_all.dat length:%d (0x%x)\n",$flashLen,$flashLen;	
	close(FILE_OUT);	
}