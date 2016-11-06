import os
import os.path
import re
import PIL.Image

def generate_chrdata(w, h, data):
    result = []
    for y in range(0, h, 8):
        for x in range(0, w, 8):
            for j in range(8):
                # Write the low bits of this column.
                c = 0
                for i in range(8):
                    c = (c << 1) | (data[x + j, y + (7 - i)] & 1)
                result.append(c)
    for y in range(0, h, 8):
        for x in range(0, w, 8):
            for j in range(8):
                # Write the high bits of this column.
                c = 0
                for i in range(8):
                    c = (c << 1) | ((data[x + j, y + (7 - i)] & 2) >> 1)
                result.append(c)                
    return result

if __name__ == '__main__':
    import sys

    if len(sys.argv) > 1:
        WIDTH = 128
        HEIGHT = None
        for arg in range(1, len(sys.argv)):
            filename = sys.argv[arg]
            
            # Reserved for commandline arguments
            if filename[0] == '-':
                exit('Invalid argument `' + filename + '`.')
                continue
            
            try:
                img = PIL.Image.open(filename)
            except IOError as e:
                if os.path.isdir(filename):
                    exit(filename + ' is a directory.')
                if os.path.exists(filename):
                    exit(filename + ' has an unsupported filetype, or you lack permission to open it.')
                else:
                    exit('File ' + filename + ' does not exist!')
                
            w, h = img.size
            if w != WIDTH or h % 8 != 0 or HEIGHT and h != HEIGHT:
                exit('Image ' + filename + ' is not ' + str(WIDTH) + 'x' + str(HEIGHT) + ' pixels in size.')
            if not img.palette:
                exit('Image ' + filename + ' has no palette.')
            data = img.load()
            
            chrdata = generate_chrdata(w, h, data)

            header_filename = os.path.splitext(filename)[0] + '_bitmap.h'
            try:
                f = open(header_filename, 'w')
            except Exception as e:
                exit('Failure attempting to write ' + header_filename)
            
            identifier = os.path.basename(os.path.splitext(filename)[0])
            camel_identifier = ''.join(x for x in re.sub('[^0-9a-zA-Z]+', ' ', identifier).title().split())
            lower_camel_identifier = camel_identifier[:1].lower() + camel_identifier[1:] if camel_identifier else ''

            f.write('#ifndef ' + identifier.upper() + '_BITMAP_H\n')
            f.write('#define ' + identifier.upper() + '_BITMAP_H\n\n')
            f.write('#include <avr/pgmspace.h>\n')
            f.write('#include <stdint.h>\n\n')
            f.write('#define ' + identifier.upper() + '_BITMAP_LEN ' + str(len(chrdata)) + '\n')
            f.write('extern const uint8_t ' + lower_camel_identifier + 'Bitmap[' + identifier.upper() + '_BITMAP_LEN] PROGMEM;\n')
            f.write('#endif\n')
            f.close()
            print('  ' + filename + ' -> ' + header_filename)

            body_filename = os.path.splitext(filename)[0] + '_bitmap.cpp'
            try:
                f = open(body_filename, 'w')
            except Exception as e:
                exit('Failure attempting to write ' + body_filename)
            
            f.write('#include "' + header_filename + '"\n\n')
            f.write('const uint8_t ' + lower_camel_identifier + 'Bitmap[' + identifier.upper() + '_BITMAP_LEN] PROGMEM = {')
            for i, v in enumerate(chrdata):
                f.write(('\n    ' if i % 8 == 0 else ' ') + '0x{:02X}'.format(v) + ',')
            f.write('\n};\n')
            f.close()
            print('  ' + filename + ' -> ' + body_filename)

    else:
        print('Usage: ' + sys.argv[0] + ' file [file...]')
        print('Converts files like foo.png into Arduboy-friendly formats like foo.h/foo.cpp')
