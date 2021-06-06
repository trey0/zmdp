/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2002-2005, Carnegie Mellon University. All rights reserved.

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

#ifndef INCzmdpCommonTime_h
#define INCzmdpCommonTime_h

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace zmdp {

void fsleep(double seconds);

class StopWatch {
public:
  StopWatch(void) { restart(); }
  void restart(void);
  double elapsedTime(void);

protected:
  struct timeval startTime;
};

struct timeval secondsToTimeval(double d);
double timevalToSeconds(const timeval &tv);
timeval operator-(const timeval &a, const timeval &b);
timeval operator+(const timeval &a, const timeval &b);
bool operator<(const timeval &a, const timeval &b);
timeval getTime(void);

}; // namespace zmdp

#endif // INCzmdpCommonTime_h
