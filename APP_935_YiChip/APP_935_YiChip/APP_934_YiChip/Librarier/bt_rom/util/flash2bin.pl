use strict;
#$source_file = $ARGV[0];
#$dest_file = $ARGV[1];


open (file,"flash.dat") or die  "Can't open out file  : $!"; 
open(BINFILE, ">flash.bin")	or die "Can't open out file  : $!"; 
binmode(BINFILE);
while (<file>) {
	printf BINFILE "%c", hex();
}
close(file);
close(BINFILE);
    
   
