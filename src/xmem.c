// xmem.c ; wrapper functions for std. memory functions
// part of the bulk pager.
// Copyright (c) 2023, Marie Eckert
// Licensed under the BSD 3-Clause License

#include <xmem.h>

void *xmalloc(size_t size) {
  void *ret = malloc(size);
  if (ret == NULL) abort();

  return ret;
}

void *xcalloc(size_t nmemb, size_t size) {
  void *ret = calloc(nmemb, size);
  if (ret == NULL) abort();

  return ret;
}
 
void *xrealloc(void *ptr, size_t size) {
  void *ret = realloc(ptr, size);
  if (ret == NULL) abort();

  return ret;
}

void *xreallocarray(void *ptr, size_t nmemb, size_t size) {
  void *ret = reallocarray(ptr, nmemb, size);
  if (ret == NULL) abort();

  return ret;

}
