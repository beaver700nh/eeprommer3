#include <Arduino.h>
#include "constants.hpp"

#include <Elegoo_TFTLCD.h>
#include <TouchScreen.h>

#include "tft.hpp"

TouchscreenCalibration::TouchscreenCalibration(uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy, int16_t (*table)[9][9][2])
  : m_minx(minx), m_maxx(maxx), m_miny(miny), m_maxy(maxy), m_table(table) {
  // Empty
}

/* this function was written specifically for my own TFT, */
/* and it may not work for your TFT screen */
/* feel free to fork and modify for your own needs */
TSPoint TouchscreenCalibration::adc_data_to_tft_coords(TSPoint p, uint16_t tft_width, uint16_t tft_height, uint8_t fineness) {
  // TEMPORARY
  if (!TouchscreenCtrl::isValidPoint(p)) return;

  const uint16_t CELL_WIDTH  = TFT_WIDTH  / fineness;
  const uint16_t CELL_HEIGHT = TFT_HEIGHT / fineness;
  int16_t ts_x = p.x, ts_y = p.y;

  Serial.println("===== BEGIN =====");
  char buf[100];
  sprintf(buf, "Mapping point (%d, %d)...\n", ts_x, ts_y);
  Serial.print(buf);
  
  ts_y = constrain(ts_y, (*m_table)[0][0][1], (*m_table)[0][fineness][1] - 1);

  for (uint8_t tft_col = 0; tft_col < fineness; ++tft_col) {
    int16_t tft_x1 = (*m_table)[0][tft_col][1];
    int16_t tft_x2 = (*m_table)[0][tft_col + 1][1];

    Serial.print("Searching column: ");
    Serial.println(tft_col, DEC);
    Serial.print("x region: ");
    Serial.print(tft_x1, DEC);
    Serial.print(" - ");
    Serial.println(tft_x2, DEC);

    if (tft_x1 <= ts_y && ts_y < tft_x2) {
      Serial.print("Found column: ");
      Serial.println(tft_col, DEC);

      ts_x = constrain(ts_x, (*m_table)[0][tft_col][0], (*m_table)[fineness][tft_col][0] - 1);

      for (uint8_t tft_row = 0; tft_row < fineness; ++tft_row) {
        int16_t tft_y1 = (*m_table)[tft_row][tft_col][0];
        int16_t tft_y2 = (*m_table)[tft_row + 1][tft_col][0];

        Serial.print("Searching row: ");
        Serial.println(tft_row, DEC);
        Serial.print("x region: ");
        Serial.print(tft_y1, DEC);
        Serial.print(" - ");
        Serial.println(tft_y2, DEC);

        if (tft_y1 <= ts_x && ts_x < tft_y2) {
          Serial.print("Found row: ");
          Serial.println(tft_row, DEC);

          char buf[100];
          sprintf(
            buf,
            "POINT {\n"
            "  raw:  %3d, %3d;\n"
            "  grid: %3d, %3d;\n"
            "  rect {\n"
            "    x1: %d;\n"
            "    y1: %d;\n"
            "    x2: %d;\n"
            "    y2: %d;\n"
            "  }\n"
            "}\n",
            ts_x, ts_y,
            tft_col, tft_row,
            tft_x1, tft_y1, tft_x2, tft_y2
          );

          Serial.print(buf);

          ts_y = map(ts_y, tft_x1, tft_x2, tft_col * CELL_WIDTH,  (tft_col + 1) * CELL_WIDTH );
          ts_x = map(ts_x, tft_y1, tft_y2, tft_row * CELL_HEIGHT, (tft_row + 1) * CELL_HEIGHT);

          break;
        }
      }
  
      break;
    }
  }

  Serial.println("===== END =====\n\n");

  return TSPoint(ts_y, ts_x, p.z);
}

TouchscreenCtrl::TouchscreenCtrl(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t resist, TouchscreenCalibration &calib)
  : TouchScreen(xp, yp, xm, ym, resist), m_calib(calib) {
  // Empty
}

TSPoint TouchscreenCtrl::getPoint(bool raw) {
  TSPoint p = TouchScreen::getPoint();
  pinMode(TS_XM, OUTPUT);
  pinMode(TS_YP, OUTPUT);

  return (raw ? p : mapPoint(p));
}

bool TouchscreenCtrl::isValidPoint(TSPoint p, int16_t maxz) {
  return isValidPressure(p.z, maxz);
}

bool TouchscreenCtrl::isValidPressure(int16_t z, int16_t maxz) {
  return TS_MIN_PRESSURE < z && (maxz < 0 ? true : z < maxz);
}

TSPoint TouchscreenCtrl::mapPoint(TSPoint p) {
  return m_calib.adc_data_to_tft_coords(p, TFT_WIDTH, TFT_HEIGHT);
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
