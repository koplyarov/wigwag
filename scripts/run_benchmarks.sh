#!/bin/bash

SCRIPT_DIR=`dirname $0`
TABSTOP=48

if [ $# -eq 0 ]; then
	BENCHMARKS_DIR="$SCRIPT_DIR/../build/bin"
else
	BENCHMARKS_DIR="$1"
fi


GetMemoryConsumption() {
	echo $(($(ps --no-headers -p $(pidof wigwag_benchmarks) -o rss) * 1024))
}


OutputParser() {
	while read DATA; do
		if echo "$DATA" | grep -q "^<.* finished: .*>$"; then
			OP=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\1/g")
			NS=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\2/g")
			echo "  $OP:	$NS ns" | expand -t $TABSTOP
		fi
		if echo "$DATA" | grep -q "^<measure memory, name: .*, count: .*>$"; then
			NAME=$(echo "$DATA" | sed "s/^<measure memory, name: \(.*\), count: \(.*\)>$/\1/g")
			COUNT=$(echo "$DATA" | sed "s/^<measure memory, name: \(.*\), count: \(.*\)>$/\2/g")
			RSS=$(GetMemoryConsumption)
			echo "  memory per $NAME:	$(($RSS / $COUNT)) bytes" | expand -t $TABSTOP
		fi
	done
}


Benchmark() {
	ARGS="--task $1 --obj $2 --count $3"
	if [ "$4" ]; then
		ARGS="$ARGS --secondary-count $4"
	fi

	echo "=== $@ ==="
	"$BENCHMARKS_DIR/wigwag_benchmarks" $ARGS | OutputParser
}


PrintCpuInfo() {
	echo "### CPU info ###"
	echo "  $(grep "^model name\>" /proc/cpuinfo | head -n1)"
}


PrintCpuInfo

FILENAME="scripts/benchmarks.list"
SKIP_EMPTY_STRINGS=1
echo
while read -r LINE; do
	if echo "$LINE" | grep -q "^#"; then
		continue
	fi
	if echo "$LINE" | grep -q "^\s*$"; then
		[ $SKIP_EMPTY_STRINGS -eq 0 ] && echo
		continue
	fi
	Benchmark $LINE
	SKIP_EMPTY_STRINGS=0
done < "$FILENAME"
