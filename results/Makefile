ALL=cnl_decoding_cyclesperinputbyte.png \
    cnl_encoding_cyclesperinputbyte.png

all: $(ALL)

cnl_decoding_cyclesperinputbyte.png: cnldecoding.txt *.gnuplot
	gnuplot cnl_decoding_cyclesperinputbyte_png.gnuplot

cnl_encoding_cyclesperinputbyte.png: cnlencoding.txt *.gnuplot
	gnuplot cnl_encoding_cyclesperinputbyte_png.gnuplot

clean:
	rm -f *.pdf *.png
