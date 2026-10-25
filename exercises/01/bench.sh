#!/bin/bash
# Usage: ./bench.sh [runs] > results.csv
set -e
cd "$(dirname "$0")"
export LC_ALL=C

RUNS=${1:-3}
NS="1000000 10000000 100000000 1000000000 10000000000"
TS="1 2 4 6 8 16 32"
MUTEX_MAX_N=100000000

cc -O2 -o pi pi.c
cc -O2 -pthread -o pi_threads pi_threads.c

measure() {
  /usr/bin/time -p "$@" 2>&1 >/dev/null | awk '{ printf "%s%s", sep, $2; sep = "," }'
}

echo "variant,n,T,run,real,user,sys"
for n in $NS; do
  for r in $(seq "$RUNS"); do
    echo "seq,$n,1,$r,$(measure ./pi "$n")"
  done
  for v in block cyclic mutex; do
    if [ "$v" = mutex ] && [ "$n" -gt "$MUTEX_MAX_N" ]; then continue; fi
    for t in $TS; do
      for r in $(seq "$RUNS"); do
        echo "$v,$n,$t,$r,$(measure ./pi_threads "$v" "$n" "$t")"
      done
    done
  done
done
