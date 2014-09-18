#ifndef __GAMERINTERFACE_HH__
#define __GAMERINTERFACE_HH__

#include <video.hh>

struct Action;

namespace GamerInterface
{
  void            init();
  void            exit();
  void            collect( Action& action );
  Point<float>    motion( Action& action );
  int             flipandwait( screen_t& _screen, int next_ticks );
};

#endif /* __GAMERINTERFACE_HH__ */
