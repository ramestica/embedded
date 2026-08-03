#ifndef _MPU6050_H_
#define _MPU6050_H_

//
// System stuff
//
#include <inttypes.h>
//#include <iterator>

//
// Library stuff
//
#include <I2C.h>
#include <MainLoop.h>

#define MPU6050_ADDRESS_AD0_LOW   0x68
#define MPU6050_ADDRESS_AD0_HIGH  0x69
#define MPU6050_DEVID             0b11010011

/** Register addresses.
 */
#define MPU6050_RA_WHO_AM_I           0x75
#define MPU6050_RA_CONFIG             0x1A
#define MPU6050_RA_PWR_MGMT_1         0x6B
#define MPU6050_RA_GYRO_CONFIG        0x1B
#define MPU6050_RA_ACCEL_CONFIG       0x1C
#define MPU6050_RA_I2C_SLV0_ADDR      0x25
#define MPU6050_RA_I2C_SLV0_REG       0x26
#define MPU6050_RA_I2C_SLV0_CTRL      0x27
#define MPU6050_RA_I2C_SLV1_ADDR      0x28
#define MPU6050_RA_I2C_SLV1_REG       0x29
#define MPU6050_RA_I2C_SLV1_CTRL      0x2A
#define MPU6050_RA_I2C_SLV2_ADDR      0x2B
#define MPU6050_RA_I2C_SLV2_REG       0x2C
#define MPU6050_RA_I2C_SLV2_CTRL      0x2D
#define MPU6050_RA_I2C_SLV3_ADDR      0x2E
#define MPU6050_RA_I2C_SLV3_REG	      0x2F
#define MPU6050_RA_I2C_SLV3_CTRL      0x30
#define MPU6050_RA_I2C_SLV4_ADDR      0x31
#define MPU6050_RA_I2C_SLV4_REG       0x32
#define MPU6050_RA_I2C_SLV4_DO        0x33
#define MPU6050_RA_I2C_SLV4_CTRL      0x34
#define MPU6050_RA_I2C_SLV4_DI        0x35
#define MPU6050_RA_I2C_MST_STATUS     0x36
#define MPU6050_RA_INT_PIN_CFG        0x37
#define MPU6050_RA_I2C_MST_DELAY_CTRL 0x67
#define MPU6050_RA_USER_CTRL          0x6A
#define MPU6050_RA_SELF_TEST_X        0x0D
#define MPU6050_RA_SELF_TEST_Y        0x0E
#define MPU6050_RA_SELF_TEST_Z        0x0F
#define MPU6050_RA_SELF_TEST_A        0x10

/** Device identifier in who-am-i register.
 */
#define MPU6050_WHO_AM_I_ID         0xD0

/** Power management 1 settings.
 */
#define MPU6050_PWR_MGMT_1_CYCLE    0b00100000
#define MPU6050_PWR_MGMT_1_SLEEP    0b01000000

/** Clock sources.
 */
#define MPU6050_CLOCK_PLL_INTERNAL  0x0
#define MPU6050_CLOCK_PLL_XGYRO     0x1
#define MPU6050_CLOCK_PLL_YGYRO     0x2
#define MPU6050_CLOCK_PLL_ZGYRO     0x3
#define MPU6050_CLOCK_PLL_EXT_32    0x4
#define MPU6050_CLOCK_PLL_EXT_19    0x5
#define MPU6050_CLOCK_STOP          0x7

/** Gyroscope full scale range.
 */
#define MPU6050_GYRO_FS_250         0b00000000
#define MPU6050_GYRO_FS_500	    0b00001000
#define MPU6050_GYRO_FS_1000	    0b00010000
#define MPU6050_GYRO_FS_2000	    0b00011000

/** Accelerometer full scale range.
 */
#define MPU6050_ACCEL_FS_2G	    0b00000000
#define MPU6050_ACCEL_FS_4G	    0b00001000
#define MPU6050_ACCEL_FS_8G	    0b00010000
#define MPU6050_ACCEL_FS_16G	    0b00011000

