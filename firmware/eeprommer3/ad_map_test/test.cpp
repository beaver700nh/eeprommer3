#include <cstdio>
#include <cstdint>

#include "../ad_map.hpp"

void print_pair(const char *id, AddrDataMapPair &pair) {
  printf("%s {\naddr: %04X,\ndata: %02X\n}\n", id, pair.addr, pair.data);
}

void print_map(const char *id, AddrDataMap &map) {
  printf("%s {\nlen: %d,\ndata: [\n", id, map.get_len());

  for (int i = 0; i < map.get_len(); ++i) {
    AddrDataMapPair temp;
    map.get_pair(i, &temp);

    char buf[10];
    sprintf(buf, "#%d", i);

    print_pair(buf, temp);
  }

  printf("]\n}\n");
}

int main() {
  AddrDataMapPair p1 = {0x5555, 0xAA};
  AddrDataMapPair p2 = {p1};
  AddrDataMapPair *q2 = &p2;

  p2.addr += 2;
  p2.data -= 2;

  print_pair("p1", p1);
  print_pair("p2", p2);

  {
    AddrDataMap map;
    print_map("m1", map);

    map.remove(0);
    print_map("m2", map);

    map.append(p1);
    print_map("m3", map);

    map.append(p2);
    print_map("m4", map);

    map.remove(0);
    print_map("m5", map);

    map.remove(0);
    print_map("m6", map);

    map.append(*q2);
    print_map("m7", map);

    map.append(p2);
    print_map("m8", map);

    map.append(p1);
    print_map("m9", map);

    map.remove(1);
    print_map("m10", map);

    map.remove(1);
    print_map("m11", map);

    map.extend(3);
    print_map("m12", map);

    map.set_pair(3, (AddrDataMapPair) {0x1234, 0xFF});
    print_map("m13", map);

    map.set_pair(1, (AddrDataMapPair) {0x4321, 0x55});
    print_map("m14", map);

    map.set_pair(2, (AddrDataMapPair) {0x5555, 0xAA});
    print_map("m15", map);

    {
      uint32_t a, b, c, d;
      map.get_24bit(0, &a);
      map.get_24bit(1, &b);
      map.get_24bit(2, &c);
      map.get_24bit(3, &d);

      printf("%04X %04X %04X %04X\n", a, b, c, d);
    }

    map.remove(0);
    print_map("m16", map);

    map.purge();
    print_map("m17", map);
  }

  print_pair("q2", *q2);
}
