#include "volumes.h"
Volume::Volume()
  : Volume(0, 0, false, true) {}

Volume::Volume(bool isCentered, uint16_t height)
  : Volume(height, 0, isCentered, true) {}

Volume::Volume(uint16_t height, uint16_t offset)
  : Volume(height, offset, false, true) {}

Volume::Volume(uint16_t height, uint16_t offset, bool isCentered, bool hasPeak)
  : m_height(height)
  , m_offset(offset)
  , m_isCentered(isCentered)
  , m_hasPeak(hasPeak)
  , m_isReversed(false) {
}

void Volume::SetHeight(uint16_t height) {
  m_height = height;
}

void Volume::SetOffset(uint16_t offset) {
  m_offset = offset;
}

void Volume::Centered(bool isCentered) {
  m_isCentered = isCentered;
}

void Volume::Peak(bool hasPeak) {
  m_hasPeak = hasPeak;
}

void Volume::Reverse(bool isReversed) {
  m_isReversed = isReversed;
  if (m_isReversed) m_isCentered = false;
}

CRGB Volume::GetAt(uint16_t index) {
  return m_leds[index];
}

uint8_t Volume::rainbowHue2(uint8_t pixel, uint8_t num_pixels) {
  uint8_t hue = 96 - pixel * (145 / num_pixels);
  return hue;
}

void Volume::dropPeak() {
  static uint8_t dotCount;
  if (++dotCount >= PEAK_FALL) { //fall rate
    if (m_peak > 0) m_peak--;
    dotCount = 0;
  }
}
void Volume::Update(uint16_t hight) {
  auto height = hight + m_prev / 2;
  if (m_isCentered) {
    const uint16_t halfheight = m_height / 2;
    // Fill with colour gradient
    fill_gradient(m_leds, m_offset + halfheight, CHSV(96, 255, 255), m_offset + m_height - 1, CHSV(224, 255, 255), SHORTEST_HUES);
    fill_gradient(m_leds, m_offset + (halfheight - 1), CHSV(96, 255, 255), m_offset, CHSV(224, 255, 255), LONGEST_HUES);

    // Black out ends
    for (uint16_t i = 0; i < halfheight; i++) {
      if (i >= height) {
        m_leds[m_offset + halfheight + i] = CRGB::Black;
        m_leds[m_offset + (halfheight - 1) - i] = CRGB::Black;
      }
    }

    if (m_hasPeak) {
      // Draw peak dot
      if (height > m_peak)
        m_peak = height / 2; // Keep 'peak' dot at top

      if (m_peak > 0 && m_peak <= halfheight - 1) {
        m_leds[m_offset + halfheight + m_peak] = CHSV(rainbowHue2(m_peak, halfheight), 255, 255);
        m_leds[m_offset + halfheight - 1 - m_peak] = CHSV(rainbowHue2(m_peak, halfheight), 255, 255);
      }
    }
  }
  else {
    if (m_isReversed) {
      // Fill with color gradient
      fill_gradient(m_leds, m_offset + m_height - 1, CHSV(96, 255, 255), m_offset, CHSV(224, 255, 255), LONGEST_HUES);

      //Black out end
      for (uint16_t i = 0; i < m_height; i++) {
        if (i >= height) m_leds[m_offset + m_height - 1 - i] = CRGB::Black;
      }
      if (m_hasPeak) {
        // Draw peak dot
        if (height > m_peak)
          m_peak = height; // Keep 'peak' dot at top
        if (m_peak > 0 && m_peak <= m_height - 1)
          m_leds[m_offset + m_height - 1 - m_peak] = CHSV(rainbowHue2(m_peak, m_offset + m_height), 255, 255); // Set peak colour correctly
      }
    }
    else {
      // Fill with color gradient
      fill_gradient(m_leds, m_offset, CHSV(96, 255, 255), m_offset + m_height - 1, CHSV(224, 255, 255), SHORTEST_HUES);

      //Black out end
      for (uint16_t i = 0; i < m_height; i++) {
        if (i >= height) m_leds[i + m_offset] = CRGB::Black;
      }
      if (m_hasPeak) {
        // Draw peak dot
        if (height > m_peak)
          m_peak = height; // Keep 'peak' dot at top
        if (m_peak > 0 && m_peak <= m_height - 1)
          m_leds[m_peak + m_offset] = CHSV(rainbowHue2(m_peak, m_offset + m_height), 255, 255); // Set peak colour correctly
      }
    }
  }
  m_prev = hight;
  dropPeak();
}

Volumes::Volumes()
  : Volumes(1, 0) {}

Volumes::Volumes(uint8_t count, uint8_t overlap)
  : m_count(count) {
  // calculate the size of each segment
  uint16_t grootte = NUM_LEDS / count;
  int highestLed = NUM_LEDS;
  const uint8_t highestIndex = count - 1;
  for (uint8_t index = 0; index < count; ++index) {
    uint16_t startindex = index * grootte;
    uint16_t endindex = (index + 1) * grootte;
    uint16_t eersteLed = max(0, startindex - overlap);
    uint16_t laatsteLed = min(highestLed, endindex + overlap);
    allVolumes[index].SetOffset(eersteLed);
    allVolumes[index].SetHeight(laatsteLed - eersteLed);
    allVolumes[index].Centered(index != 0);
    allVolumes[index].Peak(true || (index == 0) || (index == highestIndex));
    allVolumes[index].Reverse(index == highestIndex);
  }
}

CRGB Volumes::GetAt(uint16_t pixel) {
  CRGB value;
  for (uint8_t index = 0; index < m_count; ++index) {
    value += allVolumes[index].GetAt(pixel);
  }
  return value;
}

void Volumes::Update(uint8_t index, uint16_t height) {
  allVolumes[index].Update(height / m_count);
}
