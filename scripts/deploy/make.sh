#!/bin/sh

cd /app
rm main test
cmake .
make
