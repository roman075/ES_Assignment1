#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define GPIO pins
#define SWITCH1_PIN GPIO_NUM_16  // Switch 1 (Enable/Disable Waveform)
#define SWITCH2_PIN GPIO_NUM_17  // Switch 2 (Mode Change)
#define LED_PIN GPIO_NUM_27      // Signal A (Main Waveform)
#define SIGNALB_PIN GPIO_NUM_26  // Signal B (Sync Signal)

// DAFF Parameters
unsigned long a = 400;  // Pulse width for first pulse (D = 4 * 100μs = 400μs)
unsigned long b = 100;  // Spaces between pulses (A = 1 * 100μs = 100μs)
int c = 10;             // Default number of pulses in a block (F = 6 + 4 = 10)
unsigned long d = 3000; // Space between blocks (F = 6 * 500μs = 3000μs)

// Modes

// Alternative mode 
//f maps to 6, (6 % 4) + 1 = 3 
//Mode 3 is + 3 pulses to the end of a block
bool enableWaveform = false; // Start with waveform generation disabled
bool switch2Pressed = false; 
bool isAlternativeMode = false; // Track whether we are in the alternative mode (+3 pulses)

void setup_gpio() {
    // Configure GPIOs
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; 
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_PIN) | (1ULL << SIGNALB_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << SWITCH1_PIN) | (1ULL << SWITCH2_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
}

void generateWaveform() {
    printf("Generating waveform...\n");
    unsigned long pulseWidth;
    int pulseCount = c;

    // Alternative wave mode
    if (isAlternativeMode) {
        pulseCount = c + 3; // Add 3 pulses when alternative mode is selected 
    }

    // Signal B (Sync/Trigger Signal)
    gpio_set_level(SIGNALB_PIN, 1);
    esp_rom_delay_us(50); // 50 μs trigger pulse
    gpio_set_level(SIGNALB_PIN, 0);

    // Signal A (Main Waverform)
    for (int i = 0; i < pulseCount; i++) {
        pulseWidth = a + i * 50; // Increase pulse width by 50μs for each pulse
        gpio_set_level(LED_PIN, 1);
        esp_rom_delay_us(pulseWidth);
        gpio_set_level(LED_PIN, 0);
        esp_rom_delay_us(b);
        printf("Pulse %d generated\n", i + 1);
    }

    // Wait before starting the next block
    esp_rom_delay_us(d);
    printf("Waveform block complete\n");
}

void app_main() {
    setup_gpio();
    printf("ESP32 Waveform Generator with Trigger Signal Started\n");

    while (1) {
        // Read Switch 1 (Enable/Disable)
        enableWaveform = !gpio_get_level(SWITCH1_PIN); // Active LOW
        if (!enableWaveform) {
            gpio_set_level(LED_PIN, 0);      // Turn off Signal A
            gpio_set_level(SIGNALB_PIN, 0); // Turn off Signal B
            continue; 
        }

        // Read Switch 2 (Mode Selection)
        if (!gpio_get_level(SWITCH2_PIN)) { // Active LOW
            if (!switch2Pressed) { 
                isAlternativeMode = !isAlternativeMode; // Toggle between default and +3 states
                printf("Mode changed to: %s\n", isAlternativeMode ? "Alternative (+3 pulses)" : "Default");
                switch2Pressed = true; 
            }
        } else {
            switch2Pressed = false; // Reset when switch is released (prevents multiple presses)
        }

        // Generate waveform and trigger signal
        generateWaveform();
    }
}