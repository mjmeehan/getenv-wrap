# getenv-wrap
Wraps getenv/setenv with rwlocks, to be used with LD_PRELOAD. You might want this if you have an application that doesn't respect that access to these functions aren't thread safe.

NOTES
Incompatible with jemalloc, and possibly other custom allocators. See https://github.com/jemalloc/jemalloc/issues/916. To guard against this calling getenv on MALLOC_OPTIONS will cause the application to exit immediately (rather than risk deadlock).

BUILDING
./build.sh

USING
LD_PRELOAD=./inject.so your_program_here
