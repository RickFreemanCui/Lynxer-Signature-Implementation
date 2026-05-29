#!/usr/bin/env python3
import argparse
from math import ceil

def round_to_byte (n):
    return 8 * ceil(n/8)

# A generic function to estimate the size of VOLEitH signatures
def voleith_sig_size (csp, ell, Topen, tau, deg, n_leaf_com = 2):
    B = 16 # UHF Parameter
    ell_hat = round_to_byte(ell) + csp * (deg - 1) + csp + B
    size_ci = round_to_byte(ell_hat) * (tau - 1) 
    size_utilde = csp + B
    size_d = round_to_byte(ell)
    size_atilde = csp * (deg - 1)
    size_pdecom = Topen * csp + tau * n_leaf_com * csp
    size_chall3 = csp
    size_iv = 128 # AES initial block
    size_ctr = 32 # For rejection sampling
    return size_ci + size_utilde + size_d + size_atilde \
        + size_pdecom + size_chall3 + size_iv + size_ctr

def main():
    parser = argparse.ArgumentParser(description='Compute VOLEitH signature size in bytes.')
    parser.add_argument('--lambda', dest='lam', type=int)
    parser.add_argument('--ell', type=int)
    parser.add_argument('--t-open', dest='t_open', type=int)
    parser.add_argument('--tau', type=int)
    parser.add_argument('--deg', type=int, default=2)
    parser.add_argument('--n-leaf-com', type=int, default=2)
    args = parser.parse_args()

    if None not in (args.lam, args.ell, args.t_open, args.tau):
        size_bits = voleith_sig_size(args.lam, args.ell, args.t_open, args.tau, args.deg,
                                     args.n_leaf_com)
        print(size_bits // 8)
        return

    defaults = [
        (128, 128 * 3, 103, 11),
        (192, 192 * 3, 162, 16),
        (256, 256 * 3, 218, 22),
        (384, 384 * 3, 336, 32),
        (512, 512 * 3, 456, 42),
        (128, 128 * 3, 112, 16),
        (192, 192 * 3, 176, 24),
        (256, 256 * 3, 234, 32),
        (384, 384 * 3, 332, 48),
        (512, 512 * 3, 445, 64),
    ]
    for lam, ell, t_open, tau in defaults:
        print(voleith_sig_size(lam, ell, t_open, tau, args.deg, args.n_leaf_com) // 8)

if __name__ == "__main__":
    main()
