set terminal pngcairo size 1024, 512
set output "fps-chc-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps_chc.dat" title "CHC" w lp,\
"fps_chc_fc.dat" title "CHC + Frustum Culling" w lp