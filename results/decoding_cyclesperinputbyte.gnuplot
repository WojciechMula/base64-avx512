reset
load "linespointsstyle.gnuplot"
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set xlabel "input kilobytes"
set ylabel "base64 GB/s"

stats 'cnldecoding.txt' using 1
set xrange [STATS_min/1024:STATS_max/1024]
#set ytics 0.05
set yrange [0:155]
set xrange [4:]
set key top right invert
set xtics 4
plot "cnldecoding.txt" using ($1/1024):1+3*1+1  every 1 ti "Chrome"                     with linespoints  ls 2, \
     "" using ($1/1024):1+3*2+1  every 1 ti "AVX2"                      with linespoints ls 3, \
     "" using ($1/1024):1+3*3+1  every 1 ti "AVX-512"               with linespoints  ls 4,\
     "" using ($1/1024):1+3*0+1  every 1 ti "memcpy"       with linespoints   ls 1
