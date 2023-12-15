name=$(basename $0 .bash)
if [ "$#" -ne 0 ]; then
	# Legacy
	lsof=$1
	report=$2
	tcasedir=$3
	dialect=$4
else
	# Autotools
	lsof=$PWD/lsof
	report=/dev/stdout
	tcasedir=lib/dialects/${LSOF_DIALECT_DIR}/tests
	dialect=${LSOF_DIALECT}
fi