/** Accelerometer self test bits in ACCEL_CONFIG register.
 */
#define MPU6050_ACCEL_XA_ST         0b10000000
#define MPU6050_ACCEL_YA_ST         0b01000000
#define MPU6050_ACCEL_ZA_ST         0b00100000

/** Accelerometer test output bits.
 */
#define MPU6050_ACCEL_HIGH_SHIFT_TEST 0x3
#define MPU6050_ACCEL_LOW_XSHIFT_TEST 0x4
#define MPU6050_ACCEL_LOW_YSHIFT_TEST 0x2
#define MPU6050_ACCEL_LOW_ZSHIFT_TEST 0x0

/** On board digital filtering:
 **
 **             |   ACCELEROMETER    |           GYROSCOPE
 **    DLPF_CFG | Bandwidth | Delay  | Bandwidth | Delay  | Sample Rate
 **        #    |    Hz     |  ms    |    Hz     |  ms    |     Hz
 **    ---------+-----------+--------+-----------+--------+-------------
 **    0        | 260       | 0      | 256       | 0.98   | 8
 **    1	| 184	    | 2.0    | 188       | 1.9    | 1
 **    2	| 94	    | 3.0    | 98        | 2.8    | 1
 **    3	| 44	    | 4.9    | 42        | 4.8    | 1
 **    4	| 21	    | 8.5    | 20        | 8.3    | 1
 **    5	| 10	    | 13.8   | 10        | 13.4   | 1
 **    6	| 5	    | 19.0   | 5         | 18.6   | 1
 **    7        |   -- Reserved --   |   -- Reserved --   | Reserved
 */
#define MPU6050_CONFIG_DLPF_0 0b00000000
#define MPU6050_CONFIG_DLPF_1 0b00000001
#define MPU6050_CONFIG_DLPF_2 0b00000010
#define MPU6050_CONFIG_DLPF_3 0b00000011
#define MPU6050_CONFIG_DLPF_4 0b00000100
#define MPU6050_CONFIG_DLPF_5 0b00000101
#define MPU6050_CONFIG_DLPF_6 0b00000110
#define MPU6050_CONFIG_DLPF_7 0b00000111

/** Sensor data starting register.
 */
#define MPU6050_RA_ACCEL_OUT     0x3B
#define MPU6050_RA_GYRO_OUT      0x43

/** Interrupt configuration register bits.
 */
#define MPU6050_I2C_BYPASS_EN    0b00000010

/** User control register bits.
 */
#define MPU6050_I2C_MST_RESET    0b00000010
#define MPU6050_I2C_MST_EN       0b00100000

/** Slave control register bits.
 */
#define MPU6050_RA_I2C_SLV_LEN_MASK 0b00001111
#define MPU6050_I2C_SLV_EN          0b10000000

/** Slave 4 control register bits.
 */
#define MPU6050_I2C_SLV4_INT_EN  0b01000000
#define MPU6050_I2C_SLV4_EN      0b10000000

/** Master delay control register bits.
 */
#define MPU6050_DELAY_ES_SHADOW  0b10000000

/** Master status register bits.
 */
#define MPU6050_I2C_SLV4_DONE    0b01000000

namespace RAM
{
    namespace Sensor
    {
        namespace MEMS
        {
            template <unsigned N>
            struct CalibratedAxis
            {
                CalibratedAxis()
                {
                    for ( unsigned i = 0; i < N; ++i )
                    {
                        m_scale[i] = float(1);

                        m_offset[i] = float(0);
                    }
                }

                /** Apply scale/offset calibration to raw data. 
                 ** Compute result into output parameter in floating point
                 ** representation. The loop runs in reverse to allow raw and
                 ** out pointers to be the same, which is a common use-case.
                 */
                template <typename T>
                void apply(const T *raw, float *out)
                {
                    //
                    // go backwards to provide propper support for in-place
                    // raw/out bufferes
                    //
                    for ( int i = N - 1; i >= 0; --i )
                    {
                        if ( m_enabled )
                        {
                            out[i] = m_scale[i] * raw[i] + m_offset[i];
                        }
                        else
                        {
                            out[i] = raw[i];
                        }
                    }
                }

