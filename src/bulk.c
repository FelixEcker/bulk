// bulk.c ; part of the bulk pager
// Copyright (c) 2023, Marie Eckert
// Licensed under the BSD 3-Clause license

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <xmem.h>

#define FALSE 0
#define TRUE  1

// The base size of the input buffer to be allocated
#define BASE_BUFF_SIZE 512

// The amount of bytes to be read as a time. This is also the amount
// of bytes the buffer is enlarged by when it runs out of space
#define READ_CHUNK_SIZE 64

// TYPE DEFINITIONS

typedef struct line_t {
  char *start;
  size_t length;
} line_t;

typedef struct bulk_t {
  // Config
  int color_enabled;
  int style_enabled;
  int minimal_mode;

  // Terminal Setup
  unsigned char ncols;
  unsigned char nrows;

  // Buffer
  char *buff;
  size_t buff_size;
  size_t buff_allocd;
  line_t *lines;
  size_t line_count;
  size_t cline;
} bulk_t;

// SIGNAL HANDLERS

// PARSING FUNCTIONS
static void get_lines(bulk_t *bulk, size_t pos, size_t len) {
  size_t last_start = 0;
  for (size_t ix = 0; ix < len; ix++) {
    if (bulk->buff[pos + ix] != '\n')
      continue;

    bulk->lines[bulk->line_count].start = (char*) (last_start + pos);
    bulk->lines[bulk->line_count].length = ix - last_start + pos;
    bulk->line_count++;
    bulk->lines = xrealloc(bulk->lines, 
                           sizeof(line_t) * (bulk->line_count + 1));
    last_start = ix + 1;
  }
}

// DISPLAY FUNCTIONS

static void show(bulk_t bulk) {
}

int main(int argc, char **argv) {
  bulk_t bulk = {
    .color_enabled = TRUE,
    .style_enabled = TRUE,
    .minimal_mode  = FALSE,
    .buff = xmalloc(BASE_BUFF_SIZE),
    .buff_size = 0,
    .buff_allocd = BASE_BUFF_SIZE,
    .lines = xmalloc(sizeof(line_t)),
    .line_count = 0,
    .cline = 0,
  };

  struct winsize ws;
  ioctl(0, TIOCGWINSZ, &ws);
  bulk.ncols = ws.ws_col;
  bulk.nrows = ws.ws_row - 1;

  if (bulk.minimal_mode == FALSE)
    bulk.nrows -= 1;

  while (1) {
    size_t bufpos = bulk.buff_size;
    size_t bytes_read = read(STDIN_FILENO, bulk.buff+bulk.buff_size, 
                             READ_CHUNK_SIZE);
    bulk.buff_size += bytes_read;
    if (bulk.buff_size >= bulk.buff_allocd) {
      bulk.buff = xrealloc(bulk.buff, bulk.buff_allocd + READ_CHUNK_SIZE);
      bulk.buff_allocd += READ_CHUNK_SIZE;
    }

    if (bytes_read > 0)
      get_lines(&bulk, bufpos, bytes_read);
    printf("%ld\n", bulk.line_count);
    show(bulk);
  }

  free(bulk.buff);
  free(bulk.lines);
  return 0;
}
