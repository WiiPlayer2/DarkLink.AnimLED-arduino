#include "comm.h"

#define HEADER_SIZE 5

const uint8_t header[HEADER_SIZE] = { 0xFE, 0xED, 0xC0, 0xFF, 0xEE };

void respond(uint8_t code)
{
    Serial.write(code);
    Serial.flush();
}

bool readPacket(Packet* packet)
{
    packet_size_t size;
    Serial.readBytes((uint8_t*)&size, sizeof(packet_size_t));

    if(size < HEADER_SIZE)
    {
        respond(COMM_RESP_ERROR);
        return false;
    }

    uint8_t headerBytes[HEADER_SIZE];
    Serial.readBytes(headerBytes, HEADER_SIZE);
    for(auto i = 0; i < HEADER_SIZE; i++)
    {
        if(headerBytes[i] != header[i])
        {
            respond(COMM_RESP_ERROR);
            return false;
        }
    }

    packet->Size = size - HEADER_SIZE;
    packet->Data = (uint8_t*)malloc(packet->Size);
    if(packet->Data == NULL)
    {
        respond(COMM_RESP_ERROR);
        return false;
    }

    size_t bytesRead = 0;
    while(bytesRead < packet->Size)
    {
        bytesRead += Serial.readBytes(packet->Data + bytesRead, packet->Size - bytesRead);
    }

    respond(COMM_RESP_OK);
    return true;
}
