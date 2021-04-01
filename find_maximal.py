import sympy

from lfsr_lab_lfsr import *

poly = [5, 2, 0]
ifill = 1

# Basic Fibonacci LFSR
for i in range(10):
    seq, fill = ssrg(num=i, poly=poly, ifill=ifill)
    print(f"{fill:05b}")

# Try jump ahead
fill9 = ssrg(num=9, poly=poly, ifill=ifill)[1]
print(f"fill9={fill9:05b}")  # 00110

# Do jump aheads instead of just regularly advancing the LFSR
# These match as expected
for i in range(10):
    seq, expected_fill = ssrg(num=i, poly=poly, ifill=ifill)
    fill, Ts = ssgs_jump(jump=i, poly=poly, ifill=ifill)
    # print(f"{fill:05b}")
    assert fill == expected_fill


######################


def bit_reverse(val, nbits=16):
    bv = bin(val)[2:]
    bv = bv.rjust(nbits, "0")
    bv = bv[::-1]
    return int(bv, 2)


def taps_reverse(taps):
    return [taps[0] - t for t in taps[::-1]]

# galois_lfsr_print(0x2DUL, 16UL, 0xACE1UL, 10); ->
#   1, 0, 1, 0, 1, 1, 0, 0, 1, 1,
#   0xCF87

# 0x1002D reversed -> 10110100000000001
poly = [16, 14, 13, 11, 0]
ifill = bit_reverse(0xACE1, 16)

print(msrg(num=10, poly=poly, ifill=ifill))

expected_fill = bit_reverse(0xCF87)
print("expected end fill:", expected_fill)

# Good, now I can convert between my GLFSR implementation and theirs


fill, Ts = ssgm_jump(jump=10, poly=poly, ifill=ifill)
assert fill == expected_fill

# Good, now I can advance my GLFSR by an arbitrary amount

def num_to_taps(val):
    taps = []
    pos = 0
    while val:
        if val & 1:
            taps.append(pos)
        val >>= 1
        pos += 1
    return taps[::-1]

def taps_to_num(taps):
    num = 0
    for t in taps:
        num |= 1<<t
    return num


seq, fill = msrg(num=2**16 - 1, poly=poly, ifill=ifill)
assert fill == ifill


def check_if_maximal(intpoly, nbits, verbose=True):
    if verbose:
        print("Trying", intpoly)
    start_state = 1
    bitrev = bit_reverse(intpoly, nbits+1)
    taps = num_to_taps(bitrev)
    maximal_period = 2**nbits - 1
    state, _ = ssgm_jump(jump=maximal_period, poly=taps, ifill=start_state)
    if state != start_state:
        if verbose:
            print(f"{state} != {start_state}")
        return False
    if not sympy.isprime(maximal_period):
        if verbose:
            print("Non-prime maximal(?) period")
        factors = sympy.ntheory.factorint(maximal_period).keys()
        for factor in factors:
            jump = maximal_period // factor
            state, _ = ssgm_jump(jump=jump, poly=taps, ifill=start_state)
            if state == start_state:
                if verbose:
                    print("Non-maximal, cycle after", jump)
                return False

    if verbose:
        print("Maximal")
    return True

# maximals = []
# for i in range(1, 256, 2):
#     val = (1 << 16) | i
#     if check_if_maximal(val, 16):
#         maximals.append(i)
# print(maximals)

# Wew! verified against a brute force search for 16-bit


# Low-byte only polynomials
for nbits in range(16, 257, 8):
    maximals = []
    for i in range(1, 256, 2):
        val = (1 << nbits) | i
        if check_if_maximal(val, nbits, verbose=False):
            maximals.append(i)
    print(f"{nbits}: maximals = {maximals}")


# for nbits in range(40, 257, 8):
#     maximals = []
#     for i in range(1, 256, 2):
#         val = (1 << nbits) | (i<<(nbits-8)) | 1
#         if check_if_maximal(val, nbits, verbose=False):
#             maximals.append(i)
#     print(f"{nbits}: maximals = {maximals}")
