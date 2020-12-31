# getenv-wrap
Wraps getenv/setenv with rwlocks, to be used with LD_PRELOAD

BUILDING
./build.sh

USING
LD_PRELOAD=./inject.so your_program_here
