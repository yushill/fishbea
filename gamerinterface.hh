#ifndef __GAMERINTERFACE_HH__
#define __GAMERINTERFACE_HH__

#include <geometry.hh>
#include <fwd.hh>

namespace GamerInterface
{
  void            init();
  void            exit();
  void            collect( Action& action );
  Point<float>    motion( Action const& action );
  int             flipandwait( Screen& _screen, int next_ticks );
};

#endif /* __GAMERINTERFACE_HH__ */
