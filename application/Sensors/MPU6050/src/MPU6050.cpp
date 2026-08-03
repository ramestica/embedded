//
// Target stuff
//
#include <target/SystemTick.h>

//
// Library stuff
//
#include <Host.h>
#include <I2C.h>
#include <Endianness.h>
#include <Host.h>
#include <MainLoop.h>

//
// Local stuff
//
#include "MPU6050.h"

using namespace RAM::Support;
using namespace RAM::Device;
using namespace RAM::Logic;

//
// static variables
//
const unsigned int MPU6050::m_numOutputScalars;

//-------------------------------------------------------------------------------
MPU6050::MPU6050(const uint8_t addr, float *output):
    m_i2cAddr(addr),
    m_output(output),
    m_calibrationFlag(false),
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

    //
    // default calibration as per datasheet and zero offsets
    //
    for ( int i = 0; i < 3; ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            //
            // negate scale such that 'the convention will be adopted
            // that the accelerometer output is negated to give value
            // +1g in any axis aligned with the earth's downward
            // gravitational field', see AN3461.pdf note.
            //
            m_accelScale[i][j] = -1 / 16384.;
            m_accelScale[i][j] = 1.;
            m_accelOffset[i][j] = 0.;

            //
            // scale from degrees to radians
            //
            m_gyroScale[i][j] = ((250.0f / (0x1 << 15)) * 3.1415927410125732421875f) / 180.0f;

            m_gyroOffset[i][j] = 0.0f;
        }

        m_gyroOffset[i][0] = 0.0483;
        m_gyroOffset[i][1] = -0.0490;
        m_gyroOffset[i][2] = 0.0145;
    }

    //
    // accelerometer calibration parameters as in mpu6050.chip1[2019-01-19T19:35:28]
    //
    m_accelScale[0][0] = 6.060174657228001e-05;
    m_accelScale[0][1] = 6.07904501870389e-05;
    m_accelScale[0][2] = 5.920893698769557e-05;
    m_accelOffset[0][0] = 0.05778662253946197;
    m_accelOffset[0][1] = -0.00997959168514515;
    m_accelOffset[0][2] = -0.15267160007785488;
}

