#!/usr/bin/perl
# iterate.perl -- Run a single command in multiple directories
use Cwd;

$d = 0;
$c = "";
$part_two = 0;
$i = 0;

if ($#ARGV == -1) {
    print "$0 -- Execute command in multiple directories\n";
    print "\tUsage:  $0 [dir ...] - command [arguments]\n";
    exit 0;
}

while ($i <= $#ARGV) {
  if ($ARGV[$i] eq "-") {
    $part_two = 1;
  } elsif ($part_two == 1) {
    $c .= " " . $ARGV[$i];
  } else {
    $dirs[$f++] = $ARGV[$i];
  }
  $i++;
}

# print "Dirs:  @dirs\n";
# print "Commands:  $c\n";
$cw = cwd;
die "Cannot determine current directory!\n" unless (-d $cw);

for $i (@dirs) {
  if (-d $i) {
    chdir ($i);
    $exitStatus = system($c);
    if (0 != $exitStatus) {
      warn "Failed to execute \"$c\"\n";
      exit(1);
    }
    chdir ($cw);
  } else {
    warn "Cannot change directory to $i, skipping\n";
  }
}
