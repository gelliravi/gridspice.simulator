#!/bin/bash

make;
echo "Finish Making";
make install DESTDIR=/mnt/yizheng;
echo "Finish Installing"