                bool m_enabled = false;
                
                float m_scale[N];
                
                float m_offset[N];
            };

            /** A base class.
             */
            class MPU6050Base
            {
            public:
                
                struct SerialCommandId
                {
                    enum SerialCommandIdEnum
                    {
                        GETNSLAVES = 0,
                        READOUT,
                        READ_SLAVE_REGISTER,
                        WRITE_SLAVE_REGISTER,
                        SET_ACCEL_CALIB,
                        TOGGLE_CALIB_ONOFF
                    };
                };
            
                struct SerialReplyId
                {
                    enum SerialReplyIdEnum
                    {
                        INVALID_SENSOR = 1,
                        INVALID_CALIB_SIZE
                    };
                };

                MPU6050Base(
                    const uint8_t addr,
                    float *output,
                    const unsigned nslaves,
                    uint8_t *slaves,
                    CalibratedAxis<3> *gyroc,
                    CalibratedAxis<3> *accelc);

                void initialize();
                
                void configure(const uint8_t addr = 0);
                
                void calibrate();

                void readout();

            private:
                
                RAM::Logic::I2C &m_i2c;
                uint8_t m_addr;
                float *m_output;
                unsigned m_nslaves = 0;
                uint8_t *m_slaves;
                CalibratedAxis<3> *m_gyroc;
                CalibratedAxis<3> *m_accelc;

                /** Set default configuration on given slave. 
                 */
                void setSlaveReadoutConfig(
                    const uint8_t slvAddrReg,
                    const uint8_t slvRegReg,
                    const uint8_t slvCtrlReg,
                    const uint8_t addr,
                    const uint8_t reg,
                    const uint8_t length);

                uint8_t read(const uint8_t addr, const uint8_t reg);
                
                void write(const uint8_t addr, const uint8_t reg, const uint8_t val);

                /** 
                 **
                 */
                inline void bitset(
                    const uint8_t addr,
                    const uint8_t reg,
                    const uint8_t bits)
                {
                    write(addr, reg, read(addr, reg) | bits);
                }

                /** 
                 **
                 */
                inline void bitunset(
                    const uint8_t addr,
                    const uint8_t reg,
                    const uint8_t bits)
                {
                    write(addr, reg, read(addr, reg) & ~bits);
                }

                struct GetNumberSlavesCmd:
                    public RAM::Logic::Host::CommandReply::Command
                {
                    GetNumberSlavesCmd(): Command()
                    {;}

                    GetNumberSlavesCmd(MPU6050Base *ptr):
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                            SerialCommandId::GETNSLAVES),
                        m_obj_p(ptr)
                    {;}
                
                    void execute();

                    MPU6050Base *m_obj_p;
                };
                GetNumberSlavesCmd m_getNumberSlavesCmd;

                struct ReadoutCmd: public RAM::Logic::Host::CommandReply::Command
                {
                    ReadoutCmd(): Command()
                    {;}

                    ReadoutCmd(MPU6050Base *ptr):
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                            SerialCommandId::READOUT),
                        m_obj_p(ptr)
                    {;}
                
                    void execute();

                    MPU6050Base *m_obj_p;
                };
                ReadoutCmd m_readoutCmd;

                struct ReadSlaveRegisterCmd:
                    public RAM::Logic::Host::CommandReply::Command
                {
                    ReadSlaveRegisterCmd():
                        Command()
                    {;}

                    ReadSlaveRegisterCmd(MPU6050Base *ptr):
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                            SerialCommandId::READ_SLAVE_REGISTER),
                        m_obj_p(ptr)
                    {;}

                    void execute();

                    MPU6050Base *m_obj_p;
                };
                ReadSlaveRegisterCmd m_readSlaveRegisterCmd;
            
