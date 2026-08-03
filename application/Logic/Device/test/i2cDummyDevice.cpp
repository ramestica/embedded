//
// Arduino stuff
//
#include <Arduino.h>

//
// RAM stuff
//
#include <MainLoop.h>

using namespace std;

//
// main loop handler
//
RAM::Library::MainLoop *ml;

class DummyDevice: RAM::Library::Device::I2C
{
public:
    DummyDevice(const uint8_t i2cAddr,
                const uint8_t i2cDevIdReg,
                const uint8_t i2cDevIdValue):
        RAM::Library::Device::I2C(i2cAddr, i2cDevIdReg, i2cDevIdValue)
    {
    }
    
private:
    RAM::Library::Sensor::Spatial gyro(m_input, m_out_p, false);
    RAM::Library::Sensor::Spatial accl(m_input + 3, m_out_p + 3, false);
    uint16_t m_input[6];
    float m_output[6];
};

void setup()
{
    DummyDevice dd(0x69, 0x00, 0x00);
}

void loop()
{
}

/* ___oOo___ */
