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

sub usage {
    die "usage: findlibs.perl OPTIONS\n".
	"  -h or --help    Print this help.\n".
	"  -L<dir>         Add a search directory\n".
        "  -l<lib>         Add a file to search for\n".
        "\n".
        "(Other options are silently ignored)\n";
}

sub findLib {
    my $lib = shift;

    # prefer static libraries
    for (@LIBDIRS) {
	my $file = "$_/lib${lib}.a";
	if (-f $file) { return $file; }
    }
    # but also locate shared libraries if nothing else is available
    for (@LIBDIRS) {
	my $file = "$_/lib${lib}.so";
	if (-f $file) { return $file; }
    }
    return "";
}

our @LIBDIRS = ();
@libs = ();
while ($_ = shift @ARGV) {
    if ($_ eq "-h" or $_ eq "--help") {
	&usage();
    } elsif (/^-L(.*)/) {
	push @LIBDIRS, $1;
    } elsif (/^-l(.*)/) {
	push @libs, $1;
    } else {
	# ignore other options
    }
}
for (@libs) {
    $file = &findLib($_);
    if ($file ne "") {
	print " $file";
    }
}
print "\n";
