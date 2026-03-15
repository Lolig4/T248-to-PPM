#include <Arduino.h>

// Provided by the main sketch.
uint8_t sendIntOut64(const uint8_t *buf);

static const uint16_t kWheelMaxValue = 65535;
static const uint16_t kWheelMaxRangeDeg = 1080;

void SetAutoCenterStrength(uint8_t strength) {
    uint8_t ref = 100;
    uint32_t num = (uint32_t)strength * kWheelMaxValue + ref / 2;
    uint16_t v = num / ref;

    uint8_t rpt[64] = {0};
    rpt[0]=0x60; //rpt[0]=0x60;
    rpt[1]=0x08; //rpt[1]=0x10;
    rpt[2]=0x03; //rpt[2]=0x04;
                 //rpt[5]=0x03;
    rpt[3]=(uint8_t)(v & 0xFF); //rpt[6]=(uint8_t)(v & 0xFF);
    rpt[4]=(uint8_t)(v >> 8); //rpt[7]=(uint8_t)(v >> 8);

    sendIntOut64(rpt);
}

void SetRangeDeg(uint16_t deg) {
    uint16_t ref = kWheelMaxRangeDeg;
    uint32_t num = (uint32_t)deg * kWheelMaxValue + ref / 2;
    uint16_t v = num / ref;

    uint8_t rpt[64] = {0};
    rpt[0] = 0x60;
    rpt[1] = 0x08;
    rpt[2] = 0x11;
    rpt[3] = (uint8_t)(v & 0xFF);
    rpt[4] = (uint8_t)(v >> 8);

    sendIntOut64(rpt);
}

void SendProperties() {
    //HID Data: 60010500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    uint8_t rpt[64] = {0};
    rpt[0] = 0x60;
    rpt[1] = 0x01;
    rpt[2] = 0x05;
    sendIntOut64(rpt);
}

void SendExitProperties() {
    //HID Data: 60000189000289000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    uint8_t rpt[64] = {0};
    rpt[0] = 0x60;
    rpt[2] = 0x01;
    rpt[3] = 0x89;
    rpt[5] = 0x02;
    rpt[6] = 0x89;
    sendIntOut64(rpt);
}

void SendBlownTire() {
    //HID Data: 600001ebe70afeff000022020080d9000000aa024c58014fe2040000000000000000000000000000000000000000000000000000000000000000000000000000
    uint8_t rpt1[64] = {0};
    rpt1[0] = 0x60;
    rpt1[2] = 0x01;
    rpt1[3] = 0xeb;
    rpt1[4] = 0xe7;
    rpt1[5] = 0x0a;
    rpt1[6] = 0xfe;
    rpt1[7] = 0xff;
    rpt1[10] = 0x22;
    rpt1[11] = 0x02;
    rpt1[13] = 0x80;
    rpt1[14] = 0xd9;
    rpt1[18] = 0xaa;
    rpt1[19] = 0x02;
    rpt1[20] = 0x4c;
    rpt1[21] = 0x58;
    rpt1[22] = 0x01;
    rpt1[23] = 0x4f;
    rpt1[24] = 0xe2;
    rpt1[25] = 0x04;
    //HID Data: 600001890102ea01800000fc7f0000fc7f004f840300000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    uint8_t rpt2[64] = {0};
    rpt2[0]  = 0x60;
    rpt2[2]  = 0x01;
    rpt2[3]  = 0x89;
    rpt2[4]  = 0x01;
    rpt2[5]  = 0x02;
    rpt2[6]  = 0xea;
    rpt2[7]  = 0x01;
    rpt2[8]  = 0x80;
    rpt2[11] = 0xfc;
    rpt2[12] = 0x7f;
    rpt2[15] = 0xfc;
    rpt2[16] = 0x7f;
    rpt2[18] = 0x4f;
    rpt2[19] = 0x84;
    rpt2[20] = 0x03;
    //HID Data: 60000289010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    uint8_t rpt3[64] = {0};
    rpt3[0] = 0x60;
    rpt3[2] = 0x02;
    rpt3[3] = 0x89;
    rpt3[4] = 0x01;
    sendIntOut64(rpt1);
    sendIntOut64(rpt2);
    sendIntOut64(rpt3);
}

void SendEigene() {
    // HID Data: 600001eb305cb9f90000180200805501b77971050000014f980800...
    uint8_t rpt1[64] = {0};
    rpt1[0] = 0x60;
    rpt1[2] = 0x01;
    rpt1[3] = 0xeb;
    rpt1[4] = 0x30;
    rpt1[5] = 0x5c;
    rpt1[6] = 0xb9;
    rpt1[7] = 0xf9;
    rpt1[10] = 0x18;
    rpt1[11] = 0x02;
    rpt1[13] = 0x80;
    rpt1[14] = 0x55;
    rpt1[15] = 0x01;
    rpt1[16] = 0xb7;
    rpt1[17] = 0x79;
    rpt1[18] = 0x71;
    rpt1[19] = 0x05;
    rpt1[22] = 0x01;
    rpt1[23] = 0x4f;
    rpt1[24] = 0x98;
    rpt1[25] = 0x08;
    // HID Data: 600001890102eb3364feff00006b0000800c0a0000c0012918034f740e00...
    uint8_t rpt2[64] = {0};
    rpt2[0] = 0x60;
    rpt2[2] = 0x01;
    rpt2[3] = 0x89;
    rpt2[4] = 0x01;
    rpt2[5] = 0x02;
    rpt2[6] = 0xeb;
    rpt2[7] = 0x33;
    rpt2[8] = 0x64;
    rpt2[9] = 0xfe;
    rpt2[10] = 0xff;
    rpt2[13] = 0x6b;
    rpt2[16] = 0x80;
    rpt2[17] = 0x0c;
    rpt2[18] = 0x0a;
    rpt2[21] = 0xc0;
    rpt2[22] = 0x01;
    rpt2[23] = 0x29;
    rpt2[24] = 0x18;
    rpt2[25] = 0x03;
    rpt2[26] = 0x4f;
    rpt2[27] = 0x74;
    rpt2[28] = 0x0e;
    // HID Data: 600002890100...
    uint8_t rpt3[64] = {0};
    rpt3[0] = 0x60;
    rpt3[2] = 0x02;
    rpt3[3] = 0x89;
    rpt3[4] = 0x01;
    sendIntOut64(rpt1);
    sendIntOut64(rpt2);
    sendIntOut64(rpt3);
}

