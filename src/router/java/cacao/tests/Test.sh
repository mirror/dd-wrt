if ($1 $2 > $2.thisoutput 2> $2.this2output)
then
# no Error returned
	if [ -f $2.2output ]
	then
	# Error should have been returned
		echo "$2:	returned ok, but should have failed"
		head $2.output
		test -z "$CHECK_NO_FAIL_FAST" && exit 1 || exit
	fi

	if (diff $2.output $2.thisoutput > /dev/null)
	then
		echo "$2:	OK"
	else
		echo "$2:	failed"
		diff $2.output $2.thisoutput | head
		test -z "$CHECK_NO_FAIL_FAST" && exit 1
	fi
else
# Error returned
	if [ ! -f $2.2output ]
	then
	# No Error should have been returned
		echo "$2:	failed, but should have returned ok"
		head $2.this2output
		test -z "$CHECK_NO_FAIL_FAST" && exit 1 || exit
	fi
	if ((diff $2.output $2.thisoutput >/dev/null) && (diff $2.2output $2.this2output >/dev/null))
	then
		echo "$2:	OK"
	else
		echo "$2:	failed"
		diff $2.output $2.thisoutput | head
		diff $2.2output $2.this2output | head
		test -z "$CHECK_NO_FAIL_FAST" && exit 1
	fi
fi
