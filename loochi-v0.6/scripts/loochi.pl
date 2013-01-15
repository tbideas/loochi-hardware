#!/opt/local/bin/perl -w

use strict;
use Time::HiRes qw { sleep };
use IO::Select;
use Term::ReadKey;
use List::Util qw { sum };

# Based on: http://the-bus-pirate.googlecode.com/svn/trunk/scripts/BPbinmode.pl - Thanks!

#setup serial port for Linux
use Device::SerialPort;
#my $port = Device::SerialPort->new("/dev/cu.usbserial-A4013IR"); #change to your com port
my $port = Device::SerialPort->new("/dev/tty.usbserial-A4013IR5"); #change to your com port

die "Unable to open port! ($!)\n" unless $port;

#port configuration  115200/8/N/1
$port->databits(8);
$port->baudrate(115200);
$port->parity("none");
$port->stopbits(1);
$port->buffers(1, 1); #1 byte or it buffers everything forever
$port->write_settings		|| undef $port; #set
unless ($port)			{ die "couldn't write_settings"; }


print "Bus Pirate binmode tests v1.\n";
print "Use Bus Pirate firmware v2.4+.\n";

#test entering raw SPI mode from bitbang mode
print "Entering raw SPI mode: ";
if(&enterRawSPImode==1){
	print "OK.\n";
}else{
	print "failed.\n";
	die "Couldn't complete test.\n";
}

# Set SPI config: 1000 wxyz
#   w=HiZ/3.3v, x=CKP idle, y=CKE edge, z=SMP sample
$port->write("\x8A");
sleep(0.1); debug_read("Set Config: ");

# Set SPI Speed: 01100xxx  
$port->write("\x62"); # 2=> 250 kHz 3=> 1Mhz 4 => 4Mhz
sleep(0.1); debug_read("Set Speed: ");

print "\nPress 'q' to quit\n";

ReadMode 4;

$|++;

my @adcvalues = ();
my @adcstats = ();


my ($red, $green, $blue) = (0, 0, 0);

while (1) {
	if (defined (my $key = ReadKey(-1))) {
		if ($key eq "+") {
			$red++;
			$green++;
			$blue++;
		}
		elsif ($key eq "-") {
			$red--;
			$green--;
			$blue--;
		}
		elsif ($key eq "r") { $red++; }
		elsif ($key eq "R") { $red--; }
		elsif ($key eq "g") { $green++; }
		elsif ($key eq "G") { $green--; }
		elsif ($key eq "b") { $blue++; }
		elsif ($key eq "B") { $blue--; }
		elsif ($key eq "\n") {
			print "\n";
		}
		elsif ($key eq "q") {
			sendLampColor(0x00, 0x00, 0x00);
			print "\n";
			last;
		}
		elsif ($key eq "z") {
			$red = 0;
			$green = 0;
			$blue = 0;
		}
		elsif ($key eq "Z") {
			$red = 0xFF;
			$green = 0xFF;
			$blue = 0xFF;
		}
		elsif ($key eq "I") { $red = 0xFF; }
		elsif ($key eq "O") { $green = 0xFF; }
		elsif ($key eq "P") { $blue = 0xFF; }
		
		($red, $green, $blue) = map { min(max(0x00, $_), 0xFF); } ($red, $green, $blue);

		printf("Red: %02x Green: %02x Blue: %02x\n", $red, $green, $blue);
		sendLampColor($red, $green, $blue);
	}
}

ReadMode 0;

#do bus pirate hardware reset
print "Reset Bus Pirate to user terminal: ";
if(&exitBinMode==1){
	print "OK.\n";
}else{
	print "failed.\n";
	die "Couldn't complete test.\n";
}


sub sendLampColor
{
	my ($red, $green, $blue) = @_;
	$port->purge_all();
	# Note: first byte is for the bus pirate
	my $cmd = pack("CCCC", 0x12, $red, $green, $blue);
	$port->write($cmd);	
	sleep(0.05);
	
	my $ack = $port->read(1);
	my $data = $port->read(3);
	return $data;
}

sub debug_read
{
	my ($count_in, $string_in) = $port->read(42);
	my @data = unpack "C*", $string_in;
	
	my $prompt = pop || "Read ($count_in): ";
	print $prompt;
	printf "%02X ", $_ foreach (@data);
}

sub stdev{
        my($data) = @_;
        if(@$data == 1){
                return 0;
        }
        my $average = &average($data);
        my $sqtotal = 0;
        foreach(@$data) {
                $sqtotal += ($average-$_) ** 2;
        }
        my $std = ($sqtotal / (@$data-1)) ** 0.5;
        return $std;
}

sub average{
        my($data) = @_;
        if (not @$data) {
                die("Empty array\n");
        }
        my $total = 0;
        foreach (@$data) {
                $total += $_;
        }
        my $average = $total / @$data;
        return $average;
}

sub max ($$) { $_[$_[0] < $_[1]] }
sub min ($$) { $_[$_[0] > $_[1]] }

###############################
#
#
# 		Helper functions
#
#
###############################
#this function ensures we're in binmode and enters SPI mode
#returns SPIx version number, or 0 for failure
sub enterRawSPImode {
	#this section is optional, it ensures that we're in binmode and that the Bus Pirate responds
	my $ver=&enterBinMode; #return to BBIO mode (0x00), (should get BBIOx)
	if($ver == 0){
		return 0; #failed, not in binmode
	}else{
		#print "(BBIO OK) "; #debug
	}
	
	#this part actually enters binmode
	$port->write("\x01"); #send 0x01 in binmode to enter raw SPI mode
	select(undef,undef,undef, .1); #sleep for fraction of second for data to arrive #sleep(1);
	my $char= $port->read(4); #look for SPIx
	if($char){
		print "(" . $char . ") "; #debug
		if($char eq "SPI1"){#if we got text, is it BBIOx?
			return 1; #return version number
		}
	}
	return 0;

}

#Returns to user terminal mode from raw binary mode
#resets hardware  and exits binary mode
#returns BBIO version, or 0 for failure
sub exitBinMode{

	#make sure we're in BBIO (not spi, etc) binmode before sending reset command
	my $ver=&enterBinMode; #return to BBIO mode (0x00), (should get BBIOx)
	
	#if we're ready, send the reset command
	if($ver){
		$port->write("\x0F"); #send 0x0f to do a hardware reset
	}
	return $ver;
}

#this function puts the Bus Pirate in binmode
#returns binmode version number, 0 for failure
sub enterBinMode {
#it could take 1 or 20 0x00 to enter Bus Pirate binary mode
#it will take 20 if we're currently at the user terminal mode
#it will only take 1 if the Bus Pirate is already in a raw mode
#BP replies BBIOx where x is the protocol version
	
	my $count=40;
	my $char="";
	while($count){
		$port->write("\x00"); #send 0x00
		select(undef,undef,undef, .02); #sleep for fraction of second for data to arrive #sleep(1);
		$char= $port->read(5); #look for BBIOx
		if($char){
			#print "(" . $char . ") "; #debug
			if($char eq "BBIO1"){#if we got text, is it BBIOx?
				return 1; #return version number
			}
		}
		$count--; #if timeout, then try again
	}
	return 0; #for fail, version number for success
}

#The Bus Pirate might be stuck in a configuration menu or something when we connect
#send <enter> 10 times, then #<enter> to reset the Bus Pirate
#need to pause and flush buffer when complete
sub userTerminalReset{
	$port->write("\n\n\n\n\n\n\n\n\n\n#\n");
	#now flush garbage from read buffer
}

#debug variable transformation
#$char =~ s/\cM/\r\n/; #debug