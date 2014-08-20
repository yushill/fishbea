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
  
  action.moveto( SlideRoomBuf::start_incoming() );
  
  std::cerr << "Rolling!!!\n";
  
  try { action.run(); }
  
  catch (Control::Quit) { action.endstats( std::cerr ); std::cerr << "Bye!\n"; }
  
  return 0;
}

// Map connections
Gate EPRoomBuf::end_upcoming() { return DMRoomBuf::start_incoming(); }
Gate DMRoomBuf::start_upcoming() { return EPRoomBuf::start_incoming(); }
Gate DMRoomBuf::end_upcoming() { return ExpRoomBuf::start_incoming(); }
Gate SlideRoomBuf::start_upcoming() { return SlideRoomBuf::start_incoming(); }
Gate SlideRoomBuf::end_upcoming() { return EPRoomBuf::start_incoming(); }
