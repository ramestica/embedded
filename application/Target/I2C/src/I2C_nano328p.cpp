//
// AVR stuff
//
#include <util/twi.h>

//
// Arduino stuff
//
#include "Arduino.h"

//
// Local stuff
//
#include "TargetI2C.h"

static uint32_t neutralizeTime = 0;
static int16_t m_errorsCount = 0;

static void startf(uint8_t addr, const bool isRead)
{
    //
    // send REPEAT START condition
    //
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

    //
    // wait until transmission completed
    //
    RAM::Target::I2C::wait();

    //
    // send device address
    //
    //TWDR = address;
    TWDR = (addr << 1) | isRead;

    TWCR = (1<<TWINT) | (1<<TWEN);

    //
    // wait until transmission completed
    //
    RAM::Target::I2C::wait();
}

//-------------------------------------------------------------------------------
void RAM::Target::I2C::setup(
    const uint16_t kbps,
    const bool enableInternalPullup)
{
    if ( enableInternalPullup )
    {
        PORTD |= 1<<0; 
        
        PORTD |= 1<<1;
    }
    else
    {
        PORTD &= ~(1<<0);
        
        PORTD &= ~(1<<1);
    }

    TWSR = 0;                                    // no prescaler => prescaler = 1

    TWBR = ((F_CPU / (1000L * kbps)) - 16) / 2;  // change the I2C clock rate

    TWCR = 1<<TWEN;                              // enable twi module, no interrupt
}

//-------------------------------------------------------------------------------
void RAM::Target::I2C::read(
    const bool start,
    const uint8_t addr,
    void *buf,
    const size_t size)
{
    if ( start )
    {
        startf(addr, true);
    }

    for ( unsigned int i = 0; i < size; ++i )
    {
        //
        // ack?+stop?+start?+run
        //
        TWCR = (1<<TWINT) | (1<<TWEN) | ((i != size - 1)? (1<<TWEA) : 0);

        wait();

        reinterpret_cast<uint8_t *>(buf)[i] = TWDR;
    }
}

//-------------------------------------------------------------------------------
void RAM::Target::I2C::write(
    const bool start,
    const uint8_t addr,
    const void *buf,
    const size_t size)
{
    if ( start )
    {
        startf(addr, false);
    }

    for ( unsigned int i = 0; i < size; ++i )
    {
        TWDR = reinterpret_cast<const uint8_t *>(buf)[i];
        
        TWCR = (1<<TWINT) | (1<<TWEN);

        wait();
    }
}

//-------------------------------------------------------------------------------
bool RAM::Target::I2C::ping(uint8_t addr)
{
    startf(addr, true);

    bool status = ((TWSR & 0xF8) == TW_MR_SLA_ACK);

    stop();

    return status;
}

//-------------------------------------------------------------------------------
bool RAM::Target::I2C::wait()
{
    uint16_t count = 255;

    while ( !(TWCR & (1<<TWINT)) )
    {
        count--;

        //
        //we are in a blocking state => we don't insist
        //
        if (count==0)
        {            
            //
            //and we force a reset on TWINT register
            //
            TWCR = 0;

            //
            //we take a timestamp here to neutralize the value during a short delay
            //
            neutralizeTime = micros();

            m_errorsCount++;

            break;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------
void RAM::Target::I2C::stop()
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

    //
    // <- can produce a blocking state with some WMP clones
    //
    //  while(TWCR & (1<<TWSTO));
}
 
/*___oOo__*/
