#ifndef __config_h__
#define __config_h__

#include <Windows.h>
#include <io.h>

#define ssize_t SSIZE_T

#ifdef WAVE_FORMAT_PCM
#undef WAVE_FORMAT_PCM
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

static long long int llrint(double num) {
  long long int truncated = (long long int)num;
  double difference = num - (double)truncated;
  return truncated + (difference > 0.5 ? 1 : 0);
}

#define bzero(data, size) memset(data, 0, size)

#define dup _dup
#define strdup _strdup
#define snprintf _snprintf
#define isnan _isnan
#define open _open
#define close _close
#define read _read
#define write _write
#define lseek _lseek

#endif
