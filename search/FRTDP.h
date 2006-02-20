/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2006-02-20 02:04:55 $
   
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
  double maxUBVal, secondBestUBVal;
  double ubResidual;
  int maxPrioOutcome;
  double maxPrio, secondBestPrio;
};

struct FRTDP : public RTDPCore {
  FRTDP(AbstractBound* _initUpperBound);

  bool getUseLowerBound(void) const { return true; }
  void updateInternal(MDPNode& cn) { assert(0); /* never called */ }

  void getMaxPrioOutcome(MDPNode& cn, int a, FRTDPUpdateResult& result) const;
  void update2(MDPNode& cn, FRTDPUpdateResult& result);
  void trialRecurse(MDPNode& cn, double actionDelta, double altPrio, double occ, int depth);
  bool doTrial(MDPNode& cn);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
