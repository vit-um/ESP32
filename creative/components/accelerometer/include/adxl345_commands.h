/* --- Register Adresses --- */
#define ADXL345_DEVID               0x00
#define ADXL345_THRESH_TAP          0x1D
#define ADXL345_OFSX                0x1E
#define ADXL345_OFSY                0x1F
#define ADXL345_DUR                 0x20
#define ADXL345_LATENT              0x22
#define ADXL345_WINDOW              0x23
#define ADXL345_THRESH_ACT          0x24
#define ADXL345_THRESH_INACT        0x25
#define ADXL345_TIME_INACT          0x26
#define ADXL345_ACT_INACT_CTL       0x27
#define ADXL345_THRESH_FF           0x28
#define ADXL345_TIME_FF             0x28
#define ADXL345_TAP_AXES            0x2A
#define ADXL345_ACT_TAP_STATUS      0x2B
#define ADXL345_BW_RATE             0x2C
#define ADXL345_POWER_CTL           0x2D
#define ADXL345_INT_ENABLE          0x2E
#define ADXL345_INT_MAP             0x2F
#define ADXL345_INT_SOURCE          0x30
#define ADXL345_DATA_FORMAT         0x31
#define ADXL345_DATAX0              0x32
#define ADXL345_DATAX1              0x33
#define ADXL345_DATAY0              0x34
#define ADXL345_DATAY1              0x35
#define ADXL345_DATAZ0              0x36
#define ADXL345_DATAZ1              0x37
#define ADXL345_FIFO_CTL            0x38
#define ADXL345_FIFO_STATUS         0x39

/* --- Read-Write bits for commands --- */
#define ADXL345_READ                (1 << 7)
#define ADXL345_WRITE               (0 << 7)

/* --- Bandwidth definitions --- */
#define ADXL345_BW_1600_HZ          0b1111
#define ADXL345_BW_800_HZ           0b1110
#define ADXL345_BW_400_HZ           0b1101
#define ADXL345_BW_200_HZ           0b1100
#define ADXL345_BW_100_HZ           0b1011
#define ADXL345_BW_50_HZ            0b1010
#define ADXL345_BW_25_HZ            0b1001
#define ADXL345_BW_12_5_HZ          0b1000
#define ADXL345_BW_6_25_HZ          0b0111
#define ADXL345_BW_3_13_HZ          0b0110
#define ADXL345_BW_1_56_HZ          0b0101
#define ADXL345_BW_0_78_HZ          0b0100
#define ADXL345_BW_0_39_HZ          0b0011
#define ADXL345_BW_0_2_HZ           0b0010
#define ADXL345_BW_0_1_HZ           0b0001
#define ADXL345_BW_0_05_HZ          0b0000
#define ADXL345_BW_LOW_POWER        (1 << 4)

/* --- Range definitions --- */
#define ADXL345_RANGE_2G            0b00
#define ADXL345_RANGE_4G            0b01
#define ADXL345_RANGE_8G            0b10
#define ADXL345_RANGE_16G           0b11
