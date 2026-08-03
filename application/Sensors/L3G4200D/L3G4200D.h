#ifndef _L3G4200D_H_
#define _L3G4200D_H_

//
// RAM stuff
//
#include <Gyroscope.h>

#define L3G4200D_DEFAULT_ADDRESS   0x69
#define L3G4200D_DEVID             0b11010011

#define L3G4200D_RA_WHO_AM_I       0x0F
#define L3G4200D_RA_CTRL_REG1      0x20
#define L3G4200D_RA_CTRL_REG2      0x21
#define L3G4200D_RA_CTRL_REG3      0x22
#define L3G4200D_RA_CTRL_REG4      0x23
#define L3G4200D_RA_CTRL_REG5      0x24
#define L3G4200D_RA_REFERENCE      0x25
#define L3G4200D_RA_OUT_TEMP       0x26
#define L3G4200D_RA_STATUS         0x27
#define L3G4200D_RA_OUT_X_L        0x28
#define L3G4200D_RA_OUT_X_H        0x29
#define L3G4200D_RA_OUT_Y_L        0x2A
#define L3G4200D_RA_OUT_Y_H        0x2B
#define L3G4200D_RA_OUT_Z_L        0x2C
#define L3G4200D_RA_OUT_Z_H        0x2D
#define L3G4200D_RA_FIFO_CTRL  	   0x2E
#define L3G4200D_RA_FIFO_SRC	   0x2F
#define L3G4200D_RA_INT1_CFG       0x30
#define L3G4200D_RA_INT1_SRC       0x31
#define L3G4200D_RA_INT1_THS_XH    0x32
#define L3G4200D_RA_INT1_THS_XL    0X33
#define L3G4200D_RA_INT1_THS_YH    0X34
#define L3G4200D_RA_INT1_THS_YL    0x35
#define L3G4200D_RA_INT1_THS_ZH    0X36
#define L3G4200D_RA_INT1_THS_ZL    0x37
#define L3G4200D_RA_INT1_DURATION  0X38

#define L3G4200D_ODR_BIT           7
#define L3G4200D_ODR_LENGTH        2
#define L3G4200D_BW_BIT            5
#define L3G4200D_BW_LENGTH         2
#define L3G4200D_PD_BIT            3
#define L3G4200D_ZEN_BIT           2
#define L3G4200D_YEN_BIT           1
#define L3G4200D_XEN_BIT           0

#define L3G4200D_RATE_100          0b00
#define L3G4200D_RATE_200          0b01
#define L3G4200D_RATE_400          0b10
#define L3G4200D_RATE_800          0b11

#define L3G4200D_BW_LOW            0b00
#define L3G4200D_BW_MED_LOW        0b01
#define L3G4200D_BW_MED_HIGH       0b10
#define L3G4200D_BW_HIGH           0b11

#define L3G4200D_HPM_BIT           5
#define L3G4200D_HPM_LENGTH        2
#define L3G4200D_HPCF_BIT          3
#define L3G4200D_HPCF_LENGTH       4

#define L3G4200D_HPM_HRF           0b00
#define L3G4200D_HPM_REFERENCE     0b01
#define L3G4200D_HPM_NORMAL        0b10
#define L3G4200D_HPM_AUTORESET     0b11

#define L3G4200D_HPCF1             0b0000
#define L3G4200D_HPCF2             0b0001
#define L3G4200D_HPCF3             0b0010
#define L3G4200D_HPCF4             0b0011
#define L3G4200D_HPCF5             0b0100
#define L3G4200D_HPCF6             0b0101
#define L3G4200D_HPCF7             0b0110
#define L3G4200D_HPCF8             0b0111
#define L3G4200D_HPCF9             0b1000
#define L3G4200D_HPCF10            0b1001

#define L3G4200D_I1_INT1_BIT       7
#define L3G4200D_I1_BOOT_BIT       6
#define L3G4200D_H_LACTIVE_BIT     5
#define L3G4200D_PP_OD_BIT         4
#define L3G4200D_I2_DRDY_BIT       3
#define L3G4200D_I2_WTM_BIT        2
#define L3G4200D_I2_ORUN_BIT       1
#define L3G4200D_I2_EMPTY_BIT      0

#define L3G4200D_PUSH_PULL         1
#define L3G4200D_OPEN_DRAIN        0

