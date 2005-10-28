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
