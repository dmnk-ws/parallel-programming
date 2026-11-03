reset
set datafile separator ","
set datafile columnheaders

if (!exists("data")) data = "task2.csv"
if (!exists("Tseq")) Tseq = 100.0
if (!exists("pfit")) pfit = 16
if (!exists("pmax")) pmax = 128
if (!exists("out"))  out  = "gustafson_2-4.png"

amdahl(f,p)    = 1.0 / (f + ((1.0 - f) / p))
gustafson(f,p) = f + (1.0 - f) * p
f_of(Sp,p)     = (1.0/Sp - 1.0/p) / (1.0 - 1.0/p)
round2(v)      = floor(v * 100 + 0.5) / 100.0

stats data using ($1 == pfit ? $2 : NaN) nooutput
f = f_of(Tseq/STATS_min, pfit)

print sprintf("f (Amdahl fit at p = %d) = %.4f", pfit, f)
print sprintf("Amdahl    S(%d) = %.2f, S(inf) = %.2f", pfit, round2(amdahl(f, pfit)), round2(1.0/f))
print sprintf("Gustafson S(%d) = %.2f, S(inf) = unbounded (slope 1 - f = %.3f)", pfit, round2(gustafson(f, pfit)), 1.0 - f)
print ""

set table $values
plot data using (sprintf("%3d  %8.2f  %8.2f  %10.2f", $1, \
     round2(Tseq/$2), round2(amdahl(f, $1)), round2(gustafson(f, $1)))) with table
unset table
print "  p   S meas.    Amdahl   Gustafson"
print $values

set terminal pngcairo size 900,650
set output out

set logscale x 2
set logscale y 2
set xrange [1:pmax]
set yrange [1:pmax]

set title sprintf("Amdahl vs. Gustafson for f = %.3f", f)
set xlabel "number of processors p"
set ylabel "speedup S(p)"
set key top left
set grid

plot x title "ideal" with lines dashtype 2 linecolor "black", \
     gustafson(f, x) title "Gustafson (weak scaling)" with lines linewidth 2, \
     amdahl(f, x) title "Amdahl (strong scaling)" with lines linewidth 2, \
     1.0/f title "Amdahl bound 1/f" with lines dashtype 3 linewidth 2 linecolor "red", \
     data using 1:(Tseq/$2) title "measured" with points pointtype 7 pointsize 1.5 linecolor "dark-orange"

set output
