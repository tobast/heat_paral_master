#!/usr/bin/python3
#### USES PYTHON 3
from math import *

SIZE = 500
SCALE= 1

print(SIZE,SCALE)

for i in range(SIZE):
	for j in range(SIZE):
		if(i==0 or i==SIZE-1):
			print(0x8000+1000, end=' ') # 0x8000 : fixed value!
		elif(j==0 or j==SIZE-1):
			print(0x8000+15000,end=' ') # Idem
		else:
			val = abs((sin(2*pi*i/SIZE) + cos(3*pi*j/SIZE))) * 7000
			# <val> in range [0,16000]
			print(int(val), end=' ')
	print('')