//-------------------------------------------------------------------------------
void MPU6050::initialize()
{
    //
    // reset on-chip i2c master circuitry (disable, reset and wait a bit)
    //
    I2C::instance().bitunset(
        m_i2cAddr,
        MPU6050_RA_USER_CTRL,
        MPU6050_I2C_MST_EN);
    I2C::instance().bitset(
        m_i2cAddr,
        MPU6050_RA_USER_CTRL,
        MPU6050_I2C_MST_RESET);
    RAM::Target::SystemTick::delayus(1000);
    
    I2C::instance().write(
        m_i2cAddr,
        MPU6050_RA_PWR_MGMT_1,
        MPU6050_CLOCK_PLL_XGYRO);
    
    //
    // FIXME: '&' seems to be wrong!
    //
    I2C::instance().bitunset(
        m_i2cAddr,
        MPU6050_RA_PWR_MGMT_1,
        MPU6050_PWR_MGMT_1_SLEEP & MPU6050_PWR_MGMT_1_CYCLE);

    I2C::instance().bitset(
        m_i2cAddr,
        MPU6050_RA_GYRO_CONFIG,
        MPU6050_GYRO_FS_250);
    
    I2C::instance().write(
        m_i2cAddr,
        MPU6050_RA_ACCEL_CONFIG,
        MPU6050_ACCEL_FS_2G);

    I2C::instance().bitset(
        m_i2cAddr,
        MPU6050_RA_CONFIG,
        MPU6050_CONFIG_DLPF_0);

    //
    // enable master mode on auxiliary i2c bus
    //
    I2C::instance().bitset(
        m_i2cAddr,
        MPU6050_RA_USER_CTRL,
        MPU6050_I2C_MST_EN);

    //
    // disable i2c pass-through
    //
    I2C::instance().bitunset(
        m_i2cAddr,
        MPU6050_RA_INT_PIN_CFG,
        MPU6050_I2C_BYPASS_EN);

    //
    // enable sensor data shadowing, I think that this means that the when
    // sampling for all 18 variables they could be at least as old as the so
    // milliseconds that it takes for the device to readout slaves through the
    // auxiliary i2c bus. How could this be improved?
    //
    I2C::instance().bitset(
        m_i2cAddr,
        MPU6050_RA_I2C_MST_DELAY_CTRL,
        MPU6050_DELAY_ES_SHADOW);

    //
    // must wait a bit for things to settle
    //
    RAM::Target::SystemTick::delayus(250000);
    
    return;
    //////////////////////////////
    // first (lower address) slave i2c device configuration

    setSlaveConfig(MPU6050_ADDRESS_AD0_LOW);

    //////////////////////////////
    // second (higher address) slave i2c device configuration

    setSlaveConfig(MPU6050_ADDRESS_AD0_HIGH);

    //////////////////////////////
    // first slave accelerometer sampling

    setSlaveReadoutConfig(
        MPU6050_RA_I2C_SLV0_ADDR,
        MPU6050_RA_I2C_SLV0_REG,
        MPU6050_RA_I2C_SLV0_CTRL,
        MPU6050_ADDRESS_AD0_LOW,
        MPU6050_RA_ACCEL_OUT,
        6);

    //////////////////////////////
    // first slave gyroscope sampling

    setSlaveReadoutConfig(
        MPU6050_RA_I2C_SLV1_ADDR,
        MPU6050_RA_I2C_SLV1_REG,
        MPU6050_RA_I2C_SLV1_CTRL,
        MPU6050_ADDRESS_AD0_LOW,
        MPU6050_RA_GYRO_OUT,
        6);

    //////////////////////////////
    // second slave accelerometer sampling

    setSlaveReadoutConfig(
        MPU6050_RA_I2C_SLV2_ADDR,
        MPU6050_RA_I2C_SLV2_REG,
        MPU6050_RA_I2C_SLV2_CTRL,
        MPU6050_ADDRESS_AD0_HIGH,
        MPU6050_RA_ACCEL_OUT,
        6);

    //////////////////////////////
    // second slave gyroscope sampling

    setSlaveReadoutConfig(
        MPU6050_RA_I2C_SLV3_ADDR,
        MPU6050_RA_I2C_SLV3_REG,
        MPU6050_RA_I2C_SLV3_CTRL,
        MPU6050_ADDRESS_AD0_HIGH,
        MPU6050_RA_GYRO_OUT,
        6);

    //////////////////////////////

    //
    // reading 12 variables from both slaves, at 400 kHz i2c clocking, would take:
    //
    //     1000*(12*2*8)/(400e3/8) = 0.48 ms
    //
    // reading all 18+1 variables from all three devices would take:
    //
    //     1000*(19*2*8)/(400e3) = 0.76 ms
    //
    //     Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
}

