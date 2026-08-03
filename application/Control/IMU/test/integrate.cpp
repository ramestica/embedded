//
// Arduino stuff
//
#include <Arduino.h>

//
// RAM stuff
//
#include <ErrorCodes.h>
#include <SampleStatistics.h>
#include <MainLoop.h>
#include <I2C.h>
#include <L3G4200D.h>
#include <ADXL345.h>
#include <HMC5883L.h>
#include <IMU.h>

using namespace std;

//
// inertial measurement unit
//
RAM::Control::IMU imu(125.0f,
                      20.0f,
                      new RAM::Sensor::L3G4200D(L3G4200D_DEFAULT_ADDRESS),
                      new RAM::Sensor::ADXL345(ADXL345_DEFAULT_ADDRESS),
                      new RAM::Sensor::HMC5883L(HMC5883L_DEFAULT_ADDRESS));

//
// some variables to read out data from imu
//
#define DATA_LENGTH 18
float data[DATA_LENGTH];
RAM::Sensor::L3G4200D::CoordinateAxes gyro(data);
RAM::Sensor::ADXL345::CoordinateAxes accl(data + 3);
RAM::Sensor::HMC5883L::CoordinateAxes magneto(data + 6);
RAM::Sensor::ADXL345::CoordinateAxes acclLowPass(data + 9);
RAM::Sensor::ADXL345::CoordinateAxes theta(data + 12);
RAM::Sensor::L3G4200D::CoordinateAxes thetaGyro(data + 15);

void sendIMU()
{
    Serial.write(RAM::Library::MainLoop::MAINLOOP_CMD_USER1);
        
    Serial.write(RAM::Library::MainLoop::MAINLOOP_REPLY_STATUS_OK);

    Serial.write(reinterpret_cast<uint8_t *>(data), DATA_LENGTH * sizeof(gyro[gyro.X]));
}

void appSetup()
{
    RAM::Library::MainLoop::instance().setCommandHandler(RAM::Library::MainLoop::MAINLOOP_CMD_USER1, sendIMU);
    
    imu.initialize();
    
    imu.calibrate();
    
    digitalWrite(LED_BUILTIN, true);
}

void appAlgorithm()
{
    imu.readout(gyro(), thetaGyro(), accl(), acclLowPass(), theta(), magneto());
}

void setup()
{
    RAM::Library::MainLoop::Executive::instance().setup(appSetup, appAlgorithm, RAM::Library::Timer::TIMIDX2, 8000);
}

void loop()
{
    RAM::Library::MainLoop::Executive::instance().execute();
}

/* ___oOo___ */
