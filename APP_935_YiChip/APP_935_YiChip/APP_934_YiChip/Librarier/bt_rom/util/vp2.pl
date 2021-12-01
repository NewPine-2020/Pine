#第1个参数是音频文件路径
#第2个参数是output路径

use strict;
use File::Spec;
if(@ARGV < 2) {
	printf "less input \n";
	exit;
}

my $dir_voice_prompt = "$ARGV[0]";
my $dir_output =  "$ARGV[1]";

my @vp_array;
$vp_array[0] = $dir_voice_prompt."/poweron.pcm.sbc";
$vp_array[1] = $dir_voice_prompt."/poweroff.pcm.sbc";
$vp_array[2] = $dir_voice_prompt."/pairing.pcm.sbc";
$vp_array[3] = $dir_voice_prompt."/connected.pcm.sbc";
$vp_array[4] = $dir_voice_prompt."/disconnect.pcm.sbc";
$vp_array[5] = $dir_voice_prompt."/low_battery.pcm.sbc";
$vp_array[6] = $dir_voice_prompt."/max_vol.pcm.sbc";
$vp_array[7] = $dir_voice_prompt."/min_vol.pcm.sbc";
$vp_array[8] = $dir_voice_prompt."/tws_connected_l.pcm.sbc";
$vp_array[9] = $dir_voice_prompt."/tws_connected_r.pcm.sbc";
$vp_array[10] = $dir_voice_prompt."/language.pcm.sbc";
$vp_array[11] = $dir_voice_prompt."/empty_voice_0.pcm.sbc";
#=pod
$vp_array[12] = $dir_voice_prompt."/num_0.pcm.sbc";
$vp_array[13] = $dir_voice_prompt."/num_1.pcm.sbc";
$vp_array[14] = $dir_voice_prompt."/num_2.pcm.sbc";
$vp_array[15] = $dir_voice_prompt."/num_3.pcm.sbc";
$vp_array[16] = $dir_voice_prompt."/num_4.pcm.sbc";
$vp_array[17] = $dir_voice_prompt."/num_5.pcm.sbc";
$vp_array[18] = $dir_voice_prompt."/num_6.pcm.sbc";
$vp_array[19] = $dir_voice_prompt."/num_7.pcm.sbc";
$vp_array[20] = $dir_voice_prompt."/num_8.pcm.sbc";
$vp_array[21] = $dir_voice_prompt."/num_9.pcm.sbc";


$vp_array[22] = $dir_voice_prompt."/poweron_ch.pcm.sbc";
$vp_array[23] = $dir_voice_prompt."/poweroff_ch.pcm.sbc";
$vp_array[24] = $dir_voice_prompt."/pairing_ch.pcm.sbc";
$vp_array[25] = $dir_voice_prompt."/connected_ch.pcm.sbc";
$vp_array[26] = $dir_voice_prompt."/disconnect_ch.pcm.sbc";
$vp_array[27] = $dir_voice_prompt."/low_battery_ch.pcm.sbc";
$vp_array[28] = $dir_voice_prompt."/max_vol_ch.pcm.sbc";
$vp_array[29] = $dir_voice_prompt."/min_vol_ch.pcm.sbc";
$vp_array[30] = $dir_voice_prompt."/tws_connected_l_ch.pcm.sbc";
$vp_array[31] = $dir_voice_prompt."/tws_connected_r_ch.pcm.sbc";
$vp_array[32] = $dir_voice_prompt."/language_ch.pcm.sbc";
$vp_array[33] = $dir_voice_prompt."/empty_voice_0_ch.pcm.sbc";

$vp_array[34] = $dir_voice_prompt."/num_0_ch.pcm.sbc";
$vp_array[35] = $dir_voice_prompt."/num_1_ch.pcm.sbc";
$vp_array[36] = $dir_voice_prompt."/num_2_ch.pcm.sbc";
$vp_array[37] = $dir_voice_prompt."/num_3_ch.pcm.sbc";
$vp_array[38] = $dir_voice_prompt."/num_4_ch.pcm.sbc";
$vp_array[39] = $dir_voice_prompt."/num_5_ch.pcm.sbc";
$vp_array[40] = $dir_voice_prompt."/num_6_ch.pcm.sbc";
$vp_array[41] = $dir_voice_prompt."/num_7_ch.pcm.sbc";
$vp_array[42] = $dir_voice_prompt."/num_8_ch.pcm.sbc";
$vp_array[43] = $dir_voice_prompt."/num_9_ch.pcm.sbc";


$vp_array[44] = $dir_voice_prompt."/didi.pcm.sbc";
$vp_array[45] = $dir_voice_prompt."/ring.pcm.sbc";
$vp_array[46] = $dir_voice_prompt."/empty_fix_voice_0.pcm.sbc";
$vp_array[47] = $dir_voice_prompt."/empty_fix_voice_1.pcm.sbc";
#=cut

my $check_header = 0xf5fa;
my $check_header_length= 2;
my @vp_startaddr;
my @vp_size;
my $vp_array_len=@vp_array;

my $memmap = $dir_output."/memmap.format";
my $outputfile = $dir_output."/flash.dat";
my $outputfile2 = $dir_output."/flash2.dat";
my $allpcmfile = $dir_output."/all_pcm_data.dat";
my @regStr = ("aa\n","55\n","03\n","00\n","00\n","00\n");
my @args;
my $hexL,my $hexM,my $hexH;

my $count,my $countIn,my $countInin;
my $length=0;
my $buff;
my $searchRes = 0;
my $temptrans,my $temptransHex;


