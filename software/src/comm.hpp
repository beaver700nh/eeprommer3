#ifndef COMM_HPP
#define COMM_HPP

#include <cstdint>
#include <libserialport.h>

int16_t check_sp(enum sp_return status);

class PortConfig {
public:
  PortConfig() {};
  PortConfig(
    uint32_t baud, uint8_t bits, enum sp_parity parity, uint8_t stopbits, enum sp_flowcontrol fctrl
  );

  struct sp_port_config *config;
};

class PortCtrl {
public:
  PortCtrl() {};
  PortCtrl(const char *name, PortConfig &config);

  bool is_initialized();

  int16_t list_ports(char **list); /* Assumes #ports < 256 */

  int8_t set_cur_port(const char *name, PortConfig &config);
  const char *get_cur_port();

  int16_t close_cur_port();

  int16_t test_write(const char *data);
  int16_t test_read(uint16_t count, char *out);
  int16_t test_read(const char *delim, char *out);

private:
  bool initialized = false;

  struct sp_port *cur_port;
};

#endif
