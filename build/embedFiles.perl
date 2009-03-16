#!/usr/bin/perl
# $Id: embedFiles.perl,v 1.8 2007-03-05 08:56:18 trey Exp $
#
# Copyright (c) 1996-2005, Carnegie Mellon University. All rights reserved.
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

sub genEscapeTable {
    @ESCAPE_TABLE = ();
    for (0..255) {
	my $c = chr($_);
	$c =~ s/\\/\\\\/;
	$c =~ s/([^A-Za-z0-9\a\b\t\n\f\r\!\"\@\#\$\%\&\'\(\)\*\+\,\-\=\>\<\.\/\[\\\]\^\_\`\{\|\}\~\:\;\ ])
               /sprintf("\\%03o",ord($1))/ex;
	$c =~ s /([\a\b\t\n\f\r])
	        /"\\".substr("abtnvfr",ord($1)-ord("\a"),1)/ex;
	$c =~ s/\"/\\\"/;
	$ESCAPE_TABLE[$_] = $c;
    }
}

sub embedFile {
    my $varName = shift;
    my $fileToEmbed = shift;
    local(*OUT) = shift;

    #print "varName = $varName\n";
    #print "fileToEmbed = $fileToEmbed\n";

    open(FILE_TO_EMBED, "< $fileToEmbed") or
	 die "ERROR: couldn't open $fileToEmbed for reading: $!\n";

    my @fileStats = stat($fileToEmbed);
    my $fileSize = $fileStats[7];
    #print "fileSize = $fileSize\n";

    print OUT "embed_buffer_t $varName = {$fileSize,\n";
    while (<FILE_TO_EMBED>) {
	my $subst = join("", map { $ESCAPE_TABLE[$_] } unpack("C*",$_));
	#print "\"$subst\"\n";
	print OUT "\"$subst\"\n";
    }
    print OUT "};\n";

    close(FILE_TO_EMBED);
}

sub embedFiles {
    my $sourceFile = shift;
    my $outputFile = shift;
    my $doDependencies = shift;
    my @cppOptions = @_;

    $cppOptions = join(" ", map { "'$_'" } @cppOptions);

    my $ccVersion = `cc --version`;
    chop $ccVersion;
    if ($ccVersion =~ /cc \(GCC\) 3/) {
	# command that works with gcc 3.2.2 under Linux Fedora Core 2
	$cpp = "cpp -x c++";
    } elsif ($ccVersion =~ /apple-darwin/) {
	# command that works with Xcode 2.4 under Mac OS X 10.4.7
	$cpp = "g++ -E";
    } else {
	# default, may or may not work with other platforms
	$cpp = "cpp -x c++";
    }


    my $cppCmd = "$cpp $cppOptions -D__EMBEDDING_NOW__=1 $sourceFile";

    #print "source file = $sourceFile\n";
    #print "output file = $outputFile\n";
    #print "cpp command = [$cppCmd]\n";

    open(CPP_IN, "$cppCmd |") or die "ERROR: couldn't run \"$cppCmd\": $!\n";

    if (!$doDependencies) {
	open(OUT, "> $outputFile") or
	    die "ERROR: couldn't open $outputFile for writing: $!\n";
        &genEscapeTable();
    }

    my @filesToEmbed = ();
    while (<CPP_IN>) {
	next unless /%%BEGIN%% (.*) %%MIDDLE%% (.*) %%END%%/;
	my ($varName, $fileToEmbed) = ($1, $2);
	$fileToEmbed =~ s/^\"(.*)\"$/$1/;
	if ($doDependencies) {
	    push @filesToEmbed, $fileToEmbed;
	} else {
	    &embedFile($varName, $fileToEmbed, OUT);
	}
    }
    close(CPP_IN);
    if (!$doDependencies) {
	close(OUT);
    }

    return @filesToEmbed;
}

sub genDependencies {
    my $sourceFile = shift;
    my $outputFile = shift;
    my @cppOptions = @_;

    my @filesToEmbed = &embedFiles($sourceFile, $outputFile, 1, @cppOptions);

    my $depFile = $outputFile;
    $depFile =~ s:/([^/]*):$1:; # basename
    $depFile =~ s/(.*)\.(cc|cpp|c|C)/$1.d/;
    print "$outputFile obj/linux2/$depFile : $sourceFile ";
    print join(" ", @filesToEmbed), "\n";
}

sub usage {
    die
	"usage: embedFiles.perl OPTIONS -o outputFile.cc sourceFile.h\n".
	"  -h or --help     Print this help.\n".
	"  -o <file>        Output to specified file\n".
        "  -M               Just output dependencies to stdout\n".
	"  -I, -D, etc.     Passed onto cpp when parsing source file\n";
}

sub main {
    my $sourceFile;
    my $outputFile;
    my $doneWithOptions = '';
    my $doDependencies = '';
    my @cppOptions = ();
    while ($_ = shift @ARGV) {
	if (/^-/ && !$doneWithOptions) {
	    # option
	    if ($_ eq '-h' or $_ eq '--help') {
		usage();
	    } elsif ($_ eq '-o' or $_ eq '--output') {
		$_ = shift @ARGV;
		if (!defined $_) {
		    print STDERR "ERROR: -o without filename\n\n";
		    &usage();
		}
		$outputFile = $_;
	    } elsif ($_ eq '-M') {
		$doDependencies = 1;
	    } elsif ($_ eq '--') {
		$doneWithOptions = 1;
	    } else {
		push @cppOptions, $_;
	    }
	} else {
	    # argument
	    if (!defined $sourceFile) {
		$sourceFile = $_;
	    } else {
		print STDERR "ERROR: too many arguments\n\n";
		&usage();
	    }
	}
    }

    if (!defined $sourceFile) {
	print STDERR "ERROR: no source file specified\n\n";
	&usage();
    }
    if (!defined $outputFile) {
	print STDERR "ERROR: no output file specified\n\n";
	&usage();
    }

    if ($doDependencies) {
	&genDependencies($sourceFile, $outputFile, @cppOptions);
    } else {
	&embedFiles($sourceFile, $outputFile, $doDependencies, @cppOptions);
   }
}

&main();

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.7  2006/10/31 20:34:07  trey
# fixed embedding problem when using cpp from gcc 4.0.1, XCode 2.4 under Mac OS X 10.4.8
#
# Revision 1.6  2006/10/20 04:58:44  trey
# switched pre-processor command again to avoid warnings under linux
#
# Revision 1.5  2006/10/17 19:17:57  trey
# switched pre-processor invocation to use "g++ -E" rather than "cpp" so that e.g. "#include <iostream>" is processed properly
#
# Revision 1.4  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.3  2005/10/28 03:34:41  trey
# switched to simpler license
#
# Revision 1.2  2005/10/28 02:24:42  trey
# added copyright headers
#
# Revision 1.1.1.1  2004/11/09 16:18:57  trey
# imported hsvi into new repository
#
# Revision 1.2  2004/08/02 02:58:07  trey
# propagated change from atacama copy of build system
#
# Revision 1.4  2004/08/02 02:57:20  trey
# stopped escaping colon, semicolon, @ characters
#
# Revision 1.3  2004/05/12 22:22:03  trey
# made embedded rcl files more readable
#
# Revision 1.2  2004/04/21 20:53:53  trey
# fixed encoding to not turn spaces into escape sequences
#
# Revision 1.1  2004/04/19 17:53:38  trey
# initial check-in
#
#
#
