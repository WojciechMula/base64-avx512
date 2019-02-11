#!/usr/bin/env python3

import argparse
import base64
import hashlib
from os.path import basename


EMAIL_HEADER="""--%(id)s
Content-Type: mime/type
Content-Disposition: attachment; filename="%(name)s"
Content-Transfer-Encoding: base64

"""

EMAIL_FOOTER="""
--%(id)s--
"""


class Application(object):
    def __init__(self, options):
        self.options = options


    def run(self):

        def md5(s):
            h = hashlib.new('md5')
            h.update(s)
            return h.hexdigest()

        with open(self.options.output, 'w', encoding='utf-8') as out:
            for index, path in enumerate(self.options.files):
                params = {
                    'id'    : md5(bytes(path, 'utf-8')),
                    'name'  : basename(path),
                }

                if index > 0:
                    out.write('\n')

                out.write(EMAIL_HEADER % params)
                self.__encode_base64(out, path)
                out.write(EMAIL_FOOTER % params)


    def __encode_base64(self, out, path):
        with open(path, 'rb') as f:
            bin = f.read()
            enc = str(base64.standard_b64encode(bin), 'ascii')

            n = self.options.maxlength
            for i in range(0, len(enc), n):
                line = enc[i:i+n]
                out.write(line)
                out.write('\n')


def get_options():
    parser = argparse.ArgumentParser(description="Prepare pseudo-email messages with base64-encoded attachments")
    parser.add_argument("-l", dest='maxlength', metavar="LINELEN", default=76,
                        help="maximum length of line")
    parser.add_argument("-o", dest='output', metavar="FILE", type=str, required=True,
                        help="output file")
    parser.add_argument("files", nargs='+',
                        help="input file(s)")
    
    return parser.parse_args()


def main():
    options = get_options()
    app = Application(options)
    app.run()


if __name__ == '__main__':
    main()
