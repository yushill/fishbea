#ifndef __GEOMETRY_HH__
#define __GEOMETRY_HH__

struct Point {
  int m_x, m_y;
  Point() : m_x( 0 ), m_y( 0 ) {}
  Point( int _x, int _y ) : m_x( _x ), m_y( _y ) {}
  bool nextdir() { int x = m_x; m_x = -m_y; m_y = x; if (m_x == 1) return false; if ((m_x | m_y) == 0) m_x = 1; return true; }
  Point operator+( Point const& rpt ) const { return Point( m_x+rpt.m_x, m_y+rpt.m_y ); }
  Point operator-( Point const& rpt ) const { return Point( m_x-rpt.m_x, m_y-rpt.m_y ); }
  Point operator-() const { return Point( -m_x, -m_y ); }
  Point& operator+=( Point const& rpt ) { m_x+=rpt.m_x; m_y+=rpt.m_y; return *this; }
  template <typename scaleT> Point operator*( scaleT rsc ) const { return Point( m_x*rsc, m_y*rsc ); }
  template <typename scaleT> Point operator/( scaleT rsc ) const { return Point( m_x/rsc, m_y/rsc ); }
  bool operator!=( Point const& rpt ) const { return (m_x != rpt.m_x) or (m_y != rpt.m_y); }
  bool operator==( Point const& rpt ) const { return (m_x == rpt.m_x) and (m_y == rpt.m_y); }
  template <typename T>
  void pull( T& rect ) const { rect.x = m_x; rect.y = m_y; }
  int m2() const { return m_x*m_x + m_y*m_y; }
};

#endif /* __GEOMETRY_HH__ */
