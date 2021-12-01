
if (@ARGV<2){
	printf "less input \n";
	exit;
}
else{
	my $i,my $j = @ARGV;
	my $btreg_path=$ARGV[$j-1];
	open(btreg,">$btreg_path")or die "can't open $btreg_path: $!";
	print btreg ("#ifndef _BT_REG_H_\n");
	print btreg ("#define _BT_REG_H_\n");
	
	for($i = 0;$i < $j-1;$i = $i+1)
	{
		my $mem_patch = $ARGV[$i];
		open(memmap,$mem_patch) or die "can't open $mem_patch: $!"; 
		@text = <memmap>;

		foreach $text (@text){
			@word = split (/[\t ]+/,$text);
			$reg_addr = @word[0];
			if ($reg_addr =~ /0[xX][0-9abcdefABCDEF]*/){#Is 0x0000~0xffff
				$word1 = @word[1];
				@reg_name = split(/\/[\/\*]/,$word1);#delete // or /*  */
				$reg_name = @reg_name[0];
				$reg_name =~ s/[\r\n]//;#delete Carriage return
				
				printf btreg ("#define\t%-40s\t%s\n",$reg_name,$reg_addr);
				#print btreg ("#define \t $reg_name \t\t\t $reg_addr \n");
				#print btreg ("@word[1]\n");
			}
		}
		close(memmap);	
	}

	print btreg ("#endif\n");
	close(btreg);
}
