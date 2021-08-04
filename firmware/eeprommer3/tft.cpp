#include <Arduino.h>
#include "constants.hpp"

#include <Elegoo_TFTLCD.h>
#include <TouchScreen.h>

#include "tft.hpp"

TftCtrl::TftCtrl(uint8_t cs, uint8_t rs, uint8_t wr, uint8_t rd, uint8_t rst)
  : Elegoo_TFTLCD(cs, rs, wr, rd, rst) {
  // Empty
}

void TftCtrl::init(uint16_t driver_id, uint8_t orientation) {
  reset();

  delay(500);

  begin(driver_id);
  setRotation(orientation);
}

void TftCtrl::drawText(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t size) {
  setTextColor(color);
  setTextSize(size);
  setCursor(x, y);
  print(text);
}

TftBtn::TftBtn(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char *text, uint16_t fg, uint16_t bg)
  : m_x(x), m_y(y), m_w(w), m_h(h), m_fg(fg), m_bg(bg) {
  strncpy(m_text, text, 20);
}

void TftBtn::draw(TftCtrl &tft) {
  tft.fillRect(m_x, m_y, m_w, m_h, m_bg);
  tft.drawText(m_x + 3, m_y + 3, m_text, m_fg);
}

bool TftBtn::is_pressed(TouchScreen &ts) {
  TSPoint p = map_point(ts.getPoint());

  if (10 > p.z || p.z > 1000) return false;

#ifdef DEBUG_MODE
  Serial.println("is_pressed():");
  serial_print_point(p);
#endif

  if (
    m_x < p.x && p.x < (m_x + m_w) && \
    m_y < p.y && p.y < (m_y + m_h)
  ) {
    most_recent_press = p;
    return true;
  }

  return false;
}

TSPoint map_point(TSPoint p) {
  int16_t x, y, z;
  x = map(p.y, 84, 904, 0, 320);
  y = map(p.x, 98, 905, 0, 240);
  z = p.z;

  return TSPoint(x, y, z);
}

#ifdef DEBUG_MODE
void tft_print_point(TSPoint p, TftCtrl &tft) {
  char buf[50];
  sprintf(buf, "x = %d, y = %d, z = %d", p.x, p.y, p.z);
  
  tft.drawText(20, 120, buf, TftColor::PINKK, 2);
}

void serial_print_point(TSPoint p) {
  char buf[50];
  sprintf(buf, "x = %d, y = %d, z = %d", p.x, p.y, p.z);
  
  Serial.println(buf);
}
#endif
