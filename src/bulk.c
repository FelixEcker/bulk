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
  size_t start;
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
  
  // Pages
  size_t *pages;
  size_t page_count;
  size_t current_page;
} bulk_t;

// SIGNAL HANDLERS

// DISPLAY FUNCTIONS

static void show(bulk_t bulk) {
  size_t page_start = bulk.pages[bulk.current_page];

  printf("\x1b[2J\x1b[0;0f");

  unsigned char nrow = 0;
  for (size_t chr = page_start; chr < bulk.buff_size; chr++) {
    if (bulk.buff[chr] == '\n') nrow++;
    printf("%.*s", 1, bulk.buff + chr);
    fflush(stdout);
    if (nrow >= bulk.nrows) break;
  }
  for (; nrow < bulk.nrows; nrow++) printf("\n");
  printf("%d %d\n", bulk.nrows, nrow);
  //printf("%s\n", page_start);
}

int main(int argc, char **argv) {
  bulk_t bulk = {
    .color_enabled = TRUE,
    .style_enabled = TRUE,
    .minimal_mode  = FALSE,
    .buff = xmalloc(BASE_BUFF_SIZE),
    .buff_size = 0,
    .buff_allocd = BASE_BUFF_SIZE,
    .pages = malloc(sizeof(size_t)),
    .page_count = 1,
    .current_page = 0
  };

  bulk.pages[0] = 0;

  struct winsize ws;
  ioctl(1, TIOCGWINSZ, &ws);
  bulk.ncols = ws.ws_col;
  bulk.nrows = ws.ws_row - 1;

  if (bulk.minimal_mode == FALSE)
    bulk.nrows -= 1;

  while (1) {
    size_t bytes_read = read(STDIN_FILENO, bulk.buff+bulk.buff_size, 
                             READ_CHUNK_SIZE);
    bulk.buff_size += bytes_read;
    if (bulk.buff_size >= bulk.buff_allocd) {
      bulk.buff = xrealloc(bulk.buff, bulk.buff_allocd + READ_CHUNK_SIZE);
      bulk.buff_allocd += READ_CHUNK_SIZE;
    }

    if (bytes_read <= 0) continue;
    show(bulk);
  }

  free(bulk.buff);
  return 0;
}
