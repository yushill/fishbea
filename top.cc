#include <iostream>
#include <action.hh>
#include <dmmap.hh>

int
main( int argc, char** argv )
{
  VideoConfig vc;
  
  Action action( DMRoomBuf::firstroom(), Point( 50, 50 ), vc.screen );
  
  std::cerr << "Rolling!!!\n";
  
  try { action.run(); }
  
  catch (Control::Quit) { action.endstats( std::cerr ); std::cerr << "Bye!\n"; }
  
  return 0;
}

