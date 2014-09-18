#ifndef __MAP_HH__
#define __MAP_HH__

#include <geometry.hh>
#include <string>
#include <typeinfo>
#include <inttypes.h>

struct Action;

struct RoomBuf
{
  /* Room instances management (Reference Counting and comparison) */
  mutable uintptr_t _M_refcount;
  RoomBuf() : _M_refcount() {}
  void retain() const { this->_M_refcount += 1; }
  void release() const { if (--this->_M_refcount == 0) this->dispose(); };
  virtual void dispose() const { delete this; };
  virtual ~RoomBuf() {}
  virtual int            cmp( RoomBuf const& _rb ) const = 0;
  
  /* Room action methods */
  virtual std::string    getname() const = 0;
  virtual void           process( Action& ) const = 0;
};

template <typename T>
int tgcmp( T const& a, T const& b ) { return (a < b) ? -1 : (a == b) ? 0 : +1; }

struct Room
{
  RoomBuf const* m_rco;
  Room() : m_rco() {}
  Room( RoomBuf const* _rco ) : m_rco(_rco) { if (_rco) _rco->retain(); }
  Room( Room const& _rcp ) : m_rco(_rcp.m_rco) { if (m_rco) m_rco->retain(); }
  ~Room() { if (m_rco) m_rco->release(); }
  Room&                operator=( Room const& _rcp ) { Room tmp( _rcp ); std::swap( tmp.m_rco, m_rco ); return *this; }
  RoomBuf const*       operator->() const { return m_rco; }
  bool                 good() const { return m_rco; }
  int cmp(Room const& _rop) const
  {
    if ((not this->m_rco) or (not _rop.m_rco)) return tgcmp( this->m_rco, _rop.m_rco );
    std::type_info const& a = typeid( *this->m_rco );
    std::type_info const& b = typeid( *_rop.m_rco );
    if (a.before(b)) return -1;
    if (b.before(a)) return +1;
    return this->m_rco->cmp( *_rop.m_rco );
  }
  bool operator==(Room const& _rop) const { return this->cmp( _rop) == 0; }
  bool operator!=(Room const& _rop) const { return this->cmp( _rop) != 0; }
  bool operator<(Room const& _rop) const { return this->cmp( _rop ) < 0; }
  bool operator>(Room const& _rop) const { return this->cmp( _rop ) > 0; }
};

struct Gate
{
  Room            room;
  Point<int32_t>  pos;
  Gate( Room const& _room, Point<int32_t> const& _pos ) : room(_room), pos(_pos) {}
};

#endif /* __MAP_HH__ */
