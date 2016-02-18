#!/bin/bash

SCRIPT_DIR=`dirname $0`

if [ $# -eq 0 ]; then
	BENCHMARKS_DIR="$SCRIPT_DIR/../build/bin"
else
	BENCHMARKS_DIR="$1"
fi


PhaseInstrumentation() {
	PHASE="$1"
	while read DATA; do
		echo "$DATA"
		if [ "$DATA" = "<$PHASE>" ]; then
			callgrind_control --instr=on
		fi
		if echo "$DATA" | grep -q "^<$PHASE finished"; then
			callgrind_control --instr=off
		fi
	done
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


Callgrind connect ui_signal 100000 invoking
#Callgrind connect ui_signal_life_tokens 100000 invoking
#Callgrind connect boost_signal2 100000 invoking
