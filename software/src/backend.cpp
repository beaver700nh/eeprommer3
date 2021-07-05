#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libserialport.h>

#include "backend.hpp"

bool get_ports(char **list) {
  sp_port **port_list;

  sp_return result = sp_list_ports(&port_list);

  if (result != SP_OK) {
    return false;
  }

  uint8_t i = 0;

  while (port_list[i] != nullptr) {
    sp_port *port = port_list[i];

    strcpy(list[i++], sp_get_port_name(port));
  }

  list[i] = nullptr;

  sp_free_port_list(port_list);

  return true;
}

void test_write_port(const char *name, const char *data) {
  sp_port *port;
  sp_get_port_by_name(name, &port);

  check_sp(sp_open(port, SP_MODE_READ_WRITE));

  sp_port_config *config;

  check_sp(sp_new_config(&config));
  check_sp(sp_set_config_baudrate(config, 19200));
  check_sp(sp_set_config_bits(config, 8));
  check_sp(sp_set_config_parity(config, SP_PARITY_NONE));
  check_sp(sp_set_config_stopbits(config, 1));
  check_sp(sp_set_config_flowcontrol(config, SP_FLOWCONTROL_NONE));

  check_sp(sp_set_config(port, config));

  int size = strlen(data);
  int result = check_sp(sp_blocking_write(port, data, size, 1000));
 
  if (result == size) {
    printf("Sent %d bytes successfully.\n", size);
  }
  else {
    printf("Timed out, sent %d/%d bytes.\n", result, size);
  }

  check_sp(sp_close(port));

  sp_free_port(port);
  sp_free_config(config);
}

int check_sp(sp_return status) {
  char *error_message;

  switch (status) {
  case SP_ERR_ARG:
    printf("Serial Error: Invalid argument.\n");
    return 100;

  case SP_ERR_FAIL:
    error_message = sp_last_error_message();
    printf("Serial Error: Failed: %s\n", error_message);
    sp_free_error_message(error_message);
    return 101;

  case SP_ERR_SUPP:
    printf("Serial Error: Operation not supported.\n");
    return 102;

  case SP_ERR_MEM:
    printf("Serial Error: Couldn't allocate memory.\n");
    return 103;

  case SP_OK:
  default:
    return status;
  }
}
