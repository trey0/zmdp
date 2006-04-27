/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-27 20:18:08 $
   
 @file    HSVI.h
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

#ifndef INCHSVI_h
#define INCHSVI_h

#include "RTDPCore.h"

namespace zmdp {

struct HSVIUpdateResult {
  int maxUBAction;
  double maxUBVal;
  double ubResidual;
  int maxExcessUncOutcome;
  double maxExcessUnc;
};

struct HSVI : public RTDPCore {
  double trialTargetPrecision;
#if USE_HSVI_ADAPTIVE_DEPTH
  double oldMaxDepth;
  double maxDepth;
  double oldQualitySum;
  int oldNumUpdates;
  double newQualitySum;
  int newNumUpdates;
#endif

  HSVI(void);

  bool getUseLowerBound(void) const { return true; }
  void getMaxExcessUncOutcome(MDPNode& cn, int depth, HSVIUpdateResult& r) const;
  void update(MDPNode& cn, int depth, HSVIUpdateResult& result);
  void trialRecurse(MDPNode& cn, double logOcc, int depth);
  bool doTrial(MDPNode& cn);
};

}; // namespace zmdp

#endif /* INCHSVI_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/12 19:22:41  trey
 * initial check-in
 *
 *
 ***************************************************************************/
