#!/usr/bin/perl
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
