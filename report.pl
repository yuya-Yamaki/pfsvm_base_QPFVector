#! /usr/bin/perl
while (<>) {
	print "$+\n" if /^Test image is (.*)/;
	if (/^[01]$/) {
		$snr = <> . <> . <> .<>;
	}
	if (/Areal of integrated area\(\)  = (.*)/) {
		$bdsnr = $+;
	}
	if (/Percentage difference between the courves are = (.*)/) {
		$bdrate = $+; 
		print $snr;
		print "$bdsnr\n";
		print "$bdrate\n";
	}
}