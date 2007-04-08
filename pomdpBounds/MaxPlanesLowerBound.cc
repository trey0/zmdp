/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.27 $  $Author: trey $  $Date: 2007-04-08 22:54:09 $
   
 @file    MaxPlanesLowerBound.cc
 @brief   No brief

 Copyright (c) 2002-2006, Trey Smith. All rights reserved.

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

//#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "MaxPlanesLowerBound.h"
#include "BlindLBInitializer.h"

#define PRUNE_PLANES_INCREMENT (10)
#define PRUNE_PLANES_FACTOR (1.1)

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

struct MaxPlanesData {
  LBPlane* bestPlane;
  int lastSetPlaneNumBackups;

  MaxPlanesData(void) :
    bestPlane(NULL),
    lastSetPlaneNumBackups(-1)
  {}
};

/**********************************************************************
 * LOCAL HELPER FUNCTIONS
 **********************************************************************/

static std::string stripWhiteSpace(const std::string& s)
{
  string::size_type p1, p2;
  p1 = s.find_first_not_of(" \t");
  if (string::npos == p1) {
    return s;
  } else {
    p2 = s.find_last_not_of(" \t")+1;
    return s.substr(p1, p2-p1);
  }
}

static bool dominates(const LBPlane* a, const LBPlane* b, bool useMaxPlanesMasking)
{
  if (useMaxPlanesMasking) {
    return mask_dominates(a->alpha, b->alpha,
			  ZMDP_BOUNDS_PRUNE_EPS,
			  a->mask, b->mask);
  } else {
    return dominates(a->alpha, b->alpha, ZMDP_BOUNDS_PRUNE_EPS);
  }
}

/**********************************************************************
 * LBPLANE
 **********************************************************************/

LBPlane::LBPlane(void) {}


LBPlane::LBPlane(const alpha_vector& _alpha, int _action, const sla::mvector& _mask) :
  alpha(_alpha),
  action(_action),
  mask(_mask)
{}  

void LBPlane::write(std::ostream& out, bool useMaxPlanesMasking) const
{
  out << "    {" << endl;
  out << "      action => " << action << "," << endl;

  if (useMaxPlanesMasking) {
    out << "      numEntries => " << mask.filled() << "," << endl;
  } else {
    out << "      numEntries => " << alpha.size() << "," << endl;
  }

  out << "      entries => [" << endl;

  if (useMaxPlanesMasking) {
    bool firstEntry = true;
    FOR_CV (mask) {
      if (!firstEntry) {
	out << "," << endl;
      }
      int i = CV_INDEX(mask);
      out << "        " << i << ", " << alpha(i) << "";
      firstEntry = false;
    }
    out << endl;
  } else {
    int n = alpha.size();
    FOR (i, n-1) {
      out << "        " << i << ", " << alpha(i) << "," << endl;
    }
    out << "        " << (n-1) << ", " << alpha(n-1) << endl;
  }

  out << "      ]" << endl;
  out << "    }";
}

/**********************************************************************
 * MAX PLANES LOWER BOUND
 **********************************************************************/

MaxPlanesLowerBound::MaxPlanesLowerBound(const MDP* _pomdp,
					 const ZMDPConfig* _config) :
  pomdp((const Pomdp*) _pomdp),
  config(_config),
  core(NULL),
  initialized(false)
{
  lastPruneNumPlanes = 0;
  lastPruneNumBackups = -1;
  useMaxPlanesMasking = config->getBool("useMaxPlanesMasking");
  useMaxPlanesSupportList = config->getBool("useMaxPlanesSupportList");
  useMaxPlanesCache = config->getBool("useMaxPlanesCache");
  useMaxPlanesExtraPruning = config->getBool("useMaxPlanesExtraPruning");

  if (useMaxPlanesSupportList) {
    supportList.resize(pomdp->getBeliefSize());
  }
}

MaxPlanesLowerBound::~MaxPlanesLowerBound(void)
{
  FOR_EACH (planeP, planes) {
    delete *planeP;
  }
}

