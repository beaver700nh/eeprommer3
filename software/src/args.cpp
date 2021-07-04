#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <tuple>

#include "args.hpp"

void parse_args(int argc, char **argv, Arg *args) {
  if (argc < 1) {
    args[0].type = END;
    return;
  }

  int index = 0;
  int argn = 1;

  while (argn < argc) {
    int offset = 0;

    if (argv[argn][0] == '-') {
      if (argv[argn][1] == '-') {
        args[index].type = LOPT;
        offset = 2;
      }
      else {
        args[index].type = SOPT;
        offset = 1;
      }
    }
    else {
      args[index].type = ARG;
      offset = 0;
    }

    strcpy(args[index].contents, argv[argn] + offset);

    ++argn, ++index;
  }

  args[index].type = END;
}

void print_args(Arg *args) {
  while (true) {
    if (args->type == END) {
      printf("Arg: type = 'END'\n");
      break;
    }

    char *type = str_argtype(args->type);
    printf("Arg: type = '%s', contents = '%s'\n", type, args->contents);
    free(type);

    ++args;
  }
}

void print_args(int argc, char **argv) {
  for (int i = 0; i < argc; ++i) {
    printf("Arg: contents = '%s'\n", argv[i]);
  }
}

char *str_argtype(ArgType t) {
  char *buf = (char *) malloc(sizeof(char) * (10 + 1));

  if (buf == nullptr) {
    printf("FATAL ERROR: could not allocate memory.\n\n");
    exit(1);
  }

  if      (t == SOPT) strcpy(buf, "SOPT");
  else if (t == LOPT) strcpy(buf, "LOPT");
  else if (t == ARG)  strcpy(buf, "ARG");
  else if (t == END)  strcpy(buf, "END");
  else {
    printf("\nstr_argtype(t): error: invalid ArgType t.\n\n");
    exit(1);
  }

  return buf;
}

std::ptrdiff_t length_args(Arg *args) {
  Arg *ptr = args;

  while (ptr++->type != END); /* move ptr to end */

  return ptr - args - 1;
}
