#Script to convert VRAM to picture
from PIL import Image
data = []
rratio = 8

with open("pic.dat",'rb') as f:
	for i in f.read():
		for i2 in '{0:08b}'.format(ord(i)):
			data.append((1-int(i2))*256)

img = Image.new('1', (128, 64))
img.putdata(data)
img = img.resize((128*rratio,64*rratio))
img.save('pic.png')
img.show()