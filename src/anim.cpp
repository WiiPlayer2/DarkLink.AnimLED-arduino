#include "anim.h"

#pragma region Validate
size_t get_palette_frame_data_size(byte imageCount, byte bitDepth)
{
    return ((1 << bitDepth) * sizeof(CRGB)) + ((imageCount * NUM_LEDS * bitDepth) / 8);
}

size_t getImageDateSize(AnimationMeta meta)
{
    switch(meta.ColorFormat)
    {
        case COLOR_FORMAT_RGB:
            return meta.ImageCount * 3 * NUM_LEDS;

        case COLOR_FORMAT_1BPP:
            return get_palette_frame_data_size(meta.ImageCount, 1);

        case COLOR_FORMAT_2BPP:
            return get_palette_frame_data_size(meta.ImageCount, 2);

        case COLOR_FORMAT_4BPP:
            return get_palette_frame_data_size(meta.ImageCount, 4);

        default:
            return 0;
    }
}

bool validate(AnimationMeta meta, size_t size)
{
    auto expectedSize = sizeof(AnimationMeta)
        + sizeof(AnimationFrame) * meta.FrameCount
        + getImageDateSize(meta);
    return expectedSize == size;
}
#pragma endregion

#pragma region Render
void renderImageRGB(CRGB leds[], Animation* anim, byte index)
{
    memcpy(leds, anim->ImageData + index * NUM_LEDS * 3, NUM_LEDS * 3);
}

void renderImageBPP(CRGB leds[], Animation* anim, byte index, byte bitDepth)
{
    auto paletteSize = 1 << bitDepth;
    CRGB* palette = (CRGB*)anim->ImageData;
    auto bitOffset = (paletteSize * 3 * 8) + (index * NUM_LEDS * bitDepth);
    uint8_t bitMask = ~(0xFF << bitDepth);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        auto currentBitOffset = bitOffset + (i * bitDepth);
        auto byteOffset = currentBitOffset / 8;
        auto bitIndex = currentBitOffset % 8;
        auto colorIndex = (anim->ImageData[byteOffset] >> bitIndex) & bitMask;
        leds[i] = palette[colorIndex];
    }
}

void renderImage(CRGB leds[], Animation* anim, byte index)
{
    switch(anim->Meta->ColorFormat)
    {
        case COLOR_FORMAT_RGB:
            renderImageRGB(leds, anim, index);
            break;

        case COLOR_FORMAT_1BPP:
            renderImageBPP(leds, anim, index, 1);
            break;

        case COLOR_FORMAT_2BPP:
            renderImageBPP(leds, anim, index, 2);
            break;

        case COLOR_FORMAT_4BPP:
            renderImageBPP(leds, anim, index, 4);
            break;
    }

    FastLED.show();
}
#pragma endregion

void renderAnimation(CRGB leds[], AnimationState *state)
{
    auto meta = state->Data.Frames[state->CurrentFrame];
    renderImage(leds, &state->Data, meta.ImageIndex);
    state->LastFrameAdvanceTime = millis();
}

void initAnimation(CRGB leds[], AnimationState* state)
{
    if(state->Data.ImageData == nullptr)
        return;

    state->CurrentFrame = 0;
    state->IsRunning = state->Data.Meta->FrameCount != 0 && state->Data.Meta->ImageCount != 0;
    if(!state->IsRunning)
        return;

    renderAnimation(leds, state);
}

void advanceAnimation(CRGB leds[], AnimationState* state)
{
    auto newFrameIndex = state->CurrentFrame + 1;
    if(newFrameIndex >= state->Data.Meta->FrameCount)
    {
        if(state->Data.Meta->LoopStartIndex >= state->Data.Meta->FrameCount)
        {
            state->IsRunning = false;
            return;
        }

        newFrameIndex = state->Data.Meta->LoopStartIndex;
    }

    state->CurrentFrame = newFrameIndex;
    renderAnimation(leds, state);
}

void tickAnimation(CRGB leds[], AnimationState* state)
{
    if(!state->IsRunning)
        return;

    auto currentTime = millis();
    auto frameMeta = state->Data.Frames[state->CurrentFrame];
    auto delay = state->Data.Meta->BaseDelay * frameMeta.DelayFactor;
    auto timeDiff = currentTime - state->LastFrameAdvanceTime;
    if(timeDiff >= delay)
        advanceAnimation(leds, state);
}
