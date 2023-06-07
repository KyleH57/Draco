#ifndef WS2812LED_H
#define WS2812LED_H

#include <stdint.h>  // For uint8_t

struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

class WS2812LED {
public:
  WS2812LED(float xCoord, float yCoord);

  void set_Coords(float x, float y);
  float get_xCoord();

  float get_yCoord();

  void set_centroid(float x, float y);
  float get_xCoord_centroid();
  float get_yCoord_centroid();

  void set_color(const Color& new_color);
  Color get_color();

private:
  float xCoord;
  float yCoord;

  float xCoord_centroid;
  float yCoord_centroid;

  Color color;
};

#endif
