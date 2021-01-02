#!/bin/bash

gcc -fPIC -pthread -Wall -Werror -shared -o inject.so inject.c -ldl -g
#gcc -fPIC -pthread -Wall -Werror -shared -o inject.so inject.c -ldl -g -fsanitize=address
