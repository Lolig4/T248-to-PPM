/* Thrustmaster T248 */

#include <Arduino.h>
#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include <PPMEncoder.h>
#include <string.h>

#define TM_SERIAL_ENABLED 0

#if TM_SERIAL_ENABLED
#define TM_SERIAL_BEGIN(...)   Serial.begin(__VA_ARGS__)
#define TM_SERIAL_PRINT(...)   Serial.print(__VA_ARGS__)
#define TM_SERIAL_PRINTLN(...) Serial.println(__VA_ARGS__)
#define TM_SERIAL_WRITE(...)   Serial.write(__VA_ARGS__)
#else
#define TM_SERIAL_BEGIN(...)   do { } while (0)
#define TM_SERIAL_PRINT(...)   do { } while (0)
#define TM_SERIAL_PRINTLN(...) do { } while (0)
#define TM_SERIAL_WRITE(...)   do { } while (0)
#endif

USB Usb;
USB* pUsb = &Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);

uint8_t lastUsbState = 0xFF;
unsigned long lastInitAttempt = 0;

uint8_t DisplayChar = 0;
uint16_t BarGraphValue = 0;

uint16_t WheelRange = 0;
uint16_t AutoCenterStrength = 0;

uint8_t wButtons1 = 0;
uint8_t wButtons2 = 0;
uint8_t wButtons3 = 0;
uint8_t wDpad = 0;

#define FRAME_LENGTH 21500
#define FRAME_SPACE 4000
#define PULSE_LENGTH 300
#define PPM_PIN 2
#define HRNAK 0x04

// Force feedback helpers (implemented in ForceFeedback.ino)
void SetAutoCenterStrength(uint8_t strength);
void SetRangeDeg(uint16_t deg);
void SendProperties();
void SendExitProperties();
void SendBlownTire();
void SendEigene();
void SendBumpyRoad();
void SendForceField();

class WheelMapper {
public:
    const uint16_t WHEEL_MIN_VALUE = 0;
    const uint16_t WHEEL_MAX_VALUE = 65535;
    const uint16_t WHEEL_MAX_RANGE = 1080;
    const uint16_t THROTTLE_MAX_VALUE = 1023;
    const uint16_t THROTTLE_MIN_VALUE = 0;
    const uint16_t BRAKE_MAX_VALUE = 1023;
    const uint16_t BRAKE_MIN_VALUE = 0;
    const uint16_t PPM_MIN_VALUE = 1000;
    const uint16_t PPM_MAX_VALUE = 2000;
    const uint16_t PPM_CENTER_VALUE = (PPM_MAX_VALUE - PPM_MIN_VALUE) / 2 + PPM_MIN_VALUE;

    uint16_t mapWheelToPPM(uint16_t wheelValue) {
        return (uint16_t)(PPM_MIN_VALUE + (uint32_t)wheelValue * (PPM_MAX_VALUE - PPM_MIN_VALUE) / WHEEL_MAX_VALUE);
    }

    uint16_t mapBrakeThrottleToPPM(uint16_t brake, uint16_t throttle) {
        int32_t hr = (PPM_MAX_VALUE - PPM_MIN_VALUE) / 2;
        int32_t gd = (int32_t)THROTTLE_MAX_VALUE   - THROTTLE_MIN_VALUE,   bd = (int32_t)BRAKE_MAX_VALUE - BRAKE_MIN_VALUE;
        if (gd <= 0 || bd <= 0) return PPM_CENTER_VALUE;

        int32_t g = ((int32_t)throttle - THROTTLE_MIN_VALUE)   * 1000L / gd;
        int32_t b = ((int32_t)brake    - BRAKE_MIN_VALUE) * 1000L / bd;
        if (g < 0) g = 0; else if (g > 1000) g = 1000;
        if (b < 0) b = 0; else if (b > 1000) b = 1000;

        int32_t ppm = (int32_t)PPM_CENTER_VALUE + (b - g) * hr / 1000L;  // b - g statt g - b
        if (ppm < PPM_MIN_VALUE) ppm = PPM_MIN_VALUE;
        if (ppm > PPM_MAX_VALUE) ppm = PPM_MAX_VALUE;
        return (uint16_t)ppm;
    }
};
WheelMapper wheelMapper;

