# ifndef __PATTERN_H__
# define __PATTERN_H__

# include <stdint.h>

# define LED_NUM 20
# define DIVIDE_NUM 144
# define PATTERN_NUM 4

extern const uint8_t pattern[PATTERN_NUM][DIVIDE_NUM][LED_NUM][3];

# endif
