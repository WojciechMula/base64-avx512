def main():
    decode_lookup()
    decode_pack()


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
