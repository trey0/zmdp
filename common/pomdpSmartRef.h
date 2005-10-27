/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2005-10-27 22:06:00 $
 *  
 * @file    SmartRef.h
 * @brief   from http://www.geocities.com/botstein/refcnt.html
 *          Author: Boris Botstein, botstein@yahoo.com
 *          Minor modifications by Trey Smith
 ***************************************************************************/

#ifndef INCSmartRef_h
#define INCSmartRef_h

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>

/***************************************************************************
 * MACROS
 ***************************************************************************/

/***************************************************************************
 * CLASSES AND TYPEDEFS
 ***************************************************************************/

namespace pomdp {

template < class X >
class SmartRef
{
 protected:
  class holder {
  public:
    X* rep;
    unsigned count, accessed;
    
    holder(X* ptr) : rep(ptr), count(1), accessed(0) {}
    ~holder() { delete rep; }
  };
  holder* value;
  
  void unbind() { if(--value->count == 0) delete value; }
  
  X* get(void) const {
    assert(value->rep);
    value->accessed++;
    return value->rep;
  }
  
 public:
  SmartRef(void) : value(new holder(0)) {}
  SmartRef(X* ptr) : value(new holder(ptr)) {}
  SmartRef(const SmartRef< X >& rhs) : value(rhs.value) {
    value->count++;
  }
  ~SmartRef() { unbind(); }
  
  void bind(const SmartRef< X >& rhs) {
    if(rhs.value != value) {
      unbind();
      value = rhs.value;
      value->count++;
    }
  }
  void bind(X* ptr) {
    if(value->rep != ptr) {
      unbind();
      value = new holder(ptr);
    }
  }
  
  SmartRef< X >& operator=(const SmartRef< X >& rhs) {
    bind(rhs);
    return *this;
  }
  SmartRef< X >& operator=(X* ptr) {
    bind(ptr);
    return *this;
  }
  
  X& reference() const { return *get(); }
  X* pointer() const { return get(); }
  
  X* operator->() { return get(); }
  X& operator*() { return *get(); }
  
  const X* operator->() const { return get(); }
  const X& operator*() const { return *get(); }

  bool operator!() const { return 0 == value->rep; }
  bool defined() const { return 0 != value->rep; }
  
  unsigned counter() const { return value->count; }
  unsigned accessed() const { return value->accessed; }
};

}; // namespace pomdp

#endif /* INCSmartRef_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/12/16 03:58:50  trey
 * added defined()
 *
 * Revision 1.1  2004/11/09 21:28:24  trey
 * check-in to pomdp repository
 *
 * Revision 1.5  2002/10/09 00:19:17  trey
 * added ! operator as syntactic sugar
 *
 * Revision 1.4  2001/11/08 23:11:34  trey
 * added include of assert.h in case including .cc file lacks it
 *
 * Revision 1.3  2001/11/06 22:01:18  trey
 * made SmartRef initialization less picky
 *
 * Revision 1.2  2001/10/25 22:52:17  trey
 * fixed missing final #endif
 *
 * Revision 1.1  2001/10/25 21:23:03  trey
 * initial check-in
 *
 *
 ***************************************************************************/

