/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-03-17 20:05:57 $
   
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

// number of trials in a 'batch' when considering adjusting a parameter
#define ARTDP_BATCH_SIZE (1)
// rate at which batchesUntilNextAdvance grows each time an advance attempt fails
#define ARTDP_ADVANCE_GROWTH_RATE (1)
// number of values to remember when calculating an approximate quantile
//  among termination thresholds. (more values gives a more accurate estimate.)
#define ARTDP_TERMINATE_THRESHOLD_MAX_VALUES (500)
#define ARTDP_QUANTILE (0.0)

#define ARTDP_UNDEFINED (-999)

namespace zmdp {

struct ARTDPUpdateResult {
  int maxUBAction;
  double maxUBVal, secondBestUBVal;
  double ubResidual;
  int maxPrioOutcome, secondBestPrioOutcome;
  double maxPrio, secondBestPrio;
};

struct ARTDPParamInfo {
  // name: only used in debug print statements
  std::string name;
  // val: ARTDP bounces between using val[0] and val[1] as the current
  //   value until there is a batch using val[1] that outperforms the most recent
  //   batch using val[0].  this is called 'accepting' val[1].  when that happens,
  //   the array is shifted left (val[0] <- val[1] <- val[2], leaving val[2]
  //   'undefined').
  double val[3];
  // currentValIndex: indicates the index of the currently used value in
  //   the val array (always either 0 or 1)
  int currentValIndex;
  // lastBatchQualityAtVal: for each index i into the val array (i = 0 or 1),
  //   records the quality of the last batch with value=val[i]
  double lastBatchQualityAtVal[2];
  // batchesPerAdvance: after val[1] has been tried and underperformed
  //   relative to val[0], this is how many batches to execute at val[0] before
  //   trying again.  increases after each failed attempt.
  double batchesPerAdvance;
  int batchesSoFarInAdvance;
  // batchQualitySum: sum of quality values from all trials so far in this batch.
  double batchQualitySum;
  // numTrialsSoFarInBatch: counts the number of trials so far in the batch.
  //   the batch ends when the number of trials reaches ARTDP_BATCH_SIZE.
  int trialsSoFarInBatch;
  // terminateThresholds: used to record a few termination thresholds from
  //   a trial when estimating a value to advance to.
  double terminateThresholds[ARTDP_TERMINATE_THRESHOLD_MAX_VALUES];
  int thresholdsSoFarInTrial;
  // currentVal = val[currentValIndex]
  double currentVal;
  double minTermThreshold;

  ARTDPParamInfo(const std::string& _name, double initVal);
  void init(double initVal);
  void recordTermination(double termThreshold);
  void endTrial(double trialQuality);
};

struct ARTDP : public RTDPCore {
  ARTDPParamInfo minF, minLogRelevance;
  double trialQualitySum;
  int updatesSoFarInTrial;

  ARTDP(AbstractBound* _initUpperBound);


  bool getUseLowerBound(void) const { return true; }
  void updateInternal(MDPNode& cn) { assert(0); /* never called */ }

  void update2(MDPNode& cn, ARTDPUpdateResult& result);
  void trialRecurse(MDPNode& cn, double g, double logOcc, int depth);
  bool doTrial(MDPNode& cn);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
