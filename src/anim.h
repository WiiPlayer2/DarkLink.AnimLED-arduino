#ifndef ANIM_H
#define ANIM_H

#include "common.h"

#include <Arduino.h>
#include <FastLED.h>

#define COLOR_FORMAT_RGB 0x00
#define COLOR_FORMAT_1BPP 0x01
#define COLOR_FORMAT_2BPP 0x02
#define COLOR_FORMAT_4BPP 0x03

typedef struct
{
    byte ImageIndex;
    byte DelayFactor;
} AnimationFrame;

typedef struct
{
    byte FrameCount;
    AnimationFrame* Frames;
} AnimationFrames;

typedef struct
{
    byte ColorFormat;
    byte ImageCount;
    byte FrameCount;
    byte LoopStartIndex;
    uint64_t BaseDelay;
} AnimationMeta;

typedef struct
{
    AnimationMeta* Meta;
    AnimationFrame* Frames;
    byte* ImageData;
} Animation;

typedef struct
{
    Animation Data;
    byte CurrentFrame;
    uint64_t LastFrameAdvanceTime;
    bool IsRunning;
} AnimationState;

// void renderImage(CRGB leds[], Animation* anim, byte index);

bool validate(AnimationMeta meta, size_t size);

void initAnimation(CRGB leds[], AnimationState* state);

void tickAnimation(CRGB leds[], AnimationState* state);

#endif /* ANIM_H */
