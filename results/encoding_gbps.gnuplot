reset
load "linespointsstyle.gnuplot"
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set xlabel "input kilobytes"
set ylabel "GB/s"

stats 'cnlencoding.txt' using 1
set xrange [STATS_min/1024:STATS_max/1024]
#set ytics 0.5
#set yrange [0:2]
set key top right  opaque
set xtics 4
plot "cnlencoding.txt" \
     using ($1/1024):1+3*(1+6)+2 ti "Google Chrome"    smooth acsplines   ls 2, \
     "" using ($1/1024):1+3*(2+6)+2 ti "AVX2"             smooth acsplines   ls 3, \
     "" using ($1/1024):1+3*(4+6)+2 ti "AVX512VL"         smooth acsplines  ls 5, \
     "" using ($1/1024):1+3*(0+6)+2 ti "memcpy"           smooth acsplines  ls 1