void MaxPlanesLowerBound::initialize(double targetPrecision)
{
  if (initialized) return;

  BlindLBInitializer blb(pomdp, this);
  blb.initialize(targetPrecision);

  if (useMaxPlanesCache) {
    // planes from initialization should have their 'age' set appropriately
    FOR_EACH (planeP, planes) {
      (*planeP)->numBackupsAtCreation = 0;
    }
  }
  initialized = true;
}

double MaxPlanesLowerBound::getValue(const belief_vector& b, const MDPNode* cn) const
{
  double v = inner_prod(getBestLBPlaneConst(b).alpha, b);
  return v;
}

// lower bound on long-term reward for taking action a (alpha vector)
void MaxPlanesLowerBound::getNewLBPlaneQ(LBPlane& result, MDPNode& cn, int a)
{
  alpha_vector betaA(pomdp->getBeliefSize());
  const alpha_vector* betaAO;
  alpha_vector tmp, tmp2, tmp3;

  set_to_zero(betaA);
  
  bool defaultIsSet = false;
  const alpha_vector* defaultBetaAO = NULL;
  const MDPQEntry& Qa = cn.Q[a];
  alpha_vector maskedBeta;
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      betaAO = &getPlaneForNode(*e->nextState).alpha;
      if (useMaxPlanesMasking) {
	// occasionally, even if the nextState belief is sparse, the
	// corresponding betaAO is one of the dense alpha vectors produced
	// by the initial blind-policy heuristic.  in that case, we can
	// speed up the matrix multiplication below by masking betaAO with
	// the nextState belief (so it becomes much sparser).
	if (betaAO->filled() > cn.s.filled()) {
	  mask_copy(maskedBeta, *betaAO, e->nextState->s);
	  betaAO = &maskedBeta;
	}
      }
    } else {
      // impossible to see this observation, so it doesn't make sense to
      // pick the alpha that optimizes for the next belief.  plug in a
      // default alpha (arbitrarily chosen to optimize for cn.s).
      if (!defaultIsSet) {
	defaultBetaAO = &getPlaneForNode(cn).alpha;
	defaultIsSet = true;
      }
      betaAO = defaultBetaAO;
    }

    emult_column( tmp, pomdp->O[a], o, *betaAO );
    if (useMaxPlanesMasking) {
      mult( tmp3, pomdp->T[a], tmp );
      mask_copy( tmp2, tmp3, cn.s );
    } else {
      mult( tmp2, tmp, pomdp->Ttr[a] );
    }
    betaA += tmp2;
  }

  alpha_vector Rxa;
  if (useMaxPlanesMasking) {
    copy_from_column( tmp, pomdp->R, a );
    mask_copy( Rxa, tmp, cn.s );
  } else {
    copy_from_column( Rxa, pomdp->R, a );
  }
  betaA *= pomdp->getDiscount();
  betaA += Rxa;
  
  result.alpha = betaA;
  result.action = a;
  if (useMaxPlanesMasking) {
    result.mask = cn.s;
  }
  result.numBackupsAtCreation = core->numBackups;
}

void MaxPlanesLowerBound::getNewLBPlane(LBPlane& result, MDPNode& cn)
{
  timeval startTime;
  if (zmdpDebugLevelG >= 1) {
    startTime = getTime();
  }

  double val, maxVal = -99e+20;
  LBPlane betaA;
 
  FOR (a, cn.getNumActions()) {
    getNewLBPlaneQ(betaA, cn, a);
    val = inner_prod(betaA.alpha, cn.s);
    cn.Q[a].lbVal = val;
    if (val > maxVal) {
      maxVal = val;
      result = betaA;
    }
  }
  if (zmdpDebugLevelG >= 1) {
    cout << "** newLowerBound: elapsed time = "
	 << timevalToSeconds(getTime() - startTime)
	 << endl;
  }
}

void MaxPlanesLowerBound::initNodeBound(MDPNode& cn)
{
  if (useMaxPlanesCache) {
    MaxPlanesData* bdata = new MaxPlanesData();
    bdata->bestPlane = NULL;
    cn.boundsData = bdata;
  }

  // note: this should also do the right thing if cn is terminal;
  // at least one of the planes from initialization should give an
  // lbVal of 0 for cn in that case.
  cn.lbVal = -99e+20;
  setPlaneForNode(cn, &getBestLBPlane(cn.s));
}

