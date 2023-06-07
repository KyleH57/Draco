#include "WS2812LED.h"

WS2812LED::WS2812LED(float xCoord, float yCoord) 
  : xCoord(xCoord), yCoord(yCoord), xCoord_centroid(0), yCoord_centroid(0) {
  color = {0, 0, 0};
}

void WS2812LED::set_Coords(float x, float y) {
  xCoord = x;
  yCoord = y;
}

float WS2812LED::get_xCoord() {
  return xCoord;
}

float WS2812LED::get_yCoord() {
  return yCoord;
}

void WS2812LED::set_centroid(float x, float y) {
  xCoord_centroid = x;
  yCoord_centroid = y;
}

float WS2812LED::get_xCoord_centroid() {
  return xCoord_centroid;
}

float WS2812LED::get_yCoord_centroid() {
  return yCoord_centroid;
}

void WS2812LED::set_color(const Color& new_color) {
  color = new_color;
}

Color WS2812LED::get_color() {
  return color;
}
