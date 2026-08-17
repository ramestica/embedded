//
// Library stuff
//
#include <Logic/I2C.h>
#include <Logic/Endianness.h>
#include <Logic/Host.h>
#include <Logic/MainLoop.h>

//
// Local stuff
//
#include <Sensors/MPU6050.h>

using namespace RAM::Support;
using namespace RAM::Logic;
using namespace RAM::Sensor::MEMS;

//-------------------------------------------------------------------------------
MPU6050Base::MPU6050Base(
    const uint8_t addr,
    float *output,
    const unsigned nslaves,
    uint8_t *slaves,
    CalibratedAxis<3> *gyroc,
    CalibratedAxis<3> *accelc):
    m_i2c(I2C::instance()),
    m_addr(addr),
    m_output(output),
    m_nslaves(nslaves),
    m_slaves(slaves),
    m_gyroc(gyroc),
    m_accelc(accelc),
    m_getNumberSlavesCmd(this),
    m_readoutCmd(this),
    m_readSlaveRegisterCmd(this),
    m_writeSlaveRegisterCmd(this),
    m_setAccelerometerCalibrationCmd(this),
    m_toggleCalibrationCmd(this)
{
    Host::CommandReply::CommandList::instance().
        setCommandHandler(&m_getNumberSlavesCmd);

    Host::CommandReply::CommandList::instance().
        setCommandHandler(&m_readoutCmd);

    Host::CommandReply::CommandList::instance().
        setCommandHandler(&m_readSlaveRegisterCmd);

    Host::CommandReply::CommandList::instance().
        setCommandHandler(&m_writeSlaveRegisterCmd);

    Host::CommandReply::CommandList::instance().
        setCommandHandler(&m_setAccelerometerCalibrationCmd);

    Host::CommandReply::CommandList::instance().
        setCommandHandler(&m_toggleCalibrationCmd);
}

//-------------------------------------------------------------------------------
void MPU6050Base::initialize()
{
    configure();

    if ( m_nslaves == 0 )
    {
        return;
    }
                    
    //
    // enable master mode on auxiliary i2c bus
    //
    bitset(0, MPU6050_RA_USER_CTRL, MPU6050_I2C_MST_EN);

    //
    // disable i2c pass-through
    //
    bitunset(0, MPU6050_RA_INT_PIN_CFG, MPU6050_I2C_BYPASS_EN);

    //
    // enable sensor data shadowing, I think that this means that the when
    // sampling for all 18 variables they could be at least as old as the so
    // milliseconds that it takes for the device to readout slaves through the
    // auxiliary i2c bus. How could this be improved?
    //
    bitset(0, MPU6050_RA_I2C_MST_DELAY_CTRL, MPU6050_DELAY_ES_SHADOW);

    for ( unsigned i = 0; i < m_nslaves; ++i )
    {
        configure(m_slaves[i]);

        //
        // slave accelerometer sampling
        //
        setSlaveReadoutConfig(
            MPU6050_RA_I2C_SLV0_ADDR + 2 * i * 3,
            MPU6050_RA_I2C_SLV0_REG + 2 * i * 3,
            MPU6050_RA_I2C_SLV0_CTRL + 2 * i * 3,
            m_slaves[i],
            MPU6050_RA_ACCEL_OUT,
            6);

        //
        // slave gyroscope sampling
        //
        setSlaveReadoutConfig(
            MPU6050_RA_I2C_SLV0_ADDR + (2 * i + 1) * 3,
            MPU6050_RA_I2C_SLV0_REG + (2 * i + 1) * 3,
            MPU6050_RA_I2C_SLV0_CTRL + (2 * i + 1) * 3,
            m_slaves[i],
            MPU6050_RA_GYRO_OUT,
            6);

        //////////////////////////////
        //
        // reading 12 variables from both slaves, at 400 kHz i2c clocking, would
        // take:
        //
        //     1000*(12*2*8)/(400e3/8) = 0.48 ms
        //
        // reading all 18+1 variables from all three devices would take:
        //
        //     1000*(19*2*8)/(400e3) = 0.76 ms
        //
        //     Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
    }
}
                
//-------------------------------------------------------------------------------
void MPU6050Base::calibrate()
{
    return;

    for ( unsigned i = 0; i < m_nslaves + 1; ++i )
    {
        for ( unsigned j = 0; j < 3; ++j )
        {
            m_accelc[i].m_scale[j] = 1 / 16384.;

            m_accelc[i].m_offset[j] = 0.;

            m_gyroc[i].m_scale[j] = 250.0f / (0x1 << 15);

            m_gyroc[i].m_offset[j] = 0.0f;
        }
    }
}

