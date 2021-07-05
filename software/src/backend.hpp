#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <cstdint>
#include <libserialport.h>

typedef enum sp_return status;

bool get_ports(char **list); /* Assumes #ports < 256 */

void test_write_port(const char *name, const char *data);

int check_sp(sp_return status);

#endif
