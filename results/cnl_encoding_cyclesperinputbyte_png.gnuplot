set term pngcairo fontscale 1.5 size 1024,768
set out "cnl_encoding_cyclesperinputbyte.png"
load "encoding_cyclesperinputbyte.gnuplot"

set term pdfcairo fontscale 1
set out "cnl_encoding_cyclesperinputbyte.pdf"
load "encoding_cyclesperinputbyte.gnuplot"

#set term pngcairo fontscale 1.5 size 1024,768
#set out "cnl_encoding_gbps.png"
#load "encoding_gbps.gnuplot"

#set term pdfcairo fontscale 1
#set out "cnl_encoding_gbps.pdf"
#load "encoding_gbps.gnuplot"