#include <iostream>
#include <action.hh>
#include <dmmap.hh>
#include <epmap.hh>
#include <expmap.hh>
#include <slideroom.hh>
#include <pebblemap.hh>

int
main( int argc, char** argv )
{
  Action action;
  
  //action.moveto( pebble::Simple::start_incoming() );
  action.moveto( ExpMap::start_incoming() );
  
  std::cerr << "Rolling!!!\n";
  
  try { action.run(); }
  
  catch (Action::Quit) { action.endstats( std::cerr ); std::cerr << "Bye!\n"; }
  
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
Gate pebble::DiaMesh::end_upcoming() { return ExpMap::start_incoming(); }
