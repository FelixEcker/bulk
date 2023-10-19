// bulk.c ; part of the bulk pager
// Copyright (c) 2023, Marie Eckert
// Licensed under the BSD 3-Clause license

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <xmem.h>

#define FALSE 0
#define TRUE 1

// The base size of the input buffer to be allocated
#define BASE_BUFF_SIZE 4096

// The amount of bytes to be read as a time. This is also the amount
// of bytes the buffer is enlarged by when it runs out of space
#define READ_CHUNK_SIZE 2048

// Global Variable to stop bulk
int gv_stop = FALSE;

// TYPE DEFINITIONS

typedef struct bulk_t {
  int quit;

  // Config
  int color_enabled;
  int style_enabled;
  int minimal_mode;

  // Terminal Setup
  struct termios orig_term_attr;
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

void signal_handler(int signum) {
  switch (signum) {
  case SIGINT:
    gv_stop = TRUE;
    break;
  }
}

// INPUT FUNCTIONS

static int advance_page(bulk_t *bulk) {
  if (bulk->current_page + 1 == bulk->page_count)
    return FALSE;
  bulk->current_page++;
  return TRUE;
}

static int regress_page(bulk_t *bulk) {
  if (bulk->current_page == 0)
    return FALSE;
  bulk->current_page--;
  return TRUE;
}

static int process_inputs(bulk_t *bulk) {
  int ret = FALSE;

  static unsigned char response[1];
  if (read(2, &response, 1) <= 0)
    return FALSE;

  switch (response[0]) {
  case 'n':
    ret = advance_page(bulk);
    break;
  case 'b':
    ret = regress_page(bulk);
    break;
  case 'q':
    bulk->quit = TRUE;
    break;
  }

  return ret;
}

// DISPLAY FUNCTIONS

static void show(bulk_t *bulk) {
  size_t page_start = bulk->pages[bulk->current_page];

  printf("\x1b[2J\x1b[0;0f");

  int page_ended = FALSE;
  unsigned char nrow = 0;
  size_t chr;
  for (chr = page_start; chr < bulk->buff_size; chr++) {
    if (bulk->buff[chr] == '\n')
      nrow++;
    printf("%.*s", 1, bulk->buff + chr);
    if (nrow >= bulk->nrows) {
      page_ended = TRUE;
      break;
    }
  }

  if (page_ended == TRUE && bulk->current_page + 1 >= bulk->page_count) {
    bulk->pages =
        xrealloc(bulk->pages, sizeof(size_t) * (bulk->page_count + 1));
    bulk->pages[bulk->page_count] = chr + 1;
    bulk->page_count++;
  }

  for (; nrow < bulk->nrows; nrow++)
    printf("\n");
  printf("PAGE %zu/%zu\n", bulk->current_page + 1, bulk->page_count);
}

static void setup(bulk_t *bulk) {
  struct termios new_term_attr;
  tcgetattr(2, &bulk->orig_term_attr);
  memcpy(&new_term_attr, &bulk->orig_term_attr, sizeof(struct termios));
  new_term_attr.c_lflag &= ~(ECHO | ICANON);
  new_term_attr.c_cc[VTIME] = 0;
  new_term_attr.c_cc[VMIN] = 0;
  tcsetattr(2, TCSANOW, &new_term_attr);

  signal(SIGINT, signal_handler);
}

static void teardown(bulk_t bulk) {
  tcsetattr(2, TCSANOW, &bulk.orig_term_attr);
}

int main(int argc, char **argv) {
  bulk_t bulk = {
      .quit = FALSE,
      .color_enabled = TRUE,
      .style_enabled = TRUE,
      .minimal_mode = FALSE,
      .buff = xmalloc(BASE_BUFF_SIZE),
      .buff_size = 0,
      .buff_allocd = BASE_BUFF_SIZE,
      .pages = xmalloc(sizeof(size_t)),
      .page_count = 1,
      .current_page = 0,
  };
  setup(&bulk);

  bulk.pages[0] = 0;

  struct winsize ws;
  ioctl(1, TIOCGWINSZ, &ws);
  bulk.ncols = ws.ws_col;
  bulk.nrows = ws.ws_row - 1;

  if (bulk.minimal_mode == FALSE)
    bulk.nrows -= 1;

  while (bulk.quit == FALSE && gv_stop == FALSE) {
    size_t bytes_read =
        read(STDIN_FILENO, bulk.buff + bulk.buff_size, READ_CHUNK_SIZE);
    bulk.buff_size += bytes_read;
    if (bulk.buff_size >= bulk.buff_allocd) {
      bulk.buff = xrealloc(bulk.buff, bulk.buff_allocd + READ_CHUNK_SIZE);
      bulk.buff_allocd += READ_CHUNK_SIZE;
    }

    if (process_inputs(&bulk) != TRUE && bytes_read <= 0)
      continue;

    show(&bulk);
  }

  teardown(bulk);
  free(bulk.pages);
  free(bulk.buff);
  return 0;
}
