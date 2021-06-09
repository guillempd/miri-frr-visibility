set terminal pngcairo size 1024, 512
set output "fps-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps_1_fc.dat" title "Frustum Culling" w lp,\
"fps_1_sw.dat" title "Stop and Wait" w lp,\
"fps_1_sw_fc.dat" title "Stop and Wait + Frustum Culling" w lp,\
"fps_1_chc.dat" title "CHC" w lp,\
"fps_1_chc_fc.dat" title "CHC + Frustum Culling" w lp