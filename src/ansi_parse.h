#ifndef ANSI_PARSE_H
#define ANSI_PARSE_H

typedef enum ansi_parser_state_t {
  NONE,
  ESC,
  BRACKET,
  NUMBER,
  SEMICOLON,
  END,
} ansi_parser_state_t;

int ansi_parse(char in, ansi_parser_state_t *state);

#endif // ANSI_PARSE_H
