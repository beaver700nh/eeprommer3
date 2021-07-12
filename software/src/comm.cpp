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
  check_sp(sp_set_config_baudrate   (default_config, 115200)); // 115200 baud
  check_sp(sp_set_config_bits       (default_config, 8));     // 8N1
  check_sp(sp_set_config_parity     (default_config, SP_PARITY_NONE));
  check_sp(sp_set_config_stopbits   (default_config, 1));     // No flow control
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

  // Get the ports
  sp_return result = sp_list_ports(&port_list);

  if (result != SP_OK) {
    return -1;
  }

  uint8_t i = 0;

  while (port_list[i] != nullptr) {
    sp_port *port = port_list[i];

    // Record the port name
    strcpy(list[i++], sp_get_port_name(port));
  }

  // End the list
  list[i] = nullptr;

  sp_free_port_list(port_list);

  return i;
}

int8_t PortCtrl::set_cur_port(const char *name, sp_port_config *config) {
  bool was_initialized = initialized;
  initialized = false;

  if (was_initialized) {
    // Do nothing if attempting to open already open port
    if (strcmp(sp_get_port_name(cur_port), name) == 0) {
      return 0;
    }

    // Close old port only if there was one
    if (check_sp(sp_close(cur_port)) != SP_OK) {
      return 1;
    }
  }

  // Get the new port
  sp_get_port_by_name(name, &cur_port);

  // Open the new port
  if (check_sp(sp_open(cur_port, SP_MODE_READ_WRITE)) != SP_OK) {
    return 2;
  }

  // Configure the new port
  if (check_sp(sp_set_config(cur_port, config)) != SP_OK) {
    return 3;
  }

  initialized = true;
  return 0;
}

const char *PortCtrl::get_cur_port() {
  if (initialized) {
    // Return port name if there is a port open
    return sp_get_port_name(cur_port);
  }
  else {
    return nullptr;
  }
}

int16_t PortCtrl::close_cur_port() {
  if (initialized) {
    return check_sp(sp_close(cur_port));
  }
  else {
    return 0;
  }
}

int16_t PortCtrl::test_write(const char *data) {
  if (!initialized) {
    DlgBox::error("Port is not open, can't communicate.", "Port Not Open", wxOK);
    return 0;
  }

  // Write the data
  uint32_t size = strlen(data);
  int16_t result = check_sp(sp_blocking_write(cur_port, data, size, 1000));

  if (result < 100) { // >= 100 is an error code
    if (result == size) {
      DlgBox::info(
        wxString::Format("Sent %d bytes successfully.", size), "Success", wxOK
      );

      return 0;
    }
    else {
      DlgBox::info(
        wxString::Format("Timed out, sent %d/%d bytes.", result, size), "Timed Out", wxOK
      );

      return 1;
    }
  }
  else {
    DlgBox::error("Operation failed!", "Error", wxOK);

    return result;
  }
}

int16_t PortCtrl::test_read(uint16_t count, char *out) {
  if (!initialized) {
    DlgBox::error("Port is not open, can't communicate.", "Port Not Open", wxOK);
    return 0;
  }

  // Read the data
  int16_t result = check_sp(sp_blocking_read(cur_port, out, count, 1000));

  if (result < 100) { // >= 100 is an error code
    if (result == count) {
      DlgBox::info(
        wxString::Format("Recieved %d bytes successfully.", count), "Success", wxOK
      );

      return 0;
    }
    else {
      DlgBox::info(
        wxString::Format("Timed out, recieved %d/%d bytes.", result, count), "Timed Out", wxOK
      );

      out[result] = '\0'; // Terminate string just in case

      return 1;
    }
  }
  else {
    DlgBox::error("Operation failed!", "Error", wxOK);

    return result;
  }
}

int16_t PortCtrl::test_read(const char *delim, char *out) {
  if (!initialized) {
    DlgBox::error("Port is not open, can't communicate.", "Port Not Open", wxOK);
    return 0;
  }

  char ch;        // Byte to recieve into
  uint16_t i = 0; // Index in input
  uint8_t j = 0;  // Index in delim

  while (true) {
    int16_t result = check_sp(sp_blocking_read(cur_port, &ch, 1, 1000));

    if (result < 100) { // >= 100 is an error code
      if (result == 1) {
        out[i++] = ch;

        // Check if we found delim
        if (ch == delim[j]) {
          if (delim[++j] == '\0') {
            // Success - reached end of delim
            out[i] = '\0';
            return 0;
          }
        }
        else {
          // Didn't find entire delim - reset delim index
          j = 0;
        }
      }
      else {
        DlgBox::info("Timed out, failed to recieve a byte.", "Timed Out", wxOK);
        out[i] = '\0';

        return 1;
      }
    }
    else {
      DlgBox::error("Operation failed!", "Error", wxOK);
      out[i] = '\0';

      return result;
    }
  }

  return 0;
}

int16_t check_sp(sp_return status) {
  char *error_message;

  switch (status) {
  case SP_ERR_ARG:
    DlgBox::error("Invalid argument", "Serial Error", wxOK);
    return 100;

  case SP_ERR_FAIL:
    error_message = sp_last_error_message();
    DlgBox::error(wxString::Format("Failed: %s", error_message), "Serial Error", wxOK);
    sp_free_error_message(error_message);
    return 101;

  case SP_ERR_SUPP:
    DlgBox::error("Operation not supported", "Serial Error", wxOK);
    return 102;

  case SP_ERR_MEM:
    DlgBox::error("Couldn't allocate Memory", "Serial Error", wxOK);
    return 103;

  case SP_OK:
  default: // Data transfers return number of bytes sent as a status
    return status;
  }
}
