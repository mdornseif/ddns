#!/usr/bin/python

import time
import random
from cPickle import dumps, loads
import zlib 


d = {'lat': 123.456, 'lon': '654.321', 'ip': '255.255.255.255', 't':
time.time(), 'rm': random.randint(0, 0xffff) & 0xc0de0000, }

print len(d)
print len(dumps(d, 1))
print len(zlib.compress(dumps(d, 1)))


from BTcrypto.rijndael import rijndael 
r = rijndael("X" * 32, block_size = 16) 
 
ciphertext = r.encrypt("A"*16) 
plaintext = r.decrypt(ciphertext) 
print plaintext

from BTcrypto.cbccts import encrypt, decrypt

ciphertext = encrypt(r.encrypt, 16, '0123456789abcdef', zlib.compress(dumps(d, 1)))
print len(ciphertext)
plaintext = decrypt(r.encrypt, r.decrypt, 16, '0123456789abcdef', ciphertext) 
print loads(zlib.decompress(plaintext))

import struct

def encapsulate(user, key, obj):
    iv = "%s%07x%s" % ("c", user, str(time.time())[-7:])
    print iv
    print repr(struct.pack(">cL7sI", "c", user,  struct.pack("<d", time.time()), random.randint(0, 0xffff)))

encapsulate(1, "X" * 16, d)
