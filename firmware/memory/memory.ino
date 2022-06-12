#include <avr/io.h>

extern unsigned int __heap_start, __heap_end;
extern unsigned int __bss_start, __bss_end;
extern unsigned int __data_start, __data_end;
extern char *__brkval;
extern char *__malloc_heap_start;
extern char *__malloc_heap_end;

void print4hex(unsigned int adr){
  Serial.print("0x");
  if (adr < 0x1000) { Serial.print("0"); }
  if (adr < 0x0100) { Serial.print("0"); }
  if (adr < 0x0010) { Serial.print("0"); }
  Serial.print(adr, HEX);
};

void printborders() {
  print4hex((unsigned int)&__data_start);
  Serial.println(" __data_start");
  print4hex((unsigned int)&__data_end);
  Serial.println(" __data_end");
  print4hex((unsigned int)&__bss_start);
  Serial.println(" __bss_start");
  print4hex((unsigned int)&__bss_end);
  Serial.println(" __bss_end");
  print4hex((unsigned int)&__heap_start);
  Serial.println(" __heap_start");
  print4hex((unsigned int)&__heap_end);
  Serial.println(" __heap_end");
  print4hex((unsigned int)&__brkval);
  Serial.println(" __brkval");
  print4hex((unsigned int)__malloc_heap_start);
  Serial.println(" __malloc_heap_start");
  print4hex((unsigned int)__malloc_heap_end);
  Serial.println(" __malloc_heap_end");
  print4hex((unsigned int)SP);
  Serial.println(" SP");
  print4hex((unsigned int)RAMSTART);
  Serial.println(" RAMSTART");
  print4hex((unsigned int)RAMEND);
  Serial.println(" RAMEND");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Memory usage:");
  printborders();
}

void loop() {
  // Do nothing.
}