//-------------------------------------------------------------------------------
void MPU6050Base::readout()
{
    //
    // copy with only one i2c transaction all values including undesired master
    // temperature
    //
    I2C::instance().read(
        m_addr,
        MPU6050_RA_ACCEL_OUT,
        m_output,
        sizeof(int16_t)
        *
        ((m_nslaves == 0 ? 0 : 1) + 6 * (1 + m_nslaves)));

    //
    // it is important to go backwards, because the order in which data is
    // converted in-place through m_output. It is important that applyCalib goes
    // backwards as well.
    //
    //  0 -  2: accel    0 -  2: accel
    //  3 -  3: temp     3 -  5: gyro   
    //  4 -  6: gyro     6 -  8: accel  
    //  7 -  9: accel    9 - 11: gyro   
    // 10 - 12: gyro    12 - 14: accel  
    // 13 - 15: accel   15 - 17: gyro   
    // 16 - 18: gyro    
    //
    for ( int i = m_nslaves; i >= 0; --i )
    {
        m_gyroc[i].apply(
            reinterpret_cast<int16_t *>(m_output) + 3 * (2 * i + 1) + (i == 0 ? 0 : 1),
            m_output + 3 * (2 * i + 1));
        
        m_accelc[i].apply(
            reinterpret_cast<int16_t *>(m_output) + 3 * (2 * i) + (i == 0 ? 0 : 1),
            m_output + 3 * (2 * i));
    }
}

//-------------------------------------------------------------------------------
void MPU6050Base::configure(const uint8_t addr)
{
    write(addr, MPU6050_RA_PWR_MGMT_1, MPU6050_CLOCK_PLL_XGYRO);

    bitunset(
        addr,
        MPU6050_RA_PWR_MGMT_1,
        MPU6050_PWR_MGMT_1_SLEEP & MPU6050_PWR_MGMT_1_CYCLE);
    
    //
    // when addressing a slave then check that it was right
    //
    if ( addr != 0
         &&
         read(addr, MPU6050_RA_PWR_MGMT_1) != 0x1 )
    {
        RAM::Support::Error::longjmp(MPU6050_ERR_SLAVE_CHECK);
    }
    
    bitset(addr, MPU6050_RA_GYRO_CONFIG, MPU6050_GYRO_FS_250);
    
    bitset(addr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACCEL_FS_2G);
    
    bitset(addr, MPU6050_RA_CONFIG, MPU6050_CONFIG_DLPF_0);    
}

//-------------------------------------------------------------------------------
void MPU6050Base::setSlaveReadoutConfig(
    const uint8_t slvAddrReg,
    const uint8_t slvRegReg,
    const uint8_t slvCtrlReg,
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t length)
{
    I2C::instance().write(m_addr, slvAddrReg, addr | 0b10000000);
    
    I2C::instance().write(m_addr, slvRegReg, reg);
    
    I2C::instance().write(m_addr, slvCtrlReg, MPU6050_RA_I2C_SLV_LEN_MASK, length);
    
    I2C::instance().bitset(m_addr, slvCtrlReg, MPU6050_I2C_SLV_EN);
}

//-------------------------------------------------------------------------------
uint8_t MPU6050Base::read(const uint8_t addr, const uint8_t reg)
{
    if ( addr == 0 )
    {
        return I2C::instance().read(m_addr, reg);
    }
                    
    for ( unsigned int i = 0; i < m_nslaves; ++i )
    {
        if ( m_slaves[i] != addr )
        {
            continue;
        }

        I2C::instance().write(m_addr, MPU6050_RA_I2C_SLV4_ADDR, addr | 0b10000000);
        
        I2C::instance().write(m_addr, MPU6050_RA_I2C_SLV4_REG, reg);
        
        I2C::instance().bitset(m_addr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN);
        
        //
        // wait until the operation is finished (enable bit rest to zero)
        //
        while ( true )
        {
            if ( (I2C::instance().read(m_addr, MPU6050_RA_I2C_SLV4_CTRL)
                  &
                  MPU6050_I2C_SLV4_EN)
                 ==
                 0x0 )
            {
                break;
            }
        }
        
        return I2C::instance().read(m_addr, MPU6050_RA_I2C_SLV4_DI);
    }
    
    RAM::Support::Error::longjmp(MPU6050_ERR_INVALID_SENSOR);
    
    return 0;
}

