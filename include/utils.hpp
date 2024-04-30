#include "string.h"
#include "freertos/FreeRTOS.h"

char		*skip_space(char **ptr);
uint8_t		parse_int(char** ptr, uint16_t *value, char sep);
uint8_t		ft_isspace(char c);
uint8_t		ft_strtoi(const char *str, uint16_t *dest);
uint8_t		ft_isdigit(char c);
int32_t		parse_word(char* buffer, char** cursor, char** start);
