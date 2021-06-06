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

#ifndef INCRTDP_h
#define INCRTDP_h

#include "RTDPCore.h"

namespace zmdp {

struct RTDP : public RTDPCore {
  RTDP(void);

  void trialRecurse(MDPNode &cn, int depth);
  bool doTrial(MDPNode &cn);
};

}; // namespace zmdp

#endif /* INCRTDP_h */
