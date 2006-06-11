/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-06-11 14:37:39 $
   
 @file    testLSModel.cc
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

#include <iostream>

#include "LSModel.h"

using namespace std;

void usage(void) {
  cerr <<
    "usage: testLSModel OPTIONS <foo.lifeSurvey>\n"
    "  -h or --help   Display this help\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int argi;
  char *modelFileName = 0;
  for (argi=1; argi < argc; argi++) {
    if (0 == strcmp("-h",argv[argi]) || 0 == strcmp("--help",argv[argi])) {
      usage();
    } else if (0 == modelFileName) {
      modelFileName = argv[argi];
    } else {
      cerr << "too many arguments" << endl;
      usage();
    }
  }
  if ( 0 == modelFileName ) {
    usage();
  }

  LSModel m;

  // read the map in
  m.readFromFile(modelFileName);

  // modify it a bit
  m.grid.setCell(0,0,2);
  m.grid.setCell(3,6,2);
  m.grid.setCell(17,0,2);
  m.grid.setCell(20,6,2);

  // write it back out to stdout
  m.writeToFile(stdout);
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