open(FLASHOUT, "$memmap") or die "can't open $memmap: $!";
my @lines = <FLASHOUT>;
my $length = @lines;
my $i;
for($i=0;$i<$length;$i+=1){
	my @string = split(' ', $lines[$i]);
	my $length_s = @string ;
	if( $string[1] eq "mem_vp_addr"){
#		print "$string[1] $string[0]\n";
#		print @regStr;	
		$regStr[5] = substr($string[0],2,2)."\n";
		$regStr[4] = substr($string[0],4,2)."\n";
#		print $regStr[4];
#		print $regStr[5];
#		print @regStr;
	}
}
close(FLASHOUT);

# __END__


#replace file
open(FLASHOUT, "$outputfile") or die "can't open $outputfile: $!";
my @flashArray = <FLASHOUT>;
my $flashLen = @flashArray; 
#$vp_startaddr[0] = @flashArray;
$vp_startaddr[0] = 0x70000;
close(FLASHOUT);

=pod
=cut

for($count = 0; $count < $vp_array_len;$count++)
{
	@args = stat ($vp_array[$count]);
	if ("$args[7]"=="")
	{
		$vp_size[$count] = 0;
		$vp_startaddr[$count+1] = $vp_startaddr[$count];
	}
	else
	{
		$vp_size[$count] = $args[7];	
		$vp_startaddr[$count+1] = $vp_startaddr[$count]+$vp_size[$count]+3+$check_header_length;
	}
}

#remove empty voice;
for($count = 0; $count < $vp_array_len;$count++)
{
	@args = stat ($vp_array[$count]);
	if ("$args[7]"=="")
	{
		$vp_size[$count] = 0;
		$vp_startaddr[$count] = 0;
	}
}


my $regLen = @regStr;
open(FLASHOUT, ">$outputfile2") or die "can't open $outputfile2: $!";

#print FLASHOUT "$flashLen\n";
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
			for($countInin = 0; $countInin < $vp_array_len; $countInin++)
			{
				$temptrans = ($vp_startaddr[$countInin] & 0xff);
				$temptransHex = sprintf "%02x",$temptrans;
				$flashArray[$index++] = "$temptransHex\n";
				$temptrans = ($vp_startaddr[$countInin] & 0xff00)>>8;
				$temptransHex = sprintf "%02x",$temptrans;
				$flashArray[$index++] = "$temptransHex\n";
				$temptrans = ($vp_startaddr[$countInin] & 0xff0000)>>16;
				$temptransHex = sprintf "%02x",$temptrans;
				$flashArray[$index++] = "$temptransHex\n";
			}
		}
	}
	print FLASHOUT $flashArray[$count];
}

close(FLASHOUT);


=pod
=cut

#open(FLASHOUT, ">>$outputfile") or die "can't open $outputfile: $!";

open(FLASHOUT, ">$allpcmfile") or die "can't open $allpcmfile: $!";

for($count=0; $count < $vp_array_len;$count++)
{
	if("$vp_size[$count]" == "0")
	{	
		next;
	}
	
	open(VPPWO, $vp_array[$count]) or die "can't open $vp_array[$count]: $!";
	binmode(VPPWO);
	#print file size
	$hexL = $vp_size[$count]&0xff;
	$hexM = ($vp_size[$count]&0xff00) >>8;
	$hexH = ($vp_size[$count]&0xff0000) >>16;
	
	for($i=0;$i<$check_header_length;$i++)
	{
		my $temp = sprintf "%02x", ($check_header>>(8*$i))&0xff;
		print FLASHOUT "$temp\n"; 
	}
	
	my $sizestr = sprintf "%02x",$hexL;
	print FLASHOUT "$sizestr\n";
	$sizestr = sprintf "%02x",$hexM;
	print FLASHOUT "$sizestr\n";
	$sizestr = sprintf "%02x",$hexH;
	print FLASHOUT "$sizestr\n";
	
	#print file content
	while(read(VPPWO, $buff, 1))
	{
	   $hexL=unpack("H*",$buff);
	   print FLASHOUT "$hexL\n";
	}
	close(VPPWO);
}

#close file handle.
close(FLASHOUT);


open(FLASHOUT, "$outputfile2") or die "can't open $outputfile2: $!";
my @lines = <FLASHOUT>;
my $length = @lines;
if($length > 0x57000 ){	
#	printf "MAX LENGTH:356352 (0x57000) ";
	printf "%s  length:%d (0x%x)\n",$outputfile2,$length,$length;	
	printf "\n--------- %s TOO LARGE ----------\n\n",$outputfile2;	
	printf "\n--------- %s TOO LARGE ----------\n\n",$outputfile2;	
}else{
#	printf "MAX LENGTH:356352 (0x57000) ";
	printf "%s length:%d (0x%x)\n",$outputfile2,$length,$length;
}

close(FLASHOUT);

open(FLASHOUT, "$allpcmfile") or die "can't open $allpcmfile: $!";
my @lines = <FLASHOUT>;
my $length = @lines;
if($length > 0x0F000 ){	
#	printf "MAX LENGTH:61440 (0xF000) ";
	printf "%s  length:%d (0x%x)\n",$allpcmfile,$length,$length;	
	printf "\n--------- %s TOO LARGE ----------\n\n",$allpcmfile;	
	printf "\n--------- %s TOO LARGE ----------\n\n",$allpcmfile;	
}else{
#	printf "MAX LENGTH:61440 (0xF000) ";
	printf "%s  length:%d (0x%x)\n",$allpcmfile,$length,$length;
}
close(FLASHOUT);
