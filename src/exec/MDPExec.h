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

#ifndef INCMDPExec_h
#define INCMDPExec_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

#include "MDPModel.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct MDPExecCore {
  virtual ~MDPExecCore(void) {}

  virtual void setToInitialState(void) = 0;
  virtual int chooseAction(void) = 0;
  virtual void advanceToNextState(int a, int o) = 0;
};

// MDPExec adds some default class members to MDPExecCore,
// makes inheritance easier in some cases.
struct MDPExec : public MDPExecCore {
  MDP *mdp;
  bool currentStateInitialized;
  belief_vector currentState;

  MDPExec(void);

  // default implementation
  bool getStateIsTerminal(void) const;

  // helpful for debugging
  state_vector &getState(state_vector &s) const;
  int getRandomOutcome(int a) const;
};

}; // namespace zmdp

#endif // INCMDPExec_h
