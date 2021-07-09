#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libserialport.h>

#include "comm.hpp"
#include "dlgbox.hpp"

sp_port_config *default_config;

void set_default_port_config() {
  check_sp(sp_new_config(&default_config));
  check_sp(sp_set_config_baudrate   (default_config, 19200));
  check_sp(sp_set_config_bits       (default_config, 8));
  check_sp(sp_set_config_parity     (default_config, SP_PARITY_NONE));
  check_sp(sp_set_config_stopbits   (default_config, 1));
  check_sp(sp_set_config_flowcontrol(default_config, SP_FLOWCONTROL_NONE));
}

PortCtrl::PortCtrl(const char *name) {
  set_cur_port(name);
}

bool PortCtrl::is_initialized() {
  return initialized;
}

int16_t PortCtrl::list_ports(char **list) {
  sp_port **port_list;

  sp_return result = sp_list_ports(&port_list);

  if (result != SP_OK) {
    return -1;
  }

  uint8_t i = 0;

  while (port_list[i] != nullptr) {
    sp_port *port = port_list[i];

    strcpy(list[i++], sp_get_port_name(port));
  }

  list[i] = nullptr;

  sp_free_port_list(port_list);

  return i;
}

int8_t PortCtrl::set_cur_port(const char *name, sp_port_config *config) {
  bool was_initialized = initialized;
  initialized = false;

  if (was_initialized) {
    if (check_sp(sp_close(cur_port)) != SP_OK) {
      return 1;
    }

    DlgBox::info("qwertyuiopasdfghjklzxcvbnm", "Closed old port.", wxOK);
  }

  sp_get_port_by_name(name, &cur_port);

  DlgBox::info("qwertyuiopasdfghjklzxcvbnm", "Found new port.", wxOK);

  if (check_sp(sp_open(cur_port, SP_MODE_READ_WRITE)) != SP_OK) {
    return 2;
  }

  DlgBox::info("qwertyuiopasdfghjklzxcvbnm", "Opened new port.", wxOK);

  if (check_sp(sp_set_config(cur_port, config)) != SP_OK) {
    return 3;
  }

  DlgBox::info("qwertyuiopasdfghjklzxcvbnm", "Configurated new port.", wxOK);

  initialized = true;
  DlgBox::info("Done!", "Done!", wxOK);
  return 0;
}

const char *PortCtrl::get_cur_port() {
  if (initialized) {
    return sp_get_port_name(cur_port);
  }
  else {
    return nullptr;
  }
}

void PortCtrl::test_write(const char *data) {
  if (!initialized) return;

  uint32_t size = strlen(data);
  int16_t result = check_sp(sp_blocking_write(cur_port, data, size, 1000));

  if (result < 100) {
    if (result == size) {
      printf("Sent %d bytes successfully.\n", size);
    }
    else {
      printf("Timed out, sent %d/%d bytes.\n", result, size);
    }
  }
  else {
    printf("Operation failed!\n");
  }
}

int16_t check_sp(sp_return status) {
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
