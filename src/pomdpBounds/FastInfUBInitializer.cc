/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#include "FastInfUBInitializer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "FullObsUBInitializer.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "zmdpCommonDefs.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define PRUNE_EPS (1e-10)
#define MDP_RESIDUAL (1e-10)

namespace zmdp {

FastInfUBInitializer::FastInfUBInitializer(const MDP *problem,
                                           SawtoothUpperBound *_bound) {
  pomdp = (const Pomdp *)problem;
  bound = _bound;
}

void FastInfUBInitializer::initialize(double targetPrecision) {
  initFIB(targetPrecision);
}

void FastInfUBInitializer::initMDP(double targetPrecision) {
  // set alpha to be the mdp upper bound
  FullObsUBInitializer m;
  m.valueIteration(pomdp, targetPrecision);
  cvector calpha;
  copy(calpha, m.alpha);

  alphas.clear();
  alphas.push_back(calpha);

  if (zmdpDebugLevelG >= 1) {
    cout << "initUpperBoundMDP: alpha=" << sparseRep(alphas[0]).c_str() << endl;
    cout << "initUpperBoundMDP: val(b)="
         << inner_prod(alphas[0], pomdp->initialBelief) << endl;
  }
}

void FastInfUBInitializer::initFIB(double targetPrecision) {
  typedef sla::dvector dvector;
  // calculates the fast informed bound (Hauskrecht, JAIR 2000)
  std::vector<dvector> al(pomdp->numActions);
  std::vector<dvector> nextAl(pomdp->numActions);
  dvector tmp, beta_aoi, beta_ao, diff;
  double maxResidual;
  alpha_vector backup;

  initMDP(MDP_RESIDUAL);

  // initialize al array with weak MDP upper bound
  FullObsUBInitializer m;
  m.pomdp = pomdp;
  alpha_vector &alpha = alphas[0];
  copy(m.alpha, alpha);
  FOR(a, pomdp->numActions) {
    al[a].resize(pomdp->numStates);
    nextAl[a].resize(pomdp->numStates);
    m.nextAlphaAction(al[a], a);
  }

  if (zmdpDebugLevelG >= 1) {
    cout << "starting upper bound FIB iteration";
    cout.flush();
  }

  // iterate FIB update rule to approximate convergence
  do {
    FOR(a, pomdp->numActions) {
      dvector &beta_a = nextAl[a];

      set_to_zero(beta_a);

      FOR(o, pomdp->numObservations) {
        FOR(i, pomdp->numActions) {
          emult_column(tmp, pomdp->O[a], o, al[i]);
          mult(beta_aoi, tmp, pomdp->Ttr[a]);
          if (0 == i) {
            beta_ao = beta_aoi;
          } else {
            max_assign(beta_ao, beta_aoi);
          }
        }
        beta_a += beta_ao;
      }

      beta_a *= pomdp->discount;
      copy_from_column(tmp, pomdp->R, a);
      beta_a += tmp;
    }

    maxResidual = 0;
    FOR(a, pomdp->numActions) {
      diff = nextAl[a];
      diff -= al[a];
      maxResidual = std::max(maxResidual, norm_inf(diff));

      al[a] = nextAl[a];
    }

    if (zmdpDebugLevelG >= 1) {
      cout << ".";
      cout.flush();
    }
  } while (maxResidual > targetPrecision);

  if (zmdpDebugLevelG >= 1) {
    cout << endl;
  }

  dvector dalpha;
  FOR(a, pomdp->numActions) {
    if (0 == a) {
      dalpha = al[a];
    } else {
      max_assign(dalpha, al[a]);
    }
  }

  // post-process: make sure the value for all terminal states
  // is exactly 0, since that is how the ubVal field of terminal
  // nodes is initialized.
  FOR(i, pomdp->numStates) {
    if (pomdp->isTerminalState[i]) {
      dalpha(i) = 0.0;
    }
  }

  // write out result
  bound->pts.clear();
  copy(bound->cornerPts, dalpha);
}

};  // namespace zmdp
