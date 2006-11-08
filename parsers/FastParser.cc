/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-11-08 16:40:50 $
  
 @file    FastParser.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "slaMatrixUtils.h"
#include "sla_cassandra.h"
#include "FastParser.h"

#define POMDP_READ_ERROR_EPS (1e-10)

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

/***************************************************************************
 * STATIC HELPER FUNCTIONS
 ***************************************************************************/

static void readVector(char *data, dvector& b, int numValues)
{
  int i;
  char *inp = data;
  char *tok;
  
  for (i=0; i < numValues; i++) {
    tok = strtok(inp," ");
    if (0 == tok) {
      cout << "ERROR: not enough entries in initial belief distribution"
	   << endl;
      exit(EXIT_FAILURE);
    }
    inp = 0;

    b(i) = atof(tok);
  }
}

static void trimTrailingWhiteSpace(char *s)
{
  int n = strlen(s);
  int i;
  for (i = n-1; i >= 0; i--) {
    if (!isspace(s[i])) break;
  }
  s[i+1] = '\0';
}

/***************************************************************************
 * POMDP FUNCTIONS
 ***************************************************************************/

void FastParser::readGenericDiscreteMDPFromFile(GenericDiscreteMDP& mdp,
						const std::string& fileName)
{
  readModelFromFile(mdp, fileName, /* expectPomdp = */ false);
  mdp.numStateDimensions = 1;
}

void FastParser::readPomdpFromFile(Pomdp& pomdp, const std::string& fileName)
{
  readModelFromFile(pomdp, fileName, /* expectPomdp = */ true);
  pomdp.numStateDimensions = pomdp.numStates;
}

