#include "utils.hpp"

static uint8_t	increment_nbr(uint16_t *dest, char c)
{
	uint8_t	add;

	add = c - '0';
	if (*dest > 9999 / 10)
		return (1);
	*dest = *dest * 10 + add;
	return (0);
}

/// @brief Check digit character [0-9]
/// @param c Character to check
/// @return Non-zero if true, zero otherwise
uint8_t	ft_isdigit(char c)
{
	if (c >= '0' && c <= '9')
		return (1);
	return (0);
}

/// @brief Convert the initial part of a string to an integer.
/// Format : \\[n spaces\\](+-)[n digit]
/// Check for overflow.
/// @param str String to convert
/// @param dest int receiving the conversion.
/// @return ```0``` if no error occured. ```1``` if overflow.
/// ```2``` if no digit was found at the beginning.
uint8_t	ft_strtoi(const char *str, uint16_t *dest)
{
	uint16_t	i;

	i = 0;
	*dest = 0;
	while (ft_isspace(str[i]))
		i++;
	if (!ft_isdigit(str[i]))
		return (2);
	while (ft_isdigit(str[i]))
		if (increment_nbr(dest, str[i++]))
			return (1);
	return (0);
}

uint8_t	ft_isspace(char c)
{
	if ((c >= '\b' && c <= '\r') || c == ' ')
		return (1);
	return (0);
}

uint8_t	parse_int(char** ptr, uint16_t *value, char sep)
{
	if (ft_strtoi(*ptr, value))
		return (1);
	while (**ptr && **ptr != sep)
		*ptr = *ptr + 1;
	return (0);
}

char	*skip_space(char **ptr)
{
	while (ft_isspace(**ptr))
		*ptr = *ptr + 1;
	return (*ptr);
}

/// @brief Parse next word, given ```buffer``` or ```cursor```. One of both should not be ```NULL```.
/// Handle parenthesis 
/// @param buffer 
/// @param cursor 
/// @param start Address of a pointer that will store word beginning
/// @return Length of the word (doesn't include parenthesis). ```-1``` if unclosed parenthesis.
int32_t	parse_word(char* buffer, char** cursor, char** start)
{
	bool	quote = false;
	char**	cur = cursor ? cursor : &buffer;
	char	*word_start, *word_end = NULL;

	skip_space(cur);
	word_start = *cur;
	if (**cur == '"')
	{
		quote = true;
		word_start = ++*cur;
	}
	while (**cur && ((quote == false && **cur != ' ') || (quote == true && **cur != '"')))
		word_end = ++*cur;
	if (!**cur && quote == true) //If unclosed quote
		return (-1);
	else if (**cur)
		++*cur; //Skip current space or parenthesis
	if (start)
		*start = word_start;
	return (word_end - word_start);
}
