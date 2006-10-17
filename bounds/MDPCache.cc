/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-10-17 19:16:39 $
   
 @file    MDPCache.cc
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "MDPCache.h"
#include "MatrixUtils.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

void MDPNodeHashLogger::logToFile(FILE* logFile, MDPHash* nodes)
{
  EXT_NAMESPACE::hash<std::string> h;

  FOR_EACH (pr, *nodes) {
    fprintf(logFile, "%010u\n", (unsigned int) h(pr->first));
  }
}

void MDPNodeHashLogger::logToFile(const std::string& logFileName, MDPHash* nodes)
{
  FILE* logFile = fopen(logFileName.c_str(), "w");
  if (!logFile) {
    fprintf(stderr, "ERROR: couldn't open %s for reading: %s\n",
	    logFileName.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  logToFile(logFile, nodes);
  fclose(logFile);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
