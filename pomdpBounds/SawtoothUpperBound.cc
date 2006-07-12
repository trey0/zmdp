/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-07-12 19:45:55 $
   
 @file    SawtoothUpperBound.cc
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

#include <iostream>
#include <list>

#include "zmdpCommonDefs.h"
#include "SawtoothUpperBound.h"
#include "FastInfUBInitializer.h"

#define MIN_RATIO_EPS (1e-10)
#define PRUNE_PTS_INCREMENT (10)
#define PRUNE_PTS_FACTOR (1.1)
#define CORNER_EPS (1e-6)

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

SawtoothUpperBound::SawtoothUpperBound(const MDP* _pomdp)
{
  pomdp = (const Pomdp*) _pomdp;
  numStates = pomdp->getBeliefSize();
  lastPruneNumPts = 0;
}

void SawtoothUpperBound::initialize(double targetPrecision)
{
  FastInfUBInitializer fib(pomdp, this);
  fib.initialize(targetPrecision);
}

double SawtoothUpperBound::getValue(const belief_vector& b) const
{
  double val, min_val;
  double ratio, min_ratio;
  typeof(b.data.begin()) bi, bend;
  typeof(b.data.begin()) ci, cend;
  cvector tmp, bhat;
  double cval;
  double inner_cornerPts_b;
  double inner_cornerPts_c;
  bool bdone = false;

  inner_cornerPts_b = inner_prod( cornerPts, b );
  min_val = inner_cornerPts_b;

  FOR_EACH (pr, pts) {
    if (pr->disabled) {
      //cout << "skipping" << endl;
      continue;
    }
    const belief_vector& c = pr->b;
    cval = pr->v;

    inner_cornerPts_c = inner_prod( cornerPts, c );
    if (inner_cornerPts_c <= cval) {
      // FIX: should figure out a way to prune c here
      //cout << "  bad cval" << endl;
      continue;
    }

    min_ratio = 99e+20;

    bi = b.data.begin();
    bend = b.data.end();

    ci = c.data.begin();
    cend = c.data.end();

    for (; ci != cend; ci++) {
      if (0.0 == ci->value) continue;

      // advance until bi->index >= ci->index
      while (1) {
	if (bi == bend) {
	  bdone = true;
	  goto break_ci_loop;
	}
	if (bi->index >= ci->index) break;
	bi++;
      }

      if (bi->index > ci->index) {
	// we found a j such that b(j) = 0 and c(j) != 0, which implies
	// min_ratio = 0, so this c is useless and should be skipped
	//cout << "  skipping out" << endl;
	goto continue_pts_loop;
      }

      ratio = bi->value / ci->value;
      if (ratio < min_ratio) min_ratio = ratio;
    }
  break_ci_loop:
    if (bdone) {
      for (; ci != cend; ci++) {
	if (0.0 != ci->value) goto continue_pts_loop;
      }
    }
    
    val = inner_cornerPts_b
      + min_ratio * ( cval - inner_cornerPts_c );
#if 1
    if (min_ratio > 1) {
      if (min_ratio < 1 + MIN_RATIO_EPS) {
	// round-off error, correct it down to 1
	min_ratio = 1;
      } else {
	cout << "ERROR: min_ratio > 1 in upperBoundInternal!" << endl;
	cout << "  (min_ratio-1)=" << (min_ratio-1) << endl;
	cout << "  normb=" << norm_1(b) << endl;
	cout << "  b=" << sparseRep(b) << endl;
	cout << "  normc=" << norm_1(c) << endl;
	cout << "  c=" << sparseRep(c) << endl;
	exit(EXIT_FAILURE);
      }
    }
#endif

    if (val < min_val) min_val = val;

  continue_pts_loop:
    ;
  }

  return min_val;
}

void SawtoothUpperBound::maybePrune(void) {
  unsigned int nextPruneNumPts = max(lastPruneNumPts + PRUNE_PTS_INCREMENT,
				     (int) (lastPruneNumPts * PRUNE_PTS_FACTOR));
  if (pts.size() > nextPruneNumPts) {
    prune();
  }
}

