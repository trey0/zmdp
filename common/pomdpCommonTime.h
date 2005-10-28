/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-10-28 02:51:40 $

 Copyright (c) 1996-2005, Carnegie Mellon University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * This code and derivatives may not be incorporated into commercial
   products without specific prior written permission.
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of Carnegie Mellon University nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ***************************************************************************/

#ifndef INCpomdpCommonTime_h
#define INCpomdpCommonTime_h

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace pomdp {

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
timeval operator -(const timeval &a, const timeval &b);
timeval operator +(const timeval &a, const timeval &b);
bool operator <(const timeval &a, const timeval &b);
timeval getTime(void);

}; // namespace pomdp

#endif // INCpomdpCommonTime_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/10/21 20:11:52  trey
 * renamed to avoid potential conflicts with other projects
 *
 * Revision 1.1  2004/11/09 21:28:24  trey
 * check-in to pomdp repository
 *
 * Revision 1.2  2003/03/05 22:37:59  danig
 * added getTime() function
 *
 * Revision 1.1  2002/10/16 18:47:16  trey
 * initial check-in (import from DIRA)
 *
 * Revision 1.8  2001/03/11 02:21:48  hersh
 * Added stop() function to StopWatch class. :-)
 *
 * Revision 1.7  2000/12/11 22:22:11  trey
 * trimmed extra blank lines from end of file
 *
 * Revision 1.4  2000/11/28 21:46:20  trey
 * added subtraction operator for timeval structs
 *
 * Revision 1.3  2000/09/29 05:52:10  trey
 * added ability to check if StopWatch has been started yet
 *
 * Revision 1.2  2000/09/25 17:23:37  trey
 * made commonTime a C++ file and added StopWatch object definition
 *
 * Revision 1.1  2000/08/28 17:29:53  trey
 * initial check-in
 *
 *
 ***************************************************************************/
