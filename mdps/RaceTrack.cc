/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-02-01 18:03:14 $
  
 @file    racetrack.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <getopt.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <map>

#include "Solver.h"
#include "RaceTrack.h"

using namespace std;

namespace zmdp {

/**********************************************************************
 * PROPERTYLIST DATA STRUCTURE
 **********************************************************************/

struct PropertyList {
  map<string, string> data;
  FILE* inFile;
  int lnum;

  PropertyList(void);
  void setValue(const string& key, const string& value);
  const string& getValue(const string& key) const;
  void readFromFile(const string& fname);
};

PropertyList::PropertyList(void)
{
  inFile = NULL;
}

void PropertyList::setValue(const string& key, const string& value)
{
  data[key] = value;
}

const string& PropertyList::getValue(const string& key) const
{
  typeof(data.begin()) p;
  if (data.end() == (p = data.find(key))) {
    fprintf(stderr, "ERROR: value for spec file parameter %s not found\n",
	    key.c_str());
    exit(EXIT_FAILURE);
  }
  return p->second;
}

void PropertyList::readFromFile(const string& fname)
{
  FILE* f = fopen(fname.c_str(), "rb");
  if (NULL == f) {
    fprintf(stderr, "ERROR: could not open spec file %s for reading: %s\n",
	    fname.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }

  data.clear();

  char lbuf[1024];
  char key[1024], value[1024];
  lnum = 0;
  while (!feof(f)) {
    fgets(lbuf, sizeof(lbuf), f);
    lnum++;
    if (0 == strlen(lbuf)) continue;
    if ('#' == lbuf[0]) continue;
    if ('-' == lbuf[0]) break;
    if (2 != sscanf(lbuf, "%s %s", key, value)) {
      fprintf(stderr, "ERROR: %s: line %d: syntax error, expected '<key> <value>'\n",
	      fname.c_str(), lnum);
      exit(EXIT_FAILURE);
    }
    setValue(key, value);
  }
}

/**********************************************************************
 * TRACKMAP DATA STRUCTURE
 **********************************************************************/

TrackMap::TrackMap(void)
{
  open = NULL;
  finish = NULL;
}

TrackMap::~TrackMap(void)
{
  if (NULL != open) delete[] open;
  if (NULL != finish) delete[] finish;
}

// Plots a line from the center of cell (x0,y0) to the center of cell
// (x0+dx,y0+dy).  Returns false iff the line crosses a closed cell
// before it crosses a finish line cell.  Code is a modified version of
// Bresenham's line algorithm -- it is not optimized but it is exact.
bool TrackMap::lineIsOpen(int x0, int y0, int dx, int dy) const
{
  // Transform (dx,dy) into (adx,ady) so that 0 <= ady <= adx as
  // required for the plotting loop. The matrix C inverts the transform.
  int adx = (dx < 0) ? (-dx) : dx;
  int ady = (dy < 0) ? (-dy) : dy;
  int c11 = (dx < 0) ? -1 : 1;
  int c12 = 0;
  int c22 = (dy < 0) ? -1 : 1;
  int c21 = 0;
  if (ady > adx) {
    std::swap(adx,ady);
    std::swap(c11,c12);
    std::swap(c21,c22);
  }

  // Set up what it means to 'plot' a cell.
  int tx, ty;
#define RT_PLOT(A,B)                     \
  {                                      \
    tx = x0 + c11*A + c12*B;             \
    ty = y0 + c21*A + c22*B;             \
    if (getIsFinish(tx,ty)) return true; \
    if (!getIsOpen(tx,ty)) return false; \
  }

  // Plot the line.
  RT_PLOT(0,0);
  int x = 1;
  int y = 0;
  int eps = ady;
  while (x < adx) {
    if (eps < adx) RT_PLOT(x,y);
    eps += 2*ady;
    if (eps >= adx) {
      y++;
      eps -= 2*adx;
      if (eps > -adx) RT_PLOT(x,y);
    }
    x++;
  }
  RT_PLOT(adx,ady);

  return true;
}

void TrackMap::readFromFile(const string& mapFileName, FILE* mapFile, int lnum)
{
  char lbuf[1024];
  char data[1024*1024];
  char* datap = data;
  width = 0;
  while (!feof(mapFile)) {
    fgets(lbuf, sizeof(lbuf), mapFile);
    lnum++;
    if (0 == strlen(lbuf)) continue;
    if ('#' == lbuf[0]) continue;
    if (0 == width) {
      width = strlen(lbuf);
    } else {
      if (width != strlen(lbuf)) {
	fprintf(stderr, "ERROR: %s: line %d: map line lengths don't match (delete trailing whitespace?)\n",
		mapFileName.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
    }
    assert(datap + width < &data[sizeof(data)]);
    memcpy(datap, lbuf, width);
    datap += width;
  }
  height = (datap-data)/width;

  unsigned int numPixels = width*height;
  open = new unsigned char[numPixels];
  finish = new unsigned char[numPixels];
  for (unsigned int i=0; i < numPixels; i++) {
    char c = data[i];
    open[i] = '@' != c;
    finish[i] = 'f' == c;
    if ('s' == c) {
      startX.push_back(i % width);
      startY.push_back(i / width);
    }
  }
}

/**********************************************************************
 * MAIN FUNCTIONS
 **********************************************************************/

#define IS_BOGUS_INITIAL_STATE(s) (s(0) == -1)

RaceTrack::RaceTrack(const std::string& specFileName)
{
  readFromFile(specFileName);
}

void RaceTrack::readFromFile(const std::string& specFileName)
{
  PropertyList plist;
  plist.readFromFile(specFileName);
  errorProbability = atof(plist.getValue("errorProbability").c_str());

  tmap.readFromFile(specFileName, plist.inFile, plist.lnum);
  fclose(plist.inFile);
}

const state_vector& RaceTrack::getInitialState(void) const
{
  // return bogus initial state
  static state_vector s(4);
  s.push_back(0,-1);
  return s;
}

bool RaceTrack::getIsTerminalState(const state_vector& s) const
{
  return tmap.getIsFinish((int) s(0), (int) s(1));
}

outcome_prob_vector& RaceTrack::getOutcomeProbVector(outcome_prob_vector& result,
						     const state_vector& s, int a) const
{
  if (IS_BOGUS_INITIAL_STATE(s)) {
    // transition to one of the starting line cells (uniform probability distribution)
    int n = tmap.startX.size();
    double p = 1.0 / n;
    result.resize(n);
    for (int i=0; i < n; i++) {
      result(i) = p;
    }
  }

  // non-crash case: transition either obeying the commanded
  // acceleration or not
  result.resize(3);
  result(0) = 1-errorProbability;
  result(1) = errorProbability;
  result(2) = 0;

  // if the commanded acceleration was 0, outcomes 0 and 1 are the same;
  // combine the probabilities
  int ax = (a / 3) - 1;
  int ay = (a % 3) - 1;
  if (0 == ax && 0 == ay) {
    result(0) += result(1);
    result(1) = 0;
  }

  // the rest of the code determines if either of the possible outcome
  // positions causes a crash... if so, move probability mass from that
  // outcome to outcome 2 (the 'crash' outcome), which transitions to
  // the bogus initial state.
  int oldX =  (int) s(0);
  int oldY =  (int) s(1);
  int oldVX = (int) s(2);
  int oldVY = (int) s(3);

#define RT_CHECK_CRASH(AX,AY,WHICH_OUTCOME)                \
  if (!tmap.lineIsOpen(oldX, oldY, oldVX+AX, oldVY+AY)) {  \
    result(2) += result(WHICH_OUTCOME);                    \
    result(WHICH_OUTCOME) = 0;                             \
  }

  RT_CHECK_CRASH(ax, ay, 0);
  if (result(1) > 0.0) {
    RT_CHECK_CRASH(0, 0, 1);
  }

  return result;
}

state_vector& RaceTrack::getNextState(state_vector& result, const state_vector& s,
				      int a, int o) const
{
  if (IS_BOGUS_INITIAL_STATE(s)) {
    // transition to one of the starting line cells; which one indicated by o
    result.resize(4);
    result.push_back(0, tmap.startX[o]);
    result.push_back(1, tmap.startY[o]);
  }

  if (2 == o) {
    // 'crash' outcome: transition to bogus initial state
    result.resize(4);
    result.push_back(0,-1);
  } else {
    int ax = (a / 3) - 1;
    int ay = (a % 3) - 1;
    sla::dvector tmp(4);
    if (o == 0) { // commanded acceleration succeeded
      tmp(2) = s(2) + ax;
      tmp(3) = s(3) + ay;
    } else {      // commanded acceleration failed
      tmp(2) = s(2);
      tmp(3) = s(3);
    }
    tmp(0) = s(0) + tmp(2);
    tmp(1) = s(1) + tmp(3);
    copy(result, tmp);
  }

  return result;
}

double RaceTrack::getReward(const state_vector& s, int a) const
{
  if (IS_BOGUS_INITIAL_STATE(s)) {
    // the 'first move' to the starting line is bogus, no cost
    return 0;
  }

  // uniform cost (negative reward) for all real moves
  return -1;
}

AbstractBound* RaceTrack::newLowerBound(void) const
{
  return NULL; // FIX implement me
}

AbstractBound* RaceTrack::newUpperBound(void) const
{
  return NULL; // FIX implement me
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/01/31 18:12:29  trey
 * initial check-in in mdps directory
 *
 * Revision 1.1  2006/01/30 23:50:02  trey
 * initial check-in
 *
 *
 ***************************************************************************/
