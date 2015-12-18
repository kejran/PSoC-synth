import math

a=440
samplerate = 50*1000

for i in range(128) :
	value = (a/32.0) * (2.0**((i-9.0)/12.0)) / (samplerate) * 2**32
	print(str(int(value)) + ",")