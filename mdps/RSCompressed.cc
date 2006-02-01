/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-02-01 18:23:37 $
   
 @file    RSDynamics.cc
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "RSDynamics.h"
#include "MDPValueFunction.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define PRUNE_EPS (1e-10)

namespace zmdp {

const double rsRockXG[] = {3,2,1,1};
const double rsRockYG[] = {1,1,3,0};
const int    rsNumRocksG = 4;
const double rsGridSizeG = 4.0;
const int    rsBeliefSizeG = 6;
const double rsDiscountG = 0.95;
const double rsExitRewardG = 10.0;
const double rsSampleGoodRewardG = 10.0;
const double rsSampleBadPenaltyG = -10.0;
const double rsIllegalPenaltyG = -100;
  
enum RSActionEnum {
  RS_AMN, RS_AME, RS_AMS, RS_AMW,
  RS_AC0, RS_AC1, RS_AC2, RS_AC3,
  RS_AS
};

enum RSObsEnum {
  RS_OGOOD, RS_OBAD
};

static double rsSquare(double x)
{
  return x*x;
}

void RSDynamics::init(void)
{
  beliefSize = 2 + rsNumRocksG;
  numActions = 5 + rsNumRocksG;
  numObservations = 2;
  discount = rsDiscountG;

  dvector bd(beliefSize);
  bd(0) = 0 / rsGridSizeG;
  bd(1) = 2 / rsGridSizeG;
  for (int rock=0; rock < rsNumRocksG; rock++) {
    bd(2+rock) = 0.5;
  }
  copy(b0, bd);
}

const belief_vector& RSDynamics::getInitialBelief(void) const
{
  return b0;
}

obs_prob_vector& RSDynamics::getObsProbVector(obs_prob_vector& result, const belief_vector& b, int a) const
{
  result.resize(2); // sets to zero
  result(RS_OGOOD) = 1.0; // 'default observation'

  // special case for absorbing terminal state
  if (b(0) == 1.0) {
    return result;
  }

  switch (a) {
  case RS_AMN:
  case RS_AME:
  case RS_AMS:
  case RS_AMW:
    return result;
  case RS_AC0:
  case RS_AC1:
  case RS_AC2:
  case RS_AC3:
    {
      int rock = a - RS_AC0;
      double x = b(0)*rsGridSizeG;
      double y = b(1)*rsGridSizeG;
      double d = sqrt(rsSquare(rsRockXG[rock]-x) + rsSquare(rsRockYG[rock]-y));
      double eff = exp(-d);
      double priorPGood = b(rock+2);
      result(RS_OGOOD) = priorPGood * eff + 0.5 * (1-eff);
      result(RS_OBAD) = 1.0 - result(RS_OGOOD);
    }
    return result;
  case RS_AS:
    return result;
  default:
    assert(0); // never reach this point
    return result;
  }
}

belief_vector& RSDynamics::getNextBelief(belief_vector& result, const belief_vector& b, int a, int o) const
{
  // special case for absorbing terminal state
  if (b(0) == 1.0) {
    copy(result, b);
    return result;
  }

  // start with same belief and modify from there
  dvector rd;
  copy(rd, b);

  double x = rd(0)*rsGridSizeG;
  double y = rd(1)*rsGridSizeG;

  switch (a) {
  case RS_AMN:
    if (rsGridSizeG-1 != y) y++;
    break;
  case RS_AME:
    if (x == rsGridSizeG-1) {
      // exit
      rd.resize(rsBeliefSizeG); // set to zero
      x = rsGridSizeG;
      y = 0;
    } else {
      x++;
    }
    break;
  case RS_AMS:
    if (0 != y) y--;
    break;
  case RS_AMW:
    if (0 != x) x--;
    break;
  case RS_AC0:
  case RS_AC1:
  case RS_AC2:
  case RS_AC3:
    {
      int rock = a - RS_AC0;
      double d = sqrt(rsSquare(rsRockXG[rock]-x) + rsSquare(rsRockYG[rock]-y));
      double eff = exp(-d);
      double probObsIfGood, probObsIfBad;
      switch (o) {
      case RS_OGOOD:
	probObsIfGood = 1.0 * eff + 0.5 * (1-eff);
	probObsIfBad  = 0.0 * eff + 0.5 * (1-eff);
	break;
      case RS_OBAD:
	probObsIfGood = 0.0 * eff + 0.5 * (1-eff);
	probObsIfBad  = 1.0 * eff + 0.5 * (1-eff);
	break;
      default:
	assert(0); // never reach this point
      }
      double priorPGood = rd(rock+2);
      double posteriorPGood = (probObsIfGood * priorPGood)
	/ ((probObsIfGood * priorPGood)
	   + (probObsIfBad * (1-priorPGood)));
      rd(rock+2) = posteriorPGood;
    }
    break;
  case RS_AS:
    for (int rock=0; rock < rsNumRocksG; rock++) {
      if ((rsRockXG[rock] == x) && (rsRockYG[rock] == y)) {
	rd(rock+2) = 0.0;
      }
    }
    break;
  default:
    assert(0); // never reach this point
    break;
  }

  rd(0) = x/rsGridSizeG;
  rd(1) = y/rsGridSizeG;
  copy(result, rd);

  return result;
}

double RSDynamics::getReward(const belief_vector& b, int a) const
{
  // special case for absorbing terminal state
  if (b(0) == 1.0) {
    return 0;
  }

  double x = b(0)*rsGridSizeG;
  double y = b(1)*rsGridSizeG;
  int rock;

  switch (a) {
  case RS_AMN:
    if (rsGridSizeG-1 == y) return rsIllegalPenaltyG;
    else return 0;
  case RS_AME:
    if (rsGridSizeG-1 == x) return rsExitRewardG;
    else return 0;
  case RS_AMS:
    if (0 == y) return rsIllegalPenaltyG;
    else return 0;
  case RS_AMW:
    if (0 == x) return rsIllegalPenaltyG;
    else return 0;
  case RS_AC0:
  case RS_AC1:
  case RS_AC2:
  case RS_AC3:
    return 0;
  case RS_AS:
    for (rock=0; rock < rsNumRocksG; rock++) {
      if ((x == rsRockXG[rock]) && (y == rsRockYG[rock])) {
	double p = b(2+rock);
	return rsSampleBadPenaltyG * (1-p) + rsSampleGoodRewardG * p;
      }
    }
    return rsIllegalPenaltyG;
  default:
    assert(0); // never reach this point
    return 0;
  }
}

double RSDynamics::getInitUpperBound(const belief_vector& b)
{
  double x = b(0) * rsGridSizeG;
  double y = b(1) * rsGridSizeG;
  
  // add up max expected reward for each rock
  double total = 0.0;
  for (int i=0; i < rsNumRocksG; i++) {
    double minTimeToSampleRock = fabs(x-rsRockXG[i]) + fabs(y-rsRockYG[i]);
    total += pow(rsDiscountG,minTimeToSampleRock) * b(i+2) * rsSampleGoodRewardG;
  }
  
  // then add max reward for exit
  double minTimeToExit = fabs(x-5);
  total += pow(rsDiscountG,minTimeToExit) * rsExitRewardG;
  
  return total;
}

double RSDynamics::getInitLowerBound(const belief_vector& b)
{
  return 0;
}

void RSDynamics::compressBelief(belief_vector& result, const belief_vector& b)
{
  dvector xp(5), yp(4), g(4);
  FOR_EACH (b_s, b.data) {
    int s = b_s->index;
    double v = b_s->value;
    xp(s>>6)     += v;
    yp((s>>4)%4) += v;
    for (int i=0; i < 4; i++) {
      if ((s>>(3-i))%2) {
	g(i) += v;
      }
    }
  }

  int x = -1;
  double maxVal = -1;
  for (int xi=0; xi < 5; xi++) {
    if (xp(xi) > maxVal) {
      maxVal = xp(xi);
      x = xi;
    }
  }

  int y = -1;
  maxVal = -1;
  for (int yi=0; yi < 4; yi++) {
    if (yp(yi) > maxVal) {
      maxVal = yp(yi);
      y = yi;
    }
  }

  dvector dr(6);
  dr(0) = x / rsGridSizeG;
  dr(1) = y / rsGridSizeG;
  for (int i=0; i < 4; i++) {
    dr(i+2) = g(i);
  }

  copy(result, dr);
}

void RSDynamics::expandBelief(belief_vector& result, const belief_vector& b)
{
  dvector bd, brd(257);

  copy(bd, b);
  int x = (int) (bd(0) * rsGridSizeG);
  int y = (int) (bd(1) * rsGridSizeG);

  for (int s=0; s < 257; s++) {
    if (x == (s>>6) && y == ((s>>4)%4)) {
      double v = 1;
      for (int i=0; i < 4; i++) {
	v *= ((s>>(3-i))%2) ? bd(2+i) : (1-bd(2+i));
      }
      brd(s) = v;
    } else {
      brd(s) = 0;
    }
  }
  copy(result, brd);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2005/11/06 01:27:12  trey
 * initial check-in
 *
 *
 ***************************************************************************/
