reset
load "linespointsstyle.gnuplot"
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set xlabel "input kilobytes"
set ylabel "CPU cycles per input byte"

stats 'cnldecoding.txt' using 1
set xrange [STATS_min/1024:STATS_max/1024]
set ytics 0.5
set yrange [0:1.0]
set key center right  opaque
set xtics 4
plot "cnldecoding.txt" \
      using ($1/1024):1+3*1+2 ti "Google Chrome"             smooth cspline  ls 2, \
     "" using ($1/1024):1+3*2+2 ti "AVX2"                      smooth cspline  ls 3, \
     "" using ($1/1024):1+3*4+2 ti "AVX512VBMI"                smooth cspline  ls 4, \
     "" using ($1/1024):1+3*0+2 ti "memcpy"                    smooth cspline  ls 1 

