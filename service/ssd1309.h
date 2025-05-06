#define SSD1309_LCDWIDTH                  128
#define SSD1309_LCDHEIGHT                 64

// Commands
#define SSD1309_SETCONTRAST               0x81
#define SSD1309_DISPLAYALLON_RESUME       0xA4
#define SSD1309_DISPLAYALLON              0xA5
#define SSD1309_NORMALDISPLAY             0xA6
#define SSD1309_INVERTDISPLAY             0xA7
#define SSD1309_DISPLAYOFF                0xAE
#define SSD1309_DISPLAYON                 0xAF
#define SSD1309_SETDISPLAYOFFSET          0xD3
#define SSD1309_SETCOMPINS                0xDA
#define SSD1309_SETVCOMDETECT             0xDB
#define SSD1309_SETDISPLAYCLOCKDIV        0xD5
#define SSD1309_SETPRECHARGE              0xD9
#define SSD1309_SETMULTIPLEX              0xA8
#define SSD1309_SETLOWCOLUMN              0x00
#define SSD1309_SETHIGHCOLUMN             0x10
#define SSD1309_SETSTARTLINE              0x40
#define SSD1309_SETSTARTPAGE              0xB0
#define SSD1309_MEMORYMODE                0x20
#define SSD1309_COMSCANINC                0xC0
#define SSD1309_COMSCANDEC                0xC8
#define SSD1309_SEGREMAP                  0xA0
#define SSD1309_CHARGEPUMP                0x8D
#define SSD1309_EXTERNALVCC               0x1
#define SSD1309_SWITCHCAPVCC              0x2

// SSD1309-Specific Commands
#define SSD1309_NOP                       0xE3  // No operation
#define SSD1309_SCROLL_ENABLE             0x2F  // Enable scrolling
#define SSD1309_SCROLL_DISABLE            0x2E  // Disable scrolling
#define SSD1309_SET_SCROLL_AREA           0xA3  // Set vertical scroll area
#define SSD1309_SET_VERTICAL_SCROLL       0x29  // Set vertical and horizontal scroll