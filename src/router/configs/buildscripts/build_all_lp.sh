#!/bin/sh
./a_updateall.sh
./build_broadcom_K26.sh>/dev/null 2>&1
./build_a4.sh>/dev/null 2>&1
./build_a1.sh>/dev/null 2>&1
./build_a3.sh>/dev/null 2>&1
./build_a2.sh>/dev/null 2>&1
./build_openrisc.sh>/dev/null 2>&1
./build_wbd111.sh>/dev/null 2>&1
./build_wbd222.sh>/dev/null 2>&1
#./build_wbd111_maksat.sh
./build_magicbox.sh>/dev/null 2>&1
./build_rb600.sh>/dev/null 2>&1
#./build_openrb_mynetway.sh>/dev/null 2>&1
./build_tonze_ap120.sh>/dev/null 2>&1
./build_compex_wp54.sh>/dev/null 2>&1
./build_compex_np28g.sh>/dev/null 2>&1
./build_x86.sh>/dev/null 2>&1
#./build_x86_gw700.sh
#./build_mikrotik.sh>/dev/null 2>&1
./build_whrg300n.sh>/dev/null 2>&1
./build_whrg300n_openvpn.sh>/dev/null 2>&1
./build_dir600.sh>/dev/null 2>&1
./build_dir300b.sh>/dev/null 2>&1
./build_dir615d.sh>/dev/null 2>&1
./build_esr9752sc.sh>/dev/null 2>&1
./build_acxnr22.sh>/dev/null 2>&1
./build_ar670w.sh>/dev/null 2>&1
./build_ar690w.sh>/dev/null 2>&1
./build_esr6650.sh>/dev/null 2>&1
#./build_whrg300n_buffalo.sh
./build_ecb9750.sh>/dev/null 2>&1
./build_br6574n.sh>/dev/null 2>&1
./build_asus_rtn13u.sh>/dev/null 2>&1
./build_asus_rt10n_plus.sh>/dev/null 2>&1
./build_wr5422.sh>/dev/null 2>&1
chmod -R 777 /GruppenLW/releases