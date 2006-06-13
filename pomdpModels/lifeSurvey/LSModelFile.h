/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-06-13 01:02:20 $
   
 @file    LSModelFile.h
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

#ifndef INCLSModelFile_h
#define INCLSModelFile_h

#include <stdio.h>
#include <string>
#include <vector>

#define LS_OBSTACLE (100)

#define LS_BASE_NUM_OBSERVATIONS (3)
#define LS_NUM_OBSERVATIONS (LS_BASE_NUM_OBSERVATIONS*LS_BASE_NUM_OBSERVATIONS*LS_BASE_NUM_OBSERVATIONS+1)

struct LSPos {
  /* special case: x = -1 indicates the terminal state */
  int x, y;

  LSPos(void) : x(-1), y(-1) {}
  LSPos(int _x, int _y) : x(_x), y(_y) {}
};

struct LSGrid {
  unsigned int width, height;
  /* values in the data array correspond to map hexes -- either
     LS_OBSTACLE (hex is an obstacle) or 0..maxValue-1 (indicating which
     geological region the hex is in) */
  std::vector<unsigned char> data;

  LSGrid(void);

  unsigned char getCell(const LSPos& pos) const { return data[width*pos.y + pos.x]; }
  void setCell(const LSPos& pos, unsigned char c) { data[width*pos.y + pos.x] = c; }
  unsigned char getCellBounded(const LSPos& pos) const;
  bool getExitLegal(const LSPos& pos) const;
  void writeToFile(FILE* outFile) const;
  int getMaxCellValue(void);
};

struct LSModelFile {
  int startX, startY;
  std::vector<double> regionPriors;
  std::vector<double> obsDistributionLifeAbsent;
  std::vector<double> obsDistributionLifePresent;
  LSGrid grid;

  LSModelFile(void);
  void readFromFile(const std::string& mapFileName);
  void writeToFile(FILE* outFile) const;
};


#endif // INCLSModelFile_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/06/12 21:09:10  trey
 * added support for obsDistributionXXX parameters
 *
 * Revision 1.2  2006/06/12 18:44:58  trey
 * regionPriors now implemented correctly
 *
 * Revision 1.1  2006/06/12 18:12:08  trey
 * renamed LSModel to LSModelFile; minor updates
 *
 * Revision 1.1  2006/06/11 14:37:39  trey
 * initial check-in
 *
 *
 ***************************************************************************/

