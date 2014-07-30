#include <iostream>
#include <action.hh>
#include <dmmap.hh>
#include <epmap.hh>
#include <totomap.hh>

int
main( int argc, char** argv )
{
  VideoConfig vc;
  
  Action action( vc.screen );
  
  action.moveto( EPRoomBuf::start_incoming() );
  
  std::cerr << "Rolling!!!\n";
  
  try { action.run(); }
  
  catch (Control::Quit) { action.endstats( std::cerr ); std::cerr << "Bye!\n"; }
  
  return 0;
}

// Map connections
Gate EPRoomBuf::end_upcoming() { return DMRoomBuf::start_incoming(); }
