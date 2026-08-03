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
#include <I2C.h>
#include <L3G4200D.h>

using namespace std;

//
// a gyro object
//
RAM::Sensor::L3G4200D gyro(L3G4200D_DEFAULT_ADDRESS);

//
// initialization status
//
bool initStatus = false;

//
// fancy blinking led
//
bool blinkState = false;

//
// some variables to read out data from gyro
//
vector<float> av(3);

void setup()
{
    Serial.begin(38400);

    Serial.println("starting setup...");

    delay(100);

    RAM::I2C::init(400, false);

    delay(100);

    if ( !gyro.initialize() )
    {
        Serial.println("failed to initialize gyro");

        return;
    }

    if ( !gyro.calibrate() )
    {
        Serial.println("failed to calibrate gyro");
        
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
    gyro.readout(av);

    Serial.print("av:\t");
    Serial.print(av[0]);
    Serial.print("/");
    Serial.print(av[1]);
    Serial.print("/");
    Serial.print(av[2]);
    Serial.println();

    //
    // blink LED to indicate activity
    //
    digitalWrite(LED_BUILTIN, (blinkState = !blinkState));

    delay(100);
    
//    Serial.println("hola mundo!");
}

/* ___oOo___ */
