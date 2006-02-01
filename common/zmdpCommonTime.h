/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2006-02-01 01:09:37 $

 Copyright (c) 2002-2005, Carnegie Mellon University
 All rights reserved.

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
timeval operator -(const timeval &a, const timeval &b);
timeval operator +(const timeval &a, const timeval &b);
bool operator <(const timeval &a, const timeval &b);
timeval getTime(void);

}; // namespace zmdp

#endif // INCzmdpCommonTime_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.2  2005/10/28 02:51:40  trey
 * added copyright headers
 *
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
