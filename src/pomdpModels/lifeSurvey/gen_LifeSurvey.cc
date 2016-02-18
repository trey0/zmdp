/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2006-08-04 22:34:42 $
  
 @file    gen_LifeSurvey.cc
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
    "usage: " << argv0 << " OPTIONS <my.lifeSurvey> [my.pomdp]\n"
    "  -h or --help         Display this help\n"
    "  -f or --full         Use verbose identifiers in output model instead of numbers\n"
    "                        (useful only for debugging -- full identifiers are not\n"
    "                         compatible with the fast parser 'zmdp solve -f')\n"
    "  -t or --target-list  Specify known list of targets\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  static char shortOptions[] = "hft:";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"full",          0,NULL,'f'},
    {"target-list",   1,NULL,'t'},
    {NULL,0,0,0}
  };

  bool fullIdentifiers = false;
  const char* targetListFileName = NULL;

  while (1) {
    char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
    if (optchar == -1) break;

    switch (optchar) {
    case 'h': // help
      usage(argv[0]);
      break;

    case 'f': // full
      fullIdentifiers = true;
      break;

    case 't': // target-list
      targetListFileName = optarg;
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
  std::string pomdpFileName;
  if (1 == argc-optind) {
    pomdpFileName = argv[optind++];
  } else {
    // infer pomdp file name
    if (endsWith(modelFileName, ".lifeSurvey")) {
      pomdpFileName = replaceSuffix(modelFileName, ".lifeSurvey", ".pomdp");
    } else {
      fprintf(stderr, "ERROR: model filename %s does not end in '.lifeSurvey', can't infer the pomdp filename; you must specify it\n",
	      modelFileName);
      exit(EXIT_FAILURE);
    }
  }

  LSModel m;
  m.init(modelFileName);
  if (NULL != targetListFileName) {
    m.setTargetList(targetListFileName);
  }

  FILE* pomdpFile = fopen(pomdpFileName.c_str(), "w");
  if (NULL == pomdpFile) {
    fprintf(stderr, "ERROR: couldn't open '%s' for writing: %s\n",
	    pomdpFileName.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  m.writeToFile(pomdpFile, fullIdentifiers);

  printf("done writing %s\n", pomdpFileName.c_str());
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2006/07/10 19:33:08  trey
 * added ability to generate a pomdp corresponding to a particular LS target layout, for evaluation purposes
 *
 * Revision 1.6  2006/06/27 16:04:40  trey
 * refactored so outside code can access the LifeSurvey model using -lzmdpLifeSurvey
 *
 * Revision 1.6  2006/06/26 21:32:49  trey
 * moved most functions into LifeSurvey.cc
 *
 *
 ***************************************************************************/