//-------------------------------------------------------------------------------
bool MPU6050::selfTest()
{
    //
    // accelerometer self-test
    //
    I2C::instance().write(
        m_i2cAddr,
        MPU6050_RA_ACCEL_CONFIG,
        MPU6050_ACCEL_FS_8G | MPU6050_ACCEL_XA_ST | MPU6050_ACCEL_YA_ST | MPU6050_ACCEL_ZA_ST);
    RAM::Target::SystemTick::delayus(250000);
    I2C::instance().read(
        m_i2cAddr,
        MPU6050_RA_ACCEL_OUT,
        reinterpret_cast<int16_t *>(m_output),
        sizeof(int16_t) * m_numOutputScalars);
    Endianness::adapt(reinterpret_cast<int16_t *>(m_output)[0], reinterpret_cast<int16_t *>(m_output)[0]);
    Endianness::adapt(reinterpret_cast<int16_t *>(m_output)[1], reinterpret_cast<int16_t *>(m_output)[1]);
    Endianness::adapt(reinterpret_cast<int16_t *>(m_output)[2], reinterpret_cast<int16_t *>(m_output)[2]);
    uint8_t self_test_x = I2C::instance().read(m_i2cAddr, MPU6050_RA_SELF_TEST_X);
    uint8_t self_test_y = I2C::instance().read(m_i2cAddr, MPU6050_RA_SELF_TEST_Y);
    uint8_t self_test_z = I2C::instance().read(m_i2cAddr, MPU6050_RA_SELF_TEST_Z);
    uint8_t self_test_a = I2C::instance().read(m_i2cAddr, MPU6050_RA_SELF_TEST_A);
    self_test_x = (self_test_x >> MPU6050_ACCEL_HIGH_SHIFT_TEST) | ((self_test_a >> MPU6050_ACCEL_LOW_XSHIFT_TEST) & 0x3);
    self_test_y = (self_test_y >> MPU6050_ACCEL_HIGH_SHIFT_TEST) | ((self_test_a >> MPU6050_ACCEL_LOW_YSHIFT_TEST) & 0x3);
    self_test_z = (self_test_z >> MPU6050_ACCEL_HIGH_SHIFT_TEST) | ((self_test_a >> MPU6050_ACCEL_LOW_ZSHIFT_TEST) & 0x3);
    Host::Console::println(self_test_x);
    Host::Console::println(reinterpret_cast<int16_t *>(m_output)[0]);
    Host::Console::println(reinterpret_cast<int16_t *>(m_output)[1]);
    Host::Console::println(reinterpret_cast<int16_t *>(m_output)[2]);
    Host::Console::println(self_test_y);
    Host::Console::println(self_test_z);
    I2C::instance().write(
        m_i2cAddr,
        MPU6050_RA_ACCEL_CONFIG,
        MPU6050_ACCEL_FS_2G);
}

//-------------------------------------------------------------------------------
void MPU6050::calibrate()
{
    m_calibrationFlag = false;

    m_gyroOffset[0][0] = 0;

    m_gyroOffset[0][1] = 0;

    m_gyroOffset[0][2] = 0;

    for ( auto i = 0; i < m_calibrateIterations; ++i )
    {
        RAM::Target::SystemTick::delayus(2000);

        readout();

        m_gyroOffset[0][0] += m_output[3];
        
        m_gyroOffset[0][1] += m_output[4];

        m_gyroOffset[0][2] += m_output[5];
    }

    m_gyroOffset[0][0] = -(m_gyroOffset[0][0] / m_calibrateIterations) * m_gyroScale[0][0];
    m_gyroOffset[0][1] = -(m_gyroOffset[0][1] / m_calibrateIterations) * m_gyroScale[0][1];
    m_gyroOffset[0][2] = -(m_gyroOffset[0][2] / m_calibrateIterations) * m_gyroScale[0][2];

    m_calibrationFlag = true;

    RAM::Logic::Host::Console::println("zxz");
    RAM::Logic::Host::Console::println(m_gyroOffset[0][0]);
    RAM::Logic::Host::Console::println(m_gyroOffset[0][1]);
    //m_gyroOffset[0][1] = 0;
    RAM::Logic::Host::Console::println(m_gyroOffset[0][2]);
}

//-------------------------------------------------------------------------------
void MPU6050::setAccelerometerCalibration(
    const uint8_t addr,
    const float *scale,
    const float *offset)
{
    unsigned int idx;

    if ( addr == 0x0 )
    {
        idx = 0;
    }
    else if ( addr == MPU6050_ADDRESS_AD0_LOW )
    {
        idx = 1;
    }
    else
    {
        idx = 2;
    }

    for ( int i = 0; i < 3; ++i )
    {
        m_accelScale[idx][i] = scale[i];

        m_accelOffset[idx][i] = offset[i];
    }
}

