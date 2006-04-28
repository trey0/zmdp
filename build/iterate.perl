#!/usr/bin/perl
#
# Copyright (c) 2002-2005, Trey Smith. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You may
# obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

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
