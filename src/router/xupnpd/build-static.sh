#!/bin/bash


make clean; make STATIC=true bcm947x_ddwrt

make clean; make STATIC=true bcm947x_backfire

make clean; make STATIC=true ar71xx_backfire

make clean; make STATIC=true ar231x_backfire

make clean; make STATIC=true bcm947x_backfire

make clean; make STATIC=true bcm2708

make clean; make STATIC=true bcm47xx

make clean; make STATIC=true bcm63xx

make clean; make STATIC=true ar7xxx

make clean; make STATIC=true ar231x

make clean
