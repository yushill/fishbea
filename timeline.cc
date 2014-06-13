#include <timeline.hh>
#include <geometry.hh>
#include <SDL/SDL_video.h>
#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <inttypes.h>

TimeLine::~TimeLine()
{
  if (m_thumb)
    SDL_FreeSurface( m_thumb );
}

void
TimeLine::setthumb( SDL_Surface* _thumb )
{
  if (m_thumb)
    SDL_FreeSurface( m_thumb );
  m_thumb = _thumb;
}

void
TimeLine::append( date_t date, Room const& _room, Point const& _position, bool _fire )
{
  Map::iterator tail = m_map.begin();
  Chunk* chunk = &tail->second;
  uintptr_t chunksize = chunk->steps.size();
  { /* checking for time continuity */
    date_t chunkdate = tail->first;
    if ((chunkdate + chunksize) != date) {
      std::ostringstream oss( "time discontinuity: " );
      oss << "last=" << (chunkdate + chunksize) << ", now=" << date << "...";
      throw std::invalid_argument( oss.str() );
    }
  }
  if (((chunk->steps.capacity() - chunksize) == 0) or not chunk->useroom(_room)) {
    tail = m_map.insert( tail, std::make_pair( date, Chunk() ) );
    chunk = &tail->second;
    chunk->reserve();
    chunk->rooms.push_back( _room );
  }
  chunk->append( _position, _fire );
}

TimeLine::TimeLine( date_t date )
  : m_fwd(this), m_bwd(this), m_usetime(), m_thumb(0)
{
  Chunk& chunk = m_map.insert( m_map.end(), std::make_pair( date, Chunk() ) )->second;
  chunk.reserve();
  this->update_usetime();
}

TimeLine::TimeLine( date_t date, Room const& _room, Point const& _position, bool _fire, SDL_Surface* thumb )
  : m_fwd(this), m_bwd(this), m_usetime(), m_thumb(thumb)
{
  Chunk& chunk = m_map.insert( m_map.end(), std::make_pair( date, Chunk() ) )->second;
  chunk.singleton( _room, _position, _fire );
  this->update_usetime();
}

void
TimeLine::update_usetime()
{
  static uintptr_t top_usetime = 1;
  if (m_usetime == top_usetime) return;
  m_usetime = ++top_usetime;
}

namespace {
  struct MapIterator
  {
    typedef TimeLine::Map::iterator chunk_iterator;
    typedef TimeLine::Chunk chunk_type;
    chunk_iterator itr;
    date_t         offset;
    
    MapIterator( chunk_iterator _itr, uintptr_t _offset ) : itr( _itr ), offset( _offset ) {}
    MapIterator( MapIterator const& _src ) : itr( _src.itr ), offset( _src.offset ) {}
    
    
    static MapIterator begin( TimeLine::Map& _map ) { chunk_iterator start( _map.end() ); return MapIterator( --start, 0 ); }
    static MapIterator end( TimeLine::Map& _map )   { return MapIterator( _map.end(), 0 ); }
    MapIterator& operator++()
    {
      offset += 1;
      if (offset < itr->second.steps.size()) return *this;
      itr--; offset = 0;
      return *this;
    }
    bool operator==( MapIterator const& _rop ) const { return itr == _rop.itr and offset == _rop.offset; }
    bool operator!=( MapIterator const& _rop ) const { return itr != _rop.itr or offset != _rop.offset; }
    Room room() const { chunk_type& chunk = itr->second; return chunk.rooms[chunk.steps[offset].room]; }
    Character character() const { return itr->second.steps[offset]; }
    date_t date() const { return itr->first + offset; }
  };
  
  struct MapCompressor
  {
    typedef std::map<Room,int> RoomDict;
    RoomDict room_dict;
    bool insert( Room room )
    {
      RoomDict::iterator uroom = room_dict.lower_bound( room );
      if ((uroom != room_dict.end()) and (not room_dict.key_comp()( room, uroom->first )))
        return true; /* room is already in the dictionary */
      /* Room is not yet in the dictionary, so check if it can be inserted */
      uintptr_t rid = room_dict.size();
      if (rid < 128) {
        room_dict.insert(uroom, std::make_pair(room, rid));
        return true; /* success, can be inserted*/
      }
      if (rid > 128) throw std::logic_error("Room Dictionary Overflow");
      return false;
    }
    void restart( Room room ) { room_dict.clear(); room_dict[room] = 0; }
    void pull( std::vector<Room>& _rooms )
    {
      _rooms.resize(room_dict.size());
      std::set<int> indices;
      for (RoomDict::iterator itr = room_dict.begin(); itr != room_dict.end(); ++itr) {
        _rooms.at(itr->second) = itr->first;
        if (not indices.insert(itr->second).second) throw std::logic_error("Duplicated indices");
      }
    }
    uint32_t room_index( Room room )
    {
      RoomDict::iterator uroom = room_dict.lower_bound( room );
      if ((uroom == room_dict.end()) or room_dict.key_comp()( room, uroom->first ))
        throw std::logic_error( "Corrupted dictionary ?" );
      return uroom->second;
    }
  };
  
