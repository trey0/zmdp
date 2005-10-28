#!/usr/bin/perl
#
# Copyright (c) 2002-2005, Trey Smith
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * The software may not be sold or incorporated into a commercial
#   product without specific prior written permission.
# * The above copyright notice and this permission notice shall be
#   included in all copies or substantial portions of the software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

######################################################################

use Cwd;

sub usage {
    die "usage: iterate.perl OPTIONS [dir1 dir2 ...] - command [arguments]\n".
	" -h or --help    Print this help\n";
}

$cmd = "";
@dirs = ();
while ($_ = shift @ARGV) {
    if (defined $pastDirs) {
	$cmd .= "$_ ";
    } else {
	if ($_ eq "-h" or $_ eq "--help") {
	    &usage();
	} elsif ($_ eq "-") {
	    $pastDirs = 1;
	} else {
	    push @dirs, $_;
	}
    }
}
$curdir = cwd;
if (! -d $curdir) {
    die "ERROR: can't get current working directory\n";
}
for (@dirs) {
    if (chdir $_) {
	$ret = system($cmd);
	if ($ret != 0) {
	    die "ERROR: can't execute $cmd in directory $_\n";
	}
	chdir $curdir or die "ERROR: can't cd back to $curdir\n";
    } else {
	print STDERR "WARNING: can't cd to $_: $!\n";
    }
}
