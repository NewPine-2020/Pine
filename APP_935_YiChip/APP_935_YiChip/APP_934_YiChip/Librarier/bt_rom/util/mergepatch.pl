$defs = "";
$fmt = 0;


sub parseif {
	my($fname) = @_;
	if (@ARGV<2){
		$faddr="program";
	} else{
		$faddr=$ARGV[1];
	}
	#printf "$faddr\n";
	open file, $fname;
	@f = <file>;
	close file;
	open file, ">$fname";
	@valid = (1);
	for($i = 0;$i < @f;$i++) { 
		$_ = $f[$i];
		$defs .= $1 . " " if(/^define\s+(\w+)\s*/);
		if(/^ifdef\s+(\w+)\s*/) {
			push @valid, ($defs =~ /$1 / ? 1 : 0) & $valid[$#valid];
		} elsif(/^ifndef\s+(\w+)\s*/) {
			push @valid, ($defs =~ /$1 / ? 0 : 1) & $valid[$#valid];
		} elsif(/^else/) {
			printf "%s\nelse without if at line %d\n", $f[$i - 3], $i if($#valid == 0);
			$valid[$#valid] = (1 - $valid[$#valid]) & $valid[$#valid - 1];
		} elsif(/^endif/) {
			printf "%s\nendif without if at line %d\n", $_, $i if($#valid == 0);
			pop @valid;
		} elsif($valid[$#valid]) {
			if(/^include\s+(.+)\s*/) {
				open file2, "$faddr/$1";
				#printf "$faddr/$1\n";
				@f2 = <file2>;
				close file2;
				splice(@f, $i + 1, 0, @f2);
			} else {
				next if(/^\s*bpatch\s+/ && $defs !~ /ROMCODE/);
				next if(/^\s*set[01] mark_ext_patch/ && $defs !~ /ROMCODE/);
				print file $_;
			}
		}
	}
	close file;
}


sub zcode {
	open file, "output/bt_program23.meta";
	@f = <file>;
	close file;
	for($i = $label = $line = 0, $z = 1;$i < @f;$i++) {
		$_ = $f[$i];
		$line = $i - 1 if(/^org\s+0x[23]000/ && $line == 0);
		if(/^org z\s*$/) {
			$z += 0x10000;
			$f[$i] = sprintf("org 0x%x\n", $z);
			$label = 1;
			$line = $i - 1 if($line == 0);
		}
		if(/^(\w+):\s*$/ && $label == 1) {
			$lab{$1} = $z;
			$label = 2;
		}
	}
	return if($z == 1);
	for($i = 0;$i < @f;$i++) {
		$_ = $f[$i];
		$label = "";
		if(/^\s*(branch|nbranch|call|ncall)\s+(\w+)\s*$/ || /^\s*(branch|nbranch|call|ncall)\s+(\w+)\s*,\s*(\w+)\s*$/) {
			$label = $2;
		}
		if(/^\s*(bmark|beq|bne|bbit[01])\s+(\w+)\s*,\s*(\w+)\s*$/) {
			$label = $3;
		}
		if(exists($lab{$label})) {
			$nlabel = "jmpz_" . $label;
			$f[$i] =~ s/$label/eval "$nlabel"/e;
			if($lab{$label} > 100) {
				$f[$line] .= sprintf("%s:\n\tsetarg 0x%x\n\tbranch p_zcode_entrance_2Bytes_common\n\n", $nlabel, int($lab{$label}/0x1000));
				$lab{$label} = 1;
			}
		}
	}
	open file, ">output/bt_program23.meta";
	for($i = 0;$i < @f;$i++) {
		if($f[$i] =~ /^\s*bbit1 8,pf_patch_ext\s*$/) {
			printf file ("p_start:\n\tbranch p_patch_array\n\np_zcode:\n");
			for($j = 0;$j < 63;$j++) {
				printf file "\tnop %d\n", $j + 1;
			}
			printf file "p_patch_array:\n";
		}
		print file $f[$i];
	}
	close file;
}
parseif("output/bt_program23.meta");
$fmt = 1;
parseif("output/bt_format.meta");
zcode();