void SendBumpyRoad() {
    // HID Data: 600001ebfe3ffeff00005a0000800000fe3f0000fe3f034fdc0500...
    uint8_t rpt1[64] = {0};
    rpt1[0] = 0x60;
    rpt1[2] = 0x01;
    rpt1[3] = 0xeb;
    rpt1[4] = 0xfe;
    rpt1[5] = 0x3f;
    rpt1[6] = 0xfe;
    rpt1[7] = 0xff;
    rpt1[10] = 0x5a;
    rpt1[13] = 0x80;
    rpt1[16] = 0xfe;
    rpt1[17] = 0x3f;
    rpt1[20] = 0xfe;
    rpt1[21] = 0x3f;
    rpt1[22] = 0x03;
    rpt1[23] = 0x4f;
    rpt1[24] = 0xdc;
    rpt1[25] = 0x05;
    // HID Data: 600002eb9819feff00004d0100800000981900009819034fe8030000...
    uint8_t rpt2[64] = {0};
    rpt2[0] = 0x60;
    rpt2[1] = 0x00;
    rpt2[2] = 0x02;
    rpt2[3] = 0xeb;
    rpt2[4] = 0x98;
    rpt2[5] = 0x19;
    rpt2[6] = 0xfe;
    rpt2[7] = 0xff;
    rpt2[10] = 0x4d;
    rpt2[11] = 0x01;
    rpt2[14] = 0x80;
    rpt2[18] = 0x98;
    rpt2[19] = 0x19;
    rpt2[22] = 0x98;
    rpt2[23] = 0x19;
    rpt2[24] = 0x03;
    rpt2[25] = 0x4f;
    rpt2[26] = 0xe8;
    rpt2[27] = 0x03;
    // HID Data: 6000018901028901...
    uint8_t rpt3[64] = {0};
    rpt3[0] = 0x60;
    rpt3[2] = 0x01;
    rpt3[3] = 0x89;
    rpt3[4] = 0x01;
    rpt3[5] = 0x02;
    rpt3[6] = 0x89;
    rpt3[7] = 0x01;
    sendIntOut64(rpt1);
    sendIntOut64(rpt2);
    sendIntOut64(rpt3);
}

void SendForceField() {
    // HID Data: 600001ebfc7ffeff00001f0000803001cc1c12020000014fe8030000...
    uint8_t rpt1[64] = {0};
    rpt1[0]  = 0x60;
    rpt1[2]  = 0x01;
    rpt1[3]  = 0xeb;
    rpt1[4]  = 0xfc;
    rpt1[5]  = 0x7f;
    rpt1[6]  = 0xfe;
    rpt1[7]  = 0xff;
    rpt1[10] = 0x1f;
    rpt1[13] = 0x80;
    rpt1[14] = 0x30;
    rpt1[15] = 0x01;
    rpt1[16] = 0xcc;
    rpt1[17] = 0x1c;
    rpt1[18] = 0x12;
    rpt1[19] = 0x02;
    rpt1[22] = 0x01;
    rpt1[23] = 0x4f;
    rpt1[24] = 0xe8;
    rpt1[25] = 0x03;
    // HID Data: 600001890102ebfc7ffeff8d78080400800000fc7f00010000014ff4010000...
    uint8_t rpt2[64] = {0};
    rpt2[0]  = 0x60;
    rpt2[2]  = 0x01;
    rpt2[3]  = 0x89;
    rpt2[4]  = 0x01;
    rpt2[5]  = 0x02;
    rpt2[6]  = 0xeb;
    rpt2[7]  = 0xfc;
    rpt2[8]  = 0x7f;
    rpt2[9]  = 0xfe;
    rpt2[10] = 0xff;
    rpt2[11] = 0x8d;
    rpt2[12] = 0x78;
    rpt2[13] = 0x08;
    rpt2[14] = 0x04;
    rpt2[16] = 0x80;
    rpt2[19] = 0xfc;
    rpt2[20] = 0x7f;
    rpt2[22] = 0x01;
    rpt2[25] = 0x01;
    rpt2[26] = 0x4f;
    rpt2[27] = 0xf4;
    rpt2[28] = 0x01;
    // HID Data: 600002890100...
    uint8_t rpt3[64] = {0};
    rpt3[0] = 0x60;
    rpt3[2] = 0x02;
    rpt3[3] = 0x89;
    rpt3[4] = 0x01;
    sendIntOut64(rpt1);
    sendIntOut64(rpt2);
    sendIntOut64(rpt3);
}
