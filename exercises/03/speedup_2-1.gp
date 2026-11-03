reset
set datafile separator ","
set datafile columnheaders

if (!exists("data")) data = "task2.csv"
if (!exists("Tseq")) Tseq = 100.0
if (!exists("out"))  out  = "speedup_2-1.png"

# printf rounds exact halves to even (3.125 -> 3.12)
round2(v) = floor(v * 100 + 0.5) / 100.0

set table $values
plot data using (sprintf("%3d  %6.2f", $1, round2(Tseq/$2))) with table
unset table
print "  p       S"
print $values

set terminal pngcairo size 900,650
set output out

set logscale x 2
set logscale y 2
set xrange [1:16]
set yrange [1:16]

set title "Measured speedup"
set xlabel "number of processors p"
set ylabel "speedup S(p)"
set key top left
set grid

plot x title "ideal" with lines dashtype 2 linecolor "black", \
     data using 1:(Tseq/$2) title "measured" with linespoints linewidth 2 pointtype 7

set output
