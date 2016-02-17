#!/bin/bash

SCRIPT_DIR=`dirname $0`
BENCHMARKS_DIR="$SCRIPT_DIR/build/bin"


GetMemoryConsumption() {
	echo $(($(ps --no-headers -p $(pidof wigwag_benchmarks) -o rss) * 1024))
}


OutputParser() {
	NUM_OBJECTS="$1"
	while read DATA; do
		if echo "$DATA" | grep "^<.* finished: .*>$" > /dev/null; then
			OP=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\1/g")
			NS_TOTAL=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\2/g")
			echo "  $OP:	$(($NS_TOTAL / $NUM_OBJECTS)) ns" | expand -t 32
		fi
		if [ "$DATA" = "<measure memory>" ]; then
			echo "  memory per object:	$(($(GetMemoryConsumption) / $NUM_OBJECTS)) bytes" | expand -t 32
		fi
	done
}


PhaseInstrumentation() {
	PHASE="$1"
	while read DATA; do
		echo "$DATA"
		if [ "$DATA" = "<$PHASE>" ]; then
			callgrind_control --instr=on
		fi
		if echo "$DATA" | grep "^<$PHASE finished" > /dev/null; then
			callgrind_control --instr=off
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


Callgrind() {
	TYPE="$1"
	OBJ="$2"
	COUNT="$3"
	PHASE="$4"
	echo "=== $TYPE $OBJ ==="
	rm callgrind.out.*
	valgrind --tool=callgrind --instr-atstart=no "$BENCHMARKS_DIR/wigwag_benchmarks" -t "$TYPE" -o "$OBJ" -c "$COUNT" | PhaseInstrumentation "$PHASE"
}


PrintCpuInfo() {
	echo "### CPU info ###"
	echo "  $(grep "^model name\>" /proc/cpuinfo | head -n1)"
}


PrintCpuInfo

#echo
#Benchmark create_lock_unlock mutex 10000000
#Benchmark create_lock_unlock recursive_mutex 10000000
#Benchmark create_lock_unlock boost_mutex 10000000
#Benchmark create_lock_unlock boost_recursive_mutex 10000000

#echo
#Benchmark create condition_variable 10000000
#Benchmark create boost_condition_variable 10000000

#echo
#Benchmark create life_token 10000000

echo
Benchmark create signal 3000000
Benchmark create ui_signal 3000000
Benchmark create ui_signal_life_tokens 3000000
Benchmark create boost_signal2 3000000

echo
Benchmark connect signal 3000000
Benchmark connect ui_signal 3000000
Benchmark connect ui_signal_life_tokens 3000000
Benchmark connect boost_signal2 3000000
Benchmark connect_tracked boost_signal2 3000000

echo
Benchmark invoke signal 30000000
Benchmark invoke ui_signal 100000000
Benchmark invoke ui_signal_life_tokens 100000000
Benchmark invoke boost_signal2 30000000
Benchmark invoke_tracked boost_signal2 30000000

#Callgrind connect ui_signal 100000 invoking
#Callgrind connect ui_signal_life_tokens 100000 invoking
#Callgrind connect boost_signal2 100000 invoking
