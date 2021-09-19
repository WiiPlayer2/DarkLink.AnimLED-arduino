#ifndef COMM_H
#define COMM_H
#include <Arduino.h>

#define COMM_CMD_UPDATE_ANIMATION (uint8_t)0x01

#define COMM_RESP_OK (uint8_t)0x00
#define COMM_RESP_ERROR (uint8_t)0xFF

typedef uint16_t packet_size_t;

struct Packet
{
    packet_size_t Size;
    uint8_t* Data;
};

bool readPacket(Packet* packet);

#endif /* COMM_H */
