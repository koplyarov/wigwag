#!/bin/bash

SCRIPT_DIR=`dirname $0`
TABSTOP=48

JsonBeginObj() { echo -n '{'; JSON_COMMA=0; }
JsonEndObj() { echo -n '}'; JSON_COMMA=1; }
JsonValue() { [ -z "${1//[0-9]/}" ] && echo -n "$1" || echo -n "\"$1\""; JSON_COMMA=1; }
JsonFieldName() { [ $JSON_COMMA -ne 0 ] && echo -n ','; echo -n "\"$1\":"; JSON_COMMA=1; }
JsonField() { JsonFieldName "$1" && JsonValue "$2"; }


GetMemoryConsumption() {
	echo $(($(ps --no-headers -p $(pidof wigwag_benchmarks) -o rss) * 1024))
}


OutputParser() {
	while read DATA; do
		if echo "$DATA" | grep -q "^<.* finished: .*>$"; then
			OP=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\1/g")
			NS=$(echo "$DATA" | sed "s/^<\(.*\) finished: \(.*\)>$/\2/g")
			JsonField "$OP, ns" "$NS"
		fi
		if echo "$DATA" | grep -q "^<measure memory, name: .*, count: .*>$"; then
			NAME=$(echo "$DATA" | sed "s/^<measure memory, name: \(.*\), count: \(.*\)>$/\1/g")
			COUNT=$(echo "$DATA" | sed "s/^<measure memory, name: \(.*\), count: \(.*\)>$/\2/g")
			RSS=$(GetMemoryConsumption)
			JsonField "memory per $NAME" "$(($RSS / $COUNT))"
		fi
	done
}


Benchmark() {
	ARGS="--task $1 --obj $2 --count $3"
	if [ "$4" ]; then
		ARGS="$ARGS --secondary-count $4"
	fi

	JsonFieldName "$*"
	JsonBeginObj
		"$BENCHMARKS_DIR/wigwag_benchmarks" $ARGS | OutputParser
	JsonEndObj
	sleep 1
}


GetSystemInfo() {
	JsonBeginObj
		JsonField os "$(lsb_release -sd)"
		JsonField cpu "$(grep "^model name\>" /proc/cpuinfo | head -n1 | sed "s/[^:]*:\s*\(.*\)$/\1/g")"
	JsonEndObj
}


while [ $# -ne 0 ]; do
	case "$1" in
		--bin-dir)	BENCHMARKS_DIR=$2; shift ;;
		--file|-f)	FILENAME=$2; shift ;;
		*)			echo "Unknown option: $1" >&2; exit 1; ;;
	esac
	shift
done


[ "$FILENAME" ] || FILENAME="benchmarks/benchmarks.list"
[ "$BENCHMARKS_DIR" ] || BENCHMARKS_DIR="$SCRIPT_DIR/../build/bin"


{
	JsonBeginObj
		JsonFieldName "systemInfo" && GetSystemInfo 
		JsonField script "$FILENAME"
		JsonFieldName "results"
		JsonBeginObj
			while read -r LINE; do
				if echo "$LINE" | grep -q "^#"; then continue; fi
				if echo "$LINE" | grep -q "^\s*$"; then continue; fi
				Benchmark $LINE
			done < "$FILENAME"
		JsonEndObj
	JsonEndObj
} | jq -S .

