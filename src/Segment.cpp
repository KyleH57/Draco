#include "Segment.h"

Segment::Segment(float x_start, float y_start, float angle_degrees, int num_leds, float spacing, float edge_spacing, int start_index)
  : x_start(x_start), y_start(y_start), angle(angle_degrees * M_PI / 180.0), num_leds(num_leds), spacing(spacing), edge_spacing(edge_spacing), start_index(start_index) {
  
  total_length = (num_leds - 1) * spacing + 2 * edge_spacing;

  x_end = x_start + total_length * cos(angle);
  y_end = y_start + total_length * sin(angle);

  num_leds = num_leds;
}

void Segment::set_start(float x, float y) {
  x_start = x;
  y_start = y;

  x_end = x_start + total_length * cos(angle);
  y_end = y_start + total_length * sin(angle);
}

float Segment::get_x_start() {
  return x_start;
}

float Segment::get_y_start() {
  return y_start;
}

float Segment::get_angle_degrees() {
  return angle;// * 180.0 / M_PI;
}

int Segment::get_num_leds() {
  return num_leds;
}

int Segment::get_start_index() {
  return start_index;
}

float Segment::get_total_length() {
  return total_length;
}

float Segment::get_spacing() {
  return spacing;
}

float Segment::get_edge_spacing() {
  return edge_spacing;
}

float Segment::get_x_end() {
  return x_end;
}

float Segment::get_y_end() {
  return y_end;
}
