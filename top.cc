#include <iostream>
#include <action.hh>
#include <dmmap.hh>
#include <epmap.hh>
#include <expmap.hh>
#include <slideroom.hh>

int
main( int argc, char** argv )
{
  VideoConfig vc;
  
  Action action( vc.screen );
  
  action.moveto( EPMap::start_incoming() );
  
  std::cerr << "Rolling!!!\n";
  
  try { action.run(); }
  
  catch (Control::Quit) { action.endstats( std::cerr ); std::cerr << "Bye!\n"; }
  
  return 0;
}

// Map connections
Gate EPMap::end_upcoming() { return DMMap::start_incoming(); }
Gate DMMap::start_upcoming() { return EPMap::end_incoming(); }

Gate DMMap::end_upcoming() { return Spiral::start_incoming(); }
Gate Spiral::start_upcoming() { return DMMap::end_incoming(); }

Gate Spiral::end_upcoming() { return Slalom::start_incoming(); }
Gate Slalom::start_upcoming() { return Spiral::end_incoming(); }

Gate Slalom::end_upcoming() { return ExpMap::start_incoming(); }
