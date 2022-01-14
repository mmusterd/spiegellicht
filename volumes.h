#include "FastLED.h"
#pragma once
constexpr unsigned int NUM_LEDS = 31;
constexpr unsigned int PEAK_FALL = 7;               // Rate of peak falling dot [20]
constexpr unsigned int SAMPLES = 128;           //Must be a power of 2

class Volume
{
  public:
    Volume();
    Volume(bool isCentered, uint16_t height);
    Volume(uint16_t height, uint16_t offset);
    Volume(uint16_t height, uint16_t offset, bool isCentered, bool hasPeak);
    void SetHeight(uint16_t height);
    void SetOffset(uint16_t offset);
    void Centered(bool isCentered);
    void Peak(bool hasPeak);
    void Reverse(bool isReversed);

    void Update(uint16_t height);
    CRGB GetAt(uint16_t index);
  private:

    uint8_t rainbowHue2(uint8_t pixel, uint8_t num_pixels);
    void dropPeak();
    uint16_t m_height;
    uint16_t m_offset;
    bool m_isCentered;
    bool m_hasPeak;
    bool m_isReversed;
    CRGB m_leds[NUM_LEDS];
    uint16_t m_prev;
    uint16_t m_peak;
};

class Volumes
{
  public:
    Volumes();
    Volumes(uint8_t count, uint8_t overlap);
    void Update(uint8_t index, uint16_t height);
    CRGB GetAt(uint16_t pixel);
  private:
    Volume allVolumes[10];
    uint8_t m_count;
};
///
///
