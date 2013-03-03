/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2007-04-08 22:48:04 $
  
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

void FastParser::readGenericDiscreteMDPFromFile(CassandraModel& mdp,
						const std::string& _fileName)
{
  mdp.fileName = _fileName;
  readModelFromFile(mdp, /* expectPomdp = */ false);
}

void FastParser::readPomdpFromFile(CassandraModel& pomdp, const std::string& _fileName)
{
  pomdp.fileName = _fileName;
  readModelFromFile(pomdp, /* expectPomdp = */ true);
}

void FastParser::readModelFromFile(CassandraModel& problem,
				   bool expectPomdp)
{
  ifstream in;

  timeval startTime, endTime;
  if (zmdpDebugLevelG >= 1) {
    cout << "reading problem (in fast mode) from " << problem.fileName << endl;
    gettimeofday(&startTime,0);
  }

  in.open(problem.fileName.c_str());
  if (!in) {
    cerr << "ERROR: couldn't open " << problem.fileName << " for reading: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  readModelFromStream(problem, in, expectPomdp);

  in.close();

  if (zmdpDebugLevelG >= 1) {
    gettimeofday(&endTime,0);
    double numSeconds = (endTime.tv_sec - startTime.tv_sec)
      + 1e-6 * (endTime.tv_usec - startTime.tv_usec);
    cout << "[file reading took " << numSeconds << " seconds]" << endl;
  }
}

void FastParser::readModelFromStream(CassandraModel& p,
				     std::istream& in,
				     bool expectPomdp)
{
  char buf[1<<20];
  int lineNumber;
  char sbuf[512];
  bool inPreamble = true;

  kmatrix Rx;
  std::vector<kmatrix> Tx, Ox;

  bool discountSet = false;
  bool valuesSet = false;
  bool numStatesSet = false;
  bool numActionsSet = false;
  bool numObservationsSet = false;
  bool startSet = false;

  const char* rFormat = (expectPomdp
			 ? "R: %d : %d : * : * %lf"
			 : "R: %d : %d : * %lf");

#define PM_PREFIX_MATCHES(X) \
  (0 == strncmp(buf,(X),strlen(X)))

  lineNumber = 1;
  while (!in.eof()) {
    in.getline(buf,sizeof(buf));
    if (in.fail() && !in.eof()) {
      cerr << "ERROR: " << p.fileName << ": line " << lineNumber << ": line too long for buffer"
	   << " (max length " << sizeof(buf) << ")" << endl;
      exit(EXIT_FAILURE);
    }

    if ('#' == buf[0]) continue;
    trimTrailingWhiteSpace(buf);
    if ('\0' == buf[0]) continue;
    
    if (inPreamble) {
      if (PM_PREFIX_MATCHES("discount:")) {
	if (1 != sscanf(buf,"discount: %lf", &p.discount)) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": syntax error in 'discount' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	discountSet = true;
      } else if (PM_PREFIX_MATCHES("values:")) {
	if (1 != sscanf(buf,"values: %s", sbuf)) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": syntax error in 'values' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	if (0 != strcmp(sbuf,"reward")) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": expected 'values: reward', other types not supported by fast parser"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	valuesSet = true;
      } else if (PM_PREFIX_MATCHES("actions:")) {
	if (1 != sscanf(buf,"actions: %d", &p.numActions)) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": syntax error in 'actions' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numActionsSet = true;
      } else if (PM_PREFIX_MATCHES("observations:")) {
	if (expectPomdp) {
	  if (1 != sscanf(buf,"observations: %d", &p.numObservations)) {
	    cerr << "ERROR: " << p.fileName << ": line " << lineNumber
		 << ": syntax error in 'observations' statement"
		 << endl;
	    exit(EXIT_FAILURE);
	  }
	  numObservationsSet = true;
	} else {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": got unexpected 'observations' statement in MDP"
	       << endl;
	  exit(EXIT_FAILURE);
	}
      } else if (PM_PREFIX_MATCHES("states:")) {
	if (1 != sscanf(buf,"states: %d", &p.numStates)) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": syntax error in 'states' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numStatesSet = true;
      } else if (PM_PREFIX_MATCHES("start:")) {
	if (!numStatesSet) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": got 'start' statement before 'states' statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	readStartVector(p, buf, expectPomdp);
	startSet = true;
      } else {
	// the statement is not one that is expected in the preamble,
	// check that we are ready to transition to parsing the body

#define FP_CHECK_SET(VAR, NAME) \
	if (!(VAR)) { \
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber \
	       << ": at end of preamble, no '" << (NAME) << "' statement found" << endl; \
	}

	FP_CHECK_SET(discountSet,   "discount");
	FP_CHECK_SET(valuesSet,     "values");
	FP_CHECK_SET(numStatesSet,  "states");
	FP_CHECK_SET(numActionsSet, "actions");
	FP_CHECK_SET(startSet,      "start");
	if (expectPomdp) {
	  FP_CHECK_SET(numObservationsSet, "observations");
	} else {
	  p.numObservations = -1;
	}
	
	// initialize data structures
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
      if (PM_PREFIX_MATCHES("R:")) {
	int s, a;
	double reward;
	if (3 != sscanf(buf, rFormat, &a, &s, &reward)) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": syntax error in R statement\n"
	       << "  (expected format is '" << rFormat << "')"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	kmatrix_set_entry(Rx, s, a, reward);
      } else if (PM_PREFIX_MATCHES("T:")) {
	int s, a, sp;
	double prob;
	if (4 != sscanf(buf,"T: %d : %d : %d %lf", &a, &s, &sp, &prob)) {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": syntax error in T statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	kmatrix_set_entry(Tx[a], s, sp, prob);
      } else if (PM_PREFIX_MATCHES("O:")) {
	if (expectPomdp) {
	  int s, a, o;
	  double prob;
	  if (4 != sscanf(buf,"O: %d : %d : %d %lf", &a, &s, &o, &prob)) {
	    cerr << "ERROR: " << p.fileName << ": line " << lineNumber
		 << ": syntax error in O statement"
		 << endl;
	    exit(EXIT_FAILURE);
	  }
	  kmatrix_set_entry(Ox[a], s, o, prob);
	} else {
	  cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	       << ": got unexpected 'O' statement in MDP"
	       << endl;
	  exit(EXIT_FAILURE);
	}
      } else {
	cerr << "ERROR: " << p.fileName << ": line " << lineNumber
	     << ": got unexpected statement type while parsing body"
	     << endl;
	exit(EXIT_FAILURE);
      }
    }

    lineNumber++;
  }

  // post-process
  copy(p.R, Rx);
  Rx.clear();

  p.T.resize(p.numActions);
  p.Ttr.resize(p.numActions);
  if (expectPomdp) {
    p.O.resize(p.numActions);
  }
  FOR (a, p.numActions) {
    copy(p.T[a], Tx[a]);
    kmatrix_transpose_in_place(Tx[a]);
    copy(p.Ttr[a], Tx[a]);

#if 1
    // extra error checking
    cvector checkTmp;
    FOR (s, p.numStates) {
      copy_from_column(checkTmp, p.Ttr[a], s);
      if (fabs(sum(checkTmp) - 1.0) > POMDP_READ_ERROR_EPS) {
	fprintf(stderr,
		"ERROR: %s: outgoing transition probabilities do not sum to 1 for:\n"
		"  state %d, action %d, transition sum = %.10lf\n",
		p.fileName.c_str(), (int)s, (int)a, sum(checkTmp));
	exit(EXIT_FAILURE);
      }
    }
#endif

    Tx[a].clear();

    if (expectPomdp) {
      copy(p.O[a], Ox[a]);

#if 1
      cmatrix checkObs;
      
      // extra error checking
      kmatrix_transpose_in_place(Ox[a]);
      copy(checkObs, Ox[a]);

      FOR (s, p.numStates) {
	copy_from_column(checkTmp, checkObs, s);
	if (fabs(sum(checkTmp) - 1.0) > POMDP_READ_ERROR_EPS) {
	  fprintf(stderr,
		  "ERROR: %s: observation probabilities do not sum to 1 for:\n"
		  "  state %d, action %d, observation sum = %.10lf\n",
		  p.fileName.c_str(), (int)s, (int)a, sum(checkTmp));
	  exit(EXIT_FAILURE);
	}
      }
#endif

      Ox[a].clear();
    }
  }

  p.checkForTerminalStates();

#if 1
  // extra error checking
  if (expectPomdp) {
    if (fabs(sum(p.initialBelief) - 1.0) > POMDP_READ_ERROR_EPS) {
      fprintf(stderr,
	      "ERROR: %s: initial belief entries do not sum to 1:\n"
	      "  entry sum = %.10lf\n",
	      p.fileName.c_str(), sum(p.initialBelief));
      exit(EXIT_FAILURE);
    }
  }
#endif

  if (zmdpDebugLevelG >= 1) {
    p.debugDensity();
  }
}

void FastParser::readStartVector(CassandraModel& p,
				 char *data,
				 bool expectPomdp)
{
  if (expectPomdp) {
    // POMDP case

    int i;
    char *tok;
    double val;

    p.initialBelief.resize(p.numStates);

    // consume 'start:' token at the beginning of the statement
    tok = strtok(data, " ");

    for (i=0; i < p.numStates; i++) {
      if (NULL != tok) {
	tok = strtok(NULL, " ");
      }
      if (NULL == tok) {
	if (0 == i) {
	  double startState = atof(tok);
	  if (startState != floor(startState)) {
	    cout << "ERROR: " << p.fileName
		 << ": POMDP 'start' statement must either contain a single integer "
		 << "specifying a known start state or a list of the initial probabilities of "
		 << "all states"
		 << endl;
	    exit(EXIT_FAILURE);
	  }
	  p.initialBelief.push_back((int)startState, 1.0);
	  return;
	} else {
	  cout << "ERROR: " << p.fileName
	       << ": POMDP 'start' statement must either contain a single integer "
	       << "specifying a known start state or a list of the initial probabilities of "
	       << "all states"
	       << endl;
	  exit(EXIT_FAILURE);
	}
      }
      
      val = atof(tok);
      if (val > SPARSE_EPS) {
	p.initialBelief.push_back(i, val);
      }
    }
  } else {
    // MDP case

    double x, y;
    int ret = sscanf(data, "start: %lf %lf", &x, &y);
    if ((ret != 1) || (x != floor(x))) {
      cout << "ERROR: " << p.fileName
	   << ": MDP 'start' statement must contain a single integer "
	   << "specifying a known start state" << endl;
      exit(EXIT_FAILURE);
    }

    p.initialState.resize(1);
    int startState;
    if (1 != sscanf(data, "start: %d", &startState)) {
      cout << "ERROR: " << p.fileName
	   << ": MDP 'start' statement must contain a single integer "
	   << "specifying a known start state" << endl;
      exit(EXIT_FAILURE);
    }
    p.initialState.push_back(0, startState);
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2007/01/15 17:23:34  trey
 * fix problem that was causing zeros to have entries in the sparse representation of initialBelief
 *
 * Revision 1.3  2006/11/09 21:11:33  trey
 * removed obsolete code referencing variable initialBeliefx
 *
 * Revision 1.2  2006/11/09 20:48:56  trey
 * fixed some MDP vs. POMDP issues
 *
 * Revision 1.1  2006/11/08 16:40:50  trey
 * initial check-in
 *
 *
 ***************************************************************************/
