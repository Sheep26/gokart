#define LCDWIDTH                  128
#define LCDHEIGHT                 64

// Commands
#define SET_LOW_COLUMN 0x00
#define SET_HIGH_COLUMN 0x10
#define SET_MEMORY_ADDRESS_MODE 0x20
#define SET_COLUMN_ADDRESS 0x21
#define SET_PAGE_ADDRESS 0x22
#define SET_DISPLAY_START_LINE 0x40
#define SET_CONTRAST 0x81
#define SET_SEGMENT_REMAP_OFF 0xA0
#define SET_SEGMENT_REMAP_ON 0xA1
#define SET_ENTIRE_DISPLAY_OFF 0xA4
#define SET_ENTIRE_DISPLAY_ON 0xA5
#define SET_NORMAL_DISPLAY 0xA6
#define SET_INVERSE_DISPLAY 0xA7
#define SET_MULTIPLEX_RATIO 0xA8
#define DISPLAY_OFF 0xAE
#define DISPLAY_ON 0xAF
#define SET_PAGE_START_ADDRESS_0 0xB0
#define SET_PAGE_START_ADDRESS_1 0xB1
#define SET_PAGE_START_ADDRESS_2 0xB2
#define SET_PAGE_START_ADDRESS_3 0xB3
#define SET_PAGE_START_ADDRESS_4 0xB4
#define SET_PAGE_START_ADDRESS_5 0xB5
#define SET_PAGE_START_ADDRESS_6 0xB6
#define SET_PAGE_START_ADDRESS_7 0xB7
#define SET_COM_OUTPUT_SCAN_DIRECTION_0 0xC0
#define SET_COM_OUTPUT_SCAN_DIRECTION_1 0xC1
#define SET_COM_OUTPUT_SCAN_DIRECTION_2 0xC2
#define SET_COM_OUTPUT_SCAN_DIRECTION_3 0xC3
#define SET_COM_OUTPUT_SCAN_DIRECTION_4 0xC4
#define SET_COM_OUTPUT_SCAN_DIRECTION_5 0xC5
#define SET_COM_OUTPUT_SCAN_DIRECTION_6 0xC6
#define SET_COM_OUTPUT_SCAN_DIRECTION_7 0xC7
#define SET_COM_OUTPUT_SCAN_DIRECTION_8 0xC8
#define SET_DISPLAY_OFFSET 0xD3
#define SET_DISPLAY_CLOCK_DEVICE_RATIO 0xD5
#define SET_PRECHANGE_PERIOD 0xD9
#define SET_COM_PINS_HARDWARE_CONFIG 0xDA
#define SET_VCOMH_DESELECT_LEVEL 0xDB
#define SET_GPIO 0xDC
#define SET_NOP 0x3h
#define SET_COMMAND_LOCK 0xFD
#define HORIZONTAL_SCROLL_SETUP_OFF 0x26
#define HORIZONTAL_SCROLL_SETUP_ON 0x27
#define CONTINUOUS_VERTICAL_AND_HORIZONTAL_SCROLL_SETUP_OFF 0x29
#define CONTINUOUS_VERTICAL_AND_HORIZONTAL_SCROLL_SETUP_ON 0x2A
#define DEACTIVATE_SCROLL 0x2E
#define ACTIVATE_SCROLL 0x2F
#define SET_VERTICAL_SCROLL_AREA 0xA3
#define CONTENT_SCROLL_SETUP_OFF 0x2C
#define CONTENT_SCROLL_SETUP_ON 0x2D
#define SET_CHARGE_PUMP 0x8D
#define EXTERNAL_VCC 0x01
#define SWITCH_CAP_VCC 0x02