// Bildschirm löschen
void clearConsole() {
    TM_SERIAL_WRITE("\033[2J");
    TM_SERIAL_WRITE("\033[H");
}
class T248Parser : public HIDReportParser {
private:
    struct GamePadEventData {
        uint8_t  reportID;
        uint16_t wheel;
        uint16_t brake;
        uint16_t throttle;
        uint16_t clutch;
        uint16_t unknown1;
        uint16_t unknown2;
        uint16_t unknown3;
        uint8_t  buttons1;
        uint8_t  buttons2;
        uint8_t  buttons3;
        uint8_t  unknown4;
        uint8_t  dpad;
    } __attribute__((packed));

    uint32_t lastLog = 0;

    void mapGamePadToPPM(GamePadEventData* data, WheelMapper &mapper) {
        uint16_t ch0 = mapper.mapWheelToPPM(data->wheel);
        uint16_t ch1 = mapper.mapBrakeThrottleToPPM(data->brake, data->throttle);

        ppmEncoder.setChannel(0, ch0);
        ppmEncoder.setChannel(1, ch1);

        if (millis() - lastLog > 200) {
            lastLog = millis();
            clearConsole();
            TM_SERIAL_PRINT(F("W:")); TM_SERIAL_PRINT(data->wheel);
            TM_SERIAL_PRINT(F(" B:")); TM_SERIAL_PRINT(data->brake);
            TM_SERIAL_PRINT(F(" T:")); TM_SERIAL_PRINT(data->throttle);
            TM_SERIAL_PRINT(F(" | PPM CH0=")); TM_SERIAL_PRINT(ch0);
            TM_SERIAL_PRINT(F(" CH1=")); TM_SERIAL_PRINT(ch1);
            TM_SERIAL_PRINT(F(" C:")); TM_SERIAL_PRINT(data->clutch);

            TM_SERIAL_PRINT(F(" [L-Shift=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 0)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" R-Shift=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 1)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" Y=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 2)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" X=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 3)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" B=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 4)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" A=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 5)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" Window=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 6)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" Menu=")); TM_SERIAL_PRINT((data->buttons1 & (1u << 7)) ? 1 : 0);
            TM_SERIAL_PRINT(F("]"));

            TM_SERIAL_PRINT(F(" [RSB=")); TM_SERIAL_PRINT((data->buttons2 & (1u << 0)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" LSB=")); TM_SERIAL_PRINT((data->buttons2 & (1u << 1)) ? 1 : 0);
            TM_SERIAL_PRINT(F("]"));

            TM_SERIAL_PRINT(F(" [L-Up=")); TM_SERIAL_PRINT((data->buttons3 & (1u << 5)) ? 1 : 0);
            TM_SERIAL_PRINT(F(" R-Up=")); TM_SERIAL_PRINT((data->buttons3 & (1u << 6)) ? 1 : 0);
            TM_SERIAL_PRINT(F("]"));

            TM_SERIAL_PRINT(F(" DPad:")); TM_SERIAL_PRINTLN(data->dpad);  // 0-7=directions 0=Up 2=Right 4=Donw 6=Left, 15 = neutral
        }
    }

public:
    void Parse(USBHID* hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
        if (len < sizeof(GamePadEventData)) return;
        
        auto *data = (GamePadEventData*)buf;
        wButtons1 = data->buttons1;
        wButtons2 = data->buttons2;
        wButtons3 = data->buttons3;
        wDpad = data->dpad;
        mapGamePadToPPM(data, wheelMapper);
    }
};
T248Parser parser;

void initUSB() {
    if (Usb.Init() == -1) {
        TM_SERIAL_PRINTLN(F("[USB] ❌ Init failed – check power or wiring!"));
        return;
    }
    else {
        TM_SERIAL_PRINTLN(F("[USB] ✅ stack initialized."));
    }

    if (!Hid.SetReportParser(0, &parser)) {
        TM_SERIAL_PRINTLN(F("[USB] ❌ Failed to set HID report parser."));
    }
    else {
        TM_SERIAL_PRINTLN(F("[USB] ✅ HID parser registered."));
    }
}

