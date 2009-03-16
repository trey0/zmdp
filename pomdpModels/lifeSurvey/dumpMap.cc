/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-09-12 19:46:46 $
  
 @file    dumpMap.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License.  You may
 obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.

 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <queue>

#include "LifeSurvey.h"

using namespace std;
using namespace zmdp;

/**********************************************************************
 * HELPER FUNCTIONS
 **********************************************************************/

static bool endsWith(const std::string& s,
		     const std::string& suffix)
{
  if (s.size() < suffix.size()) return false;
  return (s.substr(s.size() - suffix.size()) == suffix);
}

static std::string replaceSuffix(const std::string& s,
				 const std::string& suffix,
				 const std::string& replacement)
{
  if (s.size() < suffix.size()) return s;
  if (s.substr(s.size() - suffix.size()) != suffix) return s;

  std::string ret = s;
  ret.replace(ret.size() - suffix.size(), suffix.size(), replacement);
  return ret;
}

/**********************************************************************
 * MAIN
 **********************************************************************/

void usage(const char* argv0) {
  cerr <<
    "usage: " << argv0 << " OPTIONS <my.lifeSurvey> [my.mapDump]\n"
    "  -h or --help         Display this help\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  static char shortOptions[] = "hft:";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {NULL,0,0,0}
  };

  while (1) {
    char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
    if (optchar == -1) break;

    switch (optchar) {
    case 'h': // help
      usage(argv[0]);
      break;

    case '?': // unknown option
    case ':': // option with missing parameter
      // getopt() prints an informative error message
      cerr << endl;
      usage(argv[0]);
      break;
    default:
      abort(); // never reach this point
    }
  }
  if (! (1 <= argc-optind && argc-optind <= 2)) {
    cerr << "ERROR: wrong number of arguments (should be 1-2)" << endl << endl;
    usage(argv[0]);
  }

  const char* modelFileName = argv[optind++];
  std::string mapDumpFileName;
  if (1 == argc-optind) {
    mapDumpFileName = argv[optind++];
  } else {
    // infer pomdp file name
    if (endsWith(modelFileName, ".lifeSurvey")) {
      mapDumpFileName = replaceSuffix(modelFileName, ".lifeSurvey", ".mapDump");
    } else {
      fprintf(stderr, "ERROR: model filename %s does not end in '.lifeSurvey', can't infer the pomdp filename; you must specify it\n",
	      modelFileName);
      exit(EXIT_FAILURE);
    }
  }

  LSModel m;
  m.init(modelFileName);

  FILE* mapDumpFile = fopen(mapDumpFileName.c_str(), "w");
  if (NULL == mapDumpFile) {
    fprintf(stderr, "ERROR: couldn't open '%s' for writing: %s\n",
	    mapDumpFileName.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  // FIX fill me in
  LSGrid& g = m.mfile.grid;
  for (unsigned int y=0; y < g.height; y++) {
    for (unsigned int x=0; x < g.width; x++) {
      unsigned char c = g.getCell(LSPos(x,y));
      if (LS_OBSTACLE != c) {
	fprintf(mapDumpFile, "%d %d %d\n", x, y, c);
      }
    }
  }
  fclose(mapDumpFile);

  printf("done writing %s\n", mapDumpFileName.c_str());
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
