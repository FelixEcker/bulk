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
