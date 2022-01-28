// 1. Set lower column address | 4 lower bytes
#define SH1106_SETLOWCOLUMN             0x00

// 2. Set higher column adress | 4 higher bytes
#define SH1106_SETHIGHCOLUMN            0x10

// 3. Set pump voltage value??? 0x30 - 0x33


// 4 Set start line | line number
#define SH1106_SETSTARTLINE             0x40

// 5. Set contrast control register (double byte)
#define SH1106_SETCONTRAST              0x81

// 6. Set segment remap (aka mirror)
#define SH1106_SEGREMAP_NORMAL          0xA0
#define SH1106_SEGREMAP_REVERSE         0xA1

// 7. Set entire display OFF/ON
#define SH1106_DISPLAYALLON_RESUME      0xA4
#define SH1106_DISPLAYALLON             0xA5

// 8. Set normal/reverse display
#define SH1106_NORMALDISPLAY            0xA6
#define SH1106_INVERTDISPLAY            0xA7

// 9. Set multiplex ration (double byte)
#define SH1106_SETMULTIPLEX             0xA8

// 10. Set DC ON/OFF (double byte)
#define SH1106_DC_CMD                   0x8D
#define SH1106_DC_ON                    0x14
#define SH1106_DC_OFF                   0x10

// 11. Display ON/OFF
#define SH1106_DISPLAYOFF               0xAE
#define SH1106_DISPLAYON                0xAF

// 12. Set page address | page address
#define SH1106_PAGE_ADDRESS             0xB0

// 13. Set Common Output Scan Direction (aka flips display)
#define SH1106_COMSCANINC               0xC0
#define SH1106_COMSCANDEC               0xC8

// 14. Set display offset (double byte)
#define SH1106_SETDISPLAYOFFSET         0xD3

// 15. Set Display Clock Divide Ratio/Oscillator Frequency (double byte)
#define SH1106_SETDISPLAYCLOCKDIV       0xD5

// 16. Set Discharge/Pre-charge Period (double byte)
#define SH1106_SETPRECHARGE             0xD9

// 17. Set common pads hardware configuration (double byte)
#define SH1106_SETCOMPINS               0xDA
#define SH1106_SETCOMPINS_SEQUENTIAL    0x02
#define SH1106_SETCOMPINS_ALTERNATIVE   0x12

// 18. Set VCOM Deselect level (double byte)
#define SH1106_SETVCOMDETECT            0xDB

// 19. Read-modify-write E0
#define SH1106_RD_MOD_WR                0xE0

// 20. End of Read-modify-write EE
#define SH1106_RD_MOD_WR_END            0xEE

// 21. NOP (non-operation)
#define SH1106_NOP                      0xE3
