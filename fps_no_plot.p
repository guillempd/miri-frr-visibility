set terminal pngcairo size 1024, 512
set output "fps-no-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps_2.dat" title "No Optimizations" w lp,\
"fps_2_fc.dat" title "Frustum Culling" w lp