void MaxPlanesLowerBound::update(MDPNode& cn)
{
  LBPlane* newPlane = new LBPlane();
  getNewLBPlane(*newPlane, cn);

  setPlaneForNode(cn, newPlane);

  addLBPlane(newPlane);
  maybePrune(core->numBackups);
}

int MaxPlanesLowerBound::chooseAction(const state_vector& b)
{
  return getBestLBPlaneConst(b).action; 
}

void MaxPlanesLowerBound::setPlaneForNode(MDPNode& cn, LBPlane* newPlane)
{
  double newLB = inner_prod(newPlane->alpha, cn.s);
  if (useMaxPlanesCache) {
    if (newLB < cn.lbVal - ZMDP_BOUNDS_PRUNE_EPS) {
#if 0
      // debug
      LBPlane* oldPlane = ((MaxPlanesLowerBoundData*) cn.boundsData)->bestPlane;
      printf("setPlaneForNode: ignoring new plane: newLB=%lf cn.lbVal=%lf diff=%lg ip=%lf\n",
	     newLB, cn.lbVal, (newLB-cn.lbVal), inner_prod(oldPlane->alpha, cn.s));
      LBPlane* newPlane = &lowerBound->getBestLBPlane(cn.s);
      printf("setPlaneForNode: newPlane=%p oldPlane=%p\n",
	     newPlane, oldPlane);
#endif
      return;
    }
    MaxPlanesData* bdata = (MaxPlanesData*) cn.boundsData;
    LBPlane* oldPlane = bdata->bestPlane;
    if (NULL != oldPlane) {
      // remove &bdata->bestPlane from oldPlane->backPointers
      std::list<LBPlane**>& backPointers = oldPlane->backPointers;
      typeof(backPointers.begin()) eraseList =
	std::remove(backPointers.begin(), backPointers.end(), &bdata->bestPlane);
      backPointers.erase(eraseList);
    }
    
    cn.lbVal = newLB;
    bdata->bestPlane = newPlane;
    bdata->lastSetPlaneNumBackups = core->numBackups;
    newPlane->backPointers.push_back(&bdata->bestPlane);
  } else {
    cn.lbVal = newLB;
  }
}

const LBPlane& MaxPlanesLowerBound::getPlaneForNode(MDPNode& cn)
{
  if (useMaxPlanesCache) {
#if 0
    // old version, did more harm than good
    int n = ((MaxPlanesData*) cn.boundsData)->lastSetPlaneNumBackups;
    if (numBackups > (int) std::max((double) (n + CB_CACHE_INCREMENT), n * CB_CACHE_FACTOR)) {
      LBPlane& newPlane = lowerBound->getBestLBPlane(cn.s);
      setPlaneForNode(cn, &newPlane);
      return newPlane;
    } else {
      return *((MaxPlanesData*) cn.boundsData)->bestPlane;
    }
#endif
    MaxPlanesData* bdata = (MaxPlanesData *) cn.boundsData;
    return getBestLBPlaneWithCache(cn.s, bdata->bestPlane,
				   bdata->lastSetPlaneNumBackups);
  } else {
    return getBestLBPlane(cn.s);
  }
}

// return the alpha such that alpha * b has the highest value
const LBPlane& MaxPlanesLowerBound::getBestLBPlaneConst(const belief_vector& b) const
{
  const PlaneSet* planesToCheck;
  if (useMaxPlanesSupportList) {
    planesToCheck = &supportList[b.data[0].index];
  } else {
    planesToCheck = &planes;
  }

  double val, maxval = -99e+20;
  const LBPlane* ret = NULL;
  FOR_EACH (pr, *planesToCheck) {
    const LBPlane* al = *pr;
    if (useMaxPlanesMasking) {
      if (!mask_subset( b, al->mask )) continue;
    }
    val = inner_prod( al->alpha, b );
    if (val > maxval) {
      maxval = val;
      ret = al;
    }
  }

  assert(NULL != ret);
  return *ret;
}

LBPlane& MaxPlanesLowerBound::getBestLBPlane(const belief_vector& b)
{
  // cast from 'const LBPlane&' to LBPlane&
  return (LBPlane&) getBestLBPlaneConst(b);
}

