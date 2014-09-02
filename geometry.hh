#ifndef __GEOMETRY_HH__
#define __GEOMETRY_HH__

#include <inttypes.h>

template <typename dist_type>
struct Point
{
  Point() : m_x( 0 ), m_y( 0 ) {}
  Point( dist_type _x, dist_type _y ) : m_x( _x ), m_y( _y ) {}
  
  Point operator+( Point const& rpt ) const { return Point( m_x+rpt.m_x, m_y+rpt.m_y ); }
  Point operator-( Point const& rpt ) const { return Point( m_x-rpt.m_x, m_y-rpt.m_y ); }
  Point operator-() const { return Point( -m_x, -m_y ); }
  Point operator*( dist_type rsc ) const { return Point( m_x*rsc, m_y*rsc ); }
  Point operator/( dist_type rsc ) const { return Point( m_x/rsc, m_y/rsc ); }
  dist_type operator*( Point const& rpt ) const { return (m_x*rpt.m_x + m_y*rpt.m_y); }
  Point operator!() const { return Point( -m_y, m_x ); }
  bool operator!=( Point const& rpt ) const { return (m_x != rpt.m_x) or (m_y != rpt.m_y); }
  bool operator==( Point const& rpt ) const { return (m_x == rpt.m_x) and (m_y == rpt.m_y); }
  template <typename T> void pull( T& rect ) const { rect.x = m_x; rect.y = m_y; }
  template <typename T> void pull( T& x, T& y ) const { x = m_x; y = m_y; }
  dist_type sqnorm() const { return m_x*m_x + m_y*m_y; }
  template <typename otherT> Point<otherT> rebind() const { return Point<otherT>( m_x, m_y ); }

  Point& operator+=( Point const& rpt ) { m_x+=rpt.m_x; m_y+=rpt.m_y; return *this; }
  Point& operator-=( Point const& rpt ) { m_x-=rpt.m_x; m_y-=rpt.m_y; return *this; }
  Point& operator*=( dist_type rsc ) { m_x *= rsc; m_y *= rsc; return *this; }
  Point& operator/=( dist_type rsc ) { m_x /= rsc; m_y /= rsc; return *this; }
  
  dist_type m_x, m_y;
};

#endif /* __GEOMETRY_HH__ */
