// Compit C14 (RS485 9600 8N1): request frames sent on behalf of the iNext slot (0x70) to the
// NANO ONE panel (0x29). Frame = 30 bytes: [0]=recipient+0x80, [1]=command ('R'=0x52 read,
// 'W'=0x57 write), [2]=checksum, [3]=sender, [4]=0, [5..28]=6 slots {reg_hi, reg_lo, val_hi, val_lo},
// [29]=0x23. Value is encoded as value+2000 in two 7-bit bytes. Checksum = sum of all bytes
// except [2], & 0x7F.
#pragma once
#include <vector>
#include <cstdint>
#include <array>

inline std::vector<uint8_t> c14_frame(uint8_t cmd, const int *regs, const int *vals) {
  std::vector<uint8_t> f(30, 0);
  f[0] = 0x80 + 0x29;
  f[1] = cmd;
  f[3] = 0x70;
  f[4] = 0x00;
  for (int k = 0; k < 6; k++) {
    int o = 5 + k * 4;
    int r = regs[k];
    int raw = vals[k] + 2000;
    f[o] = r / 128;
    f[o + 1] = r % 128;
    f[o + 2] = raw / 128;
    f[o + 3] = raw % 128;
  }
  f[29] = 0x23;
  int sum = 0;
  for (int i = 0; i < 30; i++)
    if (i != 2) sum += f[i];
  f[2] = sum & 0x7F;
  return f;
}

// Read 6 consecutive registers starting at `first`
inline std::vector<uint8_t> c14_read(int first) {
  int regs[6], vals[6] = {0, 0, 0, 0, 0, 0};
  for (int k = 0; k < 6; k++) regs[k] = first + k;
  return c14_frame(0x52, regs, vals);
}

// Write ONE register: the same slot is repeated 6 times, so there are no "empty" slots
// (an empty slot 00 00 0F 50 would literally mean "register 0 = 0").
inline std::vector<uint8_t> c14_write1(int reg, int value) {
  int regs[6], vals[6];
  for (int k = 0; k < 6; k++) { regs[k] = reg; vals[k] = value; }
  return c14_frame(0x57, regs, vals);
}
