# base64-avx512

Please ensure that you have a recent compiler. For example,
you may use the GNU GCC 8 compiler. On some machines, this
can be selected by the command `export CC=gcc-8`.

```
make
```

You should have a Cannon Lake processor or better.

You can also run the software using a Docker container...

```
docker build -t avx512base64 .
mkdir -p dockeroutput
docker run -v $PWD/dockeroutput:/dockeroutput --privileged  avx512base64
```

There is a bash script (`docker.sh`) to help.

## Reference


Wojciech Muła, Daniel Lemire, [Base64 encoding and decoding at almost the speed of a memory copy](https://arxiv.org/abs/1910.05109), Software: Practice and Experience (to appear)
