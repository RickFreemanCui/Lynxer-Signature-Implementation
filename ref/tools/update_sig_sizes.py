#!/usr/bin/env python3
"""Update SIG_SIZE values in meson.build for all Lynx parameter sets.

Reads LAMBDA, ELL, T_OPEN, TAU from each param_lynx_* block in meson.build,
computes the correct signature size using voleith_sig_size from sig_size.py,
and updates the SIG_SIZE lines in place.

Usage:
    python tools/update_sig_sizes.py          # default deg=2, n_leaf_com=2
    python tools/update_sig_sizes.py --deg 2  # override degree
"""
import argparse
import re
import sys
import os

sys.path.insert(0, os.path.dirname(__file__))
from sig_size import voleith_sig_size

MESON_BUILD = os.path.join(os.path.dirname(__file__), '..', 'meson.build')

def parse_lynx_params(text):
    """Extract Lynx parameter blocks from meson.build text."""
    # Match blocks like: param_lynx_128s.set('KEY', VALUE)
    block_re = re.compile(r"param_(lynx_\d+[sf])\.set\('(\w+)',\s*(.+?)\)")
    params = {}
    for m in block_re.finditer(text):
        name, key, val = m.group(1), m.group(2), m.group(3).strip()
        if name not in params:
            params[name] = {}
        # Evaluate simple expressions like "384 * 3"
        try:
            params[name][key] = int(eval(val))
        except:
            params[name][key] = val
    return params

def main():
    parser = argparse.ArgumentParser(description='Update Lynx SIG_SIZE in meson.build')
    parser.add_argument('--deg', type=int, default=2,
                        help='ZK hash degree (default: 2)')
    parser.add_argument('--n-leaf-com', type=int, default=2,
                        help='Number of leaf commitments (default: 2)')
    parser.add_argument('--dry-run', action='store_true',
                        help='Print computed sizes without modifying meson.build')
    args = parser.parse_args()

    with open(MESON_BUILD, 'r') as f:
        text = f.read()

    params = parse_lynx_params(text)
    updated = text

    for name in sorted(params):
        p = params[name]
        lam = p['LAMBDA']
        ell = p['ELL']
        tau = p['TAU']
        t_open = p['T_OPEN']
        old_sig = p['SIG_SIZE']

        sig_bits = voleith_sig_size(lam, ell, t_open, tau, args.deg, args.n_leaf_com)
        new_sig = int(sig_bits / 8)

        status = '' if old_sig == new_sig else '  <- CHANGED'
        print(f'{name}: SIG_SIZE {old_sig} -> {new_sig}{status}')

        if not args.dry_run:
            pattern = rf"(param_{name}\.set\('SIG_SIZE',\s*)\d+(\))"
            updated = re.sub(pattern, rf'\g<1>{new_sig}\2', updated)

    if not args.dry_run and updated != text:
        with open(MESON_BUILD, 'w') as f:
            f.write(updated)
        print('\nmeson.build updated.')
    elif not args.dry_run:
        print('\nNo changes needed.')

if __name__ == '__main__':
    main()