void FastParser::readModelFromFile(CassandraModel& problem,
				   const std::string& fileName,
				   bool expectPomdp)
{
  ifstream in;

  timeval startTime, endTime;
  if (zmdpDebugLevelG >= 1) {
    cout << "reading problem (in fast mode) from " << fileName << endl;
    gettimeofday(&startTime,0);
  }

  in.open(fileName.c_str());
  if (!in) {
    cerr << "ERROR: couldn't open " << fileName << " for reading: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  readModelFromStream(problem, fileName, in, expectPomdp);

  in.close();

  if (zmdpDebugLevelG >= 1) {
    gettimeofday(&endTime,0);
    double numSeconds = (endTime.tv_sec - startTime.tv_sec)
      + 1e-6 * (endTime.tv_usec - startTime.tv_usec);
    cout << "[file reading took " << numSeconds << " seconds]" << endl;
  }
}

void FastParser::readModelFromStream(CassandraModel& p,
				     const std::string& fileName,
				     std::istream& in,
				     bool expectPomdp)
{
  char buf[1<<20];
  int lineNumber;
  char sbuf[512];
  bool inPreamble = true;
  char *data;

  dvector initialBeliefx;
  kmatrix Rx;
  std::vector<kmatrix> Tx, Ox;

  bool discountSet = false;
  bool valuesSet = false;
  bool numStatesSet = false;
  bool numActionsSet = false;
  bool numObservationsSet = false;

#define PM_PREFIX_MATCHES(X) \
  (0 == strncmp(buf,(X),strlen(X)))

  lineNumber = 1;
  while (!in.eof()) {
    in.getline(buf,sizeof(buf));
    if (in.fail() && !in.eof()) {
      cerr << "ERROR: " << fileName << ": line " << lineNumber << ": line too long for buffer"
	   << " (max length " << sizeof(buf) << ")" << endl;
      exit(EXIT_FAILURE);
    }

    if ('#' == buf[0]) continue;
    trimTrailingWhiteSpace(buf);
    if ('\0' == buf[0]) continue;
    
    if (inPreamble) {
      if (PM_PREFIX_MATCHES("discount:")) {
	if (1 != sscanf(buf,"discount: %lf", &p.discount)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in 'discount' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	discountSet = true;
      } else if (PM_PREFIX_MATCHES("values:")) {
	if (1 != sscanf(buf,"values: %s", sbuf)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in 'values' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	if (0 != strcmp(sbuf,"reward")) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": expected 'values: reward', other types not supported by fast parser"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	valuesSet = true;
      } else if (PM_PREFIX_MATCHES("actions:")) {
	if (1 != sscanf(buf,"actions: %d", &p.numActions)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in 'actions' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numActionsSet = true;
      } else if (PM_PREFIX_MATCHES("observations:")) {
	if (1 != sscanf(buf,"observations: %d", &p.numObservations)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in 'observations' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numObservationsSet = true;
      } else if (PM_PREFIX_MATCHES("states:")) {
	if (1 != sscanf(buf,"states: %d", &p.numStates)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in 'states' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numStatesSet = true;
      } else {
	// the statement is not one that is expected in the preamble,
	// check that we are ready to transition to parsing the body

#define FP_CHECK_SET(VAR, NAME) \
	if (!(VAR)) { \
	  cerr << "ERROR: " << fileName << ": line " << lineNumber \
	       << ": at end of preamble, no '" << (NAME) << "' statement found" << endl; \
	}

	FP_CHECK_SET(discountSet,   "discount");
	FP_CHECK_SET(valuesSet,     "values");
	FP_CHECK_SET(numStatesSet,  "states");
	FP_CHECK_SET(numActionsSet, "actions");
	if (expectPomdp) {
	  FP_CHECK_SET(numObservationsSet, "observations");
	} else {
	  p.numObservations = -1;
	}
	
	// initialize data structures
	initialBeliefx.resize(p.numStates);
	set_to_zero(initialBeliefx);
	Rx.resize(p.numStates, p.numActions);
	Tx.resize(p.numActions);
	if (expectPomdp) {
	  Ox.resize(p.numActions);
	}
	FOR (a, p.numActions) {
	  Tx[a].resize(p.numStates, p.numStates);
	  if (expectPomdp) {
	    Ox[a].resize(p.numStates, p.numObservations);
	  }
	}

	// henceforth expect body statements instead of preamble statements
	inPreamble = false;
      }
    }

    if (!inPreamble) {
      if (PM_PREFIX_MATCHES("start:")) {
	data = buf + strlen("start: ");
	readVector(data, initialBeliefx, p.numStates);
      } else if (PM_PREFIX_MATCHES("R:")) {
	int s, a;
	double reward;
	if (3 != sscanf(buf,"R: %d : %d : * : * %lf", &a, &s, &reward)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in R statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	kmatrix_set_entry(Rx, s, a, reward);
      } else if (PM_PREFIX_MATCHES("T:")) {
	int s, a, sp;
	double prob;
	if (4 != sscanf(buf,"T: %d : %d : %d %lf", &a, &s, &sp, &prob)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in T statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	kmatrix_set_entry(Tx[a], s, sp, prob);
      } else if (PM_PREFIX_MATCHES("O:")) {
	int s, a, o;
	double prob;
	if (4 != sscanf(buf,"O: %d : %d : %d %lf", &a, &s, &o, &prob)) {
	  cerr << "ERROR: " << fileName << ": line " << lineNumber
	       << ": syntax error in O statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	if (expectPomdp) {
	  kmatrix_set_entry(Ox[a], s, o, prob);
	}
      } else {
	cerr << "ERROR: " << fileName << ": line " << lineNumber
	     << ": got unexpected statement type while parsing body"
	     << endl;
	exit(EXIT_FAILURE);
      }
    }

    lineNumber++;
  }

  cvector checkTmp;
  cmatrix checkObs;

  // post-process
  copy(p.initialBelief, initialBeliefx);
  initialBeliefx.clear();

  copy(p.R, Rx);
  Rx.clear();

  p.Ttr.resize(p.numActions);
  p.O.resize(p.numActions);
  p.T.resize(p.numActions);
  FOR (a, p.numActions) {
    copy(p.T[a], Tx[a]);
    kmatrix_transpose_in_place(Tx[a]);
    copy(p.Ttr[a], Tx[a]);
    copy(p.O[a], Ox[a]);

#if 1
    // extra error checking
    kmatrix_transpose_in_place(Ox[a]);
    copy(checkObs, Ox[a]);
    FOR (s, p.numStates) {
      copy_from_column(checkTmp, p.Ttr[a], s);
      if (fabs(sum(checkTmp) - 1.0) > POMDP_READ_ERROR_EPS) {
	fprintf(stderr,
		"ERROR: %s: outgoing transition probabilities do not sum to 1 for:\n"
		"  state %d, action %d, transition sum = 1 + %g\n",
		fileName.c_str(), (int)s, (int)a, sum(checkTmp) - 1.0);
	exit(EXIT_FAILURE);
      }

      copy_from_column(checkTmp, checkObs, s);
      if (fabs(sum(checkTmp) - 1.0) > POMDP_READ_ERROR_EPS) {
	fprintf(stderr,
		"ERROR: %s: observation probabilities do not sum to 1 for:\n"
		"  state %d, action %d, observation sum = 1 + %g\n",
		fileName.c_str(), (int)s, (int)a, sum(checkTmp) - 1.0);
	exit(EXIT_FAILURE);
      }
    }
#endif

    // deallocate temporaries as we go along in case memory is tight
    Tx[a].clear();
    Ox[a].clear();
  }

#if 1
  // extra error checking
  if (fabs(sum(p.initialBelief) - 1.0) > POMDP_READ_ERROR_EPS) {
    fprintf(stderr,
	    "ERROR: %s: initial belief entries do not sum to 1:\n"
	    "  entry sum = 1 + %g\n",
	    fileName.c_str(), sum(p.initialBelief) - 1.0);
    exit(EXIT_FAILURE);
  }
#endif

  p.checkForTerminalStates();

  if (zmdpDebugLevelG >= 1) {
    p.debugDensity();
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
