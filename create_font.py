from PIL import Image

img_src = 'power_button.jpg'
outfile = 'PowerButton.c'
font_desc = 'Poqwer button'
chars = ['A']
font_width = 48
width_in_img = 48
font_height = 48


output = {}
hex_out = {}

im = Image.open(img_src)

x_pos = 0

for char in chars:
    output[char] = []
    for y in range(font_height):
        for x in range(x_pos, x_pos + font_width):
            r, g, b = im.getpixel((x, y))
            if r > 100:
                output[char].append(0)
            else:
                output[char].append(1)
    x_pos += width_in_img

im.close()

for char in chars:
    hex_out[char] = []
    i = 0
    for y in range(font_height):
        for x in range(font_width // 8):
            byte = ''
            for j in range(8):
                byte += str(output[char][i])
                i += 1
            hex_out[char].append(hex(int(byte, 2)).upper().replace('X', 'x'))

for char in chars:
    for i in range(len(hex_out[char])):
        if len(hex_out[char][i]) == 3:
            hex_out[char][i] = hex_out[char][i].replace('x', 'x0')

with open(outfile, 'w') as f:
    f.write('// ' + outfile + '\r\n')
    f.write('// Font type\t\t: ' + font_desc + '(' + str(len(chars)))
    f.write(' characters)\r\n')
    f.write('// Font size\t\t: ' + str(font_width) + 'x' + str(font_height))
    f.write(' pixels\r\n')
    mem_usage = str((((font_width // 8) * font_height) * len(chars)) + 4)
    f.write('// Memory usage\t\t: ' + mem_usage + ' bytes\r\n\r\n')
    f.write('#if defined(__AVR__)\r\n\t#include <avr/pgmspace.h>\r\n\t')
    f.write('#define fontdatatype const uint8_t\r\n')
    f.write('#elif defined(__PIC32MX__)\r\n\t#define PROGMEM\r\n\t')
    f.write('#define fontdatatype const unsigned char\r\n')
    f.write('#elif defined(__arm__)\r\n\t#define PROGMEM\r\n\t')
    f.write('#define fontdatatype const unsigned char\r\n#endif\r\n\r\n')
    f.write('fontdatatype ' + outfile[:-2] + '[' + mem_usage + ']')
    f.write(' PROGMEM={\r\n')
    hex_w = hex(font_width).upper().replace('X', 'x')
    hex_h = hex(font_height).upper().replace('X', 'x')
    hex_strt = hex(ord(chars[0])).upper().replace('X', 'x')
    hex_cnt = hex(len(chars)).upper().replace('X', 'x')
    preamble = [hex_w, hex_h, hex_strt, hex_cnt]
    for i, elem in enumerate(preamble):
        if len(elem) == 3:
            preamble[i] = elem.replace('x', 'x0')
    f.write(','.join(preamble) + ',\r\n')

    for char in chars:
        char_string = ','.join(hex_out[char])
        char_string += ',  // ' + char + '\r\n'
        f.write(char_string)

    f.write('};\r\n')
