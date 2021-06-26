#ifndef ARGS_HPP
#define ARGS_HPP

enum ArgType {SOPT, LOPT, ARG, END};

struct Arg {
  ArgType type;
  char contents[50];
};

void parse_args(int argc, char **argv, Arg *args);
void print_args(Arg *args);
void print_args(int argc, char **argv);
std::ptrdiff_t length_args(Arg *args);

char *str_argtype(ArgType t);

#endif
