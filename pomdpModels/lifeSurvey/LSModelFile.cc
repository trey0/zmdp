/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.11 $  $Author: trey $  $Date: 2006-07-10 02:22:26 $
   
 @file    LSModelFile.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

#include "LSModelFile.h"
#include "zmdpCommonDefs.h"

using namespace std;

namespace zmdp {

/**********************************************************************
 * DATA STRUCTURES
 **********************************************************************/

typedef std::map<std::string, std::string> ParamLookup;

/**********************************************************************
 * LOCAL HELPER FUNCTIONS
 **********************************************************************/

static void convertToDoubleVector(std::vector<double>& result,
				  const std::string& s,
				  int doNormalizeVector)
{
  string::size_type p1, p2;
  p1 = 0;
  while (1) {
    p2 = s.find_first_of(" \t", p1);
    if (string::npos == p2) {
      result.push_back(atof(s.substr(p1).c_str()));
      break;
    } else {
      if (p1 != p2) {
	result.push_back(atof(s.substr(p1,(p2-p1)).c_str()));
      }
      p1 = p2+1;
    }
  }

  if (doNormalizeVector) {
    double sum = 0.0;
    FOR_EACH (vp, result) {
      sum += *vp;
    }
    FOR_EACH (vp, result) {
      *vp /= sum;
    }
  }
}

static void writeDoubleVectorToFile(FILE* outFile,
				    const char* header,
				    const std::vector<double>& vec)
{
  fprintf(outFile, "%s ", header);
  FOR_EACH (dp, vec) {
    fprintf(outFile, "%lf ", *dp);
  }
  fprintf(outFile, "\n");
}

static std::string getVal(const ParamLookup& params,
			  const std::string& paramName)
{
  typeof(params.begin()) i = params.find(paramName);
  if (params.end() == i) {
    fprintf(stderr, "ERROR: LSModelFile: preamble does not specify a value for parameter '%s'\n",
	    paramName.c_str());
    exit(EXIT_FAILURE);
  } else {
    return i->second;
  }
}

static std::string parseRow(const char* inRow,
			    int isOdd,
			    const std::string& fname,
			    int lnum)
{
  int n = strlen(inRow);
  std::string result(n/2+1, ' ');

  FOR (inIndex, n) {
    char c = inRow[inIndex];
    if (0 == ((isOdd + inIndex) % 2)) {
      /* even position */
      result[inIndex / 2] = c;
    } else {
      /* odd position -- expect a space character */
      if (' ' != c) {
	fprintf(stderr, "ERROR: %s: line %d: found an unexpected non-space character, map should have a checkerboard structure\n",
		fname.c_str(), lnum);
	printf("isOdd=%d inIndex=%d\n", isOdd, inIndex);
	exit(EXIT_FAILURE);
      }
    }
  }

  return result;
}

/**********************************************************************
 * LSGRID FUNCTIONS
 **********************************************************************/

LSGrid::LSGrid(void) :
  width(0),
  height(0)
{}

unsigned char LSGrid::getCellBounded(const LSPos& pos) const
{
  if (0 <= pos.x && pos.x < ((int)width) && 0 <= pos.y && pos.y < ((int)height)) {
    return getCell(pos);
  } else {
    return LS_OBSTACLE;
  }
}

bool LSGrid::getAtExit(const LSPos& pos) const
{
  /* exiting is legal if there are no non-obstacle cells further to the
     east of this cell in the same row */
  for (int x=pos.x+1; x < (int)width; x++) {
    if (LS_OBSTACLE != getCell(LSPos(x, pos.y))) {
      return false;
    }
  }
  return true;
}

void LSGrid::writeToFile(FILE* outFile, bool showCoords, bool binaryMap) const
{
  int y;
  if (showCoords) {
    y = height;
    fprintf(outFile, "  ");
    if ((y % 2) == 1) fputc(' ', outFile);
    for (unsigned int x=(y+1)/2; x < width; x++) {
      fprintf(outFile, "%d ", x%10);
    }
    fprintf(outFile, "\n");
  }
  FOR (y0, height) {
    y = height - y0 - 1;
    if (showCoords) {
      fprintf(outFile, "%d ", y%10);
    }
    if ((y % 2) == 1) fputc(' ', outFile);
    for (unsigned int x=(y+1)/2; x < width; x++) {
      unsigned char cell = getCell(LSPos(x,y));
      char outCell;
      if (LS_OBSTACLE == cell) {
	outCell = ' ';
      } else {
	if (binaryMap && ((0 == cell) || (1 == cell))) {
	  if (cell) {
	    outCell = 'O';
	  } else {
	    outCell = '*';
	  }
	} else {
	  outCell = cell + 97;
	}
      }
      fputc(outCell, outFile);
      fputc(' ', outFile);
    }
    if (showCoords) {
      fprintf(outFile, " %d", y%10);
    }
    fputc('\n', outFile);
  }
  if (showCoords) {
    fprintf(outFile, "   ");
    for (unsigned int x=0; x < width; x++) {
      fprintf(outFile, "%d ", x%10);
    }
    fprintf(outFile, "\n");
  }
}

int LSGrid::getMaxCellValue(void)
{
  int maxValue = 0;
  FOR (y, height) {
    FOR (x, width) {
      int cell = getCell(LSPos(x,y));
      if (LS_OBSTACLE != cell) {
	maxValue = std::max(cell, maxValue);
      }
    }
  }
  maxValue++;
  return maxValue;
}

/**********************************************************************
 * LSMODEL FUNCTIONS
 **********************************************************************/

LSModelFile::LSModelFile(void) :
  startX(-1),
  startY(-1)
{}

void LSModelFile::readFromFile(const std::string& fname)
{
  FILE* modelFile = fopen(fname.c_str(), "r");
  if (NULL == modelFile) {
    fprintf(stderr, "ERROR: LSModelFile: couldn't open %s for reading: %s\n", fname.c_str(),
	    strerror(errno));
    exit(EXIT_FAILURE);
  }

  char lbuf[1024];
  char key[1024], value[1024];
  int lnum;

  /* read parameters from preamble */
  ParamLookup params;
  lnum = 0;
  while (NULL != fgets(lbuf, sizeof(lbuf), modelFile)) {
    lnum++;
    if (strlen(lbuf) <= 1) continue;
    if ('#' == lbuf[0]) continue;
    if ('-' == lbuf[0]) goto preambleDone;
    lbuf[strlen(lbuf)-1] = '\0'; // truncate newline
    if (2 != sscanf(lbuf, "%s %[-0-9.eE \t]", key, value)) {
      fprintf(stderr, "ERROR: %s: line %d: syntax error, expected '<key> <value>'\n",
	      fname.c_str(), lnum);
      exit(EXIT_FAILURE);
    }
    params[key] = value;
  }
  fprintf(stderr, "ERROR: %s: reached EOF while still parsing preamble\n",
	  fname.c_str());
  exit(EXIT_FAILURE);
 preambleDone:

  /* store parameters */
  startX = atoi(getVal(params, "startX").c_str());
  startY = atoi(getVal(params, "startY").c_str());
  baseCost = atof(getVal(params, "baseCost").c_str());
  lookaheadCost = atof(getVal(params, "lookaheadCost").c_str());
  convertToDoubleVector(regionPriors,
			getVal(params, "regionPriors"),
			/* doNormalizeVector = */ 0);
  convertToDoubleVector(obsDistributionLifeAbsent,
			getVal(params, "obsDistributionLifeAbsent"),
			/* doNormalizeVector = */ 1);
  convertToDoubleVector(obsDistributionLifePresent,
			getVal(params, "obsDistributionLifePresent"),
			/* doNormalizeVector = */ 1);
  if ((obsDistributionLifeAbsent.size() != LS_BASE_NUM_OBSERVATIONS)
      || (obsDistributionLifePresent.size() != LS_BASE_NUM_OBSERVATIONS)) {
    fprintf(stderr, "ERROR: %s: obsDistributionLife{Absent,Present} vectors must have %d entries each\n",
	    fname.c_str(), LS_BASE_NUM_OBSERVATIONS);
    exit(EXIT_FAILURE);
  }

  /* read map data into intermediate data structure */
  std::vector<std::string> rows;
  while (NULL != fgets(lbuf, sizeof(lbuf), modelFile)) {
    lnum++;
    if (strlen(lbuf) <= 1) {
      if (feof(modelFile)) {
	break;
      } else {
	fprintf(stderr, "ERROR: %s: line %d: found unexpected zero-length line in map\n",
		fname.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
    }
    lbuf[strlen(lbuf)-1] = '\0'; // truncate trailing newline
    rows.push_back(parseRow(lbuf, rows.size() % 2, fname, lnum));
  }

  /* calculate dimensions of map */
  int maxRowWidth = 0;
  FOR (y0, rows.size()) {
    int y = rows.size() - y0 - 1;
    maxRowWidth = std::max(maxRowWidth, (int) ((y+1)/2 + rows[y0].size()));
  }

  /* allocate final map data structure */
  grid.width = maxRowWidth;
  grid.height = rows.size();
  grid.data.resize(grid.width * grid.height, LS_OBSTACLE);

  /* convert from immediate to final data structure */
  FOR (y0, rows.size()) {
    int y = rows.size() - y0 - 1;
    FOR (x0, rows[y0].size()) {
      int x = (y+1)/2 + x0;
      char c = rows[y0][x0];
      unsigned char cell;
      if (' ' == c) {
	cell = LS_OBSTACLE;
      } else if (97 <= c && c <= 122) {
	cell = c-97;
      } else {
	fprintf(stderr, "ERROR: %s: while parsing map, found character '%c', expected space or a-z.\n",
		fname.c_str(), c);
	exit(EXIT_FAILURE);
      }
      grid.setCell(LSPos(x, y), cell);
    }
  }

  if ((int)regionPriors.size() != grid.getMaxCellValue()) {
    int n = grid.getMaxCellValue();
    fprintf(stderr, "ERROR: %s: number of regionPriors (%d) should match number of regions in map (a-%c=%d)\n",
	    fname.c_str(), (int)regionPriors.size(), (char) ((n-1)+97), n);
    exit(EXIT_FAILURE);
  }
}

void LSModelFile::writeToFile(FILE* outFile) const
{
  fprintf(outFile, "startX %d\n", startX);
  fprintf(outFile, "startY %d\n", startY);
  fprintf(outFile, "baseCost %lf\n", baseCost);
  writeDoubleVectorToFile(outFile, "regionPriors", regionPriors);
  writeDoubleVectorToFile(outFile, "obsDistributionLifeAbsent", obsDistributionLifeAbsent);
  writeDoubleVectorToFile(outFile, "obsDistributionLifePresent", obsDistributionLifePresent);
  fprintf(outFile, "---\n");
  grid.writeToFile(outFile);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2006/07/03 21:20:04  trey
 * added lookaheadCost parameter
 *
 * Revision 1.9  2006/06/27 16:04:39  trey
 * refactored so outside code can access the LifeSurvey model using -lzmdpLifeSurvey
 *
 * Revision 1.9  2006/06/26 21:33:52  trey
 * put everything in zmdp namespace
 *
 * Revision 1.8  2006/06/20 01:22:25  trey
 * now normalize obsDistribution parameters
 *
 * Revision 1.7  2006/06/16 14:45:20  trey
 * added ability to generate target map
 *
 * Revision 1.6  2006/06/13 18:28:34  trey
 * added baseCost parameter
 *
 * Revision 1.5  2006/06/13 14:43:14  trey
 * renamed getExitLegal() -> getAtExit()
 *
 * Revision 1.4  2006/06/13 01:02:20  trey
 * fixed off-by-one error in odd rows of map
 *
 * Revision 1.3  2006/06/12 21:09:10  trey
 * added support for obsDistributionXXX parameters
 *
 * Revision 1.2  2006/06/12 18:44:58  trey
 * regionPriors now implemented correctly
 *
 * Revision 1.1  2006/06/12 18:12:08  trey
 * renamed LSModel to LSModelFile; minor updates
 *
 * Revision 1.1  2006/06/11 14:37:38  trey
 * initial check-in
 *
 *
 ***************************************************************************/
