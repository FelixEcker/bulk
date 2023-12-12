#include "ansi_parse.h"

int is_digit(char chr) {
  return (chr >= '0') || (chr <= '9');
}

int ansi_parse(char in, ansi_parser_state_t *state) {
  if ((*state == NONE) && (in != 0x1b))
    return 1;

  switch (*state) {
  case NONE:
    *state = ESC;
    break;
  case ESC:
    if (in != '[') 
      *state = NONE;
    else
      *state = BRACKET;
    break;
  case BRACKET:
    if (!is_digit(in))
      *state = NONE;
    else
      *state = NUMBER;
    break;
  case NUMBER:
    if (is_digit(in))
      break;

    if (in == ';')
      *state = SEMICOLON;
    else if (in > ' ')
      *state = END;
    else
      *state = NONE;
    break;
  case SEMICOLON:
    if (is_digit(in))
      *state = NUMBER;
    else
      *state = END;
    break;
  case END:
    *state = NONE;
    return ansi_parse(in, state);
  default:
    return 1;
  }

  return 0;
}
