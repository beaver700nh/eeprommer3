#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "test.hpp"
#include "args.hpp"

#define PRINT_SEP printf("====================\n")

Arg args[10];

int main(int argc, char **argv) {
  char *fname = (char *) malloc(sizeof(char) * (20 + 1));
  strcpy(fname, "/dev/ttyACM0\0\0\0\0\0\0\0\0");

  parse_args(argc, argv, args);
  print_args(args);
  PRINT_SEP;

  if (length_args(args) != 0 && args[0].type == ARG) {
    strcpy(fname, args[0].contents);
  }

  printf("File: %s\n", fname);

  return 0;
}
