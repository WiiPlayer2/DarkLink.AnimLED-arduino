#define FASTLED_ALLOW_INTERRUPTS 0

#include <Arduino.h>
#include <FastLED.h>

#include "mathHelper.h"
#include "comm.h"

#define LED_PIN 5
#define NUM_LEDS 64
#define CHIPSET WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 32

CMD_UPLOAD meta_data;
uint8_t* frame_data = NULL;
uint8_t current_frame;
uint64_t lastFrameTime;

void draw_frame(uint8_t frame);

void setup()
{
    delay(3000);

    meta_data.Frames = 0;

    Serial.begin(115200);

    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection(TypicalSMD5050);
    FastLED.setBrightness(BRIGHTNESS);

    // Initial animation
    meta_data.ColorFormat = COLOR_FORMAT_2BPP;
    meta_data.Frames = 4;
    meta_data.Loop = true;
    meta_data.Delay = 150;
    uint8_t initialFrameData[(sizeof(CRGB) * 4) + ((NUM_LEDS * 4 * 2) / 8)] = {
        0x00, 0x00, 0x00, // Color #0
        0xFF, 0x00, 0x00, // Color #1
        0x00, 0xFF, 0x00, // Color #2
        0x00, 0x00, 0xFF, // Color #3
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, // Frame #1
        0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Frame #1
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, // Frame #2
        0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Frame #2
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, // Frame #3
        0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Frame #3
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x02, // Frame #4
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Frame #4
    };
    frame_data = initialFrameData;
    current_frame = 0;
    draw_frame(0);
    lastFrameTime = millis();
}

void draw_frame_RGB(uint8_t frame)
{
    memcpy(leds, frame_data + frame * NUM_LEDS * 3, NUM_LEDS * 3);
}

void draw_frame_Palette(uint8_t frame, uint8_t bitDepth)
{
    auto paletteSize = pow(2, bitDepth);
    CRGB* palette = (CRGB*)frame_data;
    auto bitOffset = (paletteSize * 3 * 8) + (frame * NUM_LEDS * bitDepth);
    uint8_t bitMask = ~(0xFF << bitDepth);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        auto currentBitOffset = bitOffset + (i * bitDepth);
        auto byteOffset = currentBitOffset / 8;
        auto bitIndex = currentBitOffset % 8;
        auto colorIndex = (frame_data[byteOffset] >> bitIndex) & bitMask;
        leds[i] = palette[colorIndex];
    }
}

void draw_frame(uint8_t frame)
{
    switch(meta_data.ColorFormat)
    {
        case COLOR_FORMAT_RGB:
            draw_frame_RGB(frame);
            break;

        case COLOR_FORMAT_MONOCHROME:
            draw_frame_Palette(frame, 1);
            break;

        case COLOR_FORMAT_2BPP:
            draw_frame_Palette(frame, 2);
            break;

        case COLOR_FORMAT_4BPP:
            draw_frame_Palette(frame, 4);
            break;
    }

    FastLED.show();
}

size_t get_palette_frame_data_size(uint8_t frame_count, uint8_t bitDepth)
{
    return (pow(2, bitDepth) * sizeof(CRGB)) + ((frame_count * NUM_LEDS * bitDepth) / 8);
}

size_t get_frame_data_size(uint8_t color_format, uint8_t frame_count)
{
    switch(color_format)
    {
        case COLOR_FORMAT_RGB:
            return frame_count * 3 * NUM_LEDS;

        case COLOR_FORMAT_MONOCHROME:
            return get_palette_frame_data_size(frame_count, 1);

        case COLOR_FORMAT_2BPP:
            return get_palette_frame_data_size(frame_count, 2);

        case COLOR_FORMAT_4BPP:
            return get_palette_frame_data_size(frame_count, 4);

        default:
            return 0;
    }
}

void cmd_upload(Packet packet)
{
    free(frame_data);

    memcpy(&meta_data, &packet.Data[1], sizeof(CMD_UPLOAD));
    auto frameDataSize = get_frame_data_size(meta_data.ColorFormat, meta_data.Frames);

    Packet frameDataPacket;
    readPacket(&frameDataPacket);

    if(frameDataPacket.Size != frameDataSize)
        meta_data.Frames = 0;
    else
        frame_data = frameDataPacket.Data;

    current_frame = 0;
    if(meta_data.Frames == 0)
    {
        free(frame_data);
        return;
    }

    draw_frame(0);
    lastFrameTime = millis();
}

void handlePacket(Packet packet)
{
    auto cmd = packet.Data[0];
    switch(cmd)
    {
        case COMM_CMD_UPDATE:
            cmd_upload(packet);
            break;
    }
}

void loop()
{
    if (Serial.available() > 0)
    {
        Packet packet;
        if(readPacket(&packet))
        {
            handlePacket(packet);
        }
        free(packet.Data);
    }

    if(meta_data.Frames == 0 || current_frame == meta_data.Frames)
        return;

    auto current_time = millis();
    if(current_time - lastFrameTime > meta_data.Delay)
    {
        lastFrameTime = current_time;
        current_frame++;
        if(meta_data.Loop && current_frame == meta_data.Frames)
            current_frame = 0;
        
        if(current_frame == meta_data.Frames)
            return;

        draw_frame(current_frame);
    }
}