//-------------------------------------------------------------------------------
void MPU6050::readout()
{
    I2C::instance().read(
        m_i2cAddr,
        MPU6050_RA_ACCEL_OUT,
        reinterpret_cast<int16_t *>(m_output),
        sizeof(int16_t) * m_numOutputScalars);

    //
    // it is important to go backwards, because the order in which
    // data is converted in-place through m_output. It is important
    // that applyCalib goes backwards as well.
    //
    //  0 -  2: accel    0 -  2: accel
    //  3 -  3: temp     3 -  5: gyro   
    //  4 -  6: gyro     6 -  8: accel  
    //  7 -  9: accel    9 - 11: gyro   
    // 10 - 12: gyro    12 - 14: accel  
    // 13 - 15: accel   15 - 17: gyro   
    // 16 - 18: gyro    
    //
    for ( int i = 2; i >= 0; --i )
    {
        applyCalib(
            m_output + 3 * (2 * i + 1),
            reinterpret_cast<int16_t *>(m_output) + 3 * (2 * i + 1) + 1,
            &m_gyroScale[i][0],
            &m_gyroOffset[i][0]);

        applyCalib(
            m_output + 3 * (2 * i),
            reinterpret_cast<int16_t *>(m_output) + 3 * (2 * i) + (i == 0 ? 0 : 1),
            &m_accelScale[i][0],
            &m_accelOffset[i][0]);
    }
}

//-------------------------------------------------------------------------------
void MPU6050::applyCalib(
    float *output,
    int16_t *input,
    const float *scale,
    const float *offset)
{
    for ( int i = 2; i >= 0; --i )
    {
        Endianness::adapt(input[i], input[i]);

        if ( m_calibrationFlag )
        {
            output[i] = input[i] * scale[i] + offset[i];
        }
        else
        {
            output[i] = input[i]; // misra 18.2 violation!
        }
    }
}

//-------------------------------------------------------------------------------
void MPU6050::writeSlaveRegister(
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t val)
{
    //
    // note the implicit write mode in the slave address being a 7 bits value
    //
    I2C::instance().write(m_i2cAddr, MPU6050_RA_I2C_SLV4_ADDR, addr);

    I2C::instance().write(m_i2cAddr, MPU6050_RA_I2C_SLV4_REG, reg);

    I2C::instance().write(m_i2cAddr, MPU6050_RA_I2C_SLV4_DO, val);

    I2C::instance().write(m_i2cAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN);

    //
    // wait until the operation is finished (enable bit rest to zero)
    //
    while ( true )
    {
        if ( (I2C::instance().read(
                  m_i2cAddr,
                  MPU6050_RA_I2C_SLV4_CTRL) & MPU6050_I2C_SLV4_EN)
             ==
             0x0 )
        {
            break;
        }
    }
}

//-------------------------------------------------------------------------------
uint8_t MPU6050::readSlaveRegister(const uint8_t addr, const uint8_t reg)
{
    I2C::instance().write(m_i2cAddr, MPU6050_RA_I2C_SLV4_ADDR, addr | 0b10000000);

    I2C::instance().write(m_i2cAddr, MPU6050_RA_I2C_SLV4_REG, reg);

    I2C::instance().bitset(m_i2cAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN);

    //
    // wait until the operation is finished (enable bit rest to zero)
    //
    while ( true )
    {
        if ( (I2C::instance().read(m_i2cAddr, MPU6050_RA_I2C_SLV4_CTRL)
              &
              MPU6050_I2C_SLV4_EN)
             ==
             0x0 )
        {
            break;
        }
    }

    return I2C::instance().read(m_i2cAddr, MPU6050_RA_I2C_SLV4_DI);
}

//-------------------------------------------------------------------------------
void MPU6050::bitsetSlaveRegister(
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t bits)
{
    writeSlaveRegister(addr, reg, readSlaveRegister(addr, reg) | bits);
}

//-------------------------------------------------------------------------------
void MPU6050::bitunsetSlaveRegister(
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t bits)
{
    writeSlaveRegister(addr, reg, readSlaveRegister(addr, reg) & ~bits);
}

