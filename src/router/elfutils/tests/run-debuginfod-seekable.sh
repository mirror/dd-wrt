#!/usr/bin/env bash

. $srcdir/debuginfod-subr.sh

# for test case debugging, uncomment:
set -x
unset VALGRIND_CMD

mkdir R Z
cp -rvp ${abs_srcdir}/debuginfod-rpms/seekable-xz R
cp -rvp ${abs_srcdir}/debuginfod-debs/seekable-xz D

# This variable is essential and ensures no time-race for claiming ports occurs
# set base to a unique multiple of 100 not used in any other 'run-debuginfod-*' test
base=14100
get_ports

DB=${PWD}/.debuginfod_tmp.sqlite
tempfiles $DB
export DEBUGINFOD_CACHE_PATH=${PWD}/.client_cache

env LD_LIBRARY_PATH=$ldpath ${abs_builddir}/../debuginfod/debuginfod $VERBOSE \
	-d $DB -p $PORT1 -t0 -g0 \
	--fdcache-mbs=100 --fdcache-mintmp=0 --fdcache-prefetch=0 \
	-R -U R D > vlog$PORT1 2>&1 &
PID1=$!
tempfiles vlog$PORT1
errfiles vlog$PORT1

wait_ready $PORT1 'ready' 1
wait_ready $PORT1 'thread_work_total{role="traverse"}' 1
wait_ready $PORT1 'thread_work_pending{role="scan"}' 0
wait_ready $PORT1 'thread_busy{role="scan"}' 0

