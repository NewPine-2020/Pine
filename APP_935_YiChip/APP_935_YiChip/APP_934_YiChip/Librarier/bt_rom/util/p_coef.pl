
#第1个参数是输入文件，第2个参数是输出文件；

if(@ARGV < 2) {
	printf "less  input \n";
	exit;
}
else{
	open (file, "$ARGV[0]");
	open (file_out,">$ARGV[1]")
		or die "Can't open out file  : $!"; 
	
	my @file_array = <file>;
	my $file_len = @file_array;
	
	my $i = 0,$j = 0,$k = 0;
	my $length=0;

	while($file_len > $i)
	{ 
		if($file_array[$i] =~ /[^a-fA-F0-9\s]/)
		{
	#		print  $file_array[$i];		
		}
		else
		{
			@string = split(' ', $file_array[$i]);
			for($k=0; $k< @string; $k++)
			{
				print file_out $string[$k],"\n";	
				$j++;
			}
		}
		$i=$i+1;
	}
	
	if($j != 192*3+82+336+153)
	{
		printf "\n!!!!! $ARGV[1] LENGTH ERROR !!!!!:0x%x %d\n",$j,$j;
	}
	else
	{
		printf "$ARGV[1] LENGTH:%d (0x%x)\n",$j,$j;
	}
#	print __FILE__," end  ";
	close(file);
	close(file_out);
}
