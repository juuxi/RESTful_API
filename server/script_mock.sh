#!/bin/bash
g++ -c -I/usr/include/postgresql server.cpp -o server.o
g++ -o server server.o -L/usr/local/pgsql/lib -lpq
