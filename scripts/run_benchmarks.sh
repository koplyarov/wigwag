#!/bin/bash

SCRIPT_DIR=`dirname $0`

if [ $# -eq 0 ]; then
	BENCHMARKS_DIR="$SCRIPT_DIR/../build/bin"
else
	BENCHMARKS_DIR="$1"
fi


GetMemoryConsumption() {
	echo $(($(ps --no-headers -p $(pidof wigwag_benchmarks) -o rss) * 1024))
}


OutputParser() {
	NUM_OBJECTS="$1"
	while read DATA; do
		if echo "$DATA" | grep -q "^<.* finished: .*>$"; then
			OP=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\1/g")
			NS_TOTAL=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\2/g")
			echo "  $OP:	$(($NS_TOTAL / $NUM_OBJECTS)) ns" | expand -t 32
		fi
		if [ "$DATA" = "<measure memory>" ]; then
			echo "  memory per object:	$(($(GetMemoryConsumption) / $NUM_OBJECTS)) bytes" | expand -t 32
		fi
	done
}


Benchmark() {
	TYPE="$1"
	OBJ="$2"
	COUNT="$3"
	echo "=== $TYPE $OBJ ==="
	"$BENCHMARKS_DIR/wigwag_benchmarks" -t "$TYPE" -o "$OBJ" -c "$COUNT" | OutputParser "$COUNT"
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
