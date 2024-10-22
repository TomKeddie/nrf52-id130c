# converts a PNG to an RBG565 for display on ID130C band
# must be resized to 80 colums beforehand
import png
import sys

fhandle = open("crowd-supply-icon.png", "rb")
reader = png.Reader(file=fhandle)
png = reader.read()

png_width = png[0]
png_height = png[1]
png_rows = png[2]
png_info = png[3]

# print("Found image of {}x{}".format(png_width, png_height))
if png_info['alpha']:
    print("Please remove alpha")
    sys.exit(-1)

print("static const uint8_t image[]= {")
for row in png_rows:
    if len(row) != 240:
        print("Expect RGB image")
        sys.exit(-1)
    for idx in range(0, 239, 3):
        red=int(row[idx])
        green=int(row[idx+1])
        blue=int(row[idx+2])
        brg565=((red >> 3) << 11) | ((blue >> 2) << 5) | (green >> 3)
        #print("{:05b}:{:06b}:{:05b} -> {:016b}".format(blue>>3, red>>2, green>>3, brg565))
        print("0x{:02x},0x{:02x}, ".format(brg565>>8, brg565&0xff), end="")
    print("")

print("};")
