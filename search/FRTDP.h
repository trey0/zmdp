/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.10 $  $Author: trey $  $Date: 2006-04-06 04:14:50 $
   
 @file    FRTDP.h
 @brief   No brief

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

#ifndef INCFRTDP_h
#define INCFRTDP_h

#include "RTDPCore.h"

namespace zmdp {

struct FRTDPUpdateResult {
  int maxUBAction;
  double maxUBVal;
  double ubResidual;
  int maxPrioOutcome;
  double maxPrio;
};

struct FRTDPExtraNodeData {
  double prio;
};

struct FRTDP : public RTDPCore {
  double oldMaxDepth;
  double maxDepth;
  double oldQualitySum;
  int oldNumUpdates;
  double newQualitySum;
  int newNumUpdates;

  FRTDP(AbstractBound* _initLowerBound, AbstractBound* _initUpperBound);

  bool getUseLowerBound(void) const { return true; }

  void getNodeHandler(MDPNode& cn);
  static void staticGetNodeHandler(MDPNode& cn, void* handlerData);
  static double& getPrio(const MDPNode& cn);
  void getMaxPrioOutcome(MDPNode& cn, int a, FRTDPUpdateResult& result) const;
  void update(MDPNode& cn, FRTDPUpdateResult& result);
  void trialRecurse(MDPNode& cn, double logOcc, int depth);
  bool doTrial(MDPNode& cn);
  void derivedClassInit(void);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2006/04/04 17:23:34  trey
 * made getPrio() static
 *
 * Revision 1.8  2006/04/03 21:39:03  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.7  2006/03/21 21:24:43  trey
 * removed code that was obsoleted by new termination condition
 *
 * Revision 1.6  2006/03/21 21:09:24  trey
 * added adaptive maxDepth termination criterion
 *
 * Revision 1.5  2006/02/20 02:04:55  trey
 * changed altPrio margin to be based on occupancy
 *
 * Revision 1.4  2006/02/19 18:34:35  trey
 * lots of changes, trying out different termination approaches
 *
 * Revision 1.3  2006/02/15 16:24:27  trey
 * switched to a better termination criterion
 *
 * Revision 1.2  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.1  2006/02/13 21:46:46  trey
 * initial check-in
 *
 *
 ***************************************************************************/
