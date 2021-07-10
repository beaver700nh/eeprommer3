#ifndef COMM_HPP
#define COMM_HPP

#include <cstdint>
#include <libserialport.h>

typedef enum sp_return sp_return;

int16_t check_sp(sp_return status);
void set_default_port_config();

// Defined and configures in comm.cpp
extern sp_port_config *default_config;

class PortCtrl {
public:
  PortCtrl() {};
  PortCtrl(const char *name);

  bool is_initialized();

  int16_t list_ports(char **list); /* Assumes #ports < 256 */

  int8_t set_cur_port(const char *name, sp_port_config *config = default_config);
  const char *get_cur_port();

  int16_t test_write(const char *data);

private:
  bool initialized = false;

  sp_port *cur_port;
};

#endif
