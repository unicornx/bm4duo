#!/bin/bash

CASES="gpio clock irq pwm rtc rtc_irq wdt adc i2c timer spi dbg"

usage ()
{
	echo    "Usage: [BOARD=board] build.sh '-h'|'-a'|<case>"
	echo    "  board: duo/duo256, default is duo256"
	echo    "  -h: print this usage"
	echo    "  -a: try to build all cases, just used for integrity test and no output created"
	echo -n "  <case>: build <case>, <case> can be: "; for case in $CASES; do echo -n "$case "; done
	echo
}

if [ $# -lt 1 ]; then
	usage
	exit -1
fi

if [ "$1" == '-h' ]; then
	usage
elif [ "$1" == "-a" ]; then
	make clean
	for case in $CASES; do
		make TEST=$case release
		make clean
	done

else
	make clean
	for case in $CASES; do
		if [ $case = $1 ]; then
			CASE=`echo ${case^^}`
			make TEST=$CASE release
			exit
		fi
	done
	echo "Oops, case \"$1\" does not exist, please double-check!"
fi

