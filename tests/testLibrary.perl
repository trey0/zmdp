
sub dosys {
    my $cmd = shift;
    print "$cmd\n";
    my $ret = system($cmd);
    if (0 != $ret) {
	die "ERROR: '$cmd' returned exit status $ret\n";
    }
    return $ret;
}

sub testZmdpSolve {
    my %params = @_;

    my $cmd = $params{cmd};
    
    print "$cmd\n";

    open(IN, "$cmd 2>&1 |") or die "ERROR: couldn't run [$cmd]: $!\n";
    my $numpat = "(-?\\d+(\\.\\d*)?([eE][+-]\\d+)?)";
    my ($lb, $ub, $done);
    while (<IN>) {
	print;
	chop;
	if (/bounds\s*\[\s*$numpat\s*..\s*$numpat\s*\]/) {
	    $lb = $1;
	    $ub = $4;
	    #print "$1 $2 $3 $4 $5 $6\n";
	}
	if (/done$/) {
	    $done = 1;
	}
    }
    close(IN);

    my $exitStatus = $?;
    if ($exitStatus != 0) {
	die "ERROR: zmdp solve exited with error value $exitStatus\n";
    }

    if (!defined $done) {
	die "ERROR: zmdp solve did not signal successful completion by printing 'done'\n";
    }

    my $elb = $params{expectedLB};
    my $eub = $params{expectedUB};
    my $tol = $params{testTolerance};
    if (defined $elb) {
	if (!defined $lb) {
	    die "ERROR: zmdp solve never printed bounds information during the run\n";
	}
	if (abs($lb - $elb) > $tol) {
	    die "ERROR: zmdp solve final lower bound value $lb differed from the expected value $elb by more than the testing tolerance $tol\n";
	}
    }
    if (defined $eub) {
	if (!defined $ub) {
	    die "ERROR: zmdp solve never printed bounds information during the run\n";
	}
	if (abs($ub - $eub) > $tol) {
	    die "ERROR: zmdp solve final upper bound value $ub differed from the expected value $eub by more than the testing tolerance $tol\n";
	}
    }

    my @outFiles = @{$params{"outFiles"}};
    for (@outFiles) {
	if (! -f $_) {
	    die "ERROR: zmdp solve did not generate the output file $_ as expected\n";
	}
    }

    print "passed\n";
}

sub testZmdpBenchmark {
    my %params = @_;

    my $cmd = $params{cmd};
    
    print "$cmd\n";

    open(IN, "$cmd 2>&1 |") or die "ERROR: couldn't run [$cmd]: $!\n";
    my $numpat = "(-?\\d+(\\.\\d*)?([eE][+-]\\d+)?)";
    my ($lb, $ub, $done);
    while (<IN>) {
	print;
	# could look at stdout output here, guess there's no need
    }
    close(IN);

    my $exitStatus = $?;
    if ($exitStatus != 0) {
	die "ERROR: zmdp benchmark exited with error value $exitStatus\n";
    }

    my @outFiles = @{$params{"outFiles"}};
    for (@outFiles) {
	if (! -f $_) {
	    die "ERROR: zmdp benchmark did not generate the output file $_ as expected\n";
	}
    }

    my $lastBounds = `tail -1 bounds.plot`;
    chop $lastBounds;
    my @bfields = split(/\s+/, $lastBounds);
    if ($#bfields+1 != 7) {
	die "ERROR: syntax error in bounds.plot, can't find final bounds values\n";
    }
    my ($lb, $ub) = ($bfields[1], $bfields[2]);

    my $elb = $params{expectedLB};
    my $eub = $params{expectedUB};
    my $tol = $params{testTolerance};
    if (defined $elb) {
	if (abs($lb - $elb) > $tol) {
	    die "ERROR: zmdp benchmark final lower bound value $lb differed from the expected value $elb by more than the testing tolerance $tol\n";
	}
    }
    if (defined $eub) {
	if (abs($ub - $eub) > $tol) {
	    die "ERROR: zmdp benchmark final upper bound value $ub differed from the expected value $eub by more than the testing tolerance $tol\n";
	}
    }

    print "passed\n";
}

sub testZmdpEvaluate {
    my %params = @_;

    my $cmd = $params{cmd};
    
    print "$cmd\n";

    open(IN, "$cmd 2>&1 |") or die "ERROR: couldn't run [$cmd]: $!\n";
    my $numpat = "(-?\\d+(\\.\\d*)?([eE][+-]\\d+)?)";
    my $mean;
    while (<IN>) {
	print;
	my $numpat = "(-?\\d+(\\.\\d*)?([eE][+-]\\d+)?)";
	if (/^REWARD_MEAN_CONF95MIN_CONF95MAX\s+$numpat\s+$numpat\s+$numpat/) {
	    $mean = $1;
	}
	# could look at stdout output here, guess there's no need
    }
    close(IN);

    my $exitStatus = $?;
    if ($exitStatus != 0) {
	die "ERROR: zmdp evaluate exited with error value $exitStatus\n";
    }

    my @outFiles = @{$params{"outFiles"}};
    for (@outFiles) {
	if (! -f $_) {
	    die "ERROR: zmdp evaluate did not generate the output file $_ as expected\n";
	}
    }

    if (!defined $mean) {
	die "ERROR: zmdp evaluate never printed the mean reward to stdout\n";
    }

    my $em = $params{expectedMean};
    my $tol = $params{testTolerance};
    if (defined $em) {
	if (abs($mean - $em) > $tol) {
	    die "ERROR: zmdp evaluate mean reward value $mean differed from the expected value $em by more than the testing tolerance $tol\n";
	}
    }

    print "passed\n";
}

print "$TEST_DESCRIPTION\n";

$OS_SYSNAME = `uname -s | perl -ple 'tr/A-Z/a-z/;'`;
chop $OS_SYSNAME;
$OS_RELEASE = `uname -r | perl -ple 's/\\..*\$//;'`;
chop $OS_RELEASE;
$OS = $OS_SYSNAME . $OS_RELEASE;

$zmdpSolve = "../../../bin/$OS/zmdp solve";
$zmdpBenchmark = "../../../bin/$OS/zmdp benchmark";
$zmdpEvaluate = "../../../bin/$OS/zmdp evaluate";
$mdpsDir = "../../mdps";
$pomdpsDir = "../../pomdpModels";

&dosys("rm -rf testTmp");
&dosys("mkdir -p testTmp");
print "cd testTmp\n";
chdir("testTmp") or die "ERROR: could not change directory to 'tmp': $!\n";

1;
