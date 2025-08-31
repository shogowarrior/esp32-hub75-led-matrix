#include "PlasmaEffect.h"
#include <FastLED.h>
#include "Logger.h"

PlasmaEffect::PlasmaEffect()
    : time_counter(0) {
    initPalettes();
}

void PlasmaEffect::initPalettes() {
    palettes[0] = HeatColors_p;
    palettes[1] = LavaColors_p;
    palettes[2] = RainbowColors_p;
    palettes[3] = RainbowStripeColors_p;
    palettes[4] = CloudColors_p;
    currentPalette = palettes[0];
}

void PlasmaEffect::setup() {
    LOG_INFO("PlasmaEffect: Initialized");
}

void PlasmaEffect::loop(MatrixPanel_I2S_DMA *display) {
    if (!display) return;

    for (int x = 0; x < display->width(); x++) {
        for (int y = 0; y < display->height(); y++) {
            int16_t v = 128;
            uint8_t wibble = sin8(time_counter);
            v += sin16(x * wibble * 3 + time_counter);
            v += cos16(y * (128 - wibble) + time_counter);
            v += sin16(y * x * cos8(-time_counter) / 8);

            currentColor = ColorFromPalette(currentPalette, (v >> 8));
            display->drawPixelRGB888(x, y, currentColor.r, currentColor.g, currentColor.b);
        }
    }

    ++time_counter;

    if (time_counter >= 1024) {
        time_counter = 0;
        currentPalette = palettes[random(0, sizeof(palettes) / sizeof(palettes[0]))];
    }
}

void PlasmaEffect::setPalette(uint8_t paletteIndex) {
    if (paletteIndex < sizeof(palettes) / sizeof(palettes[0])) {
        currentPalette = palettes[paletteIndex];
    }
}