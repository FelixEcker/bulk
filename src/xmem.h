// xmem.h ; wrapper functions for std. memory functions
// part of the bulk pager.
// Copyright (c) 2023, Marie Eckert
// Licensed under the BSD 3-Clause License
// -----------------------------------------------------------------------------
// If any of the wrapped functions fail, the calling program
// will be aborted.
// NOTE: These functions should be used only if appropriate. DO NOT use any
// functions from here in any library.

#ifndef XMEM_H
#define XMEM_H

#include <stdlib.h>

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size); 
void *xrealloc(void *ptr, size_t size);
void *xreallocarray(void *ptr, size_t nmemb, size_t size);

#endif