void SawtoothUpperBound::prune(void) {
#if USE_DEBUG_PRINT
  int oldNum = pts.size();
#endif
  typeof(pts.begin()) x, xp1;
  list< std::list<BVPair>::iterator > erase_ptrs;

  FOR_EACH (ptp, pts) {
    BVPair& try_pair = *ptp;
    try_pair.disabled = true;
    // put pt back in if v is at least epsilon smaller than
    // upperBound(b) (smaller is better because it means the upper
    // bound is tighter)
    if (try_pair.v <= getValue(try_pair.b) - ZMDP_BOUNDS_PRUNE_EPS) {
      try_pair.disabled = false;
    } else {
      erase_ptrs.push_back(ptp);
    }
  }
  FOR_EACH (erase_ptr, erase_ptrs) {
    x = xp1 = (*erase_ptr);
    xp1++;
    pts.erase( x, xp1 );
  }

#if USE_DEBUG_PRINT
  cout << "... pruned # pts from " << oldNum << " down to " << pts.size() << endl;
#endif
  lastPruneNumPts = pts.size();
}

// if b is a corner point e_i, return i.  else return -1
int SawtoothUpperBound::whichCornerPoint(const belief_vector& b) const {
  int non_zero_count = 0;
  int non_zero_index = -1;
  FOR (i, numStates) {
    if (fabs(b(i)) > CORNER_EPS) {
      if (non_zero_count == 0) {
	non_zero_index = i;
	non_zero_count++;
      } else {
	return -1;
      }
    }
  }
  if (non_zero_count == 0) {
    return -1;
  } else {
    return non_zero_index;
  }
}

void SawtoothUpperBound::addPoint(const belief_vector& b, double val) {
  int wc = whichCornerPoint(b);
  if (-1 == wc) {
    pts.push_back(BVPair(b,val));
  } else {
    cornerPts(wc) = val;
  }
}

void SawtoothUpperBound::printToStream(ostream& out) const {
  out << "{" << endl;
  out << "  cornerPts = " << sparseRep(cornerPts) << endl;
  out << "  " << pts.size() << " points:" << endl;
  FOR_EACH (pt, pts) {
    const BVPair& pr = *pt;
    out << "    " << sparseRep(pr.b)
	<< " => " << pr.v
	<< " (" << getValue(pr.b) << ")" << endl;
  }
  out << "}" << endl;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.16  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.15  2005/10/28 03:52:15  trey
 * simplified license
 *
 * Revision 1.14  2005/10/28 02:55:36  trey
 * added copyright header
 *
 * Revision 1.13  2005/10/27 22:03:40  trey
 * cleaned out some cruft
 *
 * Revision 1.12  2005/10/21 20:20:09  trey
 * added namespace zmdp
 *
 * Revision 1.11  2005/03/28 18:14:46  trey
 * renamed updateAsSafety to useSafetyUpdate
 *
 * Revision 1.10  2005/03/25 21:43:26  trey
 * added updateAsSafety flag in BoundFunction and AlphaList, made some FocusedPomdp functions take a bound as an argument
 *
 * Revision 1.9  2005/03/25 19:23:39  trey
 * made lowerBoundV and upperBoundV explicit in FocusedPomdp
 *
 * Revision 1.8  2005/03/11 19:24:35  trey
 * switched from hash_map to list representation
 *
 * Revision 1.7  2005/02/08 23:55:47  trey
 * updated to work when alpha_vector = cvector
 *
 * Revision 1.6  2005/02/06 16:41:30  trey
 * removed broken upperBoundInternal2 and added #if USE_CPLEX around upperBoundInternal1
 *
 * Revision 1.5  2005/02/04 21:03:40  trey
 * added upperBoundInternal3 and timing code for lp optimization
 *
 * Revision 1.4  2005/01/28 03:25:53  trey
 * improved sla compatibility
 *
 * Revision 1.3  2005/01/26 04:18:39  trey
 * fixed for changed rand_vector() API, fixed some spurious warnings
 *
 * Revision 1.2  2005/01/17 19:34:30  trey
 * added some test code for comparing upper bound mechanisms
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.9  2003/09/18 21:00:28  trey
 * upper bound self-update seems to be working
 *
 * Revision 1.8  2003/09/17 20:54:24  trey
 * seeing good performance on tagAvoid (but some mods since run started...)
 *
 * Revision 1.7  2003/09/16 00:57:01  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.6  2003/09/11 01:46:41  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.5  2003/07/22 19:59:36  trey
 * several minor fixes; still observing discrepancy between sim and solver lower bound
 *
 * Revision 1.4  2003/07/18 16:07:58  trey
 * finished implementing Q caching
 *
 * Revision 1.3  2003/07/17 18:50:40  trey
 * did prep for caching upper bound Q values
 *
 * Revision 1.2  2003/07/16 16:06:41  trey
 * alpha vectors now tagged with action, tweaked interface to LP solver
 *
 * Revision 1.1  2003/06/26 15:41:18  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
