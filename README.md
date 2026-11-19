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

## Decoding rules and buffer sizes

The decoders follow the rules of `simdutf::base64_to_binary` with its default
options (standard alphabet, loose handling of the last chunk): padding is
optional, but when present it must complete the last 4-character quantum; a
final quantum of a single character is an error; non-zero trailing bits are
ignored. `decode_base64_avx512vbmi_despace` ignores ASCII white space (space,
tab, line feed, form feed, carriage return), as simdutf does; the other
decoders reject it. `chromium_base64_decode` (the baseline, unchanged)
additionally requires padding.

The output buffer of every decoder must hold at least
`base64_decoded_max_length(input, length)` bytes (see
`include/decode_base64_scalar.h`), the same bound as
`simdutf::maximal_binary_length_from_base64`. The decoders never write past it,
whether the input is valid or not. The AVX-512 encoders write exactly
`4 * ceil(length / 3)` bytes.

## Reference


Wojciech Muła, Daniel Lemire, [Base64 encoding and decoding at almost the speed of a memory copy](https://arxiv.org/abs/1910.05109), Software: Practice and Experience (to appear)