                struct WriteSlaveRegisterCmd:
                    public RAM::Logic::Host::CommandReply::Command
                {
                    WriteSlaveRegisterCmd():
                        Command()
                    {;}

                    WriteSlaveRegisterCmd(MPU6050Base *ptr):
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                            SerialCommandId::WRITE_SLAVE_REGISTER),
                        m_obj_p(ptr)
                    {;}
                
                    void execute();

                    MPU6050Base *m_obj_p;
                };
                WriteSlaveRegisterCmd m_writeSlaveRegisterCmd;
            
                struct SetAccelerometerCalibrationCmd:
                    public RAM::Logic::Host::CommandReply::Command
                {
                    SetAccelerometerCalibrationCmd():
                        Command()
                    {;}

                    SetAccelerometerCalibrationCmd(MPU6050Base *ptr):
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                            SerialCommandId::SET_ACCEL_CALIB),
                        m_obj_p(ptr)
                    {;}
                
                    void execute();

                    MPU6050Base *m_obj_p;
                };
                SetAccelerometerCalibrationCmd m_setAccelerometerCalibrationCmd;
            
                struct ToggleCalibrationCmd:
                    public RAM::Logic::Host::CommandReply::Command
                {
                    ToggleCalibrationCmd():
                        Command()
                    {;}

                    ToggleCalibrationCmd(MPU6050Base *ptr):
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                            SerialCommandId::TOGGLE_CALIB_ONOFF),
                        m_obj_p(ptr)
                    {;}
                
                    void execute();

                    MPU6050Base *m_obj_p;
                };
                ToggleCalibrationCmd m_toggleCalibrationCmd;
            };
            
            /** An MPU6050 driver class. It assumes that there might be up to
             ** two other MPU6050 devices hooked to the slave I2C bus. The first
             ** I2C address in the pack is the one associated to the 'master'
             ** device.
             */
            template <uint8_t ... SlaveAddresses>
            class MPU6050: public MPU6050Base
            {
            public:
                
                MPU6050(const uint8_t addr, float *output):
                    MPU6050Base(
                        addr,
                        output,
                        sizeof...(SlaveAddresses),
                        m_slaves,
                        m_gyroc,
                        m_accelc),
                    m_slaves{SlaveAddresses...}
                {
                }
                
            private:
                uint8_t m_slaves[sizeof...(SlaveAddresses)];
                CalibratedAxis<3> m_gyroc[1 + sizeof...(SlaveAddresses)];
                CalibratedAxis<3> m_accelc[1 + sizeof...(SlaveAddresses)];
            };
        }    
    }
    
    namespace Device
    {
        class MPU6050
        {
        public:
            struct SerialCommandId
            {
                enum SerialCommandIdEnum
                {
                    GETNSLAVES = 0,
                    READOUT,
                    READ_SLAVE_REGISTER,
                    WRITE_SLAVE_REGISTER,
                    SET_ACCEL_CALIB,
                    TOGGLE_CALIB_ONOFF
                };
            };
            
            struct SerialReplyId
            {
                enum SerialReplyIdEnum
                {
                    INVALID_SENSOR = 1,
                    INVALID_CALIB_SIZE
                };
            };

            /** Number of scalar values in input and output arrays.
             */
            constexpr static unsigned m_numOutputScalars = 19;

            /** Number of readout iterations during calibration.
             */
            constexpr static unsigned m_calibrateIterations = 20;
            
            /** Constructor must receive a pointer to buffer written on every
             ** readout from hardware. Because, serial command used to readout
             ** copies data from that pointer.
             */
            MPU6050(const uint8_t addr, float *output);

            void initialize();

            bool selfTest();
            
            void calibrate();

            void readout();

            void setAccelerometerCalibration(
                const uint8_t addr,
                const float *scale,
                const float *offset);

            uint8_t readSlaveRegister(const uint8_t addr, const uint8_t reg);

        private:
            uint8_t m_i2cAddr;
            float *m_output;
            float m_gyroScale[3][3], m_gyroOffset[3][3];
            float m_accelScale[3][3], m_accelOffset[3][3];
            bool m_calibrationFlag;

            void applyCalib(
                float *output,
                int16_t *input,
                const float *scale,
                const float *offset);
            
            void writeSlaveRegister(
                const uint8_t addr,
                const uint8_t reg,
                const uint8_t val);

            void bitsetSlaveRegister(
                const uint8_t addr,
                const uint8_t reg,
                const uint8_t bits);

            void bitunsetSlaveRegister(
                const uint8_t addr,
                const uint8_t reg,
                const uint8_t bits);

            void setSlaveConfig(const uint8_t addr);

            void setSlaveReadoutConfig(
                const uint8_t slvAddrReg,
                const uint8_t slvRegReg,
                const uint8_t slvCtrlReg,
                const uint8_t addr,
                const uint8_t reg,
                const uint8_t length);

            struct GetNumberSlavesCmd:
                public RAM::Logic::Host::CommandReply::Command
            {
                GetNumberSlavesCmd(): Command()
                {;}
                
                GetNumberSlavesCmd(MPU6050 *ptr):
                    Command(
                        RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                        SerialCommandId::GETNSLAVES),
                    m_obj_p(ptr)
                {;}
                
                void execute();
                
                MPU6050 *m_obj_p;
            };
            GetNumberSlavesCmd m_getNumberSlavesCmd;

            struct ReadoutCmd: public RAM::Logic::Host::CommandReply::Command
            {
                ReadoutCmd():
                    Command()
                {;}

                ReadoutCmd(MPU6050 *ptr):
                    Command(
                        RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                        SerialCommandId::READOUT),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                MPU6050 *m_obj_p;
            };
            ReadoutCmd m_readoutCmd;
            
            struct ReadSlaveRegisterCmd:
                public RAM::Logic::Host::CommandReply::Command
            {
                ReadSlaveRegisterCmd():
                    Command()
                {;}

                ReadSlaveRegisterCmd(MPU6050 *ptr):
                    Command(
                        RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                        SerialCommandId::READ_SLAVE_REGISTER),
                    m_obj_p(ptr)
                {;}

                void execute();

                MPU6050 *m_obj_p;
            };
            ReadSlaveRegisterCmd m_readSlaveRegisterCmd;
            
            struct WriteSlaveRegisterCmd:
                public RAM::Logic::Host::CommandReply::Command
            {
                WriteSlaveRegisterCmd():
                    Command()
                {;}

                WriteSlaveRegisterCmd(MPU6050 *ptr):
                    Command(
                        RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                        SerialCommandId::WRITE_SLAVE_REGISTER),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                MPU6050 *m_obj_p;
            };
            WriteSlaveRegisterCmd m_writeSlaveRegisterCmd;
            
            struct SetAccelerometerCalibrationCmd:
                public RAM::Logic::Host::CommandReply::Command
            {
                SetAccelerometerCalibrationCmd():
                    Command()
                {;}

                SetAccelerometerCalibrationCmd(MPU6050 *ptr):
                    Command(
                        RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                        SerialCommandId::SET_ACCEL_CALIB),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                MPU6050 *m_obj_p;
            };
            SetAccelerometerCalibrationCmd m_setAccelerometerCalibrationCmd;
            
            struct ToggleCalibrationCmd:
                public RAM::Logic::Host::CommandReply::Command
            {
                ToggleCalibrationCmd():
                    Command()
                {;}

                ToggleCalibrationCmd(MPU6050 *ptr):
                    Command(
                        RAM::Logic::Host::CommandReply::ReceiverId::MPU6050,
                        SerialCommandId::TOGGLE_CALIB_ONOFF),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                MPU6050 *m_obj_p;
            };
            ToggleCalibrationCmd m_toggleCalibrationCmd;
        };
    }
}

#endif /* _MPU6050_H_ */

/*___oOo___*/
