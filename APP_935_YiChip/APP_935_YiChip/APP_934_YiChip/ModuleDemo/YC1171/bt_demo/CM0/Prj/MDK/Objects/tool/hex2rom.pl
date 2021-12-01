#!/usr/bin/perl

my @rom = ();
die "missing start and end address" if(@ARGV < 2);

$addrhi = 0;
$baseaddr = hex($ARGV[0]);
$endaddr = hex($ARGV[1]);
$bits = 32;
$bits = $ARGV[2] if(@ARGV > 2);
$endian = 1;


$inputfile_addr = $ARGV[3];
print "\input:  $inputfile_addr\n";
$outputfile_addr =$ARGV[4];
print "\output:  $outputfile_addr\n";


open(DATA,$inputfile_addr);

while(<DATA>){
	$line++;
	$count = (hex substr($_, 1, 2));
	$addr = (hex substr($_, 3, 4));
	$type = (hex substr($_, 7, 2));
	if($type == 2 || $type == 4) {
		$addrhi = hex(substr($_, 9, 4)) << ($type == 2 ? 4 : 16);
		printf "type=%d, addr=%x, line=%d\n", $type, $addrhi, $line;
	}
	if($type == 0 && $addrhi >= $baseaddr && $addrhi <= $endaddr) {
		for($x=0; $x<$count; $x++) {
			$rom[$addrhi - $baseaddr + $addr + $x] = (hex substr($_, 9+2*$x, 2)) ; 
		}
	}
}

#删除目标文件
unlink($outputfile_addr);
#创建目标文件
$outputfile_addr =$ARGV[4];
$outputfile_addr = '+>'.$outputfile_addr;
print "$outputfile_addr\n";
open(DATA1,$outputfile_addr) or die "des file open fail,$!";

if($bits == 1) {
	for($x=0; $x<@rom; $x++) {
			printf ("%04x\n", $rom[$x]);
		}
		print "\n";
} else {
	for($x=0; $x<@rom; $x+=$bits/8) {
		for($i = 0;$i < $bits/8;$i++) {
			$tmp = sprintf("%02x",($endian ? $rom[$x+$bits/8-1-$i] : $rom[$x+$i])),
			print DATA1 "$tmp";
#			printf "$tmp";
			}
		print DATA1 "\n";
#		printf "\n";
	}
}
$inputfile_addr = $ARGV[3];
$outputfile_addr =$ARGV[4];
close(inputfile_addr);
close(outputfile_addr);
