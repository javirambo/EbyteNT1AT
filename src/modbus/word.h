#pragma once

inline uint16_t lowWord(uint32_t ww) { return (uint16_t)((ww) & 0xFFFF); }
inline uint16_t highWord(uint32_t ww) { return (uint16_t)((ww) >> 16); }

inline uint32_t makeDWord(uint32_t w) { return w; }
inline uint32_t makeDWord(uint16_t h, uint16_t l) { return (h << 16) | l; }

#define dword(...) makeDWord(__VA_ARGS__)