#define L3G4200D_BDU_BIT           7
#define L3G4200D_BLE_BIT           6
#define L3G4200D_FS_BIT            5
#define L3G4200D_FS_LENGTH         2
#define L3G4200D_ST_BIT            2
#define L3G4200D_ST_LENGTH         2
#define L3G4200D_SIM_BIT           0

#define L3G4200D_BIG_ENDIAN        1
#define L3G4200D_LITTLE_ENDIAN     0

#define L3G4200D_FS_250            0b00
#define L3G4200D_FS_500            0b01
#define L3G4200D_FS_2000           0b10

#define L3G4200D_SELF_TEST_NORMAL  0b00
#define L3G4200D_SELF_TEST_0       0b01
#define L3G4200D_SELF_TEST_1       0b11

#define L3G4200D_SPI_4_WIRE        1
#define L3G4200D_SPI_3_WIRE        0

#define L3G4200D_BOOT_BIT          7
#define L3G4200D_FIFO_EN_BIT       6
#define L3G4200D_HPEN_BIT          4
#define L3G4200D_INT1_SEL_BIT      3
#define L3G4200D_INT1_SEL_LENGTH   2
#define L3G4200D_OUT_SEL_BIT       1
#define L3G4200D_OUT_SEL_LENGTH    2

#define L3G4200D_NON_HIGH_PASS     0b00
#define L3G4200D_HIGH_PASS         0b01
#define L3G4200D_LOW_PASS          0b10
#define L3G4200D_LOW_HIGH_PASS     0b11

#define L3G4200D_ZYXOR_BIT         7
#define L3G4200D_ZOR_BIT           6
#define L3G4200D_YOR_BIT           5
#define L3G4200D_XOR_BIT           4
#define L3G4200D_ZYXDA_BIT         3
#define L3G4200D_ZDA_BIT           2
#define L3G4200D_YDA_BIT           1
#define L3G4200D_XDA_BIT           0

#define L3G4200D_FIFO_MODE_BIT     7
#define L3G4200D_FIFO_MODE_LENGTH  3
#define L3G4200D_FIFO_WTM_BIT      4
#define L3G4200D_FIFO_WTM_LENGTH   5

#define L3G4200D_FM_BYPASS         0b000
#define L3G4200D_FM_FIFO           0b001
#define L3G4200D_FM_STREAM         0b010
#define L3G4200D_FM_STREAM_FIFO    0b011
#define L3G4200D_FM_BYPASS_STREAM  0b100

#define L3G4200D_FIFO_STATUS_BIT   7
#define L3G4200D_FIFO_OVRN_BIT     6
#define L3G4200D_FIFO_EMPTY_BIT    5
#define L3G4200D_FIFO_FSS_BIT      4
#define L3G4200D_FIFO_FSS_LENGTH   5

#define L3G4200D_INT1_AND_OR_BIT   7
#define L3G4200D_INT1_LIR_BIT      6
#define L3G4200D_ZHIE_BIT          5
#define L3G4200D_ZLIE_BIT          4
#define L3G4200D_YHIE_BIT          3
#define L3G4200D_YLIE_BIT          2
#define L3G4200D_XHIE_BIT          1
#define L3G4200D_XLIE_BIT          0

#define L3G4200D_INT1_OR           0
#define L3G4200D_INT1_AND          1

#define L3G4200D_INT1_IA_BIT       6
#define L3G4200D_INT1_ZH_BIT       5
#define L3G4200D_INT1_ZL_BIT       4
#define L3G4200D_INT1_YH_BIT       3
#define L3G4200D_INT1_YL_BIT       2
#define L3G4200D_INT1_XH_BIT       1
#define L3G4200D_INT1_XL_BIT       0

#define L3G4200D_INT1_WAIT_BIT     7
#define L3G4200D_INT1_DUR_BIT      6
#define L3G4200D_INT1_DUR_LENGTH   7

namespace RAM
{
    namespace Sensor
    {
        class L3G4200D: public RAM::Sensor::Gyroscope
        {
        public:
            typedef RAM::Control::CoordinateAxes::Cartesian<float, 0, 1, 2> CoordinateAxes;

            L3G4200D()
             {;}
            
            L3G4200D(const uint8_t addr);

            void initialize();

            void calibrate();
        };
    }
}

#endif /* _L3G4200D_H_ */

/*___oOo___*/
