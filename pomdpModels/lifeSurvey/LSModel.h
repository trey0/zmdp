/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-06-11 14:37:39 $
   
 @file    LSModel.h
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

#ifndef INCLSModel_h
#define INCLSModel_h

#include <stdio.h>
#include <string>

struct LSGrid {
  unsigned int width, height;
  /* entries in the data array correspond to map hexes -- either 0 (hex
     is an obstacle) or 1..n (indicating which geological region the hex
     is in) */
  unsigned char* data;

  LSGrid(void);
  ~LSGrid(void);
  unsigned char getCell(int x, int y) const { return data[width*y + x]; }
  void setCell(int x, int y, unsigned char c) { data[width*y + x] = c; }
  void writeToFile(FILE* outFile) const;
};

struct LSModel {
  int startX, startY;
  LSGrid grid;

  LSModel(void);
  void readFromFile(const std::string& mapFileName);
  void writeToFile(FILE* outFile) const;
};


#endif // INCLSModel_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

