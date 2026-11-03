reset
set datafile separator ","
set datafile columnheaders

if (!exists("data")) data = "task2.csv"
if (!exists("Tseq")) Tseq = 100.0
if (!exists("out"))  out  = "overhead_2-2.png"

set table $values
plot data using (sprintf("%3d  %8.2f  %8.2f", $1, $1*$2, $1*$2 - Tseq)) with table
unset table
print "  p   p*Tpar      O(p)"
print $values

set terminal pngcairo size 900,650
set output out

set logscale x 2
set xrange [1:16]
set yrange [0:*]

set title "Total parallel overhead O(p) = p * Tpar(p) - Tseq"
set xlabel "number of processors p"
set ylabel "time [s]"
set key top left
set grid

plot data using 1:($1*$2 - Tseq) title "overhead O(p)" with linespoints linewidth 2 pointtype 7, \
     data using 1:($1*$2) title "total processor time p * Tpar(p)" with linespoints linewidth 2 pointtype 5, \
     Tseq title "Tseq" with lines dashtype 2 linecolor "black"

set output
