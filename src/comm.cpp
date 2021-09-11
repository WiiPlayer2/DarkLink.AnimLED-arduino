#include "comm.h"

Packet readPacket()
{
    Packet packet;
    Serial.readBytes((uint8_t*)&packet.Size, sizeof(packet_size_t));

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
