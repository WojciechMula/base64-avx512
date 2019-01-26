def main():
    decode_lookup()
    decode_lookup_despace()
    decode_pack()
    join_four_decoded()


def decode_lookup():
    print "decode lookup"
    base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

    lookup_hi = [0x80] * 64
    lookup_lo = [0x80] * 64

    for encoded, char in enumerate(base64):

        value = ord(char)
        index = value & 0x3f
        if value & 0x40:
            lookup = lookup_hi
        else:
            lookup = lookup_lo

        lookup[index] = encoded

    print("lookup_lo")
    format_lookup(lookup_lo)
    print("lookup_hi")
    format_lookup(lookup_hi)


def decode_lookup_despace():
    print "decode lookup (despace)"
    base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
    spaces = ' \r\n'

    lookup_hi = [0x80] * 64
    lookup_lo = [0x80] * 64

    for encoded, char in enumerate(base64):

        value = ord(char)
        index = value & 0x3f
        if value & 0x40:
            lookup = lookup_hi
        else:
            lookup = lookup_lo

        lookup[index] = encoded

    for char in spaces:

        value = ord(char)
        index = value & 0x3f
        if value & 0x40:
            lookup = lookup_hi
        else:
            lookup = lookup_lo

        lookup[index] = 0x40 # set 6th bit

    print("lookup_lo")
    format_lookup(lookup_lo)
    print("lookup_hi")
    format_lookup(lookup_hi)


def decode_pack():
    print "decode pack"

    pack = [0] * 64

    output = 0
    for i in xrange(16):
        pack[i*3 + 0] = output + 2
        pack[i*3 + 1] = output + 1
        pack[i*3 + 2] = output + 0
        output += 4

    format_lookup(pack)


def join_four_decoded():
    print("join four decoded vectors")
    # decoded data has 24-bit values in 32-bit words, we need to save these byte in a continous array

    # vec0 has bytes   0 ..  48
    # vec1 has bytes  49 ..  96
    # vec2 has bytes  96 .. 144
    # vec3 has bytes 144 .. 192
    
    join = []
    index = 0

    k = 0 
    while len(join) < 4*48:
        join.append(index + 2)
        join.append(index + 1)
        join.append(index + 0)
        index += 4

    join01 = join[0*64:1*64]
    join12 = [x - 64 for x in join[1*64:2*64]]
    join23 = [x - 128 for x in join[2*64:3*64]]

    print "const __m512i join01 =",
    format_lookup(join01)
    print "const __m512i join12 =",
    format_lookup(join12)
    print "const __m512i join23 =",
    format_lookup(join23)


def format_lookup(array):
    assert len(array) == 64
    assert all(0 <= x <= 255 for x in array)

    dwords = []
    for i in xrange(0, len(array), 4):
        b0 = array[i + 0]
        b1 = array[i + 1]
        b2 = array[i + 2]
        b3 = array[i + 3]

        dwords.append( b0 | (b1 << 8) | (b2 << 16) | (b3 << 24))

    dwords_fmt = ['0x%08x' % x for x in dwords]
    dwords_fmt = ', '.join(dwords_fmt)

    print '_mm512_setr_epi32(%s)' % dwords_fmt


if __name__  == '__main__':
    main()
