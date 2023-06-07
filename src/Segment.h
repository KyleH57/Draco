#ifndef SEGMENT_H
#define SEGMENT_H

#include <cmath>  // For math functions
#include <stdint.h>  // For uint8_t

class Segment {
public:
  Segment(float x_start, float y_start, float angle_degrees, int num_leds, float spacing, float edge_spacing, int start_index);

  void set_start(float x, float y);
  float get_x_start();
  float get_y_start();

  float get_angle_degrees();

  int get_num_leds();





  int get_start_index();

  float get_total_length();
  float get_spacing();
  float get_edge_spacing();

  float get_x_end();
  float get_y_end();

private:
  float x_start;
  float y_start;
  
  float angle; // in radians
  
  int num_leds;
  float spacing;
  float edge_spacing;
  int start_index;

  float total_length;
  
  float x_end;
  float y_end;
};

#endif
