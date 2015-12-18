import math

count = 512

for i in range(count) :
	value = round(2**15 * math.sin(i*2.0*math.pi/count))
	if value > 2**15-1 : value = 2**15-1
	if value < -2**15 : value = 2**15
	print(str(int(value)) + ", ")