# Mapping from build ID to sha256 of executable and debuginfo files.  Generated with:
#
# #/bin/bash
# set -e
#
# tmpdir="$(mktemp -d)"
# trap 'rm -rf "$tmpdir"' EXIT
# mkdir "$tmpdir/rpm" "$tmpdir/deb"
# rpm2cpio tests/debuginfod-rpms/seekable-xz/compressme-seekable-xz-1.0-1.x86_64.rpm | cpio -D "$tmpdir/rpm" -id --quiet
# rpm2cpio tests/debuginfod-rpms/seekable-xz/compressme-seekable-xz-debuginfo-1.0-1.x86_64.rpm | cpio -D "$tmpdir/rpm" -id --quiet
# ar p tests/debuginfod-debs/seekable-xz/compressme-seekable-xz_1.0-1_amd64.deb data.tar.xz | tar -C "$tmpdir/deb" -xJ
# ar p tests/debuginfod-debs/seekable-xz/compressme-seekable-xz-dbgsym_1.0-1_amd64.deb data.tar.xz | tar -C "$tmpdir/deb" -xJ
#
# echo "declare -A files=("
# for which in rpm deb; do
# 	cd "$tmpdir/$which/usr/bin"
# 	echo "	# $which"
# 	for file in $(ls -v); do
# 		build_id="$(eu-readelf -n "$file" | sed -n 's/^.*Build ID: \([a-f0-9]\+\).*$/\1/p')"
# 		executable_sha="$(sha256sum "$file" | cut -d' ' -f1)"
# 		debuginfo_sha="$(sha256sum "../../usr/lib/debug/.build-id/${build_id:0:2}/${build_id:2}.debug" | cut -d' ' -f1)"
# 		echo "	[$build_id]=\"$executable_sha $debuginfo_sha\" # $file"
# 	done
# done
# echo ")"
declare -A files=(
	# rpm
	[3a54b25d643025aa69d33f08f1ddeee42b63b0c7]="7e8f2bb564e1a74f1fb0344b2218ee5dd885a84885d850846980d1e7e56d8b8b 4421819cac8118e56f9fe2fa6f3becb209b115c6eb2906ed54935f970034315c" # compressme1
	[aa1dd872c917955a95be7943219a4c58886dc965]="e38b0b8494c5cb7816394c00e9c80333262d28b368c8eff59397981435e401b4 101f46d227db71ec8c080de08fd93750a976299a81a2397f6f3b19c0771e138a" # compressme2
	[c80ba826295ca18732ddc15474f166c50c81fc51]="7b1fbbe1d702770d8fa22610a9463c41c7dee8d21fce167a9a1b0588bf82f516 49962d52bd736b63975ade48f52b5431fa7f478baf102b12bbb9efce8c5ef0ba" # compressme3
	[f8617b5ea038d166417357779f6c17dec8b80690]="4bc682ee3194ed9d2efb92e828a9f6ff3c7b0f25cb5ba94832250e79261ada8b eec384c131ce68eeb8026168a6888e3acb2fbf0d60fe808ddbe0e342eabf74a9" # compressme4
	[34880de6319ba33c23c1b1c25e454abf5ec9c433]="c83e1ed93fe09b3850368bff92ed9d4e5807515920126db71bdefc25cc3cb617 7fde181eb2ecd79be1b8aa8c5929454df5bf6c91c96e458b7c36df8da9cc640b" # compressme5
	[1be17d4e02bcf6059481e9591881c6ef2d24b795]="94037ba19019ea1be06ffa20f4d9e7cc58faf7af90c4554a657395fcc86e3c3f 6e0c3b6c5daa824f30ea95587e8e3c051bebd0eca883f8cdeada5190e8c1d4cd" # compressme6
	[be3a4ca9a68fc2b200fe5acbb12f0c5b8c761b69]="c16ac0ccde84cd8f89eebec8c29ce783a7da5c5730c76393774925487e6511f7 34b08a88f131cc9d4f7f1053b26dd277956eb18a47420aa1ac8d35c99c23d574" # compressme7
	[d64dd065e26a876c79f9ce640e107075263e4595]="53ab9909861aa77eb5cb5e7d62e2882f733861fcaf5c2421800758e52c1e0dea 3d2e3a6ddd7673acaf0573f39f0cc52d95a1b52a080d34efcc02d85e788c148e" # compressme8
	[28cc53dd47f9d12673d97ab38f6a21bcad3a528f]="9c231d8ea65133479eed17425e1d3b355703bcd0bf3dfc80520e25ffdc6d5d78 79aa232366e99bcbf4adf8ee74b333979d9164f45ad6ba55abb0e85be2ccd079" # compressme9
	[a05381ea7253b6b446e2120cab2fef08445612a3]="a66b503b4decade17b1551ef82b380c1506713b36a78c776d12d9c3863a4115a 0a6d59c228e74d485e9b314cb7e1f718267103d2c179efac89e874f7b852bbec" # compressme10
	# deb
	[f2d910ae1e3e3fa717ef202120966ee4dba07ebd]="e16a38135865eff8f26b5ddfdd5719ba4b90d233f469fb07428cc6ef299214e9 047f7ec51840f6cf6977d6253ccd503f2ce2711813a53317b81e79d20452fd38" # compressme1
	[65e773f11929c579b90923ea81f36523ce2337e6]="00e2fac30ba6c494473fdba1e5ccb0cdbf12229581d2f95af519c9550af5d3d6 eee11a8c840623093de5a1518e673e50141df42e18ea1933c78edf7f11742427" # compressme2
	[a679eaf745a6da111041d208cdeb4474192c2e50]="194b79bcab83fa8140ccbee6cc199f233b0e8804b4a2231695f3c7c4604b67b4 684ff9ad385ab15a066ad4bfc3074df15f1525580310202150d7043513e9890b" # compressme3
	[8b117108d9b7f6ffce251421bdf6c6cc8c801d35]="edf8fec536efb1d9cfb534fd67c12a35b60fdb5955d3f9119ef664818fef0b22 1ef9d762fe59b03547ac7baa5596e13a4bfa8322bbdd8400987f8a95a4d038a6" # compressme4
	[2269081378c82ff119d0f0ec8cdaba3977746835]="77a5b384554a32c73f528abfd196c05c5eca6b97c948ebdccbd6d1beb441105b d29dc24c52cde04397f874660e0980f2d4a043ff132c019d0f92b238012ab464" # compressme5
	[d0840f88ceb63f52354c0b4dceda6c5011155bbd]="523c2c1c4176149a2d773307e5de520195195020bf0f2e42eb47b958b2fbdb93 170a684806ff7e55938d3afa02095a6bdd3184b62b8efdfebb72bb85bfc4120b" # compressme6
	[8101300d5d590b434b798c3edda0908346230cef]="878b7b8cedca8fb69ebc8bc3cb635a7ce0dd4b4da787cde18ff7d06503105c73 86d2cd52026cba35114201d3a1fc2ce31b2403d91785dad5366b5a5b9fe3636f" # compressme7
	[b8755c57a65fe12fa210f216dd861cf2b5101918]="2ec562f19b5b57036b48cd1f07d2d61c6a5a91b0470d14393b8c11429753d632 1ee7340245db704a0e3d5f7431706e14e7feeb9ba5c6572e61287b24b0e9fca0" # compressme8
	[ffe01fd7b7d994d9b9d9e9311ac5189d82162ba0]="cf129420e315943b9d63891455aae7fb69189158b66eba82faf1580e763aa642 3a73ccbd08a6a4088355e973bd4748d99c56b3557b12417e4fa34ed9e85c85a9" # compressme9
	[dd25b00a86c89feaf5ba61fd9c6dc8bd9e5aebef]="5bd04cadb7bce5ca3f817b84c734639764046365a96a0bc1870ecc480e7c38a9 08f9da5788224b8cfdc3bd91b784d1f9a109780100a71f6fbe0b52ec43ea436e" # compressme10
)

