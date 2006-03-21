/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-03-21 16:46:21 $
   
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
#define ARTDP_TERM_NODE_ARR_SIZE (100)
#define ARTDP_QUANTILE (0.1)

#define ARTDP_NUM_PARAMS (4)

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
  double termNodeValues[ARTDP_TERM_NODE_ARR_SIZE];
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

struct ARTDP : public RTDPCore {
  ARTDPParamInfo params[ARTDP_NUM_PARAMS];

  ARTDP(AbstractBound* _initUpperBound);

  ARTDPParamInfo& getMinF(void) { return params[0]; }
  ARTDPParamInfo& getMinLogRelevance(void) { return params[1]; }
  ARTDPParamInfo& getMinDiscrep(void) { return params[2]; }
  ARTDPParamInfo& getMinNegDepth(void) { return params[3]; }

  bool getUseLowerBound(void) const { return true; }
  void updateInternal(MDPNode& cn) { assert(0); /* never called */ }

  void endTrial(void);
  void getMaxPrioOutcome(MDPNode& cn, int a, ARTDPUpdateResult& r) const;
  void update2(MDPNode& cn, ARTDPUpdateResult& result);
  double calcOutcomePrio(MDPNode& cn, double obsProb);
  void trialRecurse(MDPNode& cn, double g, double logOcc, double discrep, int depth);
  bool doTrial(MDPNode& cn);
  void derivedClassInit(void);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/03/20 19:21:01  trey
 * quantile approximation is now stochastic, no longer biased toward values at the end of the trial
 *
 * Revision 1.2  2006/03/20 18:54:36  trey
 * adaptive params no longer advance in lock step
 *
 * Revision 1.1  2006/03/17 20:05:57  trey
 * initial check-in
 *
 *
 ***************************************************************************/
