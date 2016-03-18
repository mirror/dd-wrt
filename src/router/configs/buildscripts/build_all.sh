#!/bin/sh
#./a_updateall.sh
#./a_updateall.sh
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
./build_wdr4900.sh>/dev/null 2>&1
#./build_openrb_mynetway.sh>/dev/null 2>&1
./build_tonze_ap120.sh>/dev/null 2>&1
./build_compex_wp54.sh>/dev/null 2>&1
./build_compex_np28g.sh>/dev/null 2>&1
./build_x86.sh>/dev/null 2>&1
./build_x64.sh>/dev/null 2>&1
#./build_x86_gw700.sh
#./build_mikrotik.sh>/dev/null 2>&1
./build_whrg300n.sh>/dev/null 2>&1
./build_whrg300n_openvpn.sh>/dev/null 2>&1
./build_dir600.sh>/dev/null 2>&1
./build_dir300b.sh>/dev/null 2>&1
./build_dir615d.sh>/dev/null 2>&1
./build_dir615h.sh>/dev/null 2>&1
./build_esr9752sc.sh>/dev/null 2>&1
./build_acxnr22.sh>/dev/null 2>&1
./build_ar670w.sh>/dev/null 2>&1
./build_ar690w.sh>/dev/null 2>&1
./build_esr6650.sh>/dev/null 2>&1
#./build_whrg300n_buffalo.sh
./build_ecb9750.sh>/dev/null 2>&1
./build_br6574n.sh>/dev/null 2>&1
./build_asus_rtn13u.sh>/dev/null 2>&1
./build_asus_rtn13ub1.sh>/dev/null 2>&1
./build_asus_rt10n_plus.sh>/dev/null 2>&1
./build_wr5422.sh>/dev/null 2>&1
./build_eap9550.sh>/dev/null 2>&1
./build_f5d8235.sh>/dev/null 2>&1
./build_asus_rt15n.sh>/dev/null 2>&1
./build_wcrgn.sh>/dev/null 2>&1
./build_w502u.sh>/dev/null 2>&1
./build_whr300hp2.sh
./build_whr1166d.sh
./build_e1700.sh
./build_dir810l.sh
./build_dir860l.sh
cd broadcom_2_6_80211ac/opt 
./do_asus_rtac66u.sh >/dev/null 2>&1
./do_ea2700.sh >/dev/null 2>&1
./do_buffalo_dd-wrt_ac.sh >/dev/null 2>&1
./do_eko_big_v24-K26-nv64k.sh
./do_eko_mega_v24-K26-nv64k.sh
./do_ubnt_unifiac.sh
cd ../../

chmod -R 777 /GruppenLW/releases
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
scp -r /GruppenLW/releases/$DATE ftp.dd-wrt.com:/downloads
