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
