/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-10-27 18:25:15 $
   
 @file    ScriptedUpdater.h
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

#ifndef INCScriptedUpdater_h
#define INCScriptedUpdater_h

#include "RTDPCore.h"
#include "StateLog.h"

namespace zmdp {

struct ScriptedUpdater : public RTDPCore {
  bool filesRead;
  StateIndex* stateIndex;
  StateLog* backupsLog;
  int currentLogEntry;
  std::string boundValuesOutputFile;

  ScriptedUpdater(void);
  void readFiles(void);
  bool doTrial(MDPNode& cn);
  void finishLogging(void);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/10/20 20:04:19  trey
 * added boundValuesOutputFile support
 *
 * Revision 1.2  2006/10/20 17:10:32  trey
 * corrected some problems with initialization
 *
 * Revision 1.1  2006/10/20 04:58:08  trey
 * initial check-in
 *
 *
 ***************************************************************************/
