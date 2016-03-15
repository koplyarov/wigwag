SCRIPT_DIR=`dirname $0`

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

mkdir -p "$OUT_DIR"
for f in $(find "$SCRIPT_DIR/../benchmarks" -name "*.*"); do
	for ((i = 0; i < 10; ++i)); do
		echo "$(basename $f): $i"
		echo "$f: $i"
		sudo "$SCRIPT_DIR/run_benchmarks.sh" --file "$f" > "$OUT_DIR/$(basename $f).$i.out" || exit 1
	done
done
