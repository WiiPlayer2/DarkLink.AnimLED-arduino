#ifndef ANIM_H
#define ANIM_H

#include <Arduino.h>

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

const Animation NULL_ANIMATION = {};

#endif /* ANIM_H */
