#ifndef _ErrorCodes_H_
#define _ErrorCodes_H_

//
// System stuff
//
#include <inttypes.h>
#include <setjmp.h>

//===================== Common Errors ===========================================
#define COMMON_ERR_BASE       0x0000
#define COMMON_ERR_MEMORY     (COMMON_ERR_BASE | 0x0000)

//===================== Sensor Devices ==========================================
//
// ADXL345 (accelerometer)
//
#define ADXL345_ERR_BASE       0x0100
#define ADXL345_ERR_INVALID_ID (ADXL345_ERR_BASE | 0x0000)

//
// L3G4200D (gyroscope)
//
#define L3G4200D_ERR_BASE       0x0200
#define L3G4200D_ERR_INVALID_ID (L3G4200D_ERR_BASE | 0x0000)

//
// HMC5883L (magnetometer)
//
#define HMC5883L_ERR_BASE       0x0300
#define HMC5883L_ERR_INVALID_ID (HMC5883L_ERR_BASE | 0x0000)

//
// MPU6050 (accelerometer + gyroscope)
//
#define MPU6050_ERR_BASE           0x0400
#define MPU6050_ERR_INVALID_ID     (MPU6050_ERR_BASE | 0x0000)
#define MPU6050_ERR_SLAVE_CHECK    (MPU6050_ERR_BASE | 0x0001)
#define MPU6050_ERR_INVALID_SENSOR (MPU6050_ERR_BASE | 0x0002)
#define MPU6050_ERR_INVALID_CALIB  (MPU6050_ERR_BASE | 0x0003)

//===================== Software Modules ========================================
//
// IMU
//
#define IMU_ERR_BASE       0x1000
#define IMU_ERR_READOUT    (IMU_ERR_BASE | 0x0000)

//
// Coordinates
//
#define COORDINATEAXES_ERR_BASE                0x1100
#define COORDINATEAXES_ERR_CARTESIAN_INDEX_OUT (COORDINATEAXES_ERR_BASE | 0x0000)
#define COORDINATEAXES_ERR_CARTESIAN_NULL      (COORDINATEAXES_ERR_BASE | 0x0001)

//
// MainLoop
//
#define MAINLOOP_ERR_BASE                0x1200
#define MAINLOOP_ERR_NULL_SINGLETON      (MAINLOOP_ERR_BASE | 0x0000)
#define MAINLOOP_ERR_FULL_SINGLETON      (MAINLOOP_ERR_BASE | 0x0001)
#define MAINLOOP_ERR_UNEVEN_PERIOD       (MAINLOOP_ERR_BASE | 0x0002)
#define MAINLOOP_ERR_SYSTICK             (MAINLOOP_ERR_BASE | 0x0003)

//
// Serial commands
//
enum
{
    SERIAL_COMMANDS_ERR_DATA_STREAM_NULL = 0x1300,
    SERIAL_COMMANDS_ERR_CMD_TOO_MANY, 
    SERIAL_COMMANDS_ERR_CMD_BUSY
};

//
// Timers
//
enum
{
    TIMER_ERR_INDEX = 0x1400,
    TIMER_ERR_INVALID_MODULE_ID,
    TIMER_ERR_INVALID_SELECT_ID,
    TIMER_ERR_MODULE_IN_USE,
    TIMER_ERR_INVALID_FREQUENCY,
    TIMER_ERR_UNMATCHED_FREQUENCY,
    TIMER_ERR_GET_COUNTER,
    TIMER_ERR_GET_NUMBER_BITS,
    TIMER_ERR_SET_PRESCALER,
    TIMER_ERR_SET_MODE,
    TIMER_ERR_SET_COMPARE_OUTPUT_MODE,
    TIMER_ERR_SET_OUTPUT_COMPARE_VALUE,
    TIMER_ERR_ACKNOWLEDGE_OVERFLOW,
    TIMER_ERR_ACKNOWLEDGE_MATCH,
    TIMER_ERR_INVALID_PRESCALER,
    TIMER_ERR_UNKNOWN_OPMODE
};

//
// PWM encoder
//
enum
{
    PWM_ERR_INVALID_DC_PARAMS = 0x1500,
    PWM_ERR_CONFIG_FAILED,
    PWM_ERR_START_FAILED,
    PWM_ERR_SET_DUTY_CYCLE
};

//
// PPM decoder
//
enum
{
    PPM_ERR_TIMER_CONFIG = 0x1600,
    PPM_ERR_FRAME_PERIOD,
    PPM_ERR_TIMER_START,
    PPM_ERR_TIMER_GET_LOAD
};

//
// UART class
//
enum
{
    UART_ERR_CONFIG = 0x1700
};

//
// I2C class
//
enum
{
    I2C_ERR_CONFIG = 0x1800,
    I2C_ERR_SDA_STUCK
};

//
// Interface with host computer
//
enum
{
    HOST_ERR_NULL_PRINTOUT_DATA_STREAM = 0x1900,
    HOST_ERR_NULL_CMD_REPLY_DATA_STREAM
};

//
// Memory allocation
//
enum
{
    MEMORY_ERR_POOL_ALLOCATION = 0x2000,
    MEMORY_ERR_ENOMEM,
    MEMORY_ERR_POOL_LOCKED, 
    MEMORY_ERR_OUT_OF_MEMORY,
    MEMORY_ERR_DELETE_NOT_IMPLEMENTED
};

//
// Turnigy HAL drone
//
enum
{
    TURNIGY_HAL_ERR_RCNOK = 0x2100
};

#define SET_ERROR_JUMP_MARK() (setjmp(RAM::Support::Error::errorJump))

namespace RAM
{
    namespace Support
    {
        namespace Error
        {
            //
            // a global variable to handle errors from anywhere
            //
            extern jmp_buf errorJump;

            //
            // longjmp should set this variable to a meaningful error code.
            //
            extern int longjmpCode;
            
            //
            // long jump on error condition
            //
            void longjmp(const uint16_t err);
        }
    }
}

#endif /* _ErrorCodes_H_ */

/*___oOo__*/
