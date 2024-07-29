#!/usr/bin/perl

# $ARGV[0] is file to be scanned
# $ARGV[1] is number of children

use strict;
use warnings;

if ($#ARGV < 0) {
  print "\nUsage example: $0 output 4\n";
  print "\n  where 4 is value of NUMCHILDREN and output is file produced as follows:\n";
  print "\n  prog4 2>&1 | tee output\n\n";
  exit(0);
}

my $score = 0;
my $numChildren = $ARGV[1];
my $installing = 0;
my $childAverage = 0;
my $sending = 0;
my $caught = 0;
my $normally = 0;
my $abnormally = 0;
my $finalAverage = 0;

open(IN, "<$ARGV[0]") || die("Can't open $ARGV[0] for reading - $!");
while (<IN>) {
  if (/Parent process \d+ installing SIGUSR1 handler/i) {
    $installing++;
  } elsif (/Child process \d+ sending SIGUSR1 to parent process with the partial average \d+/i) {
    $sending++;
  } elsif (/Parent process caught SIGUSR1 with partial average: \d+/i) {
    $caught++;
  } elsif (/Child process \d+ terminated normally with exit status \d+/i) {
    $normally++;
  } elsif (/Child process \d+ terminated abnormally/i) {
    $abnormally++;
  } elsif(/Average = \d+/i){
    $finalAverage++;
  } elsif(/Child process \d+ finding the average from \d+ to \d+/i){
    $childAverage++;
  }
}
close(IN);

print "installing     = $installing\n";
print "child average  = $childAverage\n";
print "sending        = $sending\n";
print "caught         = $caught\n";
print "normally       = $normally\n";
print "abnormally     = $abnormally\n";
print "final average  = $finalAverage\n";

if ($installing == 1) {
  $score += 3;
} elsif ($installing > 1) {
  $score += 1;
}
if ($sending == $numChildren) {
  $score += 20;
} elsif ($sending > 0) {
  $score += 10;
}
if ($caught == $numChildren) {
  $score += 20;
} elsif ($caught > 0) {
  $score += 10;
}
if ($normally == $numChildren) {
  $score += 20;
} elsif ($normally > 0) {
  $score += 10;
}
if ($abnormally == 0) {
  $score += 2;
}
if($finalAverage == 1){
    $score += 5;
}
if($childAverage == $numChildren){
    $score += 20;
} elsif($childAverage > 0){
    $score += 10;
}
print "-------------------\n";
print "sub-score  = $score/90\n";

exit($score);
