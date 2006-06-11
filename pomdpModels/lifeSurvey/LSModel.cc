/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-06-11 14:37:38 $
   
 @file    LSModel.cc
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

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

#include "LSModel.h"
#include "zmdpCommonDefs.h"

using namespace std;

/**********************************************************************
 * DATA STRUCTURES
 **********************************************************************/

typedef std::map<std::string, std::string> ParamLookup;

/**********************************************************************
 * LOCAL HELPER FUNCTIONS
 **********************************************************************/

static std::string getVal(const ParamLookup& params,
			  const std::string& paramName)
{
  typeof(params.begin()) i = params.find(paramName);
  if (params.end() == i) {
    fprintf(stderr, "ERROR: LSModel: preamble does not specify a value for parameter '%s'\n",
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
  height(0),
  data(NULL)
{}

LSGrid::~LSGrid(void)
{
  if (NULL != data) {
    delete[] data;
    data = NULL;
  }
}

void LSGrid::writeToFile(FILE* outFile) const
{
  FOR (i, height) {
    int rowNum = height - i - 1;
    if ((rowNum % 2) == 1) fputc(' ', outFile);
    for (unsigned int j=rowNum/2; j < width; j++) {
      unsigned char cell = getCell(j,rowNum);
      char outCell;
      if (0 == cell) {
	outCell = ' ';
      } else {
	outCell = cell + 96;
      }
      fputc(outCell, outFile);
      fputc(' ', outFile);
    }
    fputc('\n', outFile);
  }
}

/**********************************************************************
 * LSMODEL FUNCTIONS
 **********************************************************************/

LSModel::LSModel(void) :
  startX(-1),
  startY(-1)
{}

void LSModel::readFromFile(const std::string& fname)
{
  FILE* modelFile = fopen(fname.c_str(), "r");
  if (NULL == modelFile) {
    fprintf(stderr, "ERROR: LSModel: couldn't open %s for reading: %s\n", fname.c_str(),
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
    if (2 != sscanf(lbuf, "%s %s", key, value)) {
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
  FOR (i, rows.size()) {
    int rowNum = rows.size() - i - 1;
    maxRowWidth = std::max(maxRowWidth, (int) (rowNum/2 + rows[i].size()));
  }

  /* allocate final map data structure */
  grid.width = maxRowWidth;
  grid.height = rows.size();
  grid.data = new unsigned char[grid.width*grid.height];
  memset(grid.data, grid.width*grid.height, 0);

  /* convert from immediate to final data structure */
  FOR (i, rows.size()) {
    int rowNum = rows.size() - i - 1;
    FOR (j, rows[i].size()) {
      char c = rows[i][j];
      unsigned char cell;
      if (' ' == c) {
	cell = 0;
      } else if (97 <= c && c <= 122) {
	cell = c-96;
      } else {
	fprintf(stderr, "ERROR: %s: while parsing map, found character '%c', expected space or a-z.\n",
		fname.c_str(), c);
	exit(EXIT_FAILURE);
      }
      grid.setCell(rowNum/2 + j, rowNum, cell);
    }
  }
}

void LSModel::writeToFile(FILE* outFile) const
{
  fprintf(outFile, "startX %d\n", startX);
  fprintf(outFile, "startY %d\n", startY);
  fprintf(outFile, "---\n");
  grid.writeToFile(outFile);
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