  void mapdbg( TimeLine::Map const& map )
  {
    std::cerr << map.size() << " chunks: {";
    char const* sep = "";
    for (TimeLine::Map::const_iterator itr = map.begin(); itr != map.end(); ++itr, sep=", ")
      {
        TimeLine::Chunk const& chunk = itr->second;
        std::cerr << sep << "(" << chunk.rooms.size() << " rooms [";
        char const* rmsep = "";
        for (std::vector<Room>::const_iterator rmitr = chunk.rooms.begin(), rmend = chunk.rooms.end(); rmitr != rmend; ++rmitr, rmsep = ", ")
          std::cerr << rmsep << '"' << (*rmitr)->getname() << '"';
        std::cerr << "], " << chunk.steps.size() << " steps)";
      }
    std::cerr << "}";
  }
}


void
TimeLine::compress()
{
  if (m_map.size() < 2) return;
  Map newmap;
  MapCompressor mc;
  uintptr_t stepcount = 0;
  
  for (MapIterator first = MapIterator::begin( m_map ), eomap = MapIterator::end( m_map ), itr( first );; ++itr)
    {
      bool lastchunk = (itr == eomap);
      if (not lastchunk) {
        /* continue if this room can be hold by the room dictionnary of the current new chunk */
        if (mc.insert( itr.room() )) { stepcount += 1; continue; }
      }
      {
        /* Appending new compressed chunk */
        Map::iterator newchunk = newmap.insert( newmap.begin(), std::make_pair( first.date(), Chunk() ) );
        Chunk& chunk = newchunk->second;
        chunk.steps.reserve( stepcount );
        mc.pull( chunk.rooms );
        for (MapIterator reitr( first ); reitr != itr; ++reitr) {
          Character character = reitr.character();
          character.room = mc.room_index(reitr.room());
          chunk.steps.push_back( character );
        }
      }
      /*mc.append_compressed_chunk( newmap, first, itr, room_dict );*/
      if (lastchunk) break;
      first = itr;
      stepcount = 1;
      mc.restart( itr.room() );
    }
  std::cerr << "initial map "; mapdbg( m_map ); std::cerr << std::endl;
  std::cerr << "final   map "; mapdbg( newmap ); std::cerr << std::endl;
  std::swap( m_map, newmap );
}

void
TimeLine::restore_state( Point& _pos, Room& _room ) const
{
  Chunk const& chunk = m_map.begin()->second;
  Character const& chr = chunk.steps.back();
  _pos = Point( chr.xpos, chr.ypos );
  _room = chunk.rooms[chr.room];
};

bool
TimeLine::locate( date_t _date, Room _room, Point& _pos, bool& fire ) const
{
  Map::const_iterator itr = m_map.lower_bound( _date );
  if (itr == m_map.end()) { --itr; _date = itr->first; }
  Chunk const& chunk = itr->second;
  date_t offset = std::min( _date - itr->first, date_t( chunk.steps.size()-1 ) );
  Character const& chr = chunk.steps[offset];
  if (chunk.rooms[chr.room] != _room) return false;
  _pos = Point( chr.xpos, chr.ypos );
  fire = chr.fire;
  return true;
}

void
TimeLine::getbounds( date_t& lo, date_t& hi ) const
{
  Map::const_iterator itr = m_map.begin();
  hi = itr->first + itr->second.steps.size();
  itr = m_map.end();
  lo = (--itr)->first;
}

void
TimeLine::updatebounds( date_t& lo, date_t& hi ) const
{
  Map::const_iterator itr = m_map.begin();
  hi = std::max( hi, itr->first + itr->second.steps.size());
  itr = m_map.end();
  lo = std::min( lo, (--itr)->first);
}

void
TimeLine::getstorybounds( date_t& lo, date_t& hi ) const
{
  TimeLine const* tl = this;
  lo = std::numeric_limits<date_t>::max();
  hi = std::numeric_limits<date_t>::min();

  do {
    date_t llo,lhi;
    tl->getbounds( llo, lhi );
    if (lo > llo) lo = llo;
    if (hi < lhi) hi = lhi;
  } while (tl != this);
}

uintptr_t
TimeLine::record_count() const
{
  uintptr_t res = 0;
  for (Map::const_iterator itr = m_map.begin(), end = m_map.end(); itr != end; ++itr)
    res += itr->second.steps.size();
  return res;
}
