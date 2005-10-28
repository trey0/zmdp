#!/usr/bin/perl
#
# Copyright (c) 1996-2005, Carnegie Mellon University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * This code and derivatives may not be incorporated into commercial
#   products without specific prior written permission.
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of Carnegie Mellon University nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
