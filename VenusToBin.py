# Converts hexdump from http://www.kvakil.me/venus/
# into a binary file, suitable for running on ISA simulators

import sys
import os
import binascii
import re

if len(sys.argv) != 2:
    print('usage: $pyexe VenusToBin.py hexfile.txt')
    quit()

try:
    file = open(sys.argv[1],'r')
except IOError:
    print('Invalid file')
    quit()

out = open(os.path.splitext(sys.argv[1])[0] + '.bin', 'wb')

# Decode file
fileContent = file.readlines()
dataString = ''
for line in fileContent:
    # Strip line of endline characters etc., reverse line and swap every 2nd character
    dataString = dataString + re.sub(r'(.)(.)', r'\2\1', line.rstrip()[::-1])
out.write(bytearray.fromhex(dataString))