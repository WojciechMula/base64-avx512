docker build -t avx512base64 .
mkdir -p dockeroutput
docker run -v $PWD/dockeroutput:/dockeroutput --privileged  avx512base64
docker run -v $PWD/dockeroutput:/dockeroutput --privileged  avx512base64
