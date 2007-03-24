/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.24 $  $Author: trey $  $Date: 2007-03-24 22:45:26 $
  
 @file    Pomdp.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "Pomdp.h"
#include "MatrixUtils.h"
#include "slaMatrixUtils.h"
#include "MaxPlanesLowerBound.h"
#include "SawtoothUpperBound.h"
#include "FastParser.h"
#include "CassandraParser.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

Pomdp::Pomdp(const std::string& fileName,
	     const ZMDPConfig* config)
{
  bool useFastModelParser = config->getBool("useFastModelParser");
  if (useFastModelParser) {
    FastParser parser;
    parser.readPomdpFromFile(*this, fileName);
  } else {
    CassandraParser parser;
    parser.readPomdpFromFile(*this, fileName);
  }

  maxHorizon = config->getInt("maxHorizon");

  // belief vectors are the 'state vectors' of the belief-MDP; the
  // dimensionality of these vectors is the number of states in
  // the POMDP
  numStateDimensions = numStates;
}

const belief_vector& Pomdp::getInitialBelief(void) const
{
  return initialBelief;
}

obs_prob_vector& Pomdp::getObsProbVector(obs_prob_vector& result,
					 const belief_vector& b,
					 int a) const
{
  dvector tmp; // FIX: for efficiency, should tmp be a cvector?
  // --- overall: result = O_a' * T_a' * b
  // tmp = T_a' * b
  mult( tmp, Ttr[a], b );
  // result = O_a' * tmp
  mult( result, tmp, O[a] );
  
  return result;
}

belief_vector& Pomdp::getNextBelief(belief_vector& result,
				    const belief_vector& b,
				    int a, int o) const
{
  belief_vector tmp;

  // result = O_a(:,o) .* (T_a * b)
  mult( tmp, Ttr[a], b );
  emult_column( result, O[a], o, tmp );

  // renormalize
  result *= (1.0/sum(result));

  return result;
}

double Pomdp::getReward(const belief_vector& b, int a)
{
  return inner_prod_column( R, a, b );
}

AbstractBound* Pomdp::newLowerBound(const ZMDPConfig* _config)
{
  return new MaxPlanesLowerBound(this, _config);
}

AbstractBound* Pomdp::newUpperBound(const ZMDPConfig* _config)
{
  return new SawtoothUpperBound(this, _config);
}

bool Pomdp::getIsTerminalState(const state_vector& s)
{
  double nonTerminalSum = 0.0;
  FOR_CV (s) {
    if (!isTerminalState[CV_INDEX(s)]) {
      nonTerminalSum += CV_VAL(s);
    }
  }
  return (nonTerminalSum < 1e-10);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.23  2007/02/22 22:01:57  trey
 * added comment
 *
 * Revision 1.22  2006/11/09 20:52:44  trey
 * added initialization of numStateDimensions
 *
 * Revision 1.21  2006/11/08 18:00:03  trey
 * removed #include of some headers; they were only needed for code that mas moved elsewhere
 *
 * Revision 1.20  2006/11/08 16:39:14  trey
 * moved file reading functionality into src/parsers/, in CassandraParser.cc and FastParser.cc
 *
 * Revision 1.19  2006/11/05 03:25:47  trey
 * improved error checking
 *
 * Revision 1.18  2006/10/30 20:00:15  trey
 * USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
 *
 * Revision 1.17  2006/10/24 02:13:43  trey
 * changes to match new Solver interface
 *
 * Revision 1.16  2006/10/18 18:06:37  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.15  2006/10/03 03:18:59  trey
 * added maxHorizon parameter
 *
 * Revision 1.14  2006/07/25 19:42:20  trey
 * deleted some old unused code; modified density calculation to avoid int overflow
 *
 * Revision 1.13  2006/06/05 20:10:46  trey
 * filled in reasonable defaults for pomdp bounds
 *
 * Revision 1.12  2006/05/29 06:05:43  trey
 * now mark zero-reward absorbing states as terminal, without an explicit list in the pomdp model file
 *
 * Revision 1.11  2006/05/29 05:05:19  trey
 * updated handling of isTerminalState, no longer explicitly specified in the model file
 *
 * Revision 1.10  2006/05/27 19:02:18  trey
 * cleaned up CassandraMatrix -> sla::cmatrix conversion
 *
 * Revision 1.9  2006/04/28 18:53:57  trey
 * removed obsolete #if for NO_COMPRESSED_MATRICES
 *
 * Revision 1.8  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.7  2006/04/27 23:10:48  trey
 * put some output in USE_DEBUG_PRINT
 *
 * Revision 1.6  2006/04/06 04:12:54  trey
 * removed default bounds (newLowerBound() and newUpperBound())
 *
 * Revision 1.5  2006/02/17 18:36:35  trey
 * fixed getIsTerminalState() function so RTDP can be used
 *
 * Revision 1.4  2006/02/06 19:26:09  trey
 * removed numOutcomes from MDP class because some MDPs have a varying number of outcomes depending on state; replaced with numObservations in Pomdp class
 *
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:12:44  trey
 * added newXXXBound() functions
 *
 * Revision 1.1  2006/01/31 18:31:50  trey
 * moved many files from common to pomdpCore
 *
 * Revision 1.6  2006/01/29 00:18:36  trey
 * added Pomdp() constructor that calls readFromFile()
 *
 * Revision 1.5  2006/01/28 03:03:23  trey
 * replaced BeliefMDP -> MDP, corresponding changes in API
 *
 * Revision 1.4  2005/11/03 17:45:30  trey
 * moved transition dynamics from HSVI implementation to Pomdp
 *
 * Revision 1.3  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.2  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.1  2005/10/27 21:38:16  trey
 * renamed PomdpM to Pomdp
 *
 * Revision 1.7  2005/10/21 20:08:28  trey
 * added namespace zmdp
 *
 * Revision 1.6  2005/03/10 22:53:32  trey
 * now initialize T matrix even when using sla
 *
 * Revision 1.5  2005/02/08 23:54:25  trey
 * updated to use less type-specific function names
 *
 * Revision 1.4  2005/01/27 05:31:55  trey
 * switched to use Ttr instead of T under sla
 *
 * Revision 1.3  2005/01/26 04:10:41  trey
 * modified problem reading to work with sla
 *
 * Revision 1.2  2005/01/21 15:21:19  trey
 * added readFromFileFast
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.2  2004/11/09 21:31:59  trey
 * got pomdp source tree into a building state again
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.7  2003/09/22 21:42:28  trey
 * made some minor fixes so that algorithm variants to compile
 *
 * Revision 1.6  2003/09/20 02:26:10  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.5  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/07 02:28:07  trey
 * started to adapt for boost matrix library
 *
 * Revision 1.2  2003/07/16 16:07:37  trey
 * added isTerminalState
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
