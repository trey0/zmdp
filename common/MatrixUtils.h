/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-01-21 18:07:02 $
 *  
 * @file    MatrixUtils.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCMatrixUtils_h
#define INCMatrixUtils_h

#if USE_UBLAS
#  include "ublasMatrixUtils.h"
#else
#  include "slaMatrixUtils.h"
#endif

#endif // INCMatrixUtils_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
