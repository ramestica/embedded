/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Main.h>

//
// System stuff
//
#include <stdlib.h>

//
// Contributed stuff
//
//#include "U8glib.h"

//
// Library stuff
//
#include <UART.h>
#include <MainLoop.h>

//
// Local stuff
//
#include "I2C.h"

using namespace std;

//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);	// I2C / TWI 

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here  
  //u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  //u8g.drawStr( 0, 22, "Hello Jasmina!");
}

void mySetup()
{
    RAM::Library::I2C::instance().setup(
        400,
        false);
}

void RAM::Target::Main::setup()
{
    RAM::Library::MainLoop::Executive::instance().setup(
        mySetup,
        nullptr,
        RAM::Library::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
        8000,
        1);

    // assign default color value
    // if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    //     u8g.setColorIndex(255);     // white
    // }
    // else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    //     u8g.setColorIndex(3);         // max intensity
    // }
    // else if ( u8g.getMode() == U8G_MODE_BW ) {
    //     u8g.setColorIndex(1);         // pixel on
    // }
    // else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    //     u8g.setHiColorByRGB(255,255,255);
    // }

    // u8g.firstPage();  
    // do {
    //     draw();
    // } while( u8g.nextPage() );
}

void RAM::Target::Main::loop()
{
    RAM::Library::MainLoop::Executive::instance().execute();
}

/* ___oOo___ */
