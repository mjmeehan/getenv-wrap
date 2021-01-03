/* Source: https://rachelbythebay.com/w/2017/01/30/env/ */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static void* worker(void* arg) {
  for (;;) {
    int i;
    char var[256], *p = var;
               
    for (i = 0; i < 8; ++i) {
      *p++ = 65 + (random() % 26);
    }
                   
    *p++ = '\0';
                       
    setenv(var, "test", 1);
  }
     
  return NULL;
}
 
int main() {
  unsigned long iterations = 4E7;
  pthread_t t;
     
  printf("start\n");
  setenv("foo", "bar", 0);
  pthread_create(&t, NULL, worker, 0);
           
  for (int i = 0; i < iterations; i++) {
    getenv("foo");
    if ((i % 10000) == 0) {
      printf("%d/%ld iterations completed...\n", i, iterations);
    }
  }

  printf("success!\n");
             
  return 0;
}
