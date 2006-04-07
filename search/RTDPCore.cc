/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.16 $  $Author: trey $  $Date: 2006-04-07 19:43:26 $
   
 @file    RTDPCore.cc
 @brief   Common code used by multiple RTDP variants found in this
          directory.

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "RTDPCore.h"
#include "PointBounds.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

RTDPCore::RTDPCore(void) :
  boundsFile(NULL),
  initialized(false)
{}

void RTDPCore::setBounds(IncrementalBounds* _bounds)
{
  bounds = _bounds;
}

void RTDPCore::init(double _targetPrecision)
{
  targetPrecision = _targetPrecision;

  bounds->initialize(problem, targetPrecision);

  previousElapsedTime = secondsToTimeval(0.0);
  lastPrintTime = 0;

  numTrials = 0;

  if (NULL != boundsFile) {
    (*boundsFile) << "# wallclock time"
		  << ", lower bound"
		  << ", upper bound"
		  << ", # states touched"
		  << ", # states expanded"
		  << ", # trials"
		  << ", # backups"
		  << endl;
    boundsFile->flush();
  }

  derivedClassInit();

  initialized = true;
}

void RTDPCore::planInit(const MDP* _problem,
			double _targetPrecision)
{
  problem = _problem;
  initialized = false;

#if USE_TIME_WITHOUT_HEURISTIC
  init(_targetPrecision);
#endif
}

bool RTDPCore::planFixedTime(const state_vector& s,
			     double maxTimeSeconds,
			     double _targetPrecision)
{
  boundsStartTime = getTime() - previousElapsedTime;

  if (!initialized) {
    boundsStartTime = getTime();
    init(_targetPrecision);
  }

  // disable this termination check for now
  //if (root->ubVal - root->lbVal < targetPrecision) return true;
  bool done = doTrial(*bounds->getRootNode());

  previousElapsedTime = getTime() - boundsStartTime;

  if (NULL != boundsFile) {
    double elapsed = timevalToSeconds(getTime() - boundsStartTime);
    if (done || (0 == lastPrintTime) || elapsed / lastPrintTime >= 1.01) {
      (*boundsFile) << timevalToSeconds(getTime() - boundsStartTime)
		    << " " << bounds->getRootNode()->lbVal
		    << " " << bounds->getRootNode()->ubVal
		    << " " << bounds->numStatesTouched
		    << " " << bounds->numStatesExpanded
		    << " " << numTrials
		    << " " << bounds->numBackups
		    << endl;
      boundsFile->flush();
      lastPrintTime = elapsed;
    }
  }

  return done;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int RTDPCore::chooseAction(const state_vector& s)
{
  return bounds->chooseAction(s);
}

void RTDPCore::setBoundsFile(std::ostream* _boundsFile)
{
  boundsFile = _boundsFile;
}

ValueInterval RTDPCore::getValueAt(const state_vector& s) const
{
  return bounds->getValueAt(s);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2006/04/06 04:14:11  trey
 * changed how bounds are initialized
 *
 * Revision 1.14  2006/04/04 17:25:22  trey
 * moved chooseAction() implementation to IncrementalBounds
 *
 * Revision 1.13  2006/04/03 21:39:24  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.12  2006/03/17 20:06:44  trey
 * added derivedClassInit() virtual function for more flexibility
 *
 * Revision 1.11  2006/02/27 20:12:37  trey
 * cleaned up meta-information in header
 *
 * Revision 1.10  2006/02/20 00:05:13  trey
 * added USE_RTDPCORE_UB_ACTION
 *
 * Revision 1.9  2006/02/19 18:34:52  trey
 * targetPrecision is now stored as a field; modified how prio is initialized in getNode(); lastPrintTime is now initialized properly
 *
 * Revision 1.8  2006/02/17 18:20:41  trey
 * renamed LStack -> NodeStack and moved it from LRTDP to RTDPCore so that it can also be used by HDP; added initialization of idx field of nodes in getNode()
 *
 * Revision 1.7  2006/02/15 16:26:15  trey
 * added USE_TIME_WITHOUT_HEURISTIC support, switched prio to be logarithmic, added tie-break condition for chooseAction()
 *
 * Revision 1.6  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.5  2006/02/13 21:47:27  trey
 * added initialization of prio field
 *
 * Revision 1.4  2006/02/13 20:20:33  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.3  2006/02/13 19:53:11  trey
 * reduced amount of debug output
 *
 * Revision 1.2  2006/02/13 19:08:49  trey
 * moved numBackups tracking code for better flexibility
 *
 * Revision 1.1  2006/02/11 22:38:10  trey
 * moved much of the RTDP implementation into RTDPCore, where it can be shared by many RTDP variants
 *
 *
 ***************************************************************************/
