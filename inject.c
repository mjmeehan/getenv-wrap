/* Need this to get RTLD_NEXT, non-POSIX */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#define DEBUG 0

/* Env vars can be at max pagesize * 32 long. This is a hack
   Would be better to use sysconf(_SC_PAGESIZE) at runtime    */
#define MAXENVLEN 4096*32

static void init ( void ) __attribute__ ( ( constructor ( 101 ) ) );

char * ( *_original_getenv ) ( const char *name );
char * ( *_original_secure_getenv ) ( const char *name );
int ( *_original_setenv ) ( const char *name, const char *value, int overwrite );
int ( *_original_unsetenv ) ( const char *name );

static pthread_rwlock_t  rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* Attempt to initialize the functions we override at startup. Unfortunately, other
 * constructors can run first, so we're at the mercy of the linker. We need to check
 * that the function is defined in each of the calls as well.
 */
static void init ( void )
{
        char *error;

        _original_getenv = ( char * ( * ) ( const char * ) ) dlsym ( RTLD_NEXT, "getenv" );
        _original_secure_getenv = ( char * ( * ) ( const char * ) ) dlsym ( RTLD_NEXT, "secure_getenv" );
        _original_setenv = ( int ( * ) ( const char *, const char *, int ) ) dlsym ( RTLD_NEXT, "setenv" );
        _original_unsetenv = ( int ( * ) ( const char * ) ) dlsym ( RTLD_NEXT, "unsetenv" );

        if ( ( error = dlerror() ) != NULL )  {
                fprintf ( stderr, "%s\n", error );
                exit ( EXIT_FAILURE );
        }
}

char *getenv ( const char *name )
{
        static __thread char *buffer = NULL;
        if ( buffer == NULL )
                buffer = malloc ( MAXENVLEN );
        fprintf ( stderr, "getenv-wrap: Intercepting getenv( %s )\n", name );
        if ( strcmp ( name, "MALLOC_OPTIONS" ) == 0 ) {
                fprintf ( stderr, "getenv-wrap: Custom allocators are incompatible with getenv-wrap. Terminating early on getenv( MALLOC_OPTIONS )\n" );
                exit ( EXIT_FAILURE );
        }
        if ( _original_getenv == NULL )
                _original_getenv = ( char * ( * ) ( const char * ) ) dlsym ( RTLD_NEXT, "getenv" );
        assert ( _original_getenv != NULL );
        if ( DEBUG )  fprintf ( stderr, "Read trylock\n" );
        pthread_rwlock_rdlock ( &rwlock );
        if ( DEBUG )  fprintf ( stderr, "Read lock\n" );
        char *ret = ( *_original_getenv ) ( name );
        if ( ret == NULL ) {
                pthread_rwlock_unlock ( &rwlock );
                if ( DEBUG )  fprintf ( stderr, "Read unlock\n" );
                return NULL;
        } else {
                assert ( strlen ( ret ) < MAXENVLEN );
                strcpy ( buffer, ret );
                pthread_rwlock_unlock ( &rwlock );
                if ( DEBUG )  fprintf ( stderr, "Read unlock\n" );
                return buffer;
        }
}

char *secure_getenv ( const char *name )
{
        static __thread char* buffer = NULL;
        if ( buffer == NULL )
                buffer = malloc ( MAXENVLEN );
        fprintf ( stderr, "getenv-wrap: Intercepting secure_getenv( %s )\n", name );
        if ( _original_secure_getenv == NULL )
                _original_secure_getenv = ( char * ( * ) ( const char * ) ) dlsym ( RTLD_NEXT, "secure_getenv" );
        assert ( _original_secure_getenv != NULL );
        pthread_rwlock_rdlock ( &rwlock );
        if ( DEBUG )  fprintf ( stderr, "Read lock\n" );
        char *ret = ( *_original_secure_getenv ) ( name );
        if ( ret == NULL ) {
                pthread_rwlock_unlock ( &rwlock );
                if ( DEBUG )  fprintf ( stderr, "Read unlock\n" );
                return NULL;
        } else {
                assert ( strlen ( ret ) < MAXENVLEN );
                strcpy ( buffer, ret );
                pthread_rwlock_unlock ( &rwlock );

                if ( DEBUG )  fprintf ( stderr, "Read unlock\n" );
                return buffer;
        }
}

int setenv ( const char *name, const char *value, int overwrite )
{
        fprintf ( stderr, "getenv-wrap: Intercepting setenv( %s, ******, %d )\n", name, overwrite );
        if ( _original_setenv == NULL )
                _original_setenv = ( int ( * ) ( const char *, const char *, int ) ) dlsym ( RTLD_NEXT, "setenv" );
        assert ( _original_setenv != NULL );
        pthread_rwlock_wrlock ( &rwlock );
        if ( DEBUG )  fprintf ( stderr, "Write lock\n" );
        int ret = ( *_original_setenv ) ( name, value, overwrite );
        pthread_rwlock_unlock ( &rwlock );
        if ( DEBUG )  fprintf ( stderr, "Write unlock\n" );
        return ret;
}

int unsetenv ( const char *name )
{
        fprintf ( stderr, "getenv-wrap: Intercepting unsetenv( %s )\n", name );
        if ( _original_unsetenv == NULL )
                _original_unsetenv = ( int ( * ) ( const char * ) ) dlsym ( RTLD_NEXT, "unsetenv" );
        assert ( _original_unsetenv != NULL );
        pthread_rwlock_wrlock ( &rwlock );
        if ( DEBUG )  fprintf ( stderr, "Write lock\n" );
        int ret = ( *_original_unsetenv ) ( name );
        pthread_rwlock_unlock ( &rwlock );
        if ( DEBUG )  fprintf ( stderr, "Write unlock\n" );
        return ret;
}
