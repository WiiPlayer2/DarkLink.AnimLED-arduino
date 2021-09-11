#include <Arduino.h>

#define COMM_CMD_UPDATE (uint8_t)0x01

#define COMM_RESP_OK (uint8_t)0x00
#define COMM_RESP_ERROR (uint8_t)0xFF

#define COLOR_FORMAT_RGB 0x00
#define COLOR_FORMAT_MONOCHROME 0x01

typedef uint16_t packet_size_t;

struct CMD_UPLOAD
{
    uint8_t ColorFormat;
    uint8_t Frames;
    bool Loop;
    uint64_t Delay;
};

struct Packet
{
    packet_size_t Size;
    uint8_t* Data;
};

bool readPacket(Packet* packet);
