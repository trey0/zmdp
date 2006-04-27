/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-27 23:10:31 $
   
 @file    MaxPlanesLowerBound.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ***************************************************************************/

#ifndef INCMaxPlanesLowerBound_h
#define INCMaxPlanesLowerBound_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// this causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <iostream>

#include <string>
#include <vector>
#include <list>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#if USE_MASKED_ALPHA
#  include "sla_mask.h"
#endif
#include "AbstractBound.h"
#include "Pomdp.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

class LBPlane {
public:
  alpha_vector alpha;
  int action;
#if USE_MASKED_ALPHA
  sla::mvector mask;
#endif

  LBPlane(void) {}
  LBPlane(const LBPlane& rhs);
  LBPlane& operator=(const LBPlane& rhs);

#if USE_MASKED_ALPHA
  void copyFrom(const alpha_vector& _alpha, int _action, const sla::mvector& _mask);
#else
  void copyFrom(const alpha_vector& _alpha, int _action);
#endif
  void write(std::ostream& out) const;
};

typedef std::list< LBPlane > PlaneSet;

class MaxPlanesLowerBound : public AbstractBound {
public:
  const Pomdp* pomdp;
  PlaneSet planes;
  int lastPruneNumPlanes;
  
  MaxPlanesLowerBound(const MDP* _pomdp);

  // performs any computation necessary to initialize the bound
  void initialize(double targetPrecision);

  // returns the bound value at state s
  double getValue(const belief_vector& b) const;

  const LBPlane& getBestLBPlane(const belief_vector& b) const;
  void addLBPlane(const belief_vector& b, const LBPlane& av);
  void addLBPlane(const belief_vector& b, const alpha_vector& alpha,
		  int action
#if USE_MASKED_ALPHA
		  , const sla::mvector& mask
#endif
		  );
  void read(const std::string& filename);

  void prunePlanes(void);
  void maybePrune(void);

  void writeToFile(const std::string& outFileName) const;
};
std::ostream& operator<<(std::ostream& out, const MaxPlanesLowerBound& al);

}; // namespace zmdp

#endif // INCMaxPlanesLowerBound_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.13  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.12  2005/11/08 18:13:41  trey
 * moved hash_map setup code to common/zmdpCommonTypes.h
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
 * Revision 1.7  2005/03/28 18:14:03  trey
 * renamed updateAsSafety to useSafetyUpdate
 *
 * Revision 1.6  2005/03/25 21:43:26  trey
 * added updateAsSafety flag in BoundFunction and AlphaList, made some FocusedPomdp functions take a bound as an argument
 *
 * Revision 1.5  2005/03/25 19:23:39  trey
 * made lowerBoundV and upperBoundV explicit in FocusedPomdp
 *
 * Revision 1.4  2005/03/11 19:24:12  trey
 * switched from hash_map to list representation
 *
 * Revision 1.3  2005/03/10 21:14:06  trey
 * added masked alpha support
 *
 * Revision 1.2  2005/01/28 03:24:31  trey
 * removed testAlphaList and renamed bvector -> belief_vector
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
 * Revision 1.6  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.5  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.4  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.3  2003/07/23 20:50:00  trey
 * renamed AlphaVector::init() to copyFrom() to match convention used with other structs
 *
 * Revision 1.2  2003/07/16 16:14:07  trey
 * implemented AlphaList read() function, added support for tagging alpha vectors with actions
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
