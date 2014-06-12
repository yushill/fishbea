#include <iostream>
#include <action.hh>
#include <dmmap.hh>
#include <epmap.hh>

int
main( int argc, char** argv )
{
  VideoConfig vc;
  
  Action action( vc.screen );
  
  action.moveto( DMRoomBuf::start_incoming() );
  
  std::cerr << "Rolling!!!\n";
  
  try { action.run(); }
  
  catch (Control::Quit) { action.endstats( std::cerr ); std::cerr << "Bye!\n"; }
  
  return 0;
}

Gate EPRoomBuf::end_upcoming() { return DMRoomBuf::start_incoming(); }