// return the alpha such that alpha * b has the highest value
LBPlane& MaxPlanesLowerBound::getBestLBPlaneWithCache(const belief_vector& b,
						      LBPlane* currPlane,
						      int lastSetPlaneNumBackups)
{
  const PlaneSet* planesToCheck;
  if (useMaxPlanesSupportList) {
    planesToCheck = &supportList[b.data[0].index];
  } else {
    planesToCheck = &planes;
  }

  double val;
  LBPlane* ret = currPlane;
  double maxval = inner_prod(currPlane->alpha, b);

  FOR_EACH (pr, *planesToCheck) {
    LBPlane* al = *pr;
    if (al->numBackupsAtCreation < lastSetPlaneNumBackups) continue;
    if (useMaxPlanesMasking) {
      if (!mask_subset( b, al->mask )) continue;
    }
    val = inner_prod( al->alpha, b );
    if (val > maxval) {
      maxval = val;
      ret = al;
    }
  }
  return *ret;
}

void MaxPlanesLowerBound::addLBPlane(LBPlane* av)
{
  planes.push_back(av);

  if (useMaxPlanesSupportList) {
    // add new plane to supportList
    FOR_EACH (ai, av->mask.data) {
      supportList[ai->index].push_back(av);
    }
  }
}

void MaxPlanesLowerBound::prunePlanes(int numBackups)
{
  int oldNum = -1;
  int numRefCountDeletions = 0;
  if (zmdpDebugLevelG >= 1) {
    oldNum = planes.size();
  }
  typeof(planes.begin()) candidateP, memberP;

  candidateP = planes.begin();
  while (candidateP != planes.end()) {
    LBPlane* candidate = *candidateP;
    if (useMaxPlanesExtraPruning) {
      if (candidate->backPointers.empty()) {
	deleteAndForward(candidate, NULL);
	candidateP = eraseElement(planes, candidateP);
	if (zmdpDebugLevelG >= 1) {
	  numRefCountDeletions++;
	}
	continue;
      }
    }
    memberP = planes.begin();
    while (memberP != candidateP) {
      LBPlane* member = *memberP;
      if (candidate->numBackupsAtCreation <= lastPruneNumBackups
	  && member->numBackupsAtCreation <= lastPruneNumBackups) {
	// candidate and member were compared the last time we pruned
	// and neither dominates the other; leave them both in
      } else if (dominates(candidate, member, useMaxPlanesMasking)) {
	// memberP is pruned
	deleteAndForward(member, candidate);
	memberP = eraseElement(planes, memberP);
	continue;
      } else if (dominates(member, candidate, useMaxPlanesMasking)) {
	// candidate is pruned
	deleteAndForward(candidate, member);
	candidateP = eraseElement(planes, candidateP);
	// as a heuristic, move the winning member to the front of planes
	eraseElement(planes, memberP);
	planes.push_front(member);
	goto nextCandidate;
      }
      memberP++;
    }
    candidateP++;
  nextCandidate: ;
  }

  if (zmdpDebugLevelG >= 1) {
    cout << "... pruned # planes from " << oldNum << " down to " << planes.size() << endl;
    if (useMaxPlanesExtraPruning) {
      printf("[lower bound] refCount was used for %d of %d deletions\n",
	     numRefCountDeletions, (int) (oldNum - planes.size()));
    }
  }
  lastPruneNumPlanes = planes.size();
  lastPruneNumBackups = numBackups;
}

// prune points and planes if the number has grown significantly
// since the last check
void MaxPlanesLowerBound::maybePrune(int numBackups)
{
  unsigned int nextPruneNumPlanes = max(lastPruneNumPlanes + PRUNE_PLANES_INCREMENT,
					(int) (lastPruneNumPlanes * PRUNE_PLANES_FACTOR));
  if (planes.size() > nextPruneNumPlanes) {
    prunePlanes(numBackups);
  }
}