//-------------------------------------------------------------------------------
void MPU6050::setSlaveConfig(const uint8_t addr)
{
    writeSlaveRegister(
        addr,
        MPU6050_RA_PWR_MGMT_1,
        MPU6050_CLOCK_PLL_XGYRO);
    
    bitunsetSlaveRegister(
        addr,
        MPU6050_RA_PWR_MGMT_1,
        MPU6050_PWR_MGMT_1_SLEEP & MPU6050_PWR_MGMT_1_CYCLE);

    if ( readSlaveRegister(addr, MPU6050_RA_PWR_MGMT_1) != 0x1 )
    {
        RAM::Support::Error::longjmp(MPU6050_ERR_SLAVE_CHECK);
    }

    bitsetSlaveRegister(
        addr,
        MPU6050_RA_GYRO_CONFIG,
        MPU6050_GYRO_FS_250);
    
    bitsetSlaveRegister(
        addr,
        MPU6050_RA_ACCEL_CONFIG,
        MPU6050_ACCEL_FS_2G);
    
    bitsetSlaveRegister(
        addr,
        MPU6050_RA_CONFIG,
        MPU6050_CONFIG_DLPF_0);
}

//-------------------------------------------------------------------------------
void MPU6050::setSlaveReadoutConfig(
    const uint8_t slvAddrReg,
    const uint8_t slvRegReg,
    const uint8_t slvCtrlReg,
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t length)
{
    I2C::instance().write(m_i2cAddr, slvAddrReg, addr | 0b10000000);

    I2C::instance().write(m_i2cAddr, slvRegReg, reg);

    I2C::instance().write(m_i2cAddr, slvCtrlReg, MPU6050_RA_I2C_SLV_LEN_MASK, length);

    I2C::instance().bitset(m_i2cAddr, slvCtrlReg, MPU6050_I2C_SLV_EN);
}

//-------------------------------------------------------------------------------
void MPU6050::GetNumberSlavesCmd::execute()
{
    reply(Host::CommandReply::ReplyId::OK, 2);
}

//-------------------------------------------------------------------------------
void MPU6050::ReadoutCmd::execute()
{
    reply(
        Host::CommandReply::ReplyId::OK,
        reinterpret_cast<uint8_t *>(m_obj_p->m_output),
        (m_numOutputScalars - 1) * sizeof(float));
}

//-------------------------------------------------------------------------------
void MPU6050::ReadSlaveRegisterCmd::execute()
{
    uint8_t addr, reg;

    if ( !I2C::instance().serialReadAddrReg(*this, addr, reg) )
    {
        return;
    }

    reply(Host::CommandReply::ReplyId::OK, m_obj_p->readSlaveRegister(addr, reg));
}

//-------------------------------------------------------------------------------
void MPU6050::WriteSlaveRegisterCmd::execute()
{
    uint8_t addr, reg, val;

    if ( !I2C::instance().serialReadAddrRegVal(*this, addr, reg, val) )
    {
        return;
    }

    m_obj_p->writeSlaveRegister(addr, reg, val);

    reply(Host::CommandReply::ReplyId::OK);
}

//-------------------------------------------------------------------------------
void MPU6050::SetAccelerometerCalibrationCmd::execute()
{
    unsigned int idx;
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

    //
    // if it is not clear which sensor calibration to set then complain
    //
    if ( addr == 0x0 )
    {
        idx = 0;
    }
    else if ( addr == MPU6050_ADDRESS_AD0_LOW )
    {
        idx = 1;
    }
    else if ( addr == MPU6050_ADDRESS_AD0_HIGH )
    {
        idx = 2;
    }
    else
    {
        reply(SerialReplyId::INVALID_SENSOR);
        
        return;
    }

    scale = reinterpret_cast<uint8_t *>(m_obj_p->m_accelScale[idx]);

    offset = reinterpret_cast<uint8_t *>(m_obj_p->m_accelOffset[idx]);

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
void MPU6050::ToggleCalibrationCmd::execute()
{
    m_obj_p->m_calibrationFlag = !m_obj_p->m_calibrationFlag;

    reply(Host::CommandReply::ReplyId::OK, m_obj_p->m_calibrationFlag);
}

/*___oOo___*/
