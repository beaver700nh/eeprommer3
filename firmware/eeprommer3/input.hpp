#ifndef INPUT_HPP
#define INPUT_HPP

class JoystickCtrl {
public:
  JoystickCtrl(uint8_t x_axis, uint8_t y_axis, uint8_t button);

  void init();

  void poll(uint8_t max_btn);

  uint8_t get_cur_btn();
  bool btn_changed();
  bool is_pressed();

private:
  uint8_t m_x_axis, m_y_axis, m_button;

  uint8_t m_prev_btn = 0;
  uint8_t m_cur_btn = 0;

  uint8_t m_is_pressed = false;
};

#endif