void reconnectUSB() {
    uint8_t currentUsbState = Usb.getUsbTaskState();

    if (currentUsbState == USB_STATE_ERROR || currentUsbState == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE) {
        if (millis() - lastInitAttempt > 3000) {
            TM_SERIAL_PRINTLN(F("[USB] 🔄 Trying re-init..."));
            initUSB();
            lastInitAttempt = millis();
        }
    }

    if (currentUsbState != lastUsbState) {
        lastUsbState = currentUsbState;
        TM_SERIAL_PRINT(F("[USB] New state: 0x"));
        TM_SERIAL_PRINTLN(currentUsbState, HEX);
        if (currentUsbState == 0x90) {
            TM_SERIAL_PRINTLN(F("[USB] ✅ Device ready."));
            DisplayChar = (uint8_t)'N';
            WheelRange = 180;
            AutoCenterStrength = 0;
            UpdateDisplay();
            SetRangeDeg(WheelRange);
            SetAutoCenterStrength(AutoCenterStrength);
        }
    }
}

uint8_t sendIntOut64(const uint8_t *buf) {
    for (uint16_t tries = 1; tries <= 200; tries++) {
        uint8_t r = Hid.SndRpt(64, (uint8_t*)buf);
        if (r == 0) { TM_SERIAL_PRINT(F("✅ tries=")); TM_SERIAL_PRINTLN(tries); return r; }
        if (r == HRNAK) { Usb.Task(); delay(5); continue; }
        return r;
    }
    return HRNAK;
}

uint8_t sendIntOutPadded64(const uint8_t *buf, uint16_t len) {
    uint8_t tmp[64] = {0};
    if (buf && len) {
        if (len > sizeof(tmp)) len = sizeof(tmp);
        memcpy(tmp, buf, len);
    }
    return sendIntOut64(tmp);
}

void UpdateDisplay() { 
    uint8_t rpt1[] = {
        0x60, 0x00, 0x42, 0x21, 0xd0, 0x63, 0x21,
        0x00, 0xa2, 0x88, 0x46, 0x05, 0x01, 0x00, 0x00, 0x00,
        0x43, 0x06, 0x00,
        0x54, 0x07, 0x00, 0x00,
        0x54, 0x08, 0x00, 0x00,
        0x54, 0x09, 0x00, 0x00,
        0x46, 0x0b
    };
    uint8_t rpt2[] = {
        0x60, 0x00, 0x42, 0x1c, 0xd0, 0x63, 0x1c,
        0x00, 0xc2, 0x2a, 0x46, 0x0a, 0x00, 0x00, 0x00, 0x00,
        0x54, 0x31, 0x62, 0x00,
        0x54, 0x32, 0x63, 0x00,
        0x54, 0x33, 0x00, 0x00,
        0x54, 0x34, 0x00, 0x00,
    };
    uint8_t rpt3[] = {
        0x60, 0x00, 0x42, 0x0c, 0xd0, 0x63, 0x0c,
        0x00, 0xd2, 0x0c, 0x46, 0x36
    };

    rpt1[18] = DisplayChar;

    rpt1[21] = (uint8_t)(AutoCenterStrength & 0xFF);
    rpt1[22] = (uint8_t)(AutoCenterStrength >> 8);

    rpt1[25] = (uint8_t)(BarGraphValue & 0xFF);
    rpt1[26] = (uint8_t)(BarGraphValue >> 8);

    uint16_t ref = 4070;                            //65535
    uint32_t num = (uint32_t)WheelRange * wheelMapper.WHEEL_MAX_VALUE + ref / 2;
    uint16_t v = num / ref;
    rpt1[29] = (uint8_t)(v & 0xFF);
    rpt1[30] = (uint8_t)(v >> 8);
    

    sendIntOutPadded64(rpt1, sizeof(rpt1));
    sendIntOutPadded64(rpt2, sizeof(rpt2));
    sendIntOutPadded64(rpt3, sizeof(rpt3));
}
void SendDisplayKeepAlive() {
    uint8_t rpt1[] = {
        0x60, 0x00, 0x42, 0x06, 0xd0, 0x63, 0x06,
        0x00, 0xe6, 0xb8
    };
    sendIntOutPadded64(rpt1, sizeof(rpt1));
}
void clear_bits() {
    uint8_t rpt1[64] = {0};
    sendIntOut64(rpt1);
}

