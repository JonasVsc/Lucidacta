#ifndef LUCID_UTILS_H_
#define LUCID_UTILS_H_

#include <Windows.h>
#include <stdint.h>

DWORD* LucidReadFile(const char* file_path, size_t* file_size);

static inline uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) {return val < min ? min : (val > max ? max : val);}


#endif // LUCID_UTILS_H_