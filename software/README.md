# eeprommer3 | software

\[TODO - update me with new simplified software]

This directory contains the software for eeprommer3's interface (GUI).
The software is written in C++ and uses wxWidgets as its GUI library.

The `build/` directory contains files and scripts necessary for
building/compiling the code of eeprommer3.

The `src/` directory contains the source code of eeprommer3.
Execution starts in `main.cpp`, which depends on the other files.

## Naming Scheme

| File Extension | Description                                                      |
| -------------- | ---------------------------------------------------------------- |
| `*.hpp`        | Contains declarations and prototypes                             |
| `*.cpp`        | Contains source code and definitions                             |
| `*.tpp`        | Contains template definitions; is `#include`d from `*.hpp` files |
