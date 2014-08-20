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
  
  action.moveto( SlideMap::start_incoming() );
  
  std::cerr << "Rolling!!!\n";
  
  try { action.run(); }
  
  catch (Control::Quit) { action.endstats( std::cerr ); std::cerr << "Bye!\n"; }
  
  return 0;
}

// Map connections
Gate EPMap::end_upcoming() { return DMMap::start_incoming(); }
Gate DMMap::start_upcoming() { return EPMap::start_incoming(); }
Gate DMMap::end_upcoming() { return ExpMap::start_incoming(); }
Gate SlideMap::start_upcoming() { return SlideMap::start_incoming(); }
Gate SlideMap::end_upcoming() { return EPMap::start_incoming(); }
