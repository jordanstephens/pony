#!/bin/bash

CHART_FILE="tmp/timings-$(date +%s)"
TIMINGS_FILE="tmp/bench/timings.tsv"

gnuplot <<- EOF
  set term pngcairo size 800,600
  set output "${CHART_FILE}.png"

  set logscale y 10
  set xlabel "Insert Iteration"
  set ylabel "Time (s) Log Scale"

  plot '${TIMINGS_FILE}' using 1:2 \
    title 'Timings'
EOF

echo "Generated ${CHART_FILE}.png"
