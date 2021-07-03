#include <cstdint>
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
