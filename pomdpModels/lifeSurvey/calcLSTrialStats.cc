/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-09-21 15:27:14 $
   
 @file    calcLSTrialStats.cc
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
#include <getopt.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#include <iostream>

#include "zmdpCommonDefs.h"
#include "LifeSurvey.h"

using namespace std;
using namespace zmdp;

bool verboseG = false;

struct TraceEntry {
  int a;
  int o;
};

void readTrace(std::vector<TraceEntry>& result,
	       const char* fname)
{
  FILE* f = fopen(fname, "r");
  if (NULL == f) {
    fprintf(stderr, "ERROR: couldn't open %s for reading: %s\n",
	    fname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  char buf[256];
  TraceEntry e;
  int lnum = 0;
  result.clear();
  while (fgets(buf,sizeof(buf),f)) {
    lnum++;
    if (0 == strlen(buf) || '#' == buf[0]) continue;
    if (2 != sscanf(buf, "%d %d", &e.a, &e.o)) {
      fprintf(stderr, "%s:%d: syntax error, expected integers '<a> <o>' on each line\n",
	      fname, lnum);
      exit(EXIT_FAILURE);
    }
    result.push_back(e);
  }
  fclose(f);
}

void doit(const char* lifeSurveyFileName,
	  const char* targetListFileName,
	  const char* traceFileName)
{
  const double lookaheadCost = 5.0;
  printf("LOOKAHEAD COST=%lf\n", lookaheadCost);

  LSModel m;
  m.init(lifeSurveyFileName);
  const LSModelFile& f = m.mfile;

  std::vector<LSPos> targetList;
  LSModel::readTargetList(targetList, targetListFileName);

  std::vector<TraceEntry> trace;
  readTrace(trace, traceFileName);

  // initialize state and regionRewards
  LSPos pos(f.startX, f.startY);
  std::vector<int> regionRewards(f.regionPriors.size(), 0);
  int r = f.grid.getCell(pos);
  regionRewards[r] = 1;

  // replay actions and determine relevant rewards
  int step = 0;
  int numLooks = 0;
  int numIllegalActions = 0;
  int numGoodSamples = 0;
  int numSamples = 0;
  double netSum = 0.0;
  LSGrid debugGrid = f.grid;
  FOR_EACH (entryP, trace) {
    if (verboseG) {
      printf("%d x%dy%dr", step, pos.x, pos.y);
      FOR (r, regionRewards.size()) {
	printf("%d", regionRewards[r]);
      }
      printf(" a=%d", entryP->a);
      int o = entryP->o;
      if (27 != o) {
	printf(" o=%d (%d,%d,%d)", o, o/9, (o/3)%3, o%3);
      }
      printf("\n");
    }

    LSAction a(entryP->a);
    double reward = 0;
    double cost;
    double penalty = 0;
    if (LS_ACT_LOOK == a.type) {
      numLooks++;
      cost = lookaheadCost;
    } else {
      cost = a.useSample ? 5 : 1;

      LSPos nextPos = m.getNeighbor(pos, a.moveDirection);
      
      unsigned char cellType = f.grid.getCellBounded(nextPos);
      if (LS_OBSTACLE == cellType) {
	penalty = 100;
	if (verboseG) {
	  printf("  ILLEGAL ACTION!\n");
	}
	numIllegalActions++;
      } else {
	bool isTarget = LSModel::getInTargetList(nextPos, targetList);
	int oldRewardLevel = regionRewards[cellType];
	int newRewardLevel = isTarget ? (a.useSample ? 3 : 2) : 1;
	if (newRewardLevel > oldRewardLevel) {
	  reward = m.getReward(oldRewardLevel, newRewardLevel);
	  regionRewards[cellType] = newRewardLevel;
	}
	if (a.useSample) {
	  if (isTarget) {
	    if (verboseG) {
	      printf("  good sample!\n");
	    }
	    numGoodSamples++;
	    debugGrid.setCell(nextPos, debugGrid.getCell(nextPos)+13);
	  } else {
	    if (verboseG) {
	      printf("  bad sample!\n");
	    }
	    debugGrid.setCell(nextPos, 23);
	  }
	  numSamples++;
	} else {
	  debugGrid.setCell(nextPos, 25);
	}


	pos = nextPos;
      }
    }
    double net = reward - cost - penalty;
    netSum += net * pow(0.999, step);
		 
    if (verboseG) {
      printf("  reward=%lf cost=%lf penalty=%lf\n", reward, cost, penalty);
    }
    step++;
  }
  printf("\n");
  debugGrid.writeToFile(stdout, /* showCoords = */ true);
  printf("\n");

  printf("ACTIONS_LOOK_TOTAL %d %d\n", numLooks, (int)trace.size());
  printf("SAMPLES_GOOD_TOTAL %d %d\n", numGoodSamples, numSamples);
  printf("ILLEGAL_ACTIONS %d\n", numIllegalActions);
  printf("REWARD %.3lf\n", netSum);
}

void usage(const char* binaryName)
{
  cerr <<
    "usage: " << binaryName << " OPTIONS <foo.lifeSurvey> <foo.targetList> <foo.actions>\n"
    "  -h or --help    Display this help\n"
    "  -v or --verbose Print debug information\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  static char shortOptions[] = "hv";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"verbose",       0,NULL,'v'},
    {NULL,0,0,0}
  };

  while (1) {
    char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
    if (optchar == -1) break;

    switch (optchar) {
    case 'h': // help
      usage(argv[0]);
      break;
	
    case 'v': // verbose
      verboseG = true;
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
  if (3 != argc-optind) {
    cerr << "ERROR: wrong number of arguments (should be 2)" << endl << endl;
    usage(argv[0]);
  }

  const char* lifeSurveyFileName = argv[optind++];
  const char* targetListFileName = argv[optind++];
  const char* traceFileName = argv[optind++];

  doit(lifeSurveyFileName, targetListFileName, traceFileName);
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/08/04 22:33:38  trey
 * changed lookaheadCost to match modified LifeSurvey models
 *
 * Revision 1.2  2006/07/10 19:33:35  trey
 * moved some functions to LifeSurvey.cc
 *
 * Revision 1.1  2006/07/10 02:21:35  trey
 * initial check-in
 *
 *
 ***************************************************************************/
