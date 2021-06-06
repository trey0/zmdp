/****************************************************************************
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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include "zmdpCommonTime.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

namespace zmdp {

// fsleep: like sleep, but can handle a non-integer number of seconds to
//   wait
void fsleep(double seconds) {
  timeval tv;
  int tfloor = static_cast<int>(seconds);
  tv.tv_sec = tfloor;
  tv.tv_usec = static_cast<int>((seconds - tfloor) * 1.0e+6);

  int activeFds = select(0, 0, 0, 0, &tv);
  assert(activeFds != -1);  // select returns -1 on error
}

void StopWatch::restart(void) { gettimeofday(&startTime, 0); }

double StopWatch::elapsedTime(void) {
  timeval tv;
  gettimeofday(&tv, 0);
  return (tv.tv_sec - startTime.tv_sec) +
         (tv.tv_usec - startTime.tv_usec) * 1.0e-6;
}

struct timeval secondsToTimeval(double d) {
  timeval a;
  a.tv_sec = static_cast<int>(d);
  a.tv_usec = static_cast<int>(1e+6 * (d - a.tv_sec));
  return a;
}

double timevalToSeconds(const timeval &tv) {
  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

timeval operator-(const timeval &a, const timeval &b) {
  timeval result;
  result.tv_sec = a.tv_sec - b.tv_sec;
  result.tv_usec = a.tv_usec - b.tv_usec;
  if (result.tv_usec < 0) {
    result.tv_sec--;
    result.tv_usec += 1000000;
  }
  return result;
}

timeval operator+(const timeval &a, const timeval &b) {
  timeval result;
  result.tv_sec = a.tv_sec + b.tv_sec;
  result.tv_usec = a.tv_usec + b.tv_usec;
  if (result.tv_usec > 1000000) {
    result.tv_sec++;
    result.tv_usec -= 1000000;
  }
  return result;
}

bool operator<(const timeval &a, const timeval &b) {
  return (a.tv_sec < b.tv_sec) ||
         ((a.tv_sec == b.tv_sec) && (a.tv_usec < b.tv_usec));
}

timeval getTime(void) {
  timeval tv;
  gettimeofday(&tv, 0);
  return tv;
}

};  // namespace zmdp
