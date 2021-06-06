/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2006, Trey Smith.

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

#ifndef INCStateLog_h
#define INCStateLog_h

#include <iostream>
#include <string>
#include <vector>

#include "BoundPairCore.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

using namespace sla;

namespace zmdp {

struct StateIndex {
  int numStateDimensions;
  std::vector<state_vector *> entries;
  EXT_NAMESPACE::hash_map<std::string, int> lookup;

  StateIndex(int _numStateDimensions);
  ~StateIndex(void);
  int getStateId(const state_vector &s);
  void writeToFile(const std::string &outFile) const;
  void readFromFile(const std::string &inFile);

  void writeBoundValuesToFile(const std::string &outFile,
                              const BoundPairCore &bounds) const;
  void writeQValuesToFile(const std::string &outFile,
                          const BoundPairCore &bounds, int numActions) const;
};

struct StateLog {
  std::vector<int> entries;
  StateIndex *index;

  StateLog(StateIndex *_index);
  void addState(const state_vector &s);
  void writeToFile(const std::string &outFile) const;

  void readFromFile(const std::string &inFile);
  size_t size(void) const { return entries.size(); }
  int getLogEntry(int i) const;
};

}; // namespace zmdp

#endif // INCStateLog_h
