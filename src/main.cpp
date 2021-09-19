#include "common.h"

#include <Arduino.h>
#include <FastLED.h>

#include "comm.h"
#include "anim.h"
#include "init.h"

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

    setInitialAnimation(&currentAnimation.Data);
    initAnimation(leds, &currentAnimation);
}

bool cmdUpdateAnimation(Packet packet)
{
    free(animationData);
    currentAnimation = {};
    auto size = packet.Size - 1;
    auto dataPtr = packet.Data + 1;

    if(size < sizeof(AnimationMeta))
    {
        FastLED.showColor(CRGB::Yellow);
        return true;
    }

    AnimationMeta* meta = (AnimationMeta*)dataPtr;
    if(!validate(*meta, size))
    {
        FastLED.showColor(CRGB::Red);
        return true;
    }

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
