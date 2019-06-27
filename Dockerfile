FROM gcc:9.1
COPY . /usr/src/
WORKDIR /usr/src/
RUN make benchmark
CMD ["./benchmark", "/dockeroutput"]
