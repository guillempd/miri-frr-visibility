set terminal pngcairo size 1024, 512
set output "fps-sw-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps_sw.dat" title "Stop and Wait" w lp,\
"fps_sw_fc.dat" title "Stop and Wait + Frustum Culling" w lp