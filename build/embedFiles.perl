#!/usr/bin/perl
# $Id: embedFiles.perl,v 1.2 2005-10-28 02:24:42 trey Exp $
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

    #print "source file = $sourceFile\n";
    #print "output file = $outputFile\n";
    #print "cpp options = $cppOptions\n";

    my $cppCmd = "cpp $cppOptions -D__EMBEDDING_NOW__=1 $sourceFile";
    open(CPP_IN, "$cppCmd |") or die "ERROR: couldn't run \"$cppCmd\": $!\n";

    if (!$doDependencies) {
	open(OUT, "> $outputFile") or
	    die "ERROR: couldn't open $outputFile for writing: $!\n";

	print OUT << "EOF";
typedef struct {
  unsigned int size;
  char* data;
} embed_buffer_t;
EOF
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
