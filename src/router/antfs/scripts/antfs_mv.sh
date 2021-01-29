#!/bin/sh

if [ $# -lt 1 ]; then
	echo '<INFO> Param 1 must be a device.'
	exit 255
fi

ntfspath=`mount | grep $1 | cut -d' ' -f3`
ntfspath="$ntfspath/mv"

# 500
testloop1_max=125
# 100
testloop2_max=25

mk_test_dirs() {
	local j
	for j in $(seq $testloop1_max); do
		mkdir "$ntfspath/$1" &> /dev/null &
		mkdir "$ntfspath/$1" &> /dev/null &
		mkdir "$ntfspath/$1" &> /dev/null &
		mkdir "$ntfspath/$1" &> /dev/null &
		wait
	done &
	for j in $(seq $testloop2_max); do
		rm -rf "$ntfspath/$1" &> /dev/null &
		rm -rf "$ntfspath/$1" &> /dev/null &
		rm -rf "$ntfspath/$1" &> /dev/null &
		rm -rf "$ntfspath/$1" &> /dev/null &
		wait
	done &
	wait
}

touch_test_files() {
	local j
	mkdir "$ntfspath/cccc" &> /dev/null &
	for j in $(seq $testloop1_max); do
		touch "$ntfspath/cccc/$1" &> /dev/null &
		touch "$ntfspath/cccc/$1" &> /dev/null &
		touch "$ntfspath/cccc/$1" &> /dev/null &
		touch "$ntfspath/cccc/$1" &> /dev/null &
		wait
	done &
	for j in $(seq $testloop2_max); do
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		wait
	done &
	wait
}

mv_test_dirs() {
	# 200
	local k
	for k in $(seq $testloop2_max); do
		mv "$ntfspath/$1" "$ntfspath/$2" &> /dev/null &
		mv "$ntfspath/$2" "$ntfspath/$1" &> /dev/null &
		mv "$ntfspath/$1" "$ntfspath/$2" &> /dev/null &
		mv "$ntfspath/$2" "$ntfspath/$1" &> /dev/null &
		wait
	done
}

dd_test_files() {
	local j
	mkdir "$ntfspath/cccc" &> /dev/null &
	for j in $(seq $testloop1_max); do
		dd bs=64K count=1 if=/dev/urandom of="$ntfspath/cccc/$1" &> /dev/null &
		dd bs=64K count=1 if=/dev/urandom of="$ntfspath/cccc/$1" &> /dev/null &
		dd bs=64K count=1 if=/dev/urandom of="$ntfspath/cccc/$1" &> /dev/null &
		dd bs=64K count=1 if=/dev/urandom of="$ntfspath/cccc/$1" &> /dev/null &
		wait
	done &
	for j in $(seq $testloop2_max); do
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		rm -rf "$ntfspath/cccc/$1" &> /dev/null &
		wait
	done &
	wait
}
# create and move a few directories arround ... many times ...
if ! mkdir -p $ntfspath; then
	echo "<ERROR> Cannot create $ntfspath"
	exit 255
fi
echo '<INFO> Starting mkdir/mv test sequence.'
for i in $(seq 5); do
	mk_test_dirs aaaa &
	mv_test_dirs aaaa bbbb &
	mk_test_dirs aaaa &
	mv_test_dirs aaaa bbbb &
	wait
done

echo '<INFO> Done.'

echo '<INFO> Cleanup.'
# anything deleted?
find "$ntfspath" -mindepth 1 -maxdepth 1 -depth -name '*' -type d \( -exec rm -rf {} \; \) -o -exec rm {} \; # &> /dev/null

# disk empty?
if [ ! -e $ntfspath -o $(ls "$ntfspath/" | wc -l) -gt 0 ]; then
	echo "<ERROR> Failed to delete contents of $ntfspath or it is completely gone."
	exit 255
fi

echo '<INFO> Starting touch/mv test sequence.'
for i in $(seq 10); do
	touch_test_files dddd &
	mv_test_dirs cccc/dddd cccc/eeee &
	wait
done

echo '<INFO> Starting dd/mv test sequence.'
for i in $(seq 10); do
	dd_test_files gggg &
	mv_test_dirs cccc/gggg cccc/ffff &
	wait
done

echo '<INFO> Done.'

echo '<INFO> Cleanup.'
# anything deleted?
rm -rf "$ntfspath"

# disk empty?
if [ -e $ntfspath ]; then
	echo "<ERROR> Failed to delete contents of $ntfspath."
	exit 255
fi

echo "<INFO> Finished successfully"
