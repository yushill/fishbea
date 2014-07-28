#ifndef __TIMELINE_HH__
#define __TIMELINE_HH__

#include <geometry.hh>
#include <map.hh>
#include <inttypes.h>
#include <vector>
#include <map>
#include <iosfwd>

typedef uint64_t date_t;

struct SDL_Surface;

struct Character
{
  int32_t  xpos : 12;
  int32_t  ypos : 12;
  uint32_t room : 7;
  uint32_t fire : 1;
  
  Character() {}
  Character( unsigned int _room, Point<float> const& _position, bool _fire )
    : xpos(_position.m_x), ypos(_position.m_y), room(_room), fire(_fire)
  {}
};

struct Ghost
{
  Room           room;
  Point<int32_t> pos;
  Ghost( Room _room ) : room(_room), pos() {}
  bool match( Room _room, Point<int32_t> const& _pos, bool fire )
  {
    if (_room != room) return false;
    pos = _pos;
    return true;
  }
};

struct TimeLine
{
  struct Chunk {
    std::vector<Room>      rooms;
    std::vector<Character> steps;
    
    Chunk() : rooms(), steps() {}
    Chunk( Chunk const& chunk ) : rooms(chunk.rooms), steps(chunk.steps) {}
    void append( Point<float> const& _position, bool _fire )
    { steps.push_back( Character( rooms.size()-1, _position, _fire ) ); }
    void reserve()
    { rooms.reserve(128); steps.reserve(4096); }
    void singleton( Room const& _room, Point<float> const& _position, bool _fire )
    {
      rooms.reserve(1); rooms.push_back( _room );
      steps.reserve(1); this->append( _position, _fire );
    }
    bool useroom( Room const& _room )
    {
      if ((rooms.size() > 0) and (rooms.back() == _room)) return true;
      if (rooms.size() == rooms.capacity()) return false;
      rooms.push_back( _room );
      return true;
    }
  };
  
  struct date_more { bool operator() ( date_t const& a, date_t const& b ) const { return b < a; } };
  typedef std::map<date_t,Chunk,date_more> Map;
  
  // Contruction
  TimeLine( date_t date );
  TimeLine( date_t date, Room const& _room, Point<float> const& _position, bool _fire, SDL_Surface* thumb );
  ~TimeLine();
  // No copies
  TimeLine( TimeLine const& _tl ) { throw "NoNoNo"; }
  TimeLine& operator=( TimeLine const& _tl ) { throw "NoNoNo"; }
  /* Cycling list */
  TimeLine*     fwd() { return m_fwd; }
  TimeLine*     bwd() { return m_bwd; }
  void          insbwd( TimeLine* ntl ) { ntl->m_bwd = m_bwd; ntl->m_fwd = this; m_bwd->m_fwd = ntl; m_bwd = ntl; }
  void          insfwd( TimeLine* ntl ) { ntl->m_fwd = m_fwd; ntl->m_bwd = this; m_fwd->m_bwd = ntl; m_fwd = ntl; }
  TimeLine*     extract() { m_fwd->m_bwd = m_bwd; m_bwd->m_fwd = m_fwd; m_fwd = 0; m_bwd = 0; return this; }
  bool          single() const { return (m_fwd == this) and (m_bwd == this); }
  
  void          append( date_t date, Room const& _room, Point<float> const& _position, bool _fire );
  void          setthumb( SDL_Surface* newthumb );
  SDL_Surface*  getthumb() { return m_thumb; };
  void          update_usetime();
  void          compress();
  void          restore_state( Point<float>& _pos, Room& _room ) const;
  bool          locate( date_t& _date, Ghost& _pos ) const;
  
  template <typename T>
  bool          match( date_t _date, T& _filter )
  {
    Map::const_iterator itr = m_map.lower_bound( _date );
    if (itr == m_map.end()) return false;
    Chunk const& chunk = itr->second;
    date_t offset = _date - itr->first;
    if (offset >= chunk.steps.size()) return false;
    Character const& chr = chunk.steps[offset];
    return _filter.match( chunk.rooms[chr.room], Point<int32_t>( chr.xpos, chr.ypos ), chr.fire );
  }
  void          getbounds( date_t& lo, date_t& hi ) const;
  void          updatebounds( date_t& lo, date_t& hi ) const;
  uintptr_t     record_count() const;
  
  TimeLine*     m_fwd;
  TimeLine*     m_bwd;
  Map           m_map;
  uintptr_t     m_usetime; /* last use time */
  SDL_Surface*  m_thumb;
};

struct Story
{
  TimeLine* active;
  uintptr_t record_count;
  date_t    bot; // beginning of time
  date_t    boa; // beginning of active timeline
  date_t    eoa; // end of active timeline
  date_t    eot; // end of time
  
  static uintptr_t const record_count_max = (0x1000000 / sizeof (Character));
  
  Story()
    : active(new TimeLine( 0 )),
      record_count(0), bot(0), boa(0), eoa(0), eot(0)
  {}
  ~Story()
  {
    while (not active->single()) delete popfwd();
    delete active;
  }
  
  date_t now() const { return eoa - 1; }
  TimeLine* firstghost() { return active->fwd(); }
  void newbwd( Room const& _room, Point<float> const& _pos, SDL_Surface* thumb )
  {
    active->insbwd( new TimeLine( now(), _room, _pos, false, thumb ) );
    record_count += 1;
    active->update_usetime();
  }
  
  void forward()
  {
    Point<float> loc;
    Room room;
    active->restore_state( loc, room );
    while (eoa < eot) { append( room, loc, false ); }
  }
  
  void append( Room const& _room, Point<float> const& _pos, bool _fires )
  {
    if ((++record_count) >= record_count_max) throw "NoNoNo";
    active->append( eoa++, _room, _pos, _fires );
    if (eot < eoa) eot = eoa;
  }
  
  void movfwd() { active = active->fwd(); active->getbounds( boa, eoa ); }
  void movbwd() { active = active->bwd(); active->getbounds( boa, eoa ); }
  
  TimeLine* popfwd()
  {
    TimeLine* node = active->fwd();
    record_count -= active->record_count();
    node->getbounds( boa, eoa );
    bot = boa; eot = eoa;
    for (TimeLine* tl = node->fwd(); tl != active; tl = tl->fwd()) tl->updatebounds( bot, eot );
    std::swap( active, node );
    return node->extract();
  }
  TimeLine* popbwd()
  {
    TimeLine* node = active->bwd();
    record_count -= active->record_count();
    node->getbounds( boa, eoa );
    bot = boa; eot = eoa;
    for (TimeLine* tl = node->bwd(); tl != active; tl = tl->bwd()) tl->updatebounds( bot, eot );
    std::swap( active, node );
    return node->extract();
  }
  std::ostream& writebounds( std::ostream& _sink ) const;
};

#endif /* __TIMELINE_HH__ */
