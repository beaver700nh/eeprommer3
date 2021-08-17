#include <Arduino.h>
#include "constants.hpp"

#include <Elegoo_TFTLCD.h>
#include <TouchScreen.h>

#include "tft.hpp"

TouchscreenCtrl::TouchscreenCtrl(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t resist)
  : TouchScreen(xp, yp, xm, ym, resist) {
  // Empty
}

TSPoint TouchscreenCtrl::getPoint(bool raw) {
  TSPoint p = TouchScreen::getPoint();
  pinMode(TS_XM, OUTPUT);
  pinMode(TS_YP, OUTPUT);

  return (raw ? p : mapPoint(p, TS_CALIB_MINX, TS_CALIB_MAXX, TS_CALIB_MINY, TS_CALIB_MAXY));
}

bool TouchscreenCtrl::isValidPoint(TSPoint p, int16_t maxz) {
  return isValidPressure(p.z, maxz);
}

bool TouchscreenCtrl::isValidPressure(int16_t z, int16_t maxz) {
  return TS_MIN_PRESSURE < z && (maxz < 0 ? true : z < maxz);
}

/* minx = 98; maxx = 905; miny = 84; maxy = 904 */
/* this function was written specifically for my own TFT, */
/* and it may not work for your TFT screen */
/* feel free to fork and modify for your own needs */
TSPoint TouchscreenCtrl::mapPoint(TSPoint p, uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy) {
  int16_t x = map(p.y, miny, maxy, 0, 320);
  int16_t y = map(p.x, minx, maxx, 0, 240);
  int16_t z = p.z;

#ifdef DEBUG_MODE
  if (isValidPressure(p.z)) {
    char buf[200];
    Serial.println("=== POINT ===");
    sprintf(buf, "RAW:    x = %d, y = %d, z = %d", p.x, p.y, p.z);
    Serial.println(buf);
    sprintf(buf, "MAPPED: x = %d, y = %d, z = %d", x, y, z);
    Serial.println(buf);
    Serial.println("=== END ===");
  }
#endif

//  if (x <= 120) {
//    y = map(y, 0, 240 - (-pow(x / 26.0, 2.8) + 130), 0, 240);
//  }
//  else if (x <= 140) {
//    y = map(y, 0, 240 - (-2.4 * x + 349), 0, 240);
//  }
//  else if (x <= 188) {
//    y = map(y, 0, 240 - (-0.25 * x + 47), 0, 240);
//  }

  return TSPoint(x, y, z);
}

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

bool TftBtn::is_pressed(TouchscreenCtrl &ts) {
  TSPoint p = ts.getPoint(false);

  if (!ts.isValidPoint(p)) return false;

#ifdef DEBUG_MODE
  Serial.println("is_pressed():");
  serial_print_point(p);
#endif

  if (
    m_x < p.x && p.x < (m_x + m_w) && \
    m_y < p.y && p.y < (m_y + m_h)
  ) {
    return true;
  }

  return false;
}

bool TftMenu::add_btn(TftBtn *btn) {
  ++m_num_btns;

  auto new_arr = (TftBtn **) malloc(m_num_btns * sizeof(TftBtn *));

  if (new_arr == nullptr) {
    return false;
  }

  memcpy(new_arr, m_btns, m_num_btns * sizeof(TftBtn *));
  new_arr[m_num_btns - 1] = btn;

  free(m_btns);

  m_btns = new_arr;

  return true;
}

void TftMenu::draw(TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->draw(tft);
  }
}

uint8_t TftMenu::is_pressed(TouchscreenCtrl &ts) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    if (m_btns[i]->is_pressed(ts)) return i + 1;
  }

  return 0;
}

uint8_t TftMenu::wait_any_btn_down(TouchscreenCtrl &ts) {
  uint8_t btn = 0;

  while ((btn = is_pressed(ts)) == 0) { /* wait for press */; }

  return btn;
}

void TftMenu::wait_all_btn_up(TouchscreenCtrl &ts) {
  while (is_pressed(ts) != 0) { /* wait for no press */; }
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
