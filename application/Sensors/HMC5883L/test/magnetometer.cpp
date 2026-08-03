//
// Arduino stuff
//
#include <Arduino.h>

//
// RAM stuff
//
#include <I2C.h>
#include <HMC5883L.h>

using namespace std;

//
// a compass object
//
RAM::Sensor::HMC5883L magneto(HMC5883L_DEFAULT_ADDRESS);

//
// initialization status
//
bool initStatus = false;

//
// fancy blinking led
//
bool blinkState = false;

//
// some variables to read out data from compass
//
vector<float> data(3);

void setup()
{
    Serial.begin(38400);

    Serial.println("starting setup...");

    delay(100);

    RAM::I2C::init(400, false);

    delay(100);

    if ( !magneto.initialize() )
    {
        Serial.println("failed to initialize compass");

        return;
    }

    if ( !magneto.calibrate() )
    {
        Serial.println("failed to calibrate compass");
        
        return;
    }

    initStatus = true;

    Serial.println("setup completed");
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
    magneto.readout(data);

    Serial.print("magneto:\t");
    Serial.print(data[0]);
    Serial.print("/");
    Serial.print(data[2]);
    Serial.print("/");
    Serial.print(data[1]);
    Serial.println();

    //
    // blink LED to indicate activity
    //
    digitalWrite(LED_BUILTIN, (blinkState = !blinkState));

    delay(100);
    
//    Serial.println("hola mundo!");
}

/* ___oOo___ */
