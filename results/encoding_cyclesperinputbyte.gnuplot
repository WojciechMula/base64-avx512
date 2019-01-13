load "linespointsstyle.gnuplot"
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set xlabel "input bytes"
set ylabel "CPU cycles per input byte"

stats 'cnlencoding.txt' using 1
set xrange [STATS_min:STATS_max]
set ytics 0.5
set yrange [0:2]
set key top right box opaque

plot "cnlencoding.txt" \
        using 1:2 ti "memcpy"           smooth csplines   ls 1, \
     "" using 1:3 ti "Google Chrome"    smooth csplines   ls 2, \
     "" using 1:4 ti "AVX2"             smooth csplines   ls 3,\
     "" using 1:5 ti "AVX512VBMI"       smooth acsplines  ls 4
