# coding: utf8
"""

    Securely hash and check passwords using PBKDF2.

    Use random salts to protect againt rainbow tables, many iterations against
    brute-force, and constant-time comparaison againt timing attacks.

    Keep parameters to the algorithm together with the hash so that we can
    change the parameters and keep older hashes working.

    See more details at http://exyr.org/2011/hashing-passwords/

    Author: Simon Sapin
    License: BSD
    
    Patched by Jeff Keacher to work better with mosquitto-auth-plug on Python 3.4

"""

from base64 import b64encode, b64decode
import hashlib
from os import urandom

from gracopumpapp.libs.mosquitto_auth_plug.pbkdf2 import pbkdf2_bin


# From https://github.com/mitsuhiko/python-pbkdf2
# Parameters to PBKDF2. Only affect new passwords.
SALT_LENGTH = 12
KEY_LENGTH = 24
HASH_FUNCTION = 'sha256'  # Must be in hashlib.
# Linear to the hashing time. Adjust to be high but take a reasonable
# amount of time on your server. Measure with:
# python -m timeit -s 'import passwords as p' 'p.make_hash("something")'
COST_FACTOR = 10000


def make_hash(password):
    """Generate a random salt and return a new hash for the password."""
    password = password.encode('utf-8')
    salt = b64encode(urandom(SALT_LENGTH)).decode('ascii')
    hashed = b64encode(pbkdf2_bin(password, salt, COST_FACTOR, KEY_LENGTH,
             getattr(hashlib, HASH_FUNCTION))).decode()
    return 'PBKDF2${}${}${}${}'.format(
        HASH_FUNCTION,
        COST_FACTOR,
        salt,
        hashed)


def check_hash(password, hash_):
    """Check a password against an existing hash."""
    password = password.encode('utf-8')
    algorithm, hash_function, cost_factor, salt, hash_a = hash_.split('$')
    assert algorithm == 'PBKDF2'
    hash_a = b64decode(hash_a)
    hash_b = pbkdf2_bin(password, salt, int(cost_factor), len(hash_a),
                        getattr(hashlib, hash_function))
    assert len(hash_a) == len(hash_b)  # we requested this from pbkdf2_bin()
    # Same as "return hash_a == hash_b" but takes a constant time.
    # See http://carlos.bueno.org/2011/10/timing.html
    diff = 0
    for char_a, char_b in zip(hash_a, hash_b):
        diff |= ord(char_a) ^ ord(char_b)
    return diff == 0
