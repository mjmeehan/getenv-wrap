/* Need this to get RTLD_NEXT, non-POSIX */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>

pthread_rwlock_t  rwlock = PTHREAD_RWLOCK_INITIALIZER;

char *getenv(const char *name) {
    fprintf(stderr, "Intercepting getenv\n");
    char *(*original_getenv)(const char *name);
    original_getenv = dlsym(RTLD_NEXT, "getenv");
    pthread_rwlock_rdlock(&rwlock);
    char *ret = (*original_getenv)(name);
    pthread_rwlock_unlock(&rwlock);
    return ret;
}

char *secure_getenv(const char *name) {
    fprintf(stderr, "Intercepting secure_getenv\n");
    char *(*original_secure_getenv)(const char *name);
    original_secure_getenv = dlsym(RTLD_NEXT, "secure_getenv");
    pthread_rwlock_rdlock(&rwlock);
    char *ret = (*original_secure_getenv)(name);
    pthread_rwlock_unlock(&rwlock);
    return ret;
    
}

int setenv(const char *name, const char *value, int overwrite) {
    fprintf(stderr, "Intercepting setenv\n");
    int (*original_setenv)(const char *name, const char *value, int overwrite);
    original_setenv = dlsym(RTLD_NEXT, "setenv");
    pthread_rwlock_wrlock(&rwlock);
    int ret = (*original_setenv)(name, value, overwrite);
    pthread_rwlock_unlock(&rwlock);
    return ret;
}


int unsetenv(const char *name) {
    fprintf(stderr, "Intercepting unsetenv\n");
    int (*original_unsetenv)(const char *name);
    original_unsetenv = dlsym(RTLD_NEXT, "unsetenv");
    pthread_rwlock_wrlock(&rwlock);
    int ret = (*original_unsetenv)(name);
    pthread_rwlock_unlock(&rwlock);
    return ret;
}
