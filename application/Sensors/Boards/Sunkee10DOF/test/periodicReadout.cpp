//
// Arduino stuff
//
#include <Arduino.h>

//
// Contributed stuff
//

//
// RAM stuff
//
#include "Sunkee10DOF.h"

//
// a sunkee imu object
//
RAM::IMU::Sunkee10DOF imu(0x69, 0x53, 0x1E, 0x0);

//
// initialization status
//
bool initStatus = false;

//
// fancy blinking led
//
bool blinkState = false;

//
// some variables to read out data from imu
//
int16_t avx, avy, avz;
int16_t ax, ay, az;
int16_t mx, my, mz;

void setup()
{
    //Wire.begin();
    Fastwire::setup(400, false);

    Serial.begin(38400);

    if ( !imu.init() )
    {
        Serial.println("failed to initialize sunkee imu");
    }
    else
    {
        initStatus = true;
    }

    delay(1000);

    imu.calibrate();
}

void loop()
{
    //
    // if not initialized successfully then do nothing
    //
    if ( !initStatus )
    {
        Serial.println("initialization has failed");

        delay(1000);

        return;
    }

    //
    // read raw angular velocity measurements from device
    //
    imu.getAngularVelocity(avx, avy, avz);

    //
    // read linear acceleration
    //
    imu.getAcceleration(&ax, &ay, &az);

    //
    // read magnetic field vector
    //
    imu.getHeading(&mx, &my, &mz);

    Serial.print("av/a:\t");
    Serial.print(avx);
    Serial.print("/");
    Serial.print(ax);
    Serial.print("/");
    Serial.print(mx);
    Serial.print("\t");
    Serial.print(avy);
    Serial.print("/");
    Serial.print(ay);
    Serial.print("/");
    Serial.print(my);
    Serial.print("\t");
    Serial.print(avz);
    Serial.print("/");
    Serial.print(az);
    Serial.print("/");
    Serial.println(mz);

    //
    // blink LED to indicate activity
    //
    digitalWrite(LED_BUILTIN, (blinkState = !blinkState));

    delay(500);

//    Serial.println("hola mundo!");
}

/* ___oOo___ */
