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
  void *ret = xrealloc(ptr, size);
  if (ret == NULL) abort();

  return ret;
}

void *xreallocarray(void *ptr, size_t nmemb, size_t size) {
  void *ret = xreallocarray(ptr, nmemb, size);
  if (ret == NULL) abort();

  return ret;

}
