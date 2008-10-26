#!/bin/sh

CONF4CPP=../../src/conf4cpp

pass=0
total=0
for fname in *
do
	$CONF4CPP $fname
	if test $? -ne 255 
	then
		echo "failed test case: $fname"
	else
		pass=`expr $pass + 1`
	fi
	total=`expr $total + 1`
done

echo

if test $pass -eq $total
then
	echo "******************************************"
	echo "    all test case passed: $pass/$total    "
	echo "******************************************"
else
	echo "**************************"
	echo "    $pass/$total passed   "
	echo "**************************"
fi
