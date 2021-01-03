# getenv-wrap

Wraps getenv/setenv with rwlocks, to be used with LD_PRELOAD. You might want
this if you have an application that doesn't respect that access to these
functions aren't thread safe.

## Notes

Incompatible with jemalloc, and possibly other custom allocators. See
https://github.com/jemalloc/jemalloc/issues/916. To guard against this calling
getenv on MALLOC_OPTIONS will cause the application to exit immediately (rather
than risk deadlock).

## Building

`./build.sh`

## Using

`LD_PRELOAD=./inject.so your_program_here`

## Testing

A test program (`test.c`) is included based on
[this blog post](https://rachelbythebay.com/w/2017/01/30/env/). You can try
running it with and without this wrapper to see the effect:

``` console
$ ./test
start
<snip>
23980000/40000000 iterations completed...
Segmentation fault
$ LD_PRELOAD=./inject.so ./test
start
<snip>
39990000/40000000 iterations completed...
success!
```

It takes quite a while to run the wrapped version. You can speed things up by
optimizing the library with the `-O9` option in `build.sh`.
