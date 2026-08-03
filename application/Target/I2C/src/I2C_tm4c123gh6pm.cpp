//
// Support stuff
//
#include <ErrorCodes.h>
#include <Utils.h>

//
// Local stuff
//
#include "TargetI2C.h"

using namespace RAM::Target;

I2C0_Type *i2cp[static_cast<int>(I2C::ModuleId::IDN)] =
{
    I2C0,
    I2C1,
    I2C2,
    I2C3
};

IRQn_Type irqNumber[static_cast<int>(I2C::ModuleId::IDN)] =
{
    I2C0_IRQn, 
    I2C1_IRQn, 
    I2C2_IRQn, 
    I2C3_IRQn
};

//-------------------------------------------------------------------------------
static int fixStuckSDA(const GPIO::Pin & modulePins)
{
    unsigned counter = 0;
    GPIO::Pin tmp;
    
   //
   // if i2c lines seem to be stuck (scl high and sda down) then try
   // to unlock it
   //
   // http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
   //
   if ( GPIO::configure(
            modulePins.m_portId,
            tmp,
            modulePins.m_inMask,
            0x0) )
   {
       return 1;
   }

   //
   // read current line status
   //
   GPIO::DataType val = GPIO::Digital::read(tmp);
   
   //
   // the following is tricky. GPIO module does not tell which is scl
   // and sda, but it configures sda as open-drain. If sda is down
   // and scl is not, then assume that the line is locked.
   //
   if ( (val & modulePins.m_openDrainMask) == 0
        &&
        val != 0 ) 
   {
       if ( setDirection(
                tmp,
                tmp.m_inMask & ~modulePins.m_openDrainMask,
                GPIO::DirectionMode::OUTPUT_PIN) )
       {
           return 1;
       }

       do
       {
           //
           // clock down
           //
           GPIO::Digital::write(tmp, 0x0);
           
           RAM::Support::Utils::delayus(5);

           //
           // clock up
           //
           GPIO::Digital::write(tmp, tmp.m_outMask);
           
           RAM::Support::Utils::delayus(5);
           
           val = (GPIO::Digital::read(tmp) & modulePins.m_openDrainMask);
           
           ++counter;
           
       } while ( val == 0 && counter < 20 );
   }

   //
   // leave pins disabled, as they were expected to be when entering
   // the present function (otherwise config above would have failed.)
   //
   GPIO::disable(tmp);
   
   if ( val == 0 )
   {
       return 1;
   }
   
   return 0;
}

//-------------------------------------------------------------------------------
int RAM::Target::I2C::configure(
    const ModuleId moduleId,
    const unsigned kbps,
    const bool enableInternalPullup,
    Module &module)
{
    if ( moduleId == ModuleId::IDN )
    {
        return I2C_ERR_CONFIG;
    }

    module.m_id = moduleId;
    
    module.m_index = unsigned(moduleId);
    
    module.m_pointer = reinterpret_cast<intptr_t>(i2cp[module.m_index]);
    
    module.m_irqn = irqNumber[module.m_index];

    // http://stackoverflow.com/questions/24659919/i2c-interface-on-tiva

    //
    // populate pins structure with scl and sda signals in associated
    // gpio module
    //
    if ( GPIO::configure(moduleId, module.m_pins) )
    {
        return I2C_ERR_CONFIG;
    }

    //
    // check bus lines and if they do not look okay, then, try to fix them
    //
    if ( fixStuckSDA(module.m_pins) )
    {
        return I2C_ERR_SDA_STUCK;
    }
   
    //
    // enable module (RCGCI2C register see page 348)
    //
    if ( (SYSCTL->RCGCI2C & (0x1 << module.m_index)) == 0 )
    {
        SYSCTL->RCGCI2C |= (0x1 << module.m_index); 

        //
        // wait until module ready
        //
        while ( (SYSCTL->PRI2C & (0x1 << module.m_index)) == 0 );
    }

    //
    // configure scl and sda signals in associated gpio module
    //
    if ( GPIO::enable(module.m_pins) )
    {
        return I2C_ERR_CONFIG;
    }

    //
    // enable i2c master block
    //
    i2cp[module.m_index]->MCR |= I2C_MCR_MFE;    

    //
    // i2c timer period to achive a given clock period. See equation in page
    // 1026. 400 kbps implies TPR=9 and 100 kbps implies TPR=39.
    //
    if ( kbps == 400 )
    {
        i2cp[module.m_index]->MTPR = 0x8;
    }
    else
    {
        i2cp[module.m_index]->MTPR = 39;
    }

    //
    // FIXME: High-Speed not checked.
    //

    //
    // tm4c123 has no i2c fifos :-(
    //

    return 0;
}

