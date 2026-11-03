reset
set datafile separator ","
set datafile columnheaders

if (!exists("data")) data = "task2.csv"
if (!exists("Tseq")) Tseq = 100.0
if (!exists("pfit")) pfit = 16
if (!exists("out"))  out  = "amdahl_fit_2-3.png"

S(f,p) = 1.0 / (f + ((1.0 - f) / p))
# Amdahl solved for f; undefined for p = 1
f_of(Sp,p) = (1.0/Sp - 1.0/p) / (1.0 - 1.0/p)
round2(v) = floor(v * 100 + 0.5) / 100.0

stats data using ($1 == pfit ? $2 : NaN) nooutput
Tpar_fit = STATS_min
f = f_of(Tseq/Tpar_fit, pfit)

print sprintf("measurement used:  p = %d, Tpar = %g s, S = %.4f", pfit, Tpar_fit, Tseq/Tpar_fit)
print sprintf("sequential fraction f = %.4f", f)
print sprintf("upper bound S(inf) = 1/f = %.2f", 1.0/f)
print ""

set table $values
plot data using (sprintf("%3d  %8.4f  %8.2f  %8.2f", $1, \
     ($1 == 1 ? NaN : f_of(Tseq/$2, $1)), round2(Tseq/$2), round2(S(f, $1)))) with table
unset table
print "  p    f from p  S meas.   S model"
print $values

set terminal pngcairo size 900,650
set output out

set logscale x 2
set logscale y 2
set xrange [1:16]
set yrange [1:16]

set title sprintf("Amdahl fit from p = %d: f = %.3f, S(inf) = %.1f", pfit, f, 1.0/f)
set xlabel "number of processors p"
set ylabel "speedup S(p)"
set key bottom right
set grid

plot x title "ideal" with lines dashtype 2 linecolor "black", \
     1.0/f title "upper bound 1/f" with lines dashtype 3 linewidth 2 linecolor "red", \
     S(f, x) title sprintf("Amdahl, f = %.3f", f) with lines linewidth 2, \
     data using 1:(Tseq/$2) title "measured" with points pointtype 7 pointsize 1.5

set output
