/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-03-20 18:54:36 $
   
 @file    ARTDP.h
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

#ifndef INCARTDP_h
#define INCARTDP_h

#include "RTDPCore.h"

// number of values to remember when calculating an approximate quantile
//  among termination thresholds. (more values gives a more accurate estimate.)
#define ARTDP_TERMINATE_THRESHOLD_MAX_VALUES (500)
#define ARTDP_QUANTILE (0.1)

#define ARTDP_UNDEFINED (-999)

namespace zmdp {

struct ARTDPUpdateResult {
  double maxLBVal;
  int maxUBAction;
  double maxUBVal, secondBestUBVal;
  double ubResidual;
  int maxPrioOutcome, secondBestPrioOutcome;
  double maxPrio, secondBestPrio;
};

struct ARTDPParamInfo {
  // name: only used in debug print statements
  std::string name;
  double controlVal;
  double val;
  // termNodeValues: used to record a few values of terminal nodes from
  //   a trial when estimating a value to advance to.
  double termNodeValues[ARTDP_TERMINATE_THRESHOLD_MAX_VALUES];
  double minTermNodeValue;
  double currNodeQualitySum;
  int numCurrNodes;
  double controlNodeQualitySum;
  int numControlNodes;
  int numTermNodes;

  ARTDPParamInfo(void) {}
  ARTDPParamInfo(const std::string& _name, double initVal);
  void init(const std::string& _name, double initVal);
  bool recordNode(double nodeVal, double updateQuality);
  double calcDelta(void);
};

#define ARTDP_NUM_PARAMS (2)

struct ARTDP : public RTDPCore {
  ARTDPParamInfo params[ARTDP_NUM_PARAMS];

  ARTDP(AbstractBound* _initUpperBound);

  ARTDPParamInfo& getMinF(void) { return params[0]; }
  ARTDPParamInfo& getMinLogRelevance(void) { return params[1]; }

  bool getUseLowerBound(void) const { return true; }
  void updateInternal(MDPNode& cn) { assert(0); /* never called */ }

  void endTrial(void);
  void update2(MDPNode& cn, ARTDPUpdateResult& result);
  void trialRecurse(MDPNode& cn, double g, double logOcc, int depth);
  bool doTrial(MDPNode& cn);
  void derivedClassInit(void);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/03/17 20:05:57  trey
 * initial check-in
 *
 *
 ***************************************************************************/
