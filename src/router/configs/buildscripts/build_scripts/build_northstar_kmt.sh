#!/bin/sh
NOT='^CONFIG_SQUID\\|^CONFIG_WIKAR\\|^CONFIG_KROMO\\|^CONFIG_XIRIAN\\|^CONFIG_BRAINSLAYER\\|^CONFIG_NOCAT\\|^CONFIG_MC\\|^CONFIG_SNORT\\|^CONFIG_STRACE\\|'
ADD='CONFIG_TMK=y\\nCONFIG_NOTRIAL=y\\nCONFIG_REGISTER=y\\n'
cat build_northstar.sh | \
	sed 's/\$DATE/CUSTOMER\/\$DATE\/kmt/' \
	| sed "s/^cp .config_northstar .config/(cat .config_northstar | grep -v \"${NOT}\" ; printf \"$ADD\" )>.config/"  \
	| sed "s/^cp .config_northstar_mini .config/(cat .config_northstar_mini | grep -v \"${NOT}\" ; printf \"$ADD\" )>.config/" \
	| sed "s/^cp .config_northstar_16m .config/(cat .config_northstar_16m | grep -v \"${NOT}\" ; printf \"$ADD\" )>.config/" \
	> ./build_northstar_kmt-temp.sh
chmod 755 ./build_northstar_kmt-temp.sh
#./build_northstar_kmt-temp.sh
./build_northstar_kmt-temp-1.sh
