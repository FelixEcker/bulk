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
#define BASE_BUFF_SIZE 4096

// The amount of bytes to be read as a time. This is also the amount
// of bytes the buffer is enlarged by when it runs out of space
#define READ_CHUNK_SIZE 2048

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

// INPUT FUNCTIONS

static int advance_page(bulk_t *bulk) {
  if (bulk->current_page + 1 == bulk->page_count) return FALSE;
  bulk->current_page++;
  return TRUE;
}

static int process_inputs(bulk_t *bulk) {
  char response;
  while (read(2, &response, 1)) {
    switch (response) {
    case 'n':
      return advance_page(bulk);
    }
  }
  return FALSE;
}

// DISPLAY FUNCTIONS

static void show(bulk_t bulk) {
  size_t page_start = bulk.pages[bulk.current_page];

  printf("\x1b[2J\x1b[0;0f");

  int page_ended = FALSE;
  unsigned char nrow = 0;
  size_t chr;
  for (chr = page_start; chr < bulk.buff_size; chr++) {
    if (bulk.buff[chr] == '\n') nrow++;
    printf("%.*s", 1, bulk.buff + chr);
    if (nrow >= bulk.nrows) {
      page_ended = TRUE;
      break;
    }
  }

  if (page_ended == TRUE && bulk.current_page + 1 >= bulk.page_count) {
    bulk.pages = xrealloc(bulk.pages, sizeof(size_t) * (bulk.page_count + 1));
    bulk.pages[bulk.page_count] = chr + 1;
    bulk.page_count++;
  }

  for (; nrow < bulk.nrows; nrow++) printf("\n");
  printf("PAGE %zu/%zu\n", bulk.current_page + 1, bulk.page_count);
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

    if (bytes_read <= 0 && process_inputs(&bulk) != TRUE)
      continue;
    show(bulk);
  }

  free(bulk.pages);
  free(bulk.buff);
  return 0;
}
