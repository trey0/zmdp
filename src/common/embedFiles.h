/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-03-06 06:56:24 $

 @file    embedFiles.h
 @brief   Include this header before embedding files using the EMBED_FILE()
          macro and embedFiles.perl

 Copyright (c) 2002-2006, Carnegie Mellon University.

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

#ifndef INCembedFiles_h
#define INCembedFiles_h

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned int size;
  const char* data;
} embed_buffer_t;

#ifdef __EMBEDDING_NOW__
#define EMBED_FILE(x,y) %%BEGIN%% x %%MIDDLE%% y %%END%%
#else
#define EMBED_FILE(x,y) extern embed_buffer_t x;
#endif

inline void embedWriteToFile(const char* fileName,
			     const embed_buffer_t& buf)
{
  FILE* outFile;
  if (NULL == (outFile = fopen(fileName, "w"))) {
    fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	    fileName, strerror(errno));
    exit(EXIT_FAILURE);
  }
  fwrite(buf.data, 1, buf.size, outFile);
  fclose(outFile);
}

#endif // INCembedFiles_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/03/05 08:57:47  trey
 * modified how embedded files work; extern const declaration now redundant
 *
 * Revision 1.1  2006/10/15 21:41:31  trey
 * initial check-in
 *
 *
 ***************************************************************************/

