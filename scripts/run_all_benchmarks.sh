SCRIPT_DIR=`dirname $0`

JsonBeginObj() { echo -n '{'; JSON_COMMA=0; }
JsonEndObj() { echo -n '}'; JSON_COMMA=1; }
JsonValue() { [ -z "${1//[0-9]/}" ] && echo -n "$1" || echo -n "\"$1\""; JSON_COMMA=1; }
JsonFieldName() { [ $JSON_COMMA -ne 0 ] && echo -n ','; echo -n "\"$1\":"; JSON_COMMA=1; }
JsonField() { JsonFieldName "$1" && JsonValue "$2"; }


while [ $# -ne 0 ]; do
	case "$1" in
		--bin-dir)	BENCHMARKS_DIR=$2; shift ;;
		--out-dir)	OUT_DIR=$2; shift ;;
		*)			echo "Unknown option: $1" >&2; exit 1; ;;
	esac
	shift
done

[ $BENCHMARKS_DIR ] || BENCHMARKS_DIR="$SCRIPT_DIR/../build/bin"
[ $OUT_DIR ] || OUT_DIR="$SCRIPT_DIR/../benchmarks.out"


NUM_PASSES=30

mkdir -p "$OUT_DIR"
for f in $(find "$SCRIPT_DIR/../benchmarks" -name "*.list"); do
	RESULTS=""
	for ((i = 0; i < $NUM_PASSES; ++i)); do
		echo "$(basename $f): $i"
		TMP=$(sudo "$SCRIPT_DIR/run_benchmarks.sh" --file "$f")
		if [ -z "$RESULTS" ]; then
			RESULTS="$TMP"
		else
			RESULTS=$({
					JsonBeginObj
						JsonFieldName "systemInfo"
						echo "$RESULTS" | jq ".systemInfo"
						JsonField "script" $(echo "$RESULTS" | jq -r ".script")
						JsonFieldName "results"
						JsonBeginObj
							OIFS=$IFS
							IFS=$'\n'
							for B in $(echo "$TMP" | jq ".results" | jq -r "keys[]"); do
								JsonFieldName "$B"
								JsonBeginObj
									TMP_RESULTS=$(echo "$TMP" | jq ".results[\"$B\"]")
									PREV_RESULTS=$(echo "$RESULTS" | jq ".results[\"$B\"]")
									for K in $(echo "$TMP_RESULTS" | jq -r "keys[]"); do
										V1=$(echo "$PREV_RESULTS" | jq -r ".\"$K\"")
										V2=$(echo "$TMP_RESULTS" | jq -r ".\"$K\"")
										MIN=$(($V1 < $V2 ? $V1 : $V2))
										JsonField "$K" "$MIN"
									done
								JsonEndObj
							done
							IFS=$OIFS
						JsonEndObj
					JsonEndObj
				} | jq -S .)
		fi
	done
	echo "$RESULTS" | jq -S . > "$OUT_DIR/$(basename $f).json"
done
