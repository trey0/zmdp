/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-11-08 16:41:46 $
   
 @file    sla_cassandra.h
 @brief   Code for converting from Tony Cassandra's sparse matrix format to sla.

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

#ifndef INCsla_cassandra_h
#define INCsla_cassandra_h

#include <math.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <fstream>

#include "sla.h"
#include "mdp.h"

namespace sla {

typedef Matrix CassandraMatrix;

/**********************************************************************
 * DECLARATIONS
 **********************************************************************/

// result = A
void copy(cmatrix& result, CassandraMatrix A, int numColumns);

// result = A
void copy(kmatrix& result, CassandraMatrix A, int numColumns);

/**********************************************************************
 * FUNCTIONS
 **********************************************************************/

// result = A
inline void copy(cmatrix& result, CassandraMatrix A, int numColumns)
{
  kmatrix B;
  copy(B, A, numColumns);
  copy(result, B);
}

// result = A
inline void copy(kmatrix& result, CassandraMatrix A, int numColumns)
{
  result.resize(A->num_rows, numColumns);
  FOR (r, A->num_rows) {
    int rowOffset = A->row_start[r];
    int rowSize = A->row_length[r];
    FOR (i, rowSize) {
      int j = rowOffset + i;
      int c = A->col[j];
      double val = A->mat_val[j];
      result.push_back(r, c, val);
    }
  }
  result.canonicalize();
}

} // namespace sla

#endif // INCsla_cassandra_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/05/27 19:01:26  trey
 * initial check-in
 *
 *
 ***************************************************************************/
