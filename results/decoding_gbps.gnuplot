reset
load "linespointsstyle.gnuplot"
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set xlabel "input kilobytes"
set ylabel "GB/s"

stats 'cnldecoding.txt' using 1
set xrange [STATS_min/1024:STATS_max/1024]
#set ytics 0.5
#set yrange [0:1.0]
set key top right  opaque
set xtics 4
plot "cnldecoding.txt" \
      using ($1/1024):1+3*(1+7)+2 ti "Google Chrome"             smooth acsplines  ls 2, \
     "" using ($1/1024):1+3*(2+7)+2 ti "AVX2"                      smooth acsplines  ls 3, \
     "" using ($1/1024):1+3*(4+7)+2 ti "AVX512VBMI"                smooth acsplines  ls 4, \
     "" using ($1/1024):1+3*(0+7)+2 ti "memcpy"                    smooth acsplines  ls 1 