//-------------------------------------------------------------------------------
size_t RAM::Target::I2C::read(
    const Module &module,
    const uint8_t addr,
    const uint8_t reg,
    void *buf,
    const size_t size)
{
    size_t ret = 0;

    //
    // slave address set to write
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MSA = (addr << 1);

    //
    // byte to write is register address
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MDR = reg;

    //
    // trigger start followed by transmit, remain in transmit state
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b11;

    //
    // wait for transaction to complete
    //
    if ( !wait(module) )
    {
        return ret;
    }

    //
    // slave address set to read now
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MSA = (addr << 1) | 0b1;

    for ( ; ret < size; ++ret )
    {
        if ( ret == 0 )
        {
            if ( size == 1 )
            {
                //
                // stop+start+run
                //
                reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b111;
            }
            else
            {
                //
                // ack+start+run
                //
                reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b1011;
            }
        }
        else if ( ret == size - 1 )
        {
            //
            // stop+run
            //
            reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b101;
        }
        else
        {
            //
            // ack+run
            //
            reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b1001;
        }
        
        if ( !wait(module) )
        {
            break;
        }
    
        static_cast<uint8_t *>(buf)[ret] =
            reinterpret_cast<I2C0_Type *>(module.m_pointer)->MDR;
    }

    return ret;
}

//-------------------------------------------------------------------------------
size_t RAM::Target::I2C::write(
    const Module &module,
    const uint8_t addr,
    const uint8_t reg,
    const void *buf,
    const size_t size)
{
    size_t ret = 0;
    
    //
    // slave address set to write
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MSA = (addr << 1);

    //
    // byte to write is register address
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MDR = reg;

    //
    // trigger start followed by transmit, remain in transmit state
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b11;

    //
    // wait for transaction to complete
    //
    if ( !wait(module) )
    {
        return ret;
    }

    for ( ; ret < size; ++ret )
    {
        reinterpret_cast<I2C0_Type *>(module.m_pointer)->MDR = reinterpret_cast<const uint8_t *>(buf)[ret];

        if ( ret == size - 1 )
        {
            reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b101; /* stop+run */
        }
        else
        {
            reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b1; /* run */
        }
        
        if ( !wait(module) )
        {
            break;
        }
    }

    return ret;
}

//-------------------------------------------------------------------------------
void RAM::Target::I2C::read(
    const Module &module,
    const bool start,
    const uint8_t addr,
    void *buf,
    const size_t size)
{
    //
    // R/S set 1 (Receive)
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MSA = (addr << 1) | 0b1;

    //
    // data sheet says that this wait is only needed on a multi master scenario
    //
    wait(module);

    for ( unsigned int i = 0; i < size; ++i )
    {
        //
        // ack?+stop?+start?+run
        //
        reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS =
            ((i != size - 1) ? 0b1000 : 0b0)
            |
            ((start && i == 0) ? 0b10 : 0b0)
            |
            0b1;

        wait(module);

        reinterpret_cast<uint8_t *>(buf)[i] = reinterpret_cast<I2C0_Type *>(module.m_pointer)->MDR;
    }
}

//-------------------------------------------------------------------------------
void RAM::Target::I2C::write(
    const Module &module,
    const bool start,
    const uint8_t addr,
    const void *buf,
    const size_t size)
{
    //
    // R/S bit left 0 (Send)
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MSA = (addr << 1);
    
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MDR = reinterpret_cast<const uint8_t *>(buf)[0];
    
    //
    // start?+run
    //
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b1 | (start ? 0b10 : 0b0);
    
    //
    // if not currently busy then first wait to be busy
    // FIXME: why do I need to do this here and not in 'read'?
    //
    while ( (reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS & I2C_MCS_BUSY) == 0 );

    //
    // data sheet says that this wait is only needed on a multi master scenario
    //
    wait(module);
    
    for ( unsigned int i = 1; i < size; ++i )
    {
        reinterpret_cast<I2C0_Type *>(module.m_pointer)->MDR = reinterpret_cast<const uint8_t *>(buf)[i];
        
        //
        // run
        //
        reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b1;
        
        wait(module);
    }
}

//-------------------------------------------------------------------------------
bool RAM::Target::I2C::ping(
    const Module &module,
    uint8_t addr)
{
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MSA = (addr << 1) | 0b1;

    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b111;

    wait(module);

    //
    // failed to detect address acknowledge means no slave at that address
    //
    if ( reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS & 0b100 )
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------
bool RAM::Target::I2C::wait(const Module &module)
{
    RAM::Target::RegType rval;
    
    //
    // FIXME: shouldn't be here a timeout guard?
    //
    while ( (rval = reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS) & I2C_MCS_BUSY );

    //
    // if an error happened then return false
    //
    return (rval & I2C_MCS_ERROR) ? false : true;
}

//-------------------------------------------------------------------------------
void RAM::Target::I2C::stop(const Module &module)
{
    reinterpret_cast<I2C0_Type *>(module.m_pointer)->MCS = 0b100;

    wait(module);
}
 
/*___oOo__*/
