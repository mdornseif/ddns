#!/usr/bin/python

"""
An implementation of Cipher Block Chaining mode with CipherText Stealing

This implementation is designed to have as few quirks as possible. 
The following were unavoidable -

strings of an even multiple of block size don't have any stealing done. This 
seems more reasonable than always doing it.

strings less than a block size are xored with the encryption of the iv. This 
breaks the property that it's impossible for an interloper to garble less than
a full block, but the alternative is to not support strings that small.

This last one highlights an important point about cbccts - it does *not* 
detect modification, merely makes the minimum amount of garbling possible 
'reasonably' large, which might or might not make sense for any given 
application. There are many ways modification can be detected, but they 
result in more message padding and aren't nearly as standard.

As with all implementations of cbc, it's important that the same iv never be
used twice. This can be accomplished either by carefully keeping track of 
the last used iv and incrementing, or by using a random iv and not encrypting 
more than 2 ** (blocksize / 2) strings.

This is the list of caveats for the simplest, most standard, and most 
well-behaved of the encryption modes.
"""

# by Bram Cohen, bram@gawth.com
# this file is public domain

from xor import xor
from cStringIO import StringIO
import string

def encrypt(encrypt_func, block_size, iv, s):
    return cbccts(encrypt_func, None, block_size).encrypt(iv, s)

def decrypt(encrypt_func, decrypt_func, block_size, iv, s):
    return cbccts(encrypt_func, decrypt_func, block_size).decrypt(iv, s)

class cbccts:
    def __init__(self, encrypt_func, decrypt_func, block_size):
        self.encrypt_func = encrypt_func
        self.decrypt_func = decrypt_func
        self.block_size = block_size

    def encrypt(self, iv, plaintext):
        bs = self.block_size
        lp = len(plaintext)
        last = self.encrypt_func(iv)
        if lp < bs:
            return xor(last[:lp], plaintext)
        r = StringIO()
        m = len(plaintext) % bs
        if m == 0:
            for i in xrange(0, lp, bs):
                last = self.encrypt_func(xor(last, plaintext[i:i + bs]))
                r.write(last)
        else:
            for i in xrange(0, lp - bs - m, bs):
                last = self.encrypt_func(xor(last, plaintext[i:i + bs]))
                r.write(last)
            cn = self.encrypt_func(xor(last, plaintext[-bs-m:-m]))
            r.write(self.encrypt_func(xor(cn, plaintext[-m:] + ('\000' * (bs - m)))))
            r.write(cn[:m])
        return r.getvalue()

    def decrypt(self, iv, ciphertext):
        bs = self.block_size
        lc = len(ciphertext)
        last = self.encrypt_func(iv)
        if lc < bs:
            return xor(last[:lc], ciphertext)
        r = StringIO()
        m = len(ciphertext) % bs
        if m == 0:
            for i in xrange(0, lc, bs):
                nl = ciphertext[i:i + bs]
                r.write(xor(last, self.decrypt_func(nl)))
                last = nl
        else:
            for i in xrange(0, lc - bs - m, bs):
                nl = ciphertext[i:i + bs]
                r.write(xor(last, self.decrypt_func(nl)))
                last = nl
            pn = xor(self.decrypt_func(ciphertext[-bs-m:-m]), ciphertext[-m:] + ('\000' * (bs - m)))
            r.write(xor(last, self.decrypt_func(ciphertext[-m:] + pn[m:])))
            r.write(pn[:m])
        return r.getvalue()

def dummyencrypt(s):
    assert len(s) == 7
    return string.join([chr((ord(s[i]) + 3 + i) % 256) for i in xrange(len(s))], '')
def dummydecrypt(s):
    assert len(s) == 7
    return string.join([chr((ord(s[i]) - 3 - i) % 256) for i in xrange(len(s))], '')

def test():
    def t(s):
        assert decrypt(dummyencrypt, dummydecrypt, 7, 'abcdefg', encrypt(dummyencrypt, 7, 'abcdefg', s)) == s
    t('')
    t('a')
    t('abcabc')
    t('pqrstuv')
    t('pqrstuv' * 3)
    t('abc' * 3)
    t('abc' * 50)



