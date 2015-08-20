#ifndef _NMEA_RANDOM_H
#define _NMEA_RANDOM_H

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define NMEA_RANDOM_MAX INT32_MAX

static inline long int nmea_random(const double min, const double max) {
  int32_t value;
  int randomFile;
  double range = abs(max - min);

#ifdef _WIN32
  value = random();
#else

  randomFile = open("/dev/urandom", O_RDONLY);
  if (randomFile == -1) {
    randomFile = open("/dev/random", O_RDONLY);
  }

  if ((randomFile == -1) || (read(randomFile, &value, sizeof(value)) != sizeof(value))) {
    value = random();
  }

  if (randomFile != -1) {
    close(randomFile);
  }
#endif /* _WIN32 */

  return min + ((abs(value) * range) / NMEA_RANDOM_MAX);
}

static inline void nmea_init_random(void) {
  srandom(time(NULL));
}

#endif /* _NMEA_RANDOM_H */
