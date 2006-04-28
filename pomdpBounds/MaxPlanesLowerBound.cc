/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
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
#include "MatrixUtils.h"
#include "MaxPlanesLowerBound.h"
#include "BlindLBInitializer.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

/**********************************************************************
 * LB PLANE
 **********************************************************************/

LBPlane::LBPlane(const LBPlane& rhs)
{
  copyFrom(rhs.alpha, rhs.action
#if USE_MASKED_ALPHA
	   ,rhs.mask
#endif
	   );
}

LBPlane& LBPlane::operator=(const LBPlane& rhs)
{
  copyFrom(rhs.alpha, rhs.action
#if USE_MASKED_ALPHA
	   , rhs.mask
#endif
	   );
  return *this;
}

void LBPlane::copyFrom(const alpha_vector& _alpha, int _action
#if USE_MASKED_ALPHA
			   , const sla::mvector& _mask
#endif
			   )
{
  action = _action;
  alpha.resize(_alpha.size());
  alpha = _alpha;
#if USE_MASKED_ALPHA
  mask = _mask;
#endif
}

void LBPlane::write(std::ostream& out) const
{
  out << "    {" << endl;
  out << "      action => " << action << "," << endl;
  
#if USE_MASKED_ALPHA
  out << "      numEntries => " << mask.filled() << "," << endl;
#else
  out << "      numEntries => " << alpha.size() << "," << endl;
#endif

  out << "      entries => [" << endl;

#if USE_MASKED_ALPHA
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
#else
  int n = alpha.size();
  FOR (i, n-1) {
    out << "        " << i << ", " << alpha(i) << "," << endl;
  }
  out << "        " << (n-1) << ", " << alpha(n-1) << endl;
#endif

  out << "      ]" << endl;
  out << "    }";
}

/**********************************************************************
 * MAX PLANES BOUND
 **********************************************************************/

MaxPlanesLowerBound::MaxPlanesLowerBound(const MDP* _pomdp)
{
  pomdp = (const Pomdp*) _pomdp;
}

void MaxPlanesLowerBound::initialize(double targetPrecision)
{
  BlindLBInitializer blb(pomdp, this);
  blb.initialize(targetPrecision);
}

double MaxPlanesLowerBound::getValue(const belief_vector& b) const
{
  double v = inner_prod( getBestLBPlane(b).alpha, b );
  return v;
}

// return the alpha such that alpha * b has the highest value
const LBPlane& MaxPlanesLowerBound::getBestLBPlane(const belief_vector& b) const 
{
  double val, maxval = -99e+20;
  const LBPlane* ret = NULL;
  FOR_EACH (pr, planes) {
    const LBPlane& al = *pr;
#if USE_MASKED_ALPHA
    if (!mask_subset( b, al.mask )) continue;
#endif
    val = inner_prod( al.alpha, b );
    if (val > maxval) {
      maxval = val;
      ret = &al;
    }
  }
  return *ret;
}

void MaxPlanesLowerBound::addLBPlane(const belief_vector& b, const LBPlane& av)
{
  addLBPlane(b, av.alpha, av.action
#if USE_MASKED_ALPHA
	     , av.mask
#endif
	     );
}

void MaxPlanesLowerBound::addLBPlane(const belief_vector& b,
				     const alpha_vector& alpha,
				     int action
#if USE_MASKED_ALPHA
				     , const sla::mvector& mask
#endif
				     )
{
  static int counter = 0;
  planes.push_back(LBPlane());
  planes.back().copyFrom(alpha, action
#if USE_MASKED_ALPHA
		       ,mask
#endif
		       );
  counter++;
}

