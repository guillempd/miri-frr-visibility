set terminal pngcairo size 1024, 512
set output "fps-no-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps.dat" title "No Optimizations" w lp,\
"fps_fc.dat" title "Frustum Culling" w lp