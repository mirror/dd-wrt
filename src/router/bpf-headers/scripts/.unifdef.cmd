savedcmd_scripts/unifdef := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/.unifdef.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include    -o scripts/unifdef scripts/unifdef.c -L/home/xfs/openwrt/lede/openwrt/staging_dir/host/lib 

source_scripts/unifdef := scripts/unifdef.c

deps_scripts/unifdef := \

scripts/unifdef: $(deps_scripts/unifdef)

$(deps_scripts/unifdef):
