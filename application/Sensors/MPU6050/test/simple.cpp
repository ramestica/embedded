//
// System stuff
//
#include <math.h>

//
// Target stuff
//
#include <Main.h>
#include <GPIO.h>

//
// Library stuff
//
#include <ErrorCodes.h>
#include <MainLoop.h>
#include <I2C.h>
#include <UART.h>

//
// Control stuff
//
#include <IMU.h>

//
// Local stuff
//
#include <MPU6050.h>

using namespace std;

#define SAMPLE_PERIOD_MS 2

//
// some variables to read out data from imu
//
//float output[RAM::Device::MPU6050::m_numOutputScalars - 1];
float output[19];
float angles[2] = {0., 0.};

enum SerialCommandId
{
    READOUT = 0,
    READOUT_ATTITUDE,
    READOUT_ACCELEROMETER
};

//
// application command to read out angle and filtered angle
//
struct ReadoutCmd:
    public RAM::Logic::Host::CommandReply::Command
{
    ReadoutCmd():
        Command(
            RAM::Logic::Host::CommandReply::ReceiverId::APPLICATION,
            READOUT)
    {;}
    
    void execute()
    {
        float data[5];

        data[0] = angles[0];
        data[1] = angles[1];
        data[2] = output[3];
        data[3] = output[4];
        data[4] = output[5];

        reply(
            RAM::Logic::Host::CommandReply::ReplyId::OK,
            reinterpret_cast<uint8_t *>(data),
            5 * sizeof(float));
    }
};
ReadoutCmd readoutCmd;

//
// application command to read out angle and filtered angle
//
struct ReadoutAttitudeCmd:
    public RAM::Logic::Host::CommandReply::Command
{
    ReadoutAttitudeCmd():
        Command(
            RAM::Logic::Host::CommandReply::ReceiverId::APPLICATION,
            READOUT_ATTITUDE)
    {;}
    
    void execute()
    {
        float data[2];

        data[0] = angles[0];

        data[1] = angles[1];

        reply(
            RAM::Logic::Host::CommandReply::ReplyId::OK,
            reinterpret_cast<uint8_t *>(data),
            2 * sizeof(float));
    }
};
ReadoutAttitudeCmd readoutAttitudeCmd;

//
// application command to accelerometers
//
struct ReadoutAccelerometerCmd:
    public RAM::Logic::Host::CommandReply::Command
{
    ReadoutAccelerometerCmd():
        Command(
            RAM::Logic::Host::CommandReply::ReceiverId::APPLICATION,
            READOUT_ACCELEROMETER)
    {;}
    
    void execute()
    {
        reply(
            RAM::Logic::Host::CommandReply::ReplyId::OK,
            reinterpret_cast<uint8_t *>(output),
            3 * sizeof(float));
    }
};
ReadoutAccelerometerCmd readoutAccelerometerCmd;

//
// lone simple device
//
RAM::Device::MPU6050 mpu6050(MPU6050_ADDRESS_AD0_LOW, output);
//RAM::Sensor::MEMS::MPU6050<MPU6050_ADDRESS_AD0_LOW, MPU6050_ADDRESS_AD0_HIGH> dev(MPU6050_ADDRESS_AD0_LOW, output);
//RAM::Sensor::MEMS::MPU6050<> dev(MPU6050_ADDRESS_AD0_LOW, output);
RAM::Control::IMU::Device imu(RAM::Control::IMU::AlgorithmType::COMPLEMENTARY,
                              1000.0 / SAMPLE_PERIOD_MS,
                              1000.0 / SAMPLE_PERIOD_MS,
                              output + 3,
                              output,
                              nullptr,
                              angles);

//-------------------------------------------------------------------------------
extern uint32_t SystemCoreClock;
void appSetup()
{
    RAM::Logic::I2C::instance().setup(400, false);

    RAM::Logic::Host::CommandReply::CommandList::instance().
        setCommandHandler(&readoutCmd);

    RAM::Logic::Host::CommandReply::CommandList::instance().
        setCommandHandler(&readoutAttitudeCmd);

    RAM::Logic::Host::CommandReply::CommandList::instance().
        setCommandHandler(&readoutAccelerometerCmd);

    RAM::Support::Utils::delayus(500);

    mpu6050.initialize();

    mpu6050.calibrate();
}

//-------------------------------------------------------------------------------
void appAlgorithm()
{
    mpu6050.readout();

    imu.update();
}

//-------------------------------------------------------------------------------
void RAM::Target::Main::setup()
{
    RAM::Logic::MainLoop::Executive::instance().setup(
        appSetup,
        appAlgorithm,
        RAM::Logic::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
        SAMPLE_PERIOD_MS * 1000,
        1);
}

//-------------------------------------------------------------------------------
void RAM::Target::Main::loop()
{
    RAM::Logic::MainLoop::Executive::instance().execute();
}

/*___oOo___*/
