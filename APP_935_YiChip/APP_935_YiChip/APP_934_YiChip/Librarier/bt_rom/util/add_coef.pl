
#第1个参数是output路径
#第2个参数是输入flash文件
#第3个参数是输入coef文件
#第4个参数是输出flash文件

if(@ARGV < 4) {
	printf "less  input \n";
	exit;
}
else{ 	

#####################################################
	open(MEMMAP, "$ARGV[0]"."/memmap.format") or die "can't open $memmap: $!";
	my @regStr = ("aa\n","55\n","03\n","00\n","00\n","00\n");
	my @lines = <MEMMAP>;
	my $length = @lines;
	my $i;
	for($i=0;$i<$length;$i+=1){
		my @string = split(' ', $lines[$i]);
		my $length_s = @string ;
		if( $string[1] eq "mem_coef_base_addr"){
	#		print "$string[1] $string[0]\n";
	#		print @regStr;	
			$regStr[5] = substr($string[0],2,2)."\n";
			$regStr[4] = substr($string[0],4,2)."\n";
	#		print $regStr[4];
	#		print $regStr[5];
	#		print @regStr;
		}
	}
	close(MEMMAP);
######################################################

#=pod
########################################################
	open(FLASHIN, "$ARGV[0]"."/$ARGV[1]") or die "can't open $ARGV[1] : $!";
	my @flashArray = <FLASHIN>;
	my $flashLen = @flashArray; 
	close(FLASHIN);

	my $regLen = @regStr;
	open(FLASHOUT, ">$ARGV[0]"."/$ARGV[3]") or die "can't open $ARGV[3] $!";

	for($count=0;$count<$flashLen;$count++){
		if ($searchRes == 0)
		{
			for($countIn=0; $countIn<$regLen; $countIn++){
				if ($countIn == 2){
					next;
				}
				if ($flashArray[$count+$countIn] ne $regStr[$countIn])
				{
					last;
				}
			}
			if ($countIn == $regLen)
			{
				$searchRes = 1;	
				my $index = $count+$regLen;

				$temptrans = ($flashLen & 0xff);
				$temptransHex = sprintf "%02x",$temptrans;
				$flashArray[$index++] = "$temptransHex\n";
				$temptrans = ($flashLen & 0xff00)>>8;
				$temptransHex = sprintf "%02x",$temptrans;
				$flashArray[$index++] = "$temptransHex\n";
				$temptrans = ($flashLen & 0xff0000)>>16;
				$temptransHex = sprintf "%02x",$temptrans;
				$flashArray[$index++] = "$temptransHex\n";	
			}
		}
		print FLASHOUT $flashArray[$count];
	}
########################################################
#=cut

######################################################
	open(COEF_OUT, "$ARGV[0]"."/$ARGV[2]") or die "can't open $ARGV[2]: $!";
	my @flashArray = <COEF_OUT>;
	close(COEF_OUT);

	print FLASHOUT @flashArray;
	close(FLASHOUT);
######################################################

	open(FLASHOUT, "$ARGV[0]"."/$ARGV[3]");
	my @flashArray = <FLASHOUT>;
	my $flashLen = @flashArray; 
	printf "$ARGV[0]/$ARGV[3] length:%d (0x%x)\n",$flashLen,$flashLen;	
	close(FLASHOUT);

}