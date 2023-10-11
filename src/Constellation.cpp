#include "Constellation.h"
#include <sstream>
#include <cmath> // For cos, sin functions

Constellation::Constellation(std::string angles_str, int num_leds_segment, float spacing, float edge_spacing)
    : num_leds_segment(num_leds_segment), spacing(spacing), edge_spacing(edge_spacing), num_leds(0)
{

    std::istringstream ss(angles_str);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        angles.push_back(token);
        repeat_flags.push_back(token.find('r') != std::string::npos);
    }

    createConstellation();

    ledsFastLED = new CRGB[num_leds];
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(ledsFastLED, num_leds).setCorrection(TypicalLEDStrip);
}

void Constellation::createConstellation()
{
    float x = 0, y = 0;
    int counter = 0;

    for (int i=0; i<angles.size(); i++)
    {
        std::string::size_type sz;
        float angle = std::stof(angles[i], &sz);

        Segment segment(x, y, angle, num_leds_segment, spacing, edge_spacing, num_leds_segment * counter);
        segments.push_back(segment);
        num_leds += segment.get_num_leds();
        
        counter += 1;

        // Recalculate the endpoints for the next segment
        if(repeat_flags[i]) {
            // if 'r' flag is set for this segment, keep start coordinates for the next segment
            x = segment.get_x_start();
            y = segment.get_y_start();
        } else {
            x = segment.get_x_end();
            y = segment.get_y_end();
        }
    }

    // After all segments are created, create LEDs at each point along the segment
    for (auto &segment : segments)
    {
        for (int i = 0; i < segment.get_num_leds(); i++)
        {
            float x = segment.get_x_start() + (i * segment.get_spacing() + segment.get_edge_spacing()) * cos(segment.get_angle_degrees());
            float y = segment.get_y_start() + (i * segment.get_spacing() + segment.get_edge_spacing()) * sin(segment.get_angle_degrees());

            WS2812LED led(x, y);
            led.set_centroid(x - 0, y - 408.76); // Added centroid coordinates to each LED
            leds.push_back(led);

        }
    }
}




void Constellation::run(unsigned long elapsed_time, uint8_t effect, uint8_t brightness)
{
    // Set the overall brightness
    FastLED.setBrightness(brightness);

    // Rainbow fade effect variable
    static uint8_t hue = 0; // Start at red
    static uint8_t variance = 0;

    // switch case
    switch (effect)
    {
    case 0: // Random rgb wave
    {
        for (int i = 0; i < num_leds; i++) 
        {
            CHSV color = CHSV((i * 256 / num_leds + elapsed_time / 5) % 256, 255, 255);
            ledsFastLED[i] = color;
        }
        break;
    }
    case 1: // White
    {
        for (int i = 0; i < num_leds; i++)
        {
            ledsFastLED[i] = CRGB(255, 255, 255);
        }
        break;
    }
    case 2: // Color cycle no fade
    {
        hue = (elapsed_time / 50) % 256; // Cycle through all hues

        CHSV color = CHSV(hue, 255, 255); // Create color with current hue

        for (int i = 0; i < num_leds; i++)
        {
            ledsFastLED[i] = color;
        }
        break;
    }
    case 3: // Rainbow wave
    {
        float speed = 0.6;
        float wave_length = 800.0;
        float wave_progress = elapsed_time * speed;  // You may need to adjust units here
        for (int i = 0; i < num_leds; i++)
        {
            float x_coord = leds[i].get_xCoord_centroid();
            float hue = fmod((x_coord + wave_progress), wave_length) / wave_length;
            hue = hue * 255;  // Map from [0, 1] to [0, 255]

            CHSV color_hsv = CHSV(hue, 255, 255);  // Full saturation and value for a bright rainbow

            // Convert to RGB
            CRGB color_rgb;
            hsv2rgb_spectrum(color_hsv, color_rgb);

            ledsFastLED[i] = color_rgb;

        }
        break;
    }
    case 4: // Color cycle
    {
        hue = (elapsed_time / 30) % 256;                   // Cycle through all hues
        float sin_value = std::sin(elapsed_time / 1000.0); // get sin value
        uint8_t variance = (sin_value + 1) * 127.5;        // convert sin [-1, 1] to [0, 255]

        if (variance < 35)
        {
            variance = 0;
        }

        CHSV color = CHSV(hue, 255, variance); // Create color with current hue

        for (int i = 0; i < num_leds; i++)
        {
            ledsFastLED[i] = color;
        }
        break;
    }
    case 5:
    {
        // remote server mode
    }
    }
    FastLED.show();
}

void Constellation::reset_Animation(int progress)
{
    // set brightness to 100%
    FastLED.setBrightness(255);
    
    for (int i = 0; i < num_leds; i++)
    {
        ledsFastLED[i] = CRGB(0, 0, 0);
    }
    for (int i = 0; i < progress; i++)
    {
        ledsFastLED[i] = CRGB(0, 255, 0);
    }
    FastLED.show();
}