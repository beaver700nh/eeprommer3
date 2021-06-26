#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "main.hpp"
#include "args.hpp"

#define PRINT_SEP printf("====================\n")

Arg args[10];

int main(int argc, char **argv) {
  char *fname = (char *) malloc(sizeof(char) * (20 + 1));
  strcpy(fname, "/dev/ttyACM0\0\0\0\0\0\0\0\0");

  if (length_args(args) != 0 && args[0].type == ARG) {
    strcpy(fname, args[0].contents);
  }

  //

  return 0;
}
