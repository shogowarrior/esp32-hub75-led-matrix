#ifndef PLASMA_EFFECT_H
#define PLASMA_EFFECT_H

/**
 * @file PlasmaEffect.h
 * @brief Plasma visual effects for ESP32 HUB75 LED Matrix
 *
 * Handles plasma effect rendering capabilities for the LED matrix,
 * including color palettes and animation loops.
 */

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FastLED.h>

class PlasmaEffect {
public:
    // =============================================================================
    // Lifecycle Management
    // =============================================================================

    /**
     * @brief Constructor - initializes plasma effect
     */
    PlasmaEffect();

    // =============================================================================
    // Setup and Control
    // =============================================================================

    /**
     * @brief Setup the plasma effect
     */
    void setup();

    /**
     * @brief Run the plasma animation loop
     * @param display Pointer to the LED matrix display
     */
    void loop(MatrixPanel_I2S_DMA *display);

    // =============================================================================
    // Palette Management
    // =============================================================================

    /**
     * @brief Set the current color palette
     * @param paletteIndex Index of the palette to use
     */
    void setPalette(uint8_t paletteIndex);

private:
    // =============================================================================
    // Private Members
    // =============================================================================

    // Initialize color palettes
    void initPalettes();

    uint16_t time_counter;              //< Animation time counter
    CRGB currentColor;                  //< Current color for effects
    CRGBPalette16 palettes[5];          //< Available color palettes
    CRGBPalette16 currentPalette;       //< Currently selected palette
};

#endif // PLASMA_EFFECT_H