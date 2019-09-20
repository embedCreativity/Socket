#!/bin/bash

#make clean && make all
./testRepeatedClient 127.0.0.1 52000 > out
grep -nsH ERROR out
grep -nsH WARN out
