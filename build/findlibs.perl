#!/usr/bin/perl
###  findlibs.perl
###
###	Written by Mark Maimone, 1 April 1999   mark.maimone@jpl.nasa.gov
###
###	Parse linker switches to generate dependencies for library
###	inclusion.  This allows us to extend "makedepend"s capabilities
###	by embedding not only header dependencies, but also library
###	dependencies.

###  Usage:
###	perl findlibs.perl [switches]

$dirsep = "/";
$pathsep = ':';

#if ($ENV{"OS"} eq "Windows_NT") {
#   $dirsep = "\\\\";
#   $pathsep = ';';
#}

$bn = $0;
$bn =~ s:^.*$dirsep::;

##############################################################################
###  Check for command line arguments.  No arguments returns empty.

if ($#ARGV == -1) {
    exit 0;
}

$lpath = "";
$libnum = 0;

##############################################################################
###  Parse command line arguments

for ($file = 0; $file <= $#ARGV; $file++) {
    $_ = $ARGV[$file];
  FOO: {
      if (/^-L/) {
	  $_ =~ s/^-L//;
	  if ($lpath ne "") {
	      $lpath .= "$pathsep" . $_;
	  } else {
	      $lpath = $_;
	  }
	  last FOO;
      }
      if (/^-l/) {
	  $_ =~ s/^-l//;
	  $libnames[$libnum++] = $_;
      }
  }
}

##############################################################################
###  Add extra directories into the library search path

if ($ENV{"LPATH"} ne "") {
    if ($lpath eq "") {
	$lpath = $ENV{"LPATH"};
    } else {
	$lpath .= "$pathsep" . $ENV{"LPATH"};
    }
}

##############################################################################
###  Make any references to the current directory explicit with "."

$lpath =~ s/^$pathsep/.$pathsep/;
$lpath =~ s/$pathsep$/$pathsep./;
$lpath =~ s/$pathsep$pathsep/$pathsep.$pathsep/g;

@lpathdirs = split ($pathsep, $lpath);

##############################################################################
###  Iterate over each "-l" named on the command line.  If you found it in
###  the filesystem, print the pathname

for ($i = 0; $i <= $#libnames; $i++) {
    $ind = &first_index("lib" . $libnames[$i] . ".a");
    if ($ind >= 0) {
	print " $lpathdirs[$ind]$dirsep" . "lib$libnames[$i].a";
    } else {
	$ind = &first_index("lib" . $libnames[$i] . ".so");
	if ($ind >= 0) {
	    print " $lpathdirs[$ind]$dirsep" . "lib$libnames[$i].so";
	}
    }
}
print "\n";
exit 0;

##############################################################################
###  first_index -- returns the index of the first directory in global
###  @lpathdirs that contains the file named in the subroutine parameter

sub first_index {
    for ($j = 0; $j <= $#lpathdirs; $j++) {
	$testname = "$lpathdirs[$j]$dirsep$_[0]";
	if (-f $testname) {
	    return $j;
	}
    }
    return -1;
}
