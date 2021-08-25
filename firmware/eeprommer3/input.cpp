#include <Arduino.h>
#include "constants.hpp"

#include "input.hpp"

JoystickCtrl::JoystickCtrl(uint8_t x_axis, uint8_t y_axis, uint8_t button)
  : m_x_axis(x_axis), m_y_axis(y_axis), m_button(button) {
  // Empty
}

void JoystickCtrl::init() {
  pinMode(m_button, INPUT_PULLUP);
}

void JoystickCtrl::poll(uint8_t max_btn) {
  m_is_pressed = (digitalRead(m_button) == LOW);

  m_prev_btn = m_cur_btn;
  m_cur_btn = 0; // TODO
}

uint8_t JoystickCtrl::get_cur_btn() {
  return m_cur_btn;
}

bool JoystickCtrl::btn_changed() {
  return m_prev_btn != m_cur_btn;
}

bool JoystickCtrl::is_pressed() {
  return m_is_pressed;
}