//-------------------------------------------------------------------------------
void MPU6050Base::write(
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t val)
{
    if ( addr == 0 )
    {
        I2C::instance().write(m_addr, reg, val);
        
        return;
    }

    for ( unsigned int i = 0; i < m_nslaves; ++i )
    {
        if ( m_slaves[i] != addr )
        {
            continue;
        }
        
        //
        // note the implicit write mode in the slave address being a
        // 7 bits value
        //
        I2C::instance().write(m_addr, MPU6050_RA_I2C_SLV4_ADDR, addr);
        
        I2C::instance().write(m_addr, MPU6050_RA_I2C_SLV4_REG, reg);
        
        I2C::instance().write(m_addr, MPU6050_RA_I2C_SLV4_DO, val);
        
        I2C::instance().write(m_addr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN);
        
        //
        // wait until the operation is finished (enable bit rest to zero)
        //
        while ( true )
        {
            if ( (I2C::instance().read(
                      m_addr,
                      MPU6050_RA_I2C_SLV4_CTRL) & MPU6050_I2C_SLV4_EN)
                 ==
                 0x0 )
            {
                break;
            }
        }
        
        return;
    }
    
    RAM::Support::Error::longjmp(MPU6050_ERR_INVALID_SENSOR);
}

//-------------------------------------------------------------------------------
void MPU6050Base::GetNumberSlavesCmd::execute()
{
    reply(Host::CommandReply::ReplyId::OK, m_obj_p->m_nslaves);
}

//-------------------------------------------------------------------------------
void MPU6050Base::ReadoutCmd::execute()
{
    reply(
        Host::CommandReply::ReplyId::OK,
        reinterpret_cast<uint8_t *>(m_obj_p->m_output),
        (1 + m_obj_p->m_nslaves) * 3 * 2 * sizeof(float));
}

//-------------------------------------------------------------------------------
void MPU6050Base::ReadSlaveRegisterCmd::execute()
{
    uint8_t addr, reg;

    if ( !I2C::instance().serialReadAddrReg(*this, addr, reg) )
    {
        return;
    }

    reply(Host::CommandReply::ReplyId::OK, m_obj_p->read(addr, reg));
}

//-------------------------------------------------------------------------------
void MPU6050Base::WriteSlaveRegisterCmd::execute()
{
    uint8_t addr, reg, val;

    if ( !I2C::instance().serialReadAddrRegVal(*this, addr, reg, val) )
    {
        return;
    }

    m_obj_p->write(addr, reg, val);

    reply(Host::CommandReply::ReplyId::OK);
}

//-------------------------------------------------------------------------------
void MPU6050Base::SetAccelerometerCalibrationCmd::execute()
{
    unsigned int idx = 1 + m_obj_p->m_nslaves;
    uint8_t addr = 0x0;
    uint8_t *scale;
    uint8_t *offset;

    //
    // read accelerometer address
    //
    if ( !I2C::instance().serialReadAddr(*this, addr) )
    {
        return;
    }

    if ( addr == 0x0 )
    {
        idx = 0;
    }
    else
    {
        for ( unsigned int i = 0; i < m_obj_p->m_nslaves; ++i )
        {
            if ( m_obj_p->m_slaves[i] != addr )
            {
                continue;
            }

            idx = 1 + i;
        }
    }
    
    //
    // if it is not clear which sensor calibration to set then complain
    //
    if ( idx > m_obj_p->m_nslaves )
    {
        reply(SerialReplyId::INVALID_SENSOR);
        
        return;
    }

    scale = reinterpret_cast<uint8_t *>(m_obj_p->m_accelc[idx].m_scale);

    offset = reinterpret_cast<uint8_t *>(m_obj_p->m_accelc[idx].m_offset);

    //
    // if not enough data available then complain
    //
    if ( m_dataStream_p->available() != 6 * sizeof(float) )
    {
        reply(SerialReplyId::INVALID_CALIB_SIZE);

        return;
    }

    //
    // read scales
    //
    m_dataStream_p->read(reinterpret_cast<char *>(scale), 3 * sizeof(float));

    //
    // read offsets
    //
    m_dataStream_p->read(reinterpret_cast<char *>(offset), 3 * sizeof(float));

    reply(Host::CommandReply::ReplyId::OK);
}

//-------------------------------------------------------------------------------
void MPU6050Base::ToggleCalibrationCmd::execute()
{
    bool ret = 0;
    
    for ( unsigned i = 0; i < 1 + m_obj_p->m_nslaves; ++i )
    {
        m_obj_p->m_accelc[i].m_enabled = !m_obj_p->m_accelc[i].m_enabled;

        ret |= ((m_obj_p->m_accelc[i].m_enabled ? 0x1 : 0x0) << i);
            
        m_obj_p->m_gyroc[i].m_enabled = !m_obj_p->m_gyroc[i].m_enabled;

        ret |= ((m_obj_p->m_gyroc[i].m_enabled ? 0x1 : 0x0) << i);
    }
    
    reply(Host::CommandReply::ReplyId::OK, ret);
}

/*___oOo___*/
