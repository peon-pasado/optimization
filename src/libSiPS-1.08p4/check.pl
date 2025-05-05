#! /usr/bin/perl -w
my $ifile = $ARGV[0];
my $sfile = $ARGV[1];
open(H, "< $ifile") || die "Cannot open file $ifile: $!\n";
my (@p, @d, @r) = ((), (), ());
while( <H> ) {
  my @dat = split;
  chomp;
  (push(@p, $dat[0]), push(@r, $dat[1]), push(@d, $dat[2])) if $#dat == 2;
}  
close(H);
open(H, "< $sfile") || die "Cannot open file $sfile: $!\n";
my @seq = ();
while( <H> ) {
  chomp;
  push(@seq, $_);
}  
close(H);
my ($tardiness, $c) = (0, 0);
for(local $i = 0; $i < scalar(@seq); $i++) {
  $c = $r[$seq[$i] - 1] if $r[$seq[$i] - 1] > $c;
  $c += $p[$seq[$i] - 1];
  $tardiness += $c - $d[$seq[$i] - 1] if $c - $d[$seq[$i] - 1] > 0;
}
print STDOUT "tardiness=$tardiness\n";
