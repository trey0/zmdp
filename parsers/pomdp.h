/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1.1.1 $  $Author: trey $  $Date: 2004-11-09 16:18:56 $
 *  
 * PROJECT: FIRE Architecture Project
 *
 * @file    spec.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCspec_h
#define INCspec_h

using namespace std;

#include <string>

// Exception class
class InputError { };

typedef int StateType;
typedef int ActionType;
typedef int ObsType;
typedef double ProbType;
typedef double ValueType;

struct Pomdp {
  Pomdp(void) {}

  int getNumStates(void) const;
  int getNumActions(void) const;
  int getNumObservations(void) const;
  ValueType getDiscount(void) const;
  ValueType getInitialBelief(StateType s) const;
  bool isTerminalState(StateType s) const;

  ValueType R(StateType s, ActionType a) const;
  ProbType T(StateType s, ActionType a, StateType sp) const;
  ProbType O(StateType sp, ActionType a, ObsType o) const;

#ifdef SWIG
  // SWIG only understands char * as equivalent to python strings
  void readFromFile(const char *fileName) { readFromFile(string(fileName)); }
#else
  void readFromFile(const string& fileName);
#endif
};


#endif // INCspec_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2003/07/16 16:12:44  trey
 * added support for terminal states
 *
 * Revision 1.2  2003/04/02 17:13:56  trey
 * minor changes to enable compilation under gcc 3.0.1
 *
 * Revision 1.1.1.1  2003/01/07 19:19:41  trey
 * Imported sources
 *
 *
 ***************************************************************************/