which=(executable debuginfo)
check_all() {
	local port="$1"
	for build_id in "${!files[@]}"; do
		sha=(${files["$build_id"]})
		# Check the executable and the debuginfo.
		for ((i = 0; i < 2; i++)); do
			# Check each one twice to test the fdcache.
			for ((j = 0; j < 2; j++)); do
				path="$(env LD_LIBRARY_PATH=$ldpath DEBUGINFOD_URLS=http://localhost:$port ${abs_builddir}/../debuginfod/debuginfod-find "${which[$i]}" "$build_id")"
				sha256sum --check - << EOF
${sha[$i]} $path
EOF
				rm -f "$path"
			done
		done
	done
}

check_all $PORT1

# Make sure all extractions used the seekable optimization.
curl -s http://localhost:$PORT1/metrics | awk '
/^http_responses_total\{result="seekable xz archive"\}/ {
	print
	seekable = $NF
}

/^http_responses_total\{result="archive fdcache"\}/ {
	print
	fdcache = $NF
}

/^http_responses_total\{result="(rpm|deb) archive"\}/ {
	print
	full = $NF
}

END {
	if (seekable == 0) {
		print "error: no seekable extractions" > "/dev/stderr"
		exit 1
	}
	if (fdcache == 0) {
		print "error: no fdcache hits" > "/dev/stderr"
		exit 1
	}
	if (full > 0) {
		print "error: " full " full extractions" > "/dev/stderr"
		exit 1
	}
}'

tempfiles $DB*

kill $PID1
wait $PID1
PID1=0

if type sqlite3 2>/dev/null; then
	# Emulate the case of upgrading from an old server without the seekable
	# optimization by dropping the _r_seekable table.
	sqlite3 "$DB" 'DROP TABLE buildids10_r_seekable'

	env LD_LIBRARY_PATH=$ldpath ${abs_builddir}/../debuginfod/debuginfod $VERBOSE \
		-d $DB -p $PORT2 -t0 -g0 \
		--fdcache-mbs=100 --fdcache-mintmp=0 --fdcache-prefetch=0 \
		-R -U R D > vlog$PORT2 2>&1 &
	PID2=$!
	tempfiles vlog$PORT2
	errfiles vlog$PORT2

	wait_ready $PORT2 'ready' 1

	check_all $PORT2

	# The first request per archive has to do a full extraction.  Check
	# that the rest used the seekable optimization.
	curl -s http://localhost:$PORT2/metrics | awk '
/^http_responses_total\{result="seekable xz archive"\}/ {
	print
	seekable = $NF
}

/^http_responses_total\{result="(rpm|deb) archive"\}/ {
	print
	full = $NF
}

END {
	if (seekable == 0) {
		print "error: no seekable extractions" > "/dev/stderr"
		exit 1
	}
	if (full > 4) {
		print "error: too many (" full ") full extractions" > "/dev/stderr"
		exit 1
	}
}'

	tempfiles $DB*

	kill $PID2
	wait $PID2
	PID2=0
fi

exit 0
