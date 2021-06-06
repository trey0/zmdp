/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef INCCassandraParser_h
#define INCCassandraParser_h

#include <iostream>
#include <string>
#include <vector>

#include "CassandraModel.h"

using namespace sla;

namespace zmdp {

struct CassandraParser {
  void readGenericDiscreteMDPFromFile(CassandraModel &mdp,
                                      const std::string &fileName);
  void readPomdpFromFile(CassandraModel &pomdp, const std::string &fileName);

protected:
  void readModelFromFile(CassandraModel &problem, bool expectPomdp);
};

}; // namespace zmdp

#endif // INCCassandraParser_h
