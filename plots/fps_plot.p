set terminal pngcairo size 1024, 512
set output "fps-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps_2.dat" title "No Optimizations" w lp,\
"fps_2_fc.dat" title "Frustum Culling" w lp,\
"fps_2_sw.dat" title "Stop and Wait" w lp,\
"fps_2_sw_fc.dat" title "Stop and Wait + Frustum Culling" w lp
