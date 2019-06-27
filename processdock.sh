cp dockeroutput/encodingperf.txt results/cnlencoding.txt
cp dockeroutput/decodingperf.txt results/cnldecoding.txt
cd results && make
