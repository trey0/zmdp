/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-02-06 19:27:25 $
  
 @file    RaceTrack.cc
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

#include "MatrixUtils.h"
#include "Solver.h"
#include "RaceTrack.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

#define IS_INITIAL_STATE(s) (s(0) == -1)
#define IS_TERMINAL_STATE(s) (s(0) == -2)

#define RT_DEBUG_PRINT (1)

enum RTOutcomeEnum {
  RT_NORMAL = 0,
  RT_SLIP   = 1,
  RT_FINISH = 2,
  RT_CRASH  = 3,
  RT_MAX    = 4
};

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
  void readFromFile(const string& fname, char endMarker);
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

void PropertyList::readFromFile(const string& fname, char endMarker)
{
  inFile = fopen(fname.c_str(), "r");
  if (NULL == inFile) {
    fprintf(stderr, "ERROR: could not open spec file %s for reading: %s\n",
	    fname.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }

  data.clear();

  char lbuf[1024];
  char key[1024], value[1024];
  lnum = 0;
  while (!feof(inFile)) {
    fgets(lbuf, sizeof(lbuf), inFile);
    lnum++;
    if (0 == strlen(lbuf)) continue;
    if ('#' == lbuf[0]) continue;
    if (endMarker == lbuf[0]) break;
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

struct TrackMap {
  unsigned int width, height;
  unsigned char* open;
  unsigned char* finish;
  std::vector<int> startX, startY;

  TrackMap(void);
  ~TrackMap(void);
  bool getIsOpen(int x, int y) const { return open[width*y + x]; }
  bool getIsFinish(int x, int y) const { return finish[width*y + x]; }
  int lineType(int x0, int y0, int dx, int dy) const;
  void readFromFile(const std::string& mapFileName, FILE* mapFile, int lnum);
};

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
// (x0+dx,y0+dy).  Returns RT_NORMAL if the line is entirely open cells,
// RT_FINISH if the line hits a finish cell (before it hits any closed
// cells), and RT_CRASH if it hits a closed cell (before hitting any
// finish cells).  Code is a modified version of Bresenham's line
// algorithm -- it is not optimized but it is exact.
int TrackMap::lineType(int x0, int y0, int dx, int dy) const
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
#define RT_PLOT(A,B)                          \
  {                                           \
    tx = x0 + c11*A + c12*B;                  \
    ty = y0 + c21*A + c22*B;                  \
    if (getIsFinish(tx,ty)) return RT_FINISH; \
    if (!getIsOpen(tx,ty)) return RT_CRASH;   \
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

  return RT_NORMAL;
}

void TrackMap::readFromFile(const string& mapFileName, FILE* mapFile, int lnum)
{
  char lbuf[1024];
  char data[1024*1024];
  unsigned int i = 0;
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
    assert(i + width < sizeof(data));
    memcpy(&data[i], lbuf, width);
    i += width;
  }
  height = i/width;

  unsigned int numPixels = width*height;
  open = new unsigned char[numPixels];
  finish = new unsigned char[numPixels];
  for (i=0; i < numPixels; i++) {
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
 * RTLOWERBOUND STRUCTURE
 **********************************************************************/

struct RTLowerBound : public AbstractBound {
  const RaceTrack* problem;

  RTLowerBound(const RaceTrack* _problem) : problem(_problem) {}
  void initialize(void) {}
  double getValue(const state_vector& s) const;
};

double RTLowerBound::getValue(const state_vector& s) const
{
  double maxCost;
  if (IS_TERMINAL_STATE(s)) {
    maxCost = 0;
  } else {
    maxCost = 1.0 / (1 - problem->discount);
  }
#if RT_DEBUG_PRINT
  printf("getValue: s=[%s] maxCost=%g\n", denseRep(s).c_str(), maxCost);
#endif
  return -maxCost;
}

/**********************************************************************
 * RTUPPERBOUND STRUCTURE
 **********************************************************************/

struct RTUpperBound : public AbstractBound {
  const RaceTrack* problem;
  int minFinishX, maxFinishX;
  int minFinishY, maxFinishY;
  bool initialized;

  RTUpperBound(const RaceTrack* _problem) : problem(_problem), initialized(false) {}
  void initialize(void);
  double getValue(const state_vector& s) const;
};

void RTUpperBound::initialize(void)
{
  if (initialized) return;

  TrackMap* t = problem->tmap;
  minFinishX = minFinishY = 9999;
  maxFinishX = maxFinishY = -9999;
  int h = problem->tmap->height;
  int w = problem->tmap->width;
  for (int y=0; y < h; y++) {
    for (int x=0; x < w; x++) {
      if (t->getIsFinish(x,y)) {
	if (x < minFinishX) minFinishX = x;
	if (x > maxFinishX) maxFinishX = x;
	if (y < minFinishY) minFinishY = y;
	if (y > maxFinishY) maxFinishY = y;
      }
    }
  }

  initialized = true;
}

#if 0
static int getMinTime(int x, int xmin, int xmax, int v)
{
  int dist = 0;
  if (x < xmin) {
    dist = (xmin-x);
  }
  if (x > xmax) {
    dist = (x-xmax);
    v = -v;
  }
  if (0 == dist) return 0;
  return (int) (-v + sqrt(v*v + 2*dist));
}
#endif

static double getDist(int x, int xmin, int xmax)
{
  int dist = 0;
  if (x < xmin) {
    dist = (xmin-x);
  }
  if (x > xmax) {
    dist = (x-xmax);
  }
  return dist;
}

double RTUpperBound::getValue(const state_vector& s) const
{
  assert(initialized);

  double minCost;
  if (IS_INITIAL_STATE(s) || IS_TERMINAL_STATE(s)) {
    minCost = 0;
  } else {
    int x  = (int) s(0);
    int y  = (int) s(1);
    
#if 0
    int vx = (int) s(2);
    int vy = (int) s(3);
    int xtime = getMinTime(x, minFinishX, maxFinishX, vx);
    int ytime = getMinTime(y, minFinishY, maxFinishY, vy);
    int t = std::max(xtime, ytime);
    double discount = problem->discount;
    minCost = (1 - pow(discount, t)) / (1 - discount);
#endif

    double eps = 1e-3;
    double xtime = eps * getDist(x, minFinishX, maxFinishX);
    double ytime = eps * getDist(y, minFinishY, maxFinishY);
    double t = xtime + ytime;
    minCost = t;
  }

#if RT_DEBUG_PRINT
  printf("getValue: s=[%s] minCost=%g\n", denseRep(s).c_str(), minCost);
#endif
  return -minCost;
  //return 0;
}

/**********************************************************************
 * MAIN FUNCTIONS
 **********************************************************************/

RaceTrack::RaceTrack(const std::string& specFileName)
{
  tmap = NULL;
  readFromFile(specFileName);
}

RaceTrack::~RaceTrack(void)
{
  if (NULL != tmap) delete tmap;
}

void RaceTrack::readFromFile(const std::string& specFileName)
{
  numStateDimensions = 4;
  numActions = 9;

  bogusInitialState.resize(4);
  bogusInitialState.push_back(0, -1);
  terminalState.resize(4);
  terminalState.push_back(0, -2);

  PropertyList plist;
  plist.readFromFile(specFileName, /* endMarker = */ '-');
  discount = atof(plist.getValue("discount").c_str());
  errorProbability = atof(plist.getValue("errorProbability").c_str());

  tmap = new TrackMap();
  tmap->readFromFile(specFileName, plist.inFile, plist.lnum);
  fclose(plist.inFile);
}

const state_vector& RaceTrack::getInitialState(void) const
{
  return bogusInitialState;
}

bool RaceTrack::getIsTerminalState(const state_vector& s) const
{
  bool isTerminal = IS_TERMINAL_STATE(s);
  return isTerminal;
}

outcome_prob_vector& RaceTrack::getOutcomeProbVector(outcome_prob_vector& result,
						     const state_vector& s, int a) const
{
  if (IS_INITIAL_STATE(s)) {
    // transition to one of the starting line cells (uniform probability distribution)
    int n = tmap->startX.size();
    double p = 1.0 / n;
    result.resize(n);
    for (int i=0; i < n; i++) {
      result(i) = p;
    }
  } else if (IS_TERMINAL_STATE(s)) {
    // terminal state: self-transition with probability 1
    result.resize(1);
    result(0) = 1;
  } else {
    result.resize(RT_MAX);
    result(RT_NORMAL) = 1-errorProbability;
    result(RT_SLIP)   = errorProbability;
    result(RT_FINISH) = 0;
    result(RT_CRASH)  = 0;
    
    int oldX =  (int) s(0);
    int oldY =  (int) s(1);
    int oldVX = (int) s(2);
    int oldVY = (int) s(3);
    int ax = (a / 3) - 1;
    int ay = (a % 3) - 1;

    // if the commanded acceleration was 0, outcomes RT_NORMAL and
    // RT_SLIP are the same: combine their probabilities
    if (0 == ax && 0 == ay) {
      result(RT_NORMAL) += result(RT_SLIP);
      result(RT_SLIP) = 0;
    }
    
    // the rest of the code determines if either the normal or slip case
    // causes a crash or successful finish... if so, move probability
    // mass from that outcome to outcome RT_FINISH or RT_CRASH.
#define RT_CHECK_OUTCOME(WHICH_OUTCOME,AX,AY)                      \
    int lineType = tmap->lineType(oldX, oldY, oldVX+AX, oldVY+AY); \
    if (RT_NORMAL != lineType) {                                   \
      result(lineType) += result(WHICH_OUTCOME);                   \
      result(WHICH_OUTCOME) = 0;                                   \
    }

    RT_CHECK_OUTCOME(RT_NORMAL, ax, ay);
    if (result(RT_SLIP) > 0.0) {
      RT_CHECK_OUTCOME(RT_SLIP, 0, 0);
    }
  }

#if RT_DEBUG_PRINT
  printf("getOutcomeProbVector: s=[%s] a=%d result=[%s]\n", denseRep(s).c_str(), a, denseRep(result).c_str());
#endif

  return result;
}

state_vector& RaceTrack::getNextState(state_vector& result, const state_vector& s,
				      int a, int o) const
{
  if (IS_INITIAL_STATE(s)) {
    // transition to one of the starting line cells; which one indicated by o
    result.resize(4);
    result.push_back(0, tmap->startX[o]);
    result.push_back(1, tmap->startY[o]);
  } else if (IS_TERMINAL_STATE(s)) {
    // terminal state: self-transition with probability 1
    result = s;
  } else {
    switch (o) {
    case RT_FINISH:
      // 'finish' outcome: transition to terminal state
      result = terminalState;
      break;
    case RT_CRASH:
      // 'crash' outcome: reset to bogus initial state
      result = bogusInitialState;
      break;
    default:
      int ax = (a / 3) - 1;
      int ay = (a % 3) - 1;
      sla::dvector tmp(4);
      if (RT_NORMAL == o) { // normal case
	tmp(2) = s(2) + ax;
	tmp(3) = s(3) + ay;
      } else {              // 'slip' case (acceleration command failed)
	tmp(2) = s(2);
	tmp(3) = s(3);
      }
      tmp(0) = s(0) + tmp(2);
      tmp(1) = s(1) + tmp(3);
      copy(result, tmp);
    }
  }

#if RT_DEBUG_PRINT
  printf("s=[%s] a=%d o=%d sp=[%s] r=%f\n", denseRep(s).c_str(), a, o,
	 denseRep(result).c_str(), getReward(s,a));
#endif

  return result;
}

double RaceTrack::getReward(const state_vector& s, int a) const
{
  double cost;
  if (IS_TERMINAL_STATE(s)) {
    // terminal state: model as self-transition with no cost
    cost = 0;
  }
#if 0
  else if (IS_INITIAL_STATE(s)) {
    // the 'first move' to the starting line is bogus, no cost
    cost = 0;
  }
#endif
  else {
    // uniform cost for all real moves
    cost = 1;
  }
#if RT_DEBUG_PRINT
  printf("getReward: cost=%g\n", cost);
#endif

  // reward = -cost
  return -cost;
}

AbstractBound* RaceTrack::newLowerBound(void) const
{
  return new RTLowerBound(this);
}

AbstractBound* RaceTrack::newUpperBound(void) const
{
  return new RTUpperBound(this);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/01 18:03:14  trey
 * fixed compile-time errors, not quite done yet
 *
 * Revision 1.1  2006/01/31 18:12:29  trey
 * initial check-in in mdps directory
 *
 * Revision 1.1  2006/01/30 23:50:02  trey
 * initial check-in
 *
 *
 ***************************************************************************/