void MaxPlanesLowerBound::deleteAndForward(LBPlane* victim, LBPlane* dominator)
{
  if (useMaxPlanesSupportList) {
    // remove victim from supportList
    FOR_EACH (ai, victim->mask.data) {
      PlaneSet& pi = supportList[ai->index];
      FOR_EACH (eltP, pi) {
	if (victim == *eltP) {
	  eraseElement(pi, eltP);
	  break;
	}
      }
    }
  }
  if (useMaxPlanesCache) {
    // forward backPointers from victim to dominator
    FOR_EACH (bpP, victim->backPointers) {
      LBPlane** bp = *bpP;
      *bp = dominator;
      dominator->backPointers.push_back(bp);
    }
  }

  delete victim;
}

void MaxPlanesLowerBound::writeToFile(const std::string& outFileName) const
{
  ofstream out(outFileName.c_str());
  if (!out) {
    cerr << "ERROR: MaxPlanesLowerBound::writeToFile: couldn't open " << outFileName
	 << " for writing: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  out <<
"# This file is a POMDP policy, represented as a set of \"lower bound\n"
"# planes\", each of which consists of an alpha vector and a corresponding\n"
"# action.  Given a particular belief b, this information can be used to\n"
"# answer two queries of interest:\n"
"#\n"
"#   1. What is a lower bound on the expected long-term reward starting\n"
"#        from belief b?\n"
"#   2. What is an action that achieves that expected reward lower bound?\n"
"#\n"
"# Each lower bound plane is only defined over a subset of the belief\n"
"# simplex--it is defined for those beliefs b such that the non-zero\n"
"# entries of b are a subset of the entries present in the plane's alpha\n"
"# vector.  If this condition holds we say the plane is 'applicable' to b.\n"
"#\n"
"# Given a belief b, both of the queries above can be answered by the\n"
"# following process: first, throw out all the planes that are not\n"
"# applicable to b.  Then, for each of the remaining planes, take the inner\n"
"# product of the plane's alpha vector with b.  The highest inner product\n"
"# value is the expected long-term reward lower bound, and the action label\n"
"# for that plane is the action that achieves the bound.\n"
"\n"
    ;
  out << "{" << endl;
  out << "  policyType => \"MaxPlanesLowerBound\"," << endl;
  out << "  numPlanes => " << planes.size() << "," << endl;
  out << "  planes => [" << endl;

  PlaneSet::const_iterator pi = planes.begin();
  FOR (i, planes.size()-1) {
    (*pi)->write(out, useMaxPlanesMasking);
    out << "," << endl;
    pi++;
  }
  if (planes.size() > 0) {
    (*pi)->write(out, useMaxPlanesMasking);
  }
  out << endl;

  out << "  ]" << endl;
  out << "}" << endl;

  out.close();
}

void MaxPlanesLowerBound::readFromFile(const std::string& inFileName)
{
  ifstream inFile(inFileName.c_str());
  if (!inFile) {
    cerr << "ERROR: couldn't open " << inFileName << " for reading: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  
  char buf[1024];
  std::string s;
  LBPlane plane;
  int entryIndex;
  double entryVal;
  int parseState = 0;
  int lnum = 0;
  while (!inFile.eof()) {
    inFile.getline(buf, sizeof(buf));
    lnum++;

    // strip whitespace, ignore empty lines and comments
    s = stripWhiteSpace(buf);
    if (0 == s.size()) continue;
    if ('#' == s[0]) continue;

    // ignore these types of lines because they contain redundant information
    if (s == "{") continue;
    if (s == "}" or s == "},") continue;
    if (s == "[") continue;
    if (s == "]" or s == "],") continue;
    if (string::npos != s.find("numPlanes")) continue;
    if (string::npos != s.find("planes")) continue;
    if (string::npos != s.find("numEntries")) continue;
    if (string::npos != s.find("entries")) continue;

  parseLineAgain:
    switch (parseState) {
    case 0:
      /* at the beginning of the file, check that this is the right type of policy */
      if (string::npos != s.find("policyType")
	  && string::npos != s.find("MaxPlanesLowerBound")) {
	parseState = 1;
      } else {
	fprintf(stderr, "ERROR: %s: line %d: expected 'policyType => \"MaxPlanesLowerBound\"'\n",
		inFileName.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
      break;
    case 1:
      /* at the start of a plane, read the action */
      if (string::npos != s.find("action")
	  && (1 == sscanf(s.c_str(), "action => %d", &plane.action))) {
	plane.alpha.resize(pomdp->numStates);
	parseState = 2;
      } else {
	fprintf(stderr, "ERROR: %s: line %d: expected 'action => <int>'\n",
		inFileName.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
      break;
    case 2:
      if (string::npos != s.find("action")) {
	/* finding the 'action' keyword indicates we are at the start of
	   another plane; finish up the plane we were working on and
	   reparse the current line in parse state 1 */
	mask_set_to_one(plane.mask, plane.alpha);
	addLBPlane(new LBPlane(plane));
	parseState = 1;
	goto parseLineAgain;
      } else if (2 == sscanf(s.c_str(), "%d, %lf", &entryIndex, &entryVal)) {
	/* push another entry into the plane */
	plane.alpha.push_back(entryIndex, entryVal);
      } else {
	printf("s=[%s]\n", s.c_str());
	fprintf(stderr, "ERROR: %s: line %d: expected entry '<int>, <double>'\n",
		inFileName.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
      break;
    default:
      assert(0); // never reach this point
    }
  }
  
  /* reached EOF, finish up the last plane */
  if (2 == parseState) {
    mask_set_to_one(plane.mask, plane.alpha);
    addLBPlane(new LBPlane(plane));
  }

  inFile.close();

  // the set of planes should have been pruned before it was written out
  lastPruneNumPlanes = planes.size();
  lastPruneNumBackups = -1;

  initialized = true;
}

void MaxPlanesLowerBound::readFromCassandraAlphaFile(const std::string& inFileName)
{
  ifstream inFile(inFileName.c_str());
  if (!inFile) {
    cerr << "ERROR: couldn't open " << inFileName << " for reading: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  
  double val;
  LBPlane plane;

  // initialize elements that are the same for all planes
  mask_set_all(plane.mask, pomdp->numStates);
  plane.numBackupsAtCreation = -1;

  while (!inFile.eof()) {
    inFile >> plane.action;
    plane.alpha.clear();
    plane.alpha.resize(pomdp->numStates);
    for (int i=0; i < pomdp->numStates; i++) {
      inFile >> val;
      plane.alpha.push_back(i, val);
    }
    addLBPlane(new LBPlane(plane));
  }
    
  inFile.close();

  // the set of planes should have been pruned before it was written out
  lastPruneNumPlanes = planes.size();
  lastPruneNumBackups = -1;

  initialized = true;
}

int MaxPlanesLowerBound::getStorage(int whichMetric) const
{
  switch (whichMetric) {
  case ZMDP_S_NUM_ELTS:
    // return number of planes
    return planes.size();

  case ZMDP_S_NUM_ENTRIES: {
    // return total number of entries in all planes
    int entryCount = 0;
    FOR_EACH (planeP, planes) {
      const LBPlane& p = **planeP;
      entryCount += p.alpha.filled();
      if (useMaxPlanesMasking) {
	entryCount += p.mask.filled();
      }
    }
    return entryCount;
  }

  default:
    /* N/A */
    return 0;
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.26  2007/03/23 00:04:59  trey
 * added an implementation of chooseAction() that uses the direct policy rather than the lookahead-one policy; should speed policy evaluation for benchmarking significantly
 *
 * Revision 1.25  2007/02/22 22:01:28  trey
 * made change to support list calculation to avoid bad interactions with pruning that caused a crash; parallels an earlier change to the sawtooth upper bound
 *
 * Revision 1.24  2007/01/14 00:54:10  trey
 * added hooks for logging storage space during a run
 *
 * Revision 1.23  2006/11/07 21:08:39  trey
 * corrected printf format warning
 *
 * Revision 1.22  2006/10/30 20:00:15  trey
 * USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
 *
 * Revision 1.21  2006/10/25 19:13:01  trey
 * migrated to more run-time config parameters
 *
 * Revision 1.20  2006/10/24 19:12:13  trey
 * replaced useConvexSupportList with useMaxPlanesSupportList
 *
 * Revision 1.19  2006/10/24 02:12:47  trey
 * distributed update code from ConvexBounds to MaxPlanesLowerBound, allows more flexibility
 *
 * Revision 1.18  2006/10/18 18:07:13  trey
 * USE_TIME_WITHOUT_HEURISTIC is now a run-time config parameter
 *
 * Revision 1.17  2006/08/08 21:17:20  trey
 * fixed a bug in LB backPointers code; added USE_REF_COUNT_PRUNE
 *
 * Revision 1.16  2006/08/04 22:31:50  trey
 * MaxPlanes policy reading works again; it was broken when support lists were introduced
 *
 * Revision 1.15  2006/07/26 20:38:10  trey
 * fixed off-by-one error in rule for skipping alpha vectors in value function query
 *
 * Revision 1.14  2006/07/26 20:21:53  trey
 * new implementation of USE_CONVEX_CACHE; during pruning, now skip comparison of planes if they were compared during last pruning cycle
 *
 * Revision 1.13  2006/07/26 16:32:10  trey
 * removed dependence of pruning on USE_CONVEX_SUPPORT_LIST
 *
 * Revision 1.12  2006/07/25 23:24:23  trey
 * fixed subtle error in support list
 *
 * Revision 1.11  2006/07/25 19:40:49  trey
 * added USE_CONVEX_CACHE support
 *
 * Revision 1.10  2006/07/24 17:07:47  trey
 * added USE_CONVEX_SUPPORT_LIST
 *
 * Revision 1.9  2006/07/14 15:09:13  trey
 * cleaned up pruning; removed belief argument from addLBPlane()
 *
 * Revision 1.8  2006/07/12 19:41:34  trey
 * cleaned out some cruft
 *
 * Revision 1.7  2006/06/15 16:05:12  trey
 * fixed size() information for alpha vectors, had to reorder some libs
 *
 * Revision 1.6  2006/06/14 18:17:21  trey
 * added abliity to read in a policy
 *
 * Revision 1.5  2006/05/27 19:21:50  trey
 * corrected a sentence in header comment that did not make sense
 *
 * Revision 1.4  2006/04/28 21:14:20  trey
 * now automatically write an explanation of the policy format to top of each policy output file
 *
 * Revision 1.3  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.2  2006/04/27 23:10:31  trey
 * added support for writing policies
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.13  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.12  2005/11/03 17:48:20  trey
 * removed MATRIX_NAMESPACE macro
 *
 * Revision 1.11  2005/10/28 03:52:15  trey
 * simplified license
 *
 * Revision 1.10  2005/10/28 02:55:36  trey
 * added copyright header
 *
 * Revision 1.9  2005/10/27 22:03:40  trey
 * cleaned out some cruft
 *
 * Revision 1.8  2005/10/21 20:19:37  trey
 * added namespace zmdp
 *
 * Revision 1.7  2005/03/28 18:14:19  trey
 * fixed inaccurate comment
 *
 * Revision 1.6  2005/03/25 19:23:39  trey
 * made lowerBoundV and upperBoundV explicit in FocusedPomdp
 *
 * Revision 1.5  2005/03/11 19:24:12  trey
 * switched from hash_map to list representation
 *
 * Revision 1.4  2005/03/10 21:14:06  trey
 * added masked alpha support
 *
 * Revision 1.3  2005/02/08 23:55:29  trey
 * updated to work when alpha_vector = cvector
 *
 * Revision 1.2  2005/01/28 03:25:14  trey
 * switched using call from ublas -> MATRIX_NAMESPACE, added some debug statements
 *
 * Revision 1.1  2004/11/24 20:13:07  trey
 * split AlphaList.h out of ValueFunction.h
 *
 * Revision 1.2  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.9  2003/09/22 18:48:13  trey
 * made several algorithm configurations depend on makefile settings, added extra timing output
 *
 * Revision 1.8  2003/09/20 02:26:10  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.7  2003/09/17 20:54:25  trey
 * seeing good performance on tagAvoid (but some mods since run started...)
 *
 * Revision 1.6  2003/09/17 18:30:17  trey
 * seems to show best performance so far
 *
 * Revision 1.5  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.2  2003/07/16 16:14:07  trey
 * implemented AlphaList read() function, added support for tagging alpha vectors with actions
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
