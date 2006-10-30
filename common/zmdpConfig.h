/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-10-30 20:00:15 $
   
 @file    zmdpConfig.h
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

#ifndef INCzmdpConfig_h
#define INCzmdpConfig_h

#include <map>
#include <iostream>

namespace zmdp {

struct ZMDPConfig {
  typedef std::map<std::string, std::string> CMap;
  CMap cmap;
  std::string sourceName;
  bool overWriteOnlyMode;

  ZMDPConfig(void);

  void readFromStream(const std::string& _sourceName, std::istream& in);
  void readFromFile(const std::string& fname);
  void readFromString(const std::string& _sourceName, const char* buf);
  
  void readFromConfig(const ZMDPConfig& x);

  const std::string& getString(const std::string& field) const;
  int getInt(const std::string& field) const;
  double getDouble(const std::string& field) const;
  bool getBool(const std::string& field) const;
  bool defined(const std::string& field) const;

  void setString(const std::string& field, const std::string& val);
  void setInt(const std::string& field, int val);
  void setDouble(const std::string& field, double val);
  void setBool(const std::string& field, bool val);

  void setOverWriteOnlyMode(bool _overWriteOnlyMode);
};

}; // namespace zmdp

extern int zmdpDebugLevelG;

#endif // INCzmdpConfig_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/10/15 21:41:22  trey
 * initial check-in
 *
 *
 ***************************************************************************/