void MaxPlanesLowerBound::read(const std::string& filename)
{
  ifstream in(filename.c_str());
  if (!in) {
    cerr << "ERROR: couldn't open " << filename << " for reading: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  belief_vector dummy_b(1);
  LBPlane pr;
  pr.alpha.resize(pomdp->getBeliefSize());
  dvector alpha_tmp(pomdp->getBeliefSize());
  while (1) {
    in >> pr.action;
    FOR (s, pomdp->getBeliefSize()) {
      in >> alpha_tmp(s);
    }
    copy( pr.alpha, alpha_tmp );
    addLBPlane(dummy_b, pr);

    if (in.eof()) break;
  }
  in.close();
}

ostream& operator<<(ostream& out, const MaxPlanesLowerBound& al)
{
  out << "{" << endl;
  //out << "  precision = " << al.precision << endl;
  out << "  planes = {" << endl;
  FOR_EACH (pr, al.planes) {
    out << "    (" << pr->action << ") "
	<< sparseRep(pr->alpha) << endl;
  }
  out << "  }" << endl;
  out << "}" << endl;
  return out;
}

/**********************************************************************
 * PRUNING
 **********************************************************************/

void MaxPlanesLowerBound::prunePlanes(void)
{
  PlaneSet next_planes;
  list<PlaneSet::iterator> erase_ptrs;
  bool try_dominates_in, in_dominates_try;

  FOR_EACH (try_pair, planes) {
    LBPlane& try_alpha = *try_pair;
    erase_ptrs.clear();
    FOR_EACH (in_pair, next_planes) {
      LBPlane& in_alpha = *in_pair;
#if USE_MASKED_ALPHA
      try_dominates_in = mask_dominates(try_alpha.alpha, in_alpha.alpha, ZMDP_BOUNDS_PRUNE_EPS,
					try_alpha.mask,  in_alpha.mask);
#else
      try_dominates_in = dominates(try_alpha.alpha, in_alpha.alpha, ZMDP_BOUNDS_PRUNE_EPS);
#endif
      if (try_dominates_in) {
	// delay erasure since we're still iterating through the set
	erase_ptrs.push_back(in_pair);
      } else {
#if USE_MASKED_ALPHA
	in_dominates_try = mask_dominates(in_alpha.alpha, try_alpha.alpha, ZMDP_BOUNDS_PRUNE_EPS,
					  in_alpha.mask,  try_alpha.mask);
#else
	in_dominates_try = dominates(in_alpha.alpha, try_alpha.alpha, ZMDP_BOUNDS_PRUNE_EPS);
#endif
	if (in_dominates_try) goto next_try_pair;
      }
    }
    // resolve delayed erasure
    FOR_EACH (erase_ptr, erase_ptrs) {
      typeof(next_planes.begin()) x, xp1;
      x = xp1 = (*erase_ptr);
      xp1++;
      next_planes.erase(x,xp1);
    }
    next_planes.push_back( try_alpha );
  next_try_pair: ;
  }
#if USE_DEBUG_PRINT
  cout << "... pruned # planes from " << planes.size()
       << " down to " << next_planes.size() << endl;
#endif

  planes = next_planes;

  lastPruneNumPlanes = planes.size();
}

// prune points and planes if the number has grown significantly
// since the last check
void MaxPlanesLowerBound::maybePrune(void)
{
  unsigned int nextPruneNumPlanes = max(lastPruneNumPlanes + 10,
					(int) (lastPruneNumPlanes * 1.1));
  if (planes.size() > nextPruneNumPlanes) {
    prunePlanes();
  }
}

/**********************************************************************
 * WRITE POLICY
 **********************************************************************/

void MaxPlanesLowerBound::writeToFile(const std::string& outFileName) const
{
  ofstream out(outFileName.c_str());
  if (!out) {
    cerr << "ERROR: MaxPlanesLowerBound::writeToFile: couldn't open " << outFileName
	 << " for writing: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  out << "{" << endl;
  out << "  policyType => \"MaxPlanesLowerBound\"," << endl;
  out << "  numPlanes => " << planes.size() << "," << endl;
  out << "  planes => [" << endl;

  PlaneSet::const_iterator pi = planes.begin();
  FOR (i, planes.size()-1) {
    pi->write(out);
    out << "," << endl;
    pi++;
  }
  pi->write(out);
  out << endl;

  out << "  ]" << endl;
  out << "}" << endl;

  out.close();
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
