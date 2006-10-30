/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-10-30 20:23:03 $
   
 @file    zmdpConfig.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "zmdpConfig.h"
#include "zmdpCommonDefs.h"

using namespace std;

namespace zmdp {

static bool allWhiteSpace(const char* buf)
{
  for (const char* p = buf; *p != '\0'; p++) {
    if (!isspace(*p)) return false;
  }
  return true;
}

ZMDPConfig::ZMDPConfig(void) :
  overWriteOnlyMode(false)
{}

void ZMDPConfig::readFromStream(const std::string& _sourceName, std::istream& in)
{
  sourceName = _sourceName;

  char buf[1024], field[1024], val[1024];
  int lnum = 0;
  while (1) {
    in.getline(buf, sizeof(buf));
    lnum++;
    if (in.eof()) break;
    if (allWhiteSpace(buf)) continue;
    if ('#' == buf[0]) continue;
    if (2 != sscanf(buf, "%s %s", field, val)) {
      fprintf(stderr, "ERROR: %s:%d: syntax error, expected '<field> <value>'\n",
	      sourceName.c_str(), lnum);
      exit(EXIT_FAILURE);
    }
    setString(field, val);
  }
}

void ZMDPConfig::readFromFile(const std::string& fname)
{
  ifstream in(fname.c_str());
  if (!in) {
    fprintf(stderr, "ERROR: couldn't open config file '%s' for reading: %s\n",
	    fname.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  readFromStream(fname, in);
  in.close();
}

void ZMDPConfig::readFromString(const std::string& _sourceName, const char* buf)
{
  istringstream in(buf);
  readFromStream(_sourceName, in);
}

void ZMDPConfig::readFromConfig(const ZMDPConfig& x)
{
  typeof(x.cmap.begin()) xi;
  for (xi = x.cmap.begin(); xi != x.cmap.end(); xi++) {
    setString(xi->first, xi->second);
  }
}

const std::string& ZMDPConfig::getString(const std::string& field) const
{
  typeof(cmap.begin()) ci = cmap.find(field);
  if (cmap.end() == ci) {
    fprintf(stderr, "ERROR: required config field '%s' was not specified\n", field.c_str());
    exit(EXIT_FAILURE);
  }
  return ci->second;
}

int ZMDPConfig::getInt(const std::string& field) const
{
  const std::string& s = getString(field);
  // it would be smart to check if s looks like an integer here
  return atoi(s.c_str());
}

double ZMDPConfig::getDouble(const std::string& field) const
{
  const std::string& s = getString(field);
  // it would be smart to check if s looks like a double here
  return atof(s.c_str());
}

bool ZMDPConfig::getBool(const std::string& field) const
{
  const std::string& s = getString(field);
  // it would be smart to check if s looks like a boolean value here
  return (0 != atoi(s.c_str()));
}

bool ZMDPConfig::defined(const std::string& field) const
{
  return (cmap.end() != cmap.find(field));
}

void ZMDPConfig::setString(const std::string& field, const std::string& val)
{
  if (overWriteOnlyMode) {
    if (!defined(field)) {
      fprintf(stderr, "ERROR: unknown config field '%s' was specified\n", field.c_str());
      exit(EXIT_FAILURE);
    }
  }

  cmap[field] = val;
}

void ZMDPConfig::setInt(const std::string& field, int val)
{
  ostringstream out;
  out << val;
  setString(field, out.str());
}

void ZMDPConfig::setDouble(const std::string& field, double val)
{
  ostringstream out;
  out << setprecision(20) << val;
  setString(field, out.str());
}

void ZMDPConfig::setBool(const std::string& field, bool val)
{
  setInt(field, val);
}

void ZMDPConfig::setOverWriteOnlyMode(bool _overWriteOnlyMode)
{
  overWriteOnlyMode = _overWriteOnlyMode;
}

void ZMDPConfig::writeToStream(std::ostream& out) const
{
  FOR_EACH (pr, cmap) {
    out << pr->first << " " << pr->second << endl;
  }
}

};

int zmdpDebugLevelG = 0;

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/10/30 20:00:15  trey
 * USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
 *
 * Revision 1.1  2006/10/15 21:41:22  trey
 * initial check-in
 *
 *
 *
 ***************************************************************************/
