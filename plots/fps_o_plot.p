set terminal pngcairo size 1024, 512
set output "fps-o-comparison.png"

set xrange [0:60]
set yrange [0:300]
set xlabel "time"
set ylabel "fps"
plot\
"fps.dat"     title "No optimizations" lt 2 lc 7 w l,\
"fps_a.dat"   title "Advanced"         lt 2 lc 2 w l,\
"fps_sw.dat"  title "Stop and Wait"    lt 2 lc 6 w l,\
"fps_chc.dat" title "CHC"              lt 2 lc -1 w l,\
30 lt 0 lc 0 w lp