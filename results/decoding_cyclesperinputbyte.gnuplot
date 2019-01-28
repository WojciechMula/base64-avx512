load "linespointsstyle.gnuplot"
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set xlabel "input bytes"
set ylabel "CPU cycles per input byte"

stats 'cnldecoding.txt' using 1
set xrange [STATS_min:STATS_max]
set ytics 0.5
set yrange [0:1.5]
set key top right box opaque

plot "cnldecoding.txt" \
        using 1:2 ti "memcpy"                   w lines  ls 1, \
     "" using 1:3 ti "Google Chrome"            w lines  ls 2, \
     "" using 1:4 ti "AVX2"                     w lines  ls 3, \
     "" using 1:6 ti "AVX512VBMI"               w lines  ls 4
