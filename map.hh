#ifndef __MAP_HH__
#define __MAP_HH__

#include <geometry.hh>
#include <inttypes.h>
#include <typeinfo>
#include <string>

struct RefCounter
{
  mutable uintptr_t _M_refcount;
  virtual void dispose() const = 0;
  RefCounter() : _M_refcount() {}
  void retain() const { this->_M_refcount += 1; }
  void release() const { if (--this->_M_refcount == 0) this->dispose(); };
};

template <typename T>
struct RCPtr
{
  T const* m_rco;
  RCPtr() : m_rco(0) {}
  RCPtr( T const* _rco ) : m_rco(_rco) { if (_rco) _rco->retain(); }
  RCPtr( RCPtr const& _rcp )
    : m_rco(_rcp.m_rco)
  { if (m_rco) m_rco->retain(); }
  ~RCPtr() { if (m_rco) m_rco->release(); }
  RCPtr&    operator=( RCPtr const& _rcp )
  {
    T const* tmp = m_rco; m_rco = _rcp.m_rco;
    if (m_rco) m_rco->retain();
    if (tmp) tmp->release();
    return *this;
  }
  bool     good() const { return m_rco; }
  T const*       operator->() const { return m_rco; }
  bool operator==(RCPtr const& _rop) const
  {
    if ((not this->m_rco) or (not _rop.m_rco))
      return this->m_rco == _rop.m_rco;
    std::type_info const& a = typeid( *this->m_rco );
    std::type_info const& b = typeid( *_rop.m_rco );
    return (a == b) and (this->m_rco->cmp( *_rop.m_rco ) == 0);
  }
  bool operator!=(RCPtr const& _rop) const
  {
    if ((not this->m_rco) or (not _rop.m_rco))
      return this->m_rco != _rop.m_rco;
    std::type_info const& a = typeid( *this->m_rco );
    std::type_info const& b = typeid( *_rop.m_rco );
    return (a != b) or (this->m_rco->cmp( *_rop.m_rco ) != 0);
  }
  bool operator<(RCPtr const& _rop) const
  {
    if ((not this->m_rco) or (not _rop.m_rco))
      return this->m_rco < _rop.m_rco;
    std::type_info const& a = typeid( *this->m_rco );
    std::type_info const& b = typeid( *_rop.m_rco );
    if (a.before(b)) return true;
    if (b.before(a)) return false;
    return this->m_rco->cmp( *_rop.m_rco ) < 0;
  }
  bool operator>(RCPtr const& _rop) const
  {
    if ((not this->m_rco) or (not _rop.m_rco))
      return this->m_rco > _rop.m_rco;
    std::type_info const& a = typeid( *this->m_rco );
    std::type_info const& b = typeid( *_rop.m_rco );
    if (a.before(b)) return false;
    if (b.before(a)) return true;
    return this->m_rco->cmp( *_rop.m_rco ) > 0;
  }
  int cmp(RCPtr const& _rop) const
  {
    if ((not this->m_rco) or (not _rop.m_rco)) {
      if (this->m_rco < _rop.m_rco) return -1;
      if (this->m_rco > _rop.m_rco) return +1;
      return 0;
    }
    std::type_info const& a = typeid( *this->m_rco );
    std::type_info const& b = typeid( *_rop.m_rco );
    if (a.before(b)) return -1;
    if (b.before(a)) return +1;
    return this->m_rco->cmp( *_rop.m_rco );
  }
};

struct Action;

struct RoomBuf : public virtual RefCounter
{
  virtual std::string    getname() const = 0;
  virtual void           process( Action& ) const = 0;
  virtual int            cmp( RoomBuf const& _rb ) const = 0;
};

typedef RCPtr<RoomBuf> Room;


template <class T>
struct RecycleHeap {
  struct Link { Link* next; Link( Link* _next ) : next( _next ) {} };
  Link* pool;
  RecycleHeap() : pool( 0 ) {}
  static uintptr_t const TSZ = sizeof(T);
  T* allocate() { T* res = (T*)(pool); if (res) pool = pool->next; else res = (T*)(new uint8_t[TSZ]); return res; }
  void deallocate( T const* _src ) {
    T* src = const_cast<T*>(_src);
    src->~T();
    Link* l = (Link*)src; l->next = pool; pool = l;
  }
};

#endif /* __MAP_HH__ */
