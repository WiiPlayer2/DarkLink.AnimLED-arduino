#include <Arduino.h>

#define COMM_CMD_UPLOAD 0x01
#define COMM_RESP_OK 0x00
#define COMM_RESP_ERROR 0xFF

#define COLOR_FORMAT_RGB 0x00

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

Packet readPacket()
{
    Packet packet;
    Serial.readBytes((uint8_t*)&packet.Size, sizeof(packet_size_t));
    // Serial.print(packet.Size);
    // Serial.flush();

    packet.Data = (uint8_t*)malloc(packet.Size);
    if(packet.Data != NULL)
    {
        int bytesRead = 0;
        while(bytesRead < packet.Size)
        {
            bytesRead += Serial.readBytes(packet.Data + bytesRead, packet.Size - bytesRead);
        }
    }

    return packet;
}
