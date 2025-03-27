#ifndef DJIBattery_H
#define DJIBattery_H

#define BATTERY_PORT_BAUDRATE   115200
#define BATTERY_PORT_RECEIVE    LOW
#define BATTERY_PORT_SEND       HIGH
#define ELEMENTS(x) (sizeof(x)/sizeof(x[0]))  
#define CHAR_CONNECT     'c'
#define CHAR_SNPIN       'i'
#define CHAR_4PARAM      '4'
#define CHAR_NUMCHARGER  'n'   
#define CHAR_CELL        'p'
#define CHAR_EXCEPTCASE  'a'

class DJIBattery {
public:
    DJIBattery(HardwareSerial &serial_peripheral, uint8_t pin_control);
    ~DJIBattery();
    struct batteryStruct
    {
        volatile bool active = false;
        volatile uint8_t seriNumber[14] = {};
        volatile uint8_t version[4] = {};
        volatile uint16_t voltage;
        volatile int32_t current;
        volatile uint8_t percent;
        volatile uint16_t cell[14] = {};
        volatile int16_t numberCharge = -1;
        volatile uint16_t temperature;
        volatile uint16_t capacity;
        volatile uint8_t countError = 0;
        volatile uint8_t led = 0;
        volatile uint16_t tempTotalMaxHigh = 800;
        volatile uint16_t tempTotalMaxLow = 700;
        volatile uint8_t percentMaxLow = 95;
        volatile uint8_t percentMaxHigh = 96;
        volatile uint8_t current_coef = 100;
    }get;
    enum BATTERY_TYPE
    {
        BATTERY_UNKNOWN = 0,
        BATTERY_T30 = 29000,
        BATTERY_T20P = 13000,
        BATTERY_T40 = 30000,
        BATTERY_T25 = 16818,
        BATTERY_T50 = 33022,
    };

    enum LED_TYPE
    {
        TYPE_INIT = 0x04,
        BATTERY_WAITTING = 0x24,
        BATTERY_OPERATION = 0x26,
        BATTERY_CHARGING = 0x28, // nháy theo tốc độ sạc.
        BATTERY_FULL_CHARGE = 0x30,
        BATTERY_ERROR_CHARGE = 0x32,
        BATTERY_OVER_TEMPERATURE = 0x34,
        BATTERY_WAITING_BUTTON = 0x72,
    };

    bool init();
    bool update(HardwareSerial &debugPort);
    bool updateT25T50(HardwareSerial &debugPort);
private:
    HardwareSerial *batteryPort;
    /**
     * @brief pin control RS485
     */
    uint8_t pin_control;
    bool lv_success = false;
    bool check_return_active = false;
    bool lv_charge_t25t50_active = false;
    unsigned long lv_time_charge_t25t50 = 1;
    bool vfs_receiveFail = false;
    bool vfs_recCheckCell = false;
    uint16_t vfs_countReceive = 0; 
    uint16_t vfs_countTemp = 0;
    uint8_t vfs_cmdRecSnPin[44] = {};
    uint8_t vfs_cmdRec4Param[63] = {};
    uint8_t vfs_cmdRecNumCharger[54] = {};
    uint8_t vfs_cmdRecCell[43] = {};
    uint16_t vfs_cellValue[14] = {};
    uint8_t vfs_byteReceive;
    uint32_t vfs_time;
    // Private constructor to prevent instantiation
    bool portSendReceive (uint8_t cmd[], uint8_t cmd_length, uint8_t receive_length, char package, bool *success, HardwareSerial &debugPort);
    void changeCMDSub(uint8_t cmd[], uint8_t cmd_sub[], uint8_t x, uint8_t y, uint8_t z, uint8_t t, uint16_t *start);
};

#endif