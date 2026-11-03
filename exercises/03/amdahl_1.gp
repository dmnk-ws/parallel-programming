reset

if (!exists("fs"))   fs   = "0.001 0.01 0.05 0.1 0.2"
if (!exists("pmax")) pmax = 128
if (!exists("out"))  out  = "amdahl_1.png"

S(f,p) = 1.0 / (f + ((1.0 - f) / p))

set terminal pngcairo size 900,650
set output out

set xrange [1:pmax]
set yrange [1:pmax]
set logscale x 2
set logscale y

set title "Theoretical speedup curves according to Amdahl's law"
set xlabel "number of cores p"
set ylabel "speedup S(p)"
set key top left
set grid

plot x title "ideal" with lines dashtype 2 linecolor "black", \
     for [f in fs] S(real(f), x) title sprintf("f = %s", f) with lines linewidth 2

set output
