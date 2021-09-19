#include "common.h"

#include <Arduino.h>
#include <FastLED.h>

#include "comm.h"
#include "anim.h"

#define LED_PIN 5
#define CHIPSET WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS 16

CRGB leds[NUM_LEDS];

AnimationState currentAnimation = {};
byte* animationData = nullptr;

void setup()
{
    delay(3000);

    Serial.begin(115200);

    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection(TypicalSMD5050);
    FastLED.setBrightness(BRIGHTNESS);

// //     // Initial animation
// //     meta_data.ColorFormat = COLOR_FORMAT_2BPP;
// //     meta_data.Frames = 4;
// //     meta_data.Loop = true;
// //     meta_data.Delay = 250;
// // #define INITIAL_FRAME_DATA_SIZE ((sizeof(CRGB) * 4) + ((NUM_LEDS * 4 * 2) / 8))
// //     uint8_t initialFrameData[INITIAL_FRAME_DATA_SIZE] = {
// //         0x00, 0x00, 0x00, // Color #0
// //         0xFF, 0x00, 0x00, // Color #1
// //         0x00, 0xFF, 0x00, // Color #2
// //         0x00, 0x00, 0xFF, // Color #3
// //         0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, // Frame #1
// //         0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, // Frame #1
// //         0x55, 0x00, 0x55, 0x00, 0x55, 0x00, 0x55, 0x00, // Frame #2
// //         0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, // Frame #2
// //         0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, // Frame #3
// //         0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, // Frame #3
// //         0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, // Frame #4
// //         0x00, 0x55, 0x00, 0x55, 0x00, 0x55, 0x00, 0x55, // Frame #4
// //     };
// //     frame_data = (uint8_t*)malloc(INITIAL_FRAME_DATA_SIZE);
// //     memcpy(frame_data, initialFrameData, INITIAL_FRAME_DATA_SIZE);
// //     current_frame = 0;
// //     draw_frame(0);
// //     lastFrameTime = millis();
}

bool cmdUpdateAnimation(Packet packet)
{
    free(animationData);
    currentAnimation = {};
    auto size = packet.Size - 1;
    auto dataPtr = packet.Data + 1;

    if(size < sizeof(AnimationMeta))
        return true;

    AnimationMeta* meta = (AnimationMeta*)dataPtr;
    if(!validate(*meta, size))
        return true;

    animationData = dataPtr;
    currentAnimation.Data.Meta = meta;
    currentAnimation.Data.Frames = (AnimationFrame*)(dataPtr + sizeof(AnimationMeta));
    currentAnimation.Data.ImageData = ((byte*) currentAnimation.Data.Frames) + sizeof(AnimationFrame) * meta->FrameCount;
    initAnimation(leds, &currentAnimation);
    return false;
}

// Returns true if packet can be freed.
bool handlePacket(Packet packet)
{
    auto cmd = packet.Data[0];
    switch(cmd)
    {
        case COMM_CMD_UPDATE_ANIMATION:
            return cmdUpdateAnimation(packet);

        default:
            return true;
    }
}

void loop()
{
    if (Serial.available() > 0)
    {
        Packet packet;
        bool freePacket = true;
        if(readPacket(&packet))
            freePacket = handlePacket(packet);
        if(freePacket)
            free(packet.Data);
    }

    if(!currentAnimation.IsRunning)
        return;

    tickAnimation(leds, &currentAnimation);
}
