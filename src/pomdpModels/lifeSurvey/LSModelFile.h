/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_POMDPMODELS_LIFESURVEY_LSMODELFILE_H_
#define ZMDP_SRC_POMDPMODELS_LIFESURVEY_LSMODELFILE_H_

#include <stdio.h>

#include <string>
#include <vector>

#define LS_OBSTACLE (100)

#define LS_BASE_NUM_OBSERVATIONS (3)
#define _N LS_BASE_NUM_OBSERVATIONS
#define LS_NUM_OBSERVATIONS (_N * _N * _N + 1)

namespace zmdp {

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

  unsigned char getCell(const LSPos &pos) const {
    return data[width * pos.y + pos.x];
  }
  void setCell(const LSPos &pos, unsigned char c) {
    data[width * pos.y + pos.x] = c;
  }
  unsigned char getCellBounded(const LSPos &pos) const;
  bool getAtExit(const LSPos &pos) const;
  void writeToFile(FILE *outFile, bool showCoords = false,
                   bool binaryMap = false) const;
  int getMaxCellValue(void);
};

struct LSModelFile {
  int startX, startY;
  double baseCost;
  double lookaheadCost;
  std::vector<double> regionPriors;
  std::vector<double> obsDistributionLifeAbsent;
  std::vector<double> obsDistributionLifePresent;
  LSGrid grid;

  LSModelFile(void);
  void readFromFile(const std::string &mapFileName);
  void writeToFile(FILE *outFile) const;
};

}  // namespace zmdp

#endif  // ZMDP_SRC_POMDPMODELS_LIFESURVEY_LSMODELFILE_H_