void handleSettings() {
    static bool Settings = false;

    static bool prevMenuPressed = false;
    const bool MenuPressed = ((wButtons1 & (1u << 7)) != 0);
    if (MenuPressed && !prevMenuPressed) {
        if (Settings) {
            DisplayChar = (uint8_t)'N';
            UpdateDisplay();
            Settings = false;
        }
        else {
            DisplayChar = (uint8_t)'S';
            UpdateDisplay();
            Settings = true;
        }
    }
    prevMenuPressed = MenuPressed;

    if (!Settings) return;

    static bool prevXPressed = false;
    const bool XPressed = ((wButtons1 & (1u << 3)) != 0);
    if (XPressed && !prevXPressed) {
        WheelRange = 180;
        SetRangeDeg(WheelRange);
        UpdateDisplay();
    }
    prevXPressed = XPressed;

    static bool prevAPressed = false;
    const bool APressed = ((wButtons1 & (1u << 5)) != 0);
    if (APressed && !prevAPressed) {
        WheelRange = 360;
        SetRangeDeg(WheelRange);
        UpdateDisplay();
    }
    prevAPressed = APressed;

    static bool prevBPressed = false;
    const bool BPressed = ((wButtons1 & (1u << 4)) != 0);
    if (BPressed && !prevBPressed) {
        WheelRange = 540;
        SetRangeDeg(WheelRange);
        UpdateDisplay();
    }
    prevBPressed = BPressed;

    static bool prevYPressed = false;
    const bool YPressed = ((wButtons1 & (1u << 2)) != 0);
    if (YPressed && !prevYPressed) {
        WheelRange = 720;
        SetRangeDeg(WheelRange);
        UpdateDisplay();
    }
    prevYPressed = YPressed;

    static uint8_t prevDpadUpPressed = 15;
    const uint8_t DpadUpPressed = wDpad;
    if (DpadUpPressed == 0 && prevDpadUpPressed != 0) {
        if (WheelRange < 850) { 
            WheelRange += 10;
            SetRangeDeg(WheelRange);
            UpdateDisplay();
        }
    }
    prevDpadUpPressed = DpadUpPressed;

    static uint8_t prevDpadDownPressed = 15;
    const uint8_t DpadDownPressed = wDpad;
    if (DpadDownPressed == 4 && prevDpadDownPressed != 4) {
        if (WheelRange > 140) { 
            WheelRange -= 10;
            SetRangeDeg(WheelRange);
            UpdateDisplay();
        }
    }
    prevDpadDownPressed = DpadDownPressed;

    static uint8_t prevDpadLeftPressed = 15;
    const uint8_t DpadLeftPressed = wDpad;
    if (DpadLeftPressed == 6 && prevDpadLeftPressed != 6) {
        if (AutoCenterStrength < 100) { 
            AutoCenterStrength += 1;
            SetAutoCenterStrength(AutoCenterStrength);
            UpdateDisplay();
        }
    }
    prevDpadLeftPressed = DpadLeftPressed;

    static uint8_t prevDpadRightPressed = 15;
    const uint8_t DpadRightPressed = wDpad;
    if (DpadRightPressed == 2 && prevDpadRightPressed != 2) {
        if (AutoCenterStrength > 0) { 
            AutoCenterStrength -= 1;
            SetAutoCenterStrength(AutoCenterStrength);
            UpdateDisplay();
        }
    }
    prevDpadRightPressed = DpadRightPressed;
}


void initPPM() {
  ppmEncoder.begin(PPM_PIN, 8, FRAME_LENGTH, 8);
  
  for (int i = 0; i <= 8; i++) {
    ppmEncoder.setChannel(i, wheelMapper.PPM_CENTER_VALUE);
  }
    TM_SERIAL_PRINTLN("📟 All PPM channels set to neutral (1500µs)");
}

void setup() {
    TM_SERIAL_BEGIN(115200);
    clearConsole();

    initPPM();

    TM_SERIAL_PRINTLN(F("[USB] 🔄 init..."));
    initUSB();
}

void loop() {
    Usb.Task();
    reconnectUSB();
    handleSettings();

    static unsigned long lastKeepAlive = 0;
    if ((millis() - lastKeepAlive >= 1000)) {
        lastKeepAlive = millis();
        SendDisplayKeepAlive();
    }
}