/* Need this to get RTLD_NEXT, non-POSIX */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

/* Env vars can be at max pagesize * 32 long. This is a hack */
#define MAXENVLEN 4096*32

static void init(void) __attribute__((constructor));

char *(*_original_getenv)(const char *name);
char *(*_original_secure_getenv)(const char *name);
int (*_original_setenv)(const char *name, const char *value, int overwrite);
int (*_original_unsetenv)(const char *name);

static pthread_rwlock_t  rwlock = PTHREAD_RWLOCK_INITIALIZER;

static void init(void) {
    char *error;
    
    _original_getenv = (char *(*)(const char *)) dlsym(RTLD_NEXT, "getenv");
    _original_secure_getenv = (char *(*)(const char *)) dlsym(RTLD_NEXT, "secure_getenv");
    _original_setenv = (int (*)(const char *, const char *, int)) dlsym(RTLD_NEXT, "setenv");
    _original_unsetenv = (int (*)(const char *)) dlsym(RTLD_NEXT, "unsetenv");
    
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
}

char *strcpyxtnd(char *dest, const char *src) {
    int len = strlen(src);
    if(len > strlen(dest)) {
        free(dest);
        dest = malloc(sizeof(char) * (len + 1));
    }
    return strcpy(dest, src);
}

char *getenv(const char *name) {
    static __thread char buffer[MAXENVLEN];
    fprintf(stderr, "Intercepting getenv\n");
    if(!_original_getenv)
	    _original_getenv = (char *(*)(const char *)) dlsym(RTLD_NEXT, "getenv");
    assert(_original_getenv != NULL);
    pthread_rwlock_rdlock(&rwlock);
    char *ret = (*_original_getenv)(name);
    if(ret == NULL) {
        pthread_rwlock_unlock(&rwlock);
        return NULL;
    } else {
        assert(strlen(ret) + 1 < MAXENVLEN);
        strcpy(buffer, ret);
        pthread_rwlock_unlock(&rwlock);
        return buffer;
    }
}

char *secure_getenv(const char *name) {
    static __thread char buffer[MAXENVLEN];
    fprintf(stderr, "Intercepting secure_getenv\n");
    if(!_original_secure_getenv)
	   _original_secure_getenv = (char *(*)(const char *)) dlsym(RTLD_NEXT, "secure_getenv");
    assert(_original_secure_getenv != NULL);
    pthread_rwlock_rdlock(&rwlock);
    char *ret = (*_original_secure_getenv)(name);
    if(ret == NULL) {
        pthread_rwlock_unlock(&rwlock);
        return NULL;
    } else {
        assert(strlen(ret) + 1 < MAXENVLEN);
        strcpy(buffer, ret);
        pthread_rwlock_unlock(&rwlock);
        return buffer;
    }
}

int setenv(const char *name, const char *value, int overwrite) {
    fprintf(stderr, "Intercepting setenv\n");
    if(!_original_setenv)
	    _original_setenv = (int (*)(const char *, const char *, int)) dlsym(RTLD_NEXT, "setenv");
    assert(_original_setenv != NULL);
    pthread_rwlock_wrlock(&rwlock);
    int ret = (*_original_setenv)(name, value, overwrite);
    pthread_rwlock_unlock(&rwlock);
    return ret;
}


int unsetenv(const char *name) {
    fprintf(stderr, "Intercepting unsetenv\n");
    if(!_original_unsetenv)
	    _original_unsetenv = (int (*)(const char *)) dlsym(RTLD_NEXT, "unsetenv");
    assert(_original_unsetenv != NULL);
    pthread_rwlock_wrlock(&rwlock);
    int ret = (*_original_unsetenv)(name);
    pthread_rwlock_unlock(&rwlock);
    return ret;
}
