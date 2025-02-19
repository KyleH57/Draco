#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include <vector>
#include <string>
#include "Segment.h"
#include "WS2812LED.h"

#include <FastLED.h>  // Include FastLED library

#define DATA_PIN 4 // TODO change to go in main

class Constellation {
public:
  Constellation(std::string angles, int num_leds_segment, float spacing, float edge_spacing);

  void run(unsigned long elapsed_time, uint8_t effect, uint8_t brightness);

  void reset_Animation(int progress);
  // Add your getter and setter functions here

private:
  std::vector<std::string> angles;
  std::vector<bool> repeat_flags;
  int num_leds_segment;
  float spacing;
  float edge_spacing;

  std::vector<Segment> segments;
  std::vector<WS2812LED> leds;

  int num_leds;
  CRGB* ledsFastLED; // Array to hold LED color data


  void createConstellation();

};

#endif
