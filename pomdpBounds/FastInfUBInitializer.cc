/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-05 21:43:20 $
   
 @file    FastInfUBInitializer.cc
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
#include "FastInfUBInitializer.h"
#include "FullObsUBInitializer.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define PRUNE_EPS (1e-10)
#define MDP_RESIDUAL (1e-10)

namespace zmdp {

FastInfUBInitializer::FastInfUBInitializer(const MDP* problem, SawtoothUpperBound* _bound)
{
  pomdp = (const Pomdp*) problem;
  bound = _bound;
}

void FastInfUBInitializer::initialize(double targetPrecision)
{
  initFIB(targetPrecision);
}

void FastInfUBInitializer::initMDP(double targetPrecision)
{
  // set alpha to be the mdp upper bound
  FullObsUBInitializer m;
  m.valueIteration(pomdp, targetPrecision);
  cvector calpha;
  copy(calpha, m.alpha);

  alphas.clear();
  alphas.push_back(calpha);

  cout << "initUpperBoundMDP: alpha=" << sparseRep(alphas[0]).c_str() << endl;
  cout << "initUpperBoundMDP: val(b)=" << inner_prod(alphas[0], pomdp->initialBelief) << endl;
}

void FastInfUBInitializer::initFIB(double targetPrecision)
{
  typedef sla::dvector dvector;
  // calculates the fast informed bound (Hauskrecht, JAIR 2000)
  std::vector< dvector > al(pomdp->numActions);
  std::vector< dvector > nextAl(pomdp->numActions);
  dvector tmp, beta_aoi, beta_ao, diff;
  double maxResidual;
  alpha_vector backup;

  initMDP(MDP_RESIDUAL);

  // initialize al array with weak MDP upper bound
  FullObsUBInitializer m;
  m.pomdp = pomdp;
  alpha_vector& alpha = alphas[0];
  copy(m.alpha, alpha);
  FOR (a, pomdp->numActions) {
    al[a].resize(pomdp->numStates);
    nextAl[a].resize(pomdp->numStates);
    m.nextAlphaAction(al[a], a);
  }

  cout << "upper bound FIB iteration";
  cout.flush();

  // iterate FIB update rule to approximate convergence
  do {
    FOR (a, pomdp->numActions) {
      dvector& beta_a = nextAl[a];

      set_to_zero( beta_a );

      FOR (o, pomdp->numObservations) {
	FOR (i, pomdp->numActions) {
	  emult_column( tmp, pomdp->O[a], o, al[i] );
	  mult( beta_aoi, tmp, pomdp->Ttr[a] );
	  if (0 == i) {
	    beta_ao = beta_aoi;
	  } else {
	    max_assign( beta_ao, beta_aoi );
	  }
	}
	beta_a += beta_ao;
      }
      
      beta_a *= pomdp->discount;
      copy_from_column( tmp, pomdp->R, a );
      beta_a += tmp;
    }

    maxResidual = 0;
    FOR (a, pomdp->numActions) {
      diff = nextAl[a];
      diff -= al[a];
      maxResidual = std::max( maxResidual, norm_inf(diff) );

      al[a] = nextAl[a];
    }

    cout << ".";
    cout.flush();

  } while ( maxResidual > targetPrecision );

  cout << endl;

  dvector dalpha;
  FOR (a, pomdp->numActions) {
    if (0 == a) {
      dalpha = al[a];
    } else {
      max_assign(dalpha, al[a]);
    }
  }

  // write out result
  bound->pts.clear();
  copy(bound->cornerPts, dalpha);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/02/14 19:33:55  trey
 * added targetPrecision argument for bounds initialization
 *
 * Revision 1.4  2006/02/06 19:26:09  trey
 * removed numOutcomes from MDP class because some MDPs have a varying number of outcomes depending on state; replaced with numObservations in Pomdp class
 *
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:13:45  trey
 * changed when MDP* arguments are passed into bounds initialization
 *
 * Revision 1.1  2006/01/31 19:18:24  trey
 * initial check-in
 *
 *
 ***************************************************************************/
