import hashlib, os, subprocess, sys
from ecdsa import SigningKey, NIST256p
from ecdsa.util import sigencode_strings
N = NIST256p.order
# RFC 6979 A.2.5, P-256 / SHA-256
x = 'C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721'
vec = [(b'sample', 'EFD48B2AACB6A8FD1140DD9CD45E81D69D2C877B56AAF991C34D0EA84EAF3716',
                   'F7CB1C942D657C41D436C7A1B6E29F65F3E900DBB9AFF4064DC4AB2F843ACDA8'),
       (b'test',   'F1ABB023518351CD71D881567B1EA663ED3EFCF6C5132B354F28D3B0B7D38367',
                   '019F4113742A2B14BD25926B49C649155F267E60D3814B4C0CC84250E46F0083')]
cases = [(x, hashlib.sha256(m).hexdigest(), (r + s).lower()) for m, r, s in vec]
def ref(priv, h):
    sk = SigningKey.from_string(bytes.fromhex(priv), curve=NIST256p)
    r, s = sk.sign_digest_deterministic(bytes.fromhex(h), hashfunc=hashlib.sha256, sigencode=sigencode_strings)
    return (r + s).hex()
# random keys/hashes + hash >= n edge cases + tiny/huge keys
for i in range(500):
    p = int.from_bytes(os.urandom(32), 'big') % (N - 1) + 1
    cases.append(('%064x' % p, os.urandom(32).hex(), None))
for h in ('ff' * 32, '%064x' % N, '%064x' % (N + 1), '00' * 32):
    cases.append((x.lower(), h, None))
for p in (1, 2, N - 1):
    cases.append(('%064x' % p, hashlib.sha256(b'edge').hexdigest(), None))
cases = [(p, h, e if e else ref(p, h)) for p, h, e in cases]
inp = ''.join('%s %s\n' % (p, h) for p, h, _ in cases)
for be in sys.argv[1:]:
    out = subprocess.run([be], input=inp, capture_output=True, text=True).stdout.split()
    bad = [i for i, (c, o) in enumerate(zip(cases, out)) if c[2] != o] + list(range(len(out), len(cases)))
    print('%-10s RFC-Vektoren: %s   python-ecdsa (%d Faelle): %s' % (be.split('-')[-1],
          'OK' if not [b for b in bad if b < 2] else 'FEHLER',
          len(cases) - 2, 'OK' if not [b for b in bad if b >= 2] else 'FEHLER bei %s' % bad[:8]))
