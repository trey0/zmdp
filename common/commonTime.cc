/****************************************************************************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2004-11-09 21:28:24 $
 *
 * PROJECT:      Distributed Robotic Agents
 * DESCRIPTION:  
 *
 * (c) Copyright 2000 CMU. All rights reserved.
 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "commonTime.h"

// fsleep: like sleep, but can handle a non-integer number of seconds to
//   wait
void
fsleep(double seconds) {
  timeval tv;
  int tfloor = (int) seconds;
  tv.tv_sec = tfloor;
  tv.tv_usec = (int) ((seconds - tfloor) * 1.0e+6);

  int activeFds = select(0,0,0,0,&tv);
  assert(activeFds != -1); // select returns -1 on error
}

void
StopWatch::restart(void) {
  gettimeofday(&startTime,0);
}

double
StopWatch::elapsedTime(void) {
  timeval tv;
  gettimeofday(&tv,0);
  return (tv.tv_sec - startTime.tv_sec)
    + (tv.tv_usec - startTime.tv_usec)*1.0e-6;
}

struct timeval
secondsToTimeval(double d) {
  timeval a;
  a.tv_sec = (int) d;
  a.tv_usec = (int) (1e+6 * (d-a.tv_sec));
  return a;
}

double
timevalToSeconds(const timeval &tv) {
  return tv.tv_sec + 1e-6*tv.tv_usec;
}

timeval
operator -(const timeval &a, const timeval &b) {
  timeval result;
  result.tv_sec = a.tv_sec - b.tv_sec;
  result.tv_usec = a.tv_usec - b.tv_usec;
  if (result.tv_usec < 0) {
    result.tv_sec--;
    result.tv_usec += 1000000;
  }
  return result;
}

timeval
operator +(const timeval &a, const timeval &b) {
  timeval result;
  result.tv_sec = a.tv_sec + b.tv_sec;
  result.tv_usec = a.tv_usec + b.tv_usec;
  if (result.tv_usec > 1000000) {
    result.tv_sec++;
    result.tv_usec -= 1000000;
  }
  return result;
}

bool
operator <(const timeval &a, const timeval &b) {
  return (a.tv_sec < b.tv_sec)
    || ((a.tv_sec == b.tv_sec) && (a.tv_usec < b.tv_usec));
}

timeval
getTime(void) {
  timeval tv;
  gettimeofday(&tv, 0);
  return tv;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2003/03/05 22:37:59  danig
 * added getTime() function
 *
 * Revision 1.1  2002/10/16 18:47:15  trey
 * initial check-in (import from DIRA)
 *
 * Revision 1.4  2000/12/06 04:47:28  trey
 * added some new functions for use in EventTracker
 *
 * Revision 1.3  2000/11/28 21:46:21  trey
 * added subtraction operator for timeval structs
 *
 * Revision 1.2  2000/09/29 05:52:10  trey
 * added ability to check if StopWatch has been started yet
 *
 * Revision 1.1  2000/09/25 17:23:27  trey
 * made commonTime a C++ file and added StopWatch object definition
 *
 * Revision 1.1  2000/08/28 17:29:52  trey
 * initial check-in
 *
 *
 ***************************************************************************/
