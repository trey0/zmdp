/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-09 21:28:24 $
 *
 * PROJECT:      Distributed Robotic Agents
 * DESCRIPTION:  
 *
 * (c) Copyright 2000 CMU. All rights reserved.
 ***************************************************************************/

#ifndef INCcommonTime_h
#define INCcommonTime_h

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

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

#endif // INCcommonTime_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
