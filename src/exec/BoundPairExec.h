/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#ifndef ZMDP_SRC_EXEC_BOUNDPAIREXEC_H_
#define ZMDP_SRC_EXEC_BOUNDPAIREXEC_H_

/**********************************************************************
 * INCLUDES
 **********************************************************************/

#include <iostream>
#include <string>
#include <vector>

#include "BoundPair.h"
#include "MDPExec.h"
#include "Pomdp.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct BoundPairExec : public MDPExec {
  BoundPair *bounds;

  BoundPairExec(void);

  // initializer to use if you already have data structures for the model
  // and the bounds
  void init(MDP *_mdp, BoundPair *_bounds);

  // alternate initializer that reads the model and a policy from files
  void initReadFiles(const std::string &modelFileName,
                     const std::string &policyFileName,
                     const ZMDPConfig &config);

  // implement MDPExec virtual methods
  void setToInitialState(void);
  int chooseAction(void);
  void advanceToNextState(int a, int o);

  // can use for finer control
  void setBelief(const belief_vector &b);
};

};  // namespace zmdp

#endif  // ZMDP_SRC_EXEC_BOUNDPAIREXEC_H_
