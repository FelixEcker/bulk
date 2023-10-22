// bulk.c ; part of the bulk pager
// Copyright (c) 2023, Marie Eckert
// Licensed under the BSD 3-Clause license

// TODO: Recognize all escaped chars and exclude them from the column counts
// TODO: Prettier
// TODO: Minimal Mode
// TODO: ARG PARSING

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <argp.h>

#include <xmem.h>

#define VERSION_STRING "bulk 0.0.0"

#define FALSE 0
#define TRUE 1

// The base size of the input buffer to be allocated
#define BASE_BUFF_SIZE 4096

// The amount of bytes to be read as a time. This is also the amount
// of bytes the buffer is enlarged by when it runs out of space
#define READ_CHUNK_SIZE 2048

#define READ_REALLOC_SIZE READ_CHUNK_SIZE

#define STATUS_LINE_BG 44
#define STATUS_LINE_FG 33

#define DEBUG_STATS

// Global Variable to stop bulk
int gv_stop = FALSE;

// TYPE DEFINITIONS

typedef struct bulk_t {
  int quit;

  // Config
  int linewrapping;
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

  unsigned char action;
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

  bulk->action = response[0];
  switch (response[0]) {
  case 'n':
    ret = advance_page(bulk);
    break;
  case 'b':
    ret = regress_page(bulk);
    break;
  case 'q':
    ret = TRUE;
    bulk->quit = TRUE;
    break;
  }

  return ret;
}

// DISPLAY FUNCTIONS

static void show(bulk_t *bulk) {
  size_t page_start = bulk->pages[bulk->current_page];

  printf("\x1b[2J\x1b[0;0f");

  int cols_overflowed = FALSE;
  int page_ended = FALSE;
  unsigned char nrow = 0;
  unsigned char ncol = 0;
  size_t chr;
  for (chr = page_start; chr < bulk->buff_size; chr++) {
    if (bulk->buff[chr] == '\n') {
      nrow++;
      ncol = 0;
      cols_overflowed = FALSE;
    } else {
      ncol++;
    }

    if (ncol > bulk->ncols || cols_overflowed == TRUE) {
      if (bulk->linewrapping == FALSE) {
        if (cols_overflowed == FALSE)
          printf("\x1b[1D\x1b[1C$");

        cols_overflowed = TRUE;
        continue;
      }
      printf("\n");
      cols_overflowed = FALSE;
      ncol = 0;
    }

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

  char statusline[bulk->ncols];

  if (bulk->minimal_mode == TRUE)
    goto show_action_line;

  memset(statusline, 0x20, bulk->ncols);
  sprintf(statusline, " PAGE %zu/%zu", bulk->current_page + 1,
          bulk->page_count);
  statusline[strlen(statusline)] = ' ';
  statusline[bulk->ncols - 1] = 0;
  printf("\x1b[1m\x1b[%d;%dm%s\x1b[0m\n", STATUS_LINE_FG, STATUS_LINE_BG,
         statusline);

show_action_line:
  fprintf(stdout, ":%c", bulk->action);
  fflush(stdout);
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

// STARTUP FUNCTIONS

const char *argp_program_version = VERSION_STRING;
const char *argp_program_bug_address =
  "https://github.com/FelixEcker/bulk/issues";
const char description[] =
  "A simple pager; less than less but more like most\n"
  "Author: Marie Eckert";
const char args_doc[] = "";

static struct argp_option options[] = {
  {"no-color", 'c', 0, 0, "Check if a build file is valid"}
, {"no-style", 's', 0, 0, "Check if a build file is valid"}
, {"line-wrapping", 'w', 0, 0, "Check if a build file is valid"}
, {"minimal", '\0', 0, 0, "Check if a build file is valid"}
, {0, 0, 0, 0}
};

struct arguments {
  char *build_file;
  char *exec_script;
  int check_file;
  char *mode;
  char *platform;
  int disable_extensions;
  int  log_level;
  int print_structure;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *args = state->input;
  switch (key) {
  case 'i': args->build_file = arg; break;
  case 'e': args->exec_script = arg; break;
  case 'c': args->check_file = TRUE; break;
  case 'm': args->mode = arg; break;
  case 'p': args->platform = arg; break;
  case 'd': args->disable_extensions = TRUE; break;
//  case 'q': args->log_level = MB_LOGLVL_IMP; break;
//  case 'v': args->log_level = MB_LOGLVL_LOW; break;
  case 's': args->print_structure = TRUE; break;
  default: return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, description };

int main(int argc, char **argv) {
  struct arguments args;
  argp_parse(&argp, argc, argv, 0, 0, &args);
  bulk_t bulk = {
      .quit = FALSE,
      .linewrapping = TRUE,
      .color_enabled = TRUE,
      .style_enabled = TRUE,
      .minimal_mode = FALSE,
      .buff = xmalloc(BASE_BUFF_SIZE),
      .buff_size = 0,
      .buff_allocd = BASE_BUFF_SIZE,
      .pages = xmalloc(sizeof(size_t)),
      .page_count = 1,
      .current_page = 0,
      .action = '\0',
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
    if (bulk.buff_size + READ_CHUNK_SIZE >= bulk.buff_allocd) {
      bulk.buff = xrealloc(bulk.buff, bulk.buff_allocd + READ_REALLOC_SIZE);
      bulk.buff_allocd += READ_REALLOC_SIZE;
    }

    if (process_inputs(&bulk) != TRUE && bytes_read <= 0)
      continue;

    show(&bulk);
  }

  printf("\n");
#ifdef DEBUG_STATS
  printf("total buffer-bytes allocated: %zu\n", bulk.buff_allocd);
  printf("total bytes read: %zu\n", bulk.buff_size);
#endif

  teardown(bulk);
  free(bulk.pages);
  free(bulk.buff);
  return 0;
}
