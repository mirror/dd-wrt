#!/bin/sh
./build_ls2_sputnik.sh
./build_ls5_sputnik.sh
./build_lc2_sputnik.sh
./build_lc5_sputnik.sh
./build_ns2_sputnik.sh
./build_ns5_sputnik.sh
./build_ps2_sputnik.sh
./build_ps5_sputnik.sh
./build_bs2_sputnik.sh
./build_bs2hp_sputnik.sh
./build_bs5_sputnik.sh
./build_pico2_sputnik.sh
./build_pico2hp_sputnik.sh
./build_pico5_sputnik.sh
./build_wbd500_sputnik.sh
./build_xscale_sputnik.sh
./build_x86_gw700.sh
./build_eoc2610_sputnik.sh
./build_eoc1650_sputnik.sh
./build_eap3660_sputnik.sh
./build_ecb3500_sputnik.sh
./build_mr3201a_sputnik.sh

DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/sputnik
mv -f ~/GruppenLW/releases/$DATE/*sputnik ~/GruppenLW/releases/CUSTOMER/$DATE/sputnik
