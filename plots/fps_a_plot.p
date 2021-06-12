set terminal pngcairo size 1024, 512
set output "fps-a-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps_a.dat" title "Advanced" lt 2 lc 7 w lp,\
"fps_a_fc.dat" title "Advanced + Frustum Culling" lt 2 lc 2 w lp