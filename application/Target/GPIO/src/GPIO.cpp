/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//

//
// System stuff
//

//
// Local stuff
//
#include "GPIO.h"

using namespace std;
using namespace RAM::Target;

/** Pin used as a standard led to indicate some application specific state.
 */
#ifdef TARGET_tivac
static const GPIO::PortId portId = GPIO::PortId::GPIOIDF;
static const unsigned bitIndex = 3;
#elif defined TARGET_minim4
static const GPIO::PortId portId = GPIO::PortId::GPIOIDC;
static const unsigned bitIndex = 5;
#elif defined TARGET_discovery
static const GPIO::PortId portId = GPIO::PortId::GPIOIDD;
static const unsigned bitIndex = 12;
#else
#error "unknown target board" 
#endif
static GPIO::Pin statusPin;

//-------------------------------------------------------------------------------
void GPIO::StatusLED::enable()
{
    GPIO::configure(
        portId,
        statusPin,
        0x0,
        (0x1 << bitIndex));
}

//-------------------------------------------------------------------------------
void GPIO::StatusLED::on()
{
    GPIO::Digital::write(statusPin, statusPin.m_outMask);
}

//-------------------------------------------------------------------------------
void GPIO::StatusLED::off()
{
    GPIO::Digital::write(statusPin, 0x0);
}

//-------------------------------------------------------------------------------
void GPIO::StatusLED::toggle()
{
    GPIO::Digital::toggle(statusPin);
}

/*___oOo___*/
