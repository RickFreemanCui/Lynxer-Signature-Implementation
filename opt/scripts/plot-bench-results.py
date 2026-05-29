#!/usr/bin/env python3

import csv
import sys
from pathlib import Path


SEC_LEVELS = [128, 192, 256, 384, 512]
SCHEMES = [
    ("faest", "FAEST", "#4E79A7"),
    ("faest-em", "FAEST-EM", "#59A14F"),
    ("lynx", "Lynx", "#E15759"),
    ("aimer", "AIMer", "#F28E2B"),
    ("rainier-3", "Rainier-3", "#B07AA1"),
    ("rainier-4", "Rainier-4", "#76B7B2"),
]
VARIANT_KINDS = [
    ("s", "Small", 0.0),
    ("f", "Fast", 0.35),
]
METRICS = [
    ("keygen", "Keygen"),
    ("sign", "Sign"),
    ("verify", "Verify"),
]


def parse_variant(variant: str):
    if variant.startswith("v2::faest_em_"):
        scheme = "faest-em"
    elif variant.startswith("v2::faest_"):
        scheme = "faest"
    elif variant.startswith("lynx::lynx_"):
        scheme = "lynx"
    else:
        return None

    secpar = None
    for token in variant.replace("::", "_").split("_"):
        if token.isdigit():
            secpar = int(token)
            break
    if secpar is None:
        return None
    variant_kind = variant.rsplit("_", 1)[-1]
    if variant_kind not in {"s", "f"}:
        variant_kind = "s"
    return scheme, secpar, variant_kind


def parse_time_us(raw: str) -> float:
    value_str, unit = raw.strip().split()
    value = float(value_str)
    if unit == "ns":
        return value / 1_000.0
    if unit == "us":
        return value
    if unit == "ms":
        return value * 1_000.0
    if unit == "s":
        return value * 1_000_000.0
    raise ValueError(f"unsupported time unit: {raw}")


def load_rows(csv_path: Path):
    table = {}
    with csv_path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        fields = reader.fieldnames or []

        # Detect format: raw CSV has "variant"; summary CSV has "scheme" and *_us columns.
        if "variant" in fields:
            for row in reader:
                parsed = parse_variant(row["variant"])
                if parsed is None:
                    continue
                scheme, secpar, variant_kind = parsed
                table[(scheme, secpar, variant_kind)] = {
                    "pk_bytes": row["pk_bytes"],
                    "sk_bytes": row["sk_bytes"],
                    "sig_bytes": row["sig_bytes"],
                    "keygen": parse_time_us(row["keygen"]),
                    "sign": parse_time_us(row["sign"]),
                    "verify": parse_time_us(row["verify"]),
                }
        else:
            for row in reader:
                scheme = row["scheme"]
                secpar = int(row["secpar"])
                variant_kind = row.get("variant_kind", "s") or "s"
                table[(scheme, secpar, variant_kind)] = {
                    "pk_bytes": row["pk_bytes"],
                    "sk_bytes": row["sk_bytes"],
                    "sig_bytes": row["sig_bytes"],
                    "keygen": float(row["keygen_us"]),
                    "sign": float(row["sign_us"]),
                    "verify": float(row["verify_us"]),
                }
    return table


def svg_text(x, y, text, size=12, anchor="middle", weight="normal"):
    return (
        f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{weight}" '
        f'font-family="Arial, sans-serif" text-anchor="{anchor}">{text}</text>'
    )


def format_ms(us_val, suffix=False):
    ms = us_val / 1000.0
    if ms == 0:
        s = "0"
    elif ms >= 100:
        s = f"{ms:.0f}"
    elif ms >= 10:
        s = f"{ms:.0f}"
    elif ms >= 1:
        s = f"{ms:.1f}"
    else:
        s = f"{ms:.2f}"
    return s + "ms" if suffix else s


def mix_with_white(hex_color: str, amount: float) -> str:
    if amount <= 0:
        return hex_color
    color = hex_color.lstrip("#")
    red = int(color[0:2], 16)
    green = int(color[2:4], 16)
    blue = int(color[4:6], 16)
    red = round(red + (255 - red) * amount)
    green = round(green + (255 - green) * amount)
    blue = round(blue + (255 - blue) * amount)
    return f"#{red:02X}{green:02X}{blue:02X}"


def write_svg(metric_key: str, metric_label: str, rows, output_path: Path):
    # Determine which schemes and security levels have data
    active_series = [
        (scheme_key, scheme_label, variant_kind, variant_label, mix_with_white(color, lighten))
        for scheme_key, scheme_label, color in SCHEMES
        for variant_kind, variant_label, lighten in VARIANT_KINDS
        if any((scheme_key, sp, variant_kind) in rows for sp in SEC_LEVELS)
    ]
    active_secpars = [
        sp for sp in SEC_LEVELS if any((scheme_key, sp, variant_kind) in rows for scheme_key, _, variant_kind, _, _ in active_series)
    ]
    n_series = len(active_series)
    n_groups = len(active_secpars)

    width = 1200
    height = 640
    margin_left = 90
    margin_right = 30
    margin_top = 70
    margin_bottom = 130

    plot_w = width - margin_left - margin_right
    plot_h = height - margin_top - margin_bottom
    x0 = margin_left
    y0 = margin_top

    values = [rows[(scheme, secpar, variant_kind)][metric_key]
              for scheme, _, variant_kind, _, _ in active_series
              for secpar in active_secpars
              if (scheme, secpar, variant_kind) in rows]
    y_max = max(values) * 1.15 if values else 1.0
    y_max = max(y_max, 1.0)

    group_w = plot_w / n_groups
    bar_w = group_w * 0.76 / n_series
    gap_w = group_w * 0.03
    total_bar_w = n_series * bar_w + (n_series - 1) * gap_w

    parts = []
    parts.append(
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
        f'viewBox="0 0 {width} {height}">'
    )
    parts.append('<rect x="0" y="0" width="100%" height="100%" fill="white"/>')
    parts.append(svg_text(width / 2, 36, f"{metric_label} Time by Security Level", 22, weight="bold"))

    parts.append(f'<line x1="{x0}" y1="{y0 + plot_h}" x2="{x0 + plot_w}" y2="{y0 + plot_h}" stroke="black"/>')
    parts.append(f'<line x1="{x0}" y1="{y0}" x2="{x0}" y2="{y0 + plot_h}" stroke="black"/>')

    # Y-axis ticks and gridlines
    for i in range(6):
        val = y_max * i / 5
        yy = y0 + plot_h - (val / y_max) * plot_h
        parts.append(f'<line x1="{x0 - 5}" y1="{yy:.2f}" x2="{x0}" y2="{yy:.2f}" stroke="black"/>')
        if i > 0:
            parts.append(
                f'<line x1="{x0}" y1="{yy:.2f}" x2="{x0 + plot_w}" y2="{yy:.2f}" '
                f'stroke="#e0e0e0" stroke-dasharray="4,3"/>'
            )
        parts.append(svg_text(x0 - 10, yy + 4, format_ms(val), 11, "end"))

    parts.append(
        svg_text(24, y0 + plot_h / 2, "Time (ms)", 13).replace(
            'x="24"', f'x="24" transform="rotate(-90 24 {y0 + plot_h / 2})"'
        )
    )

    # Bars
    for group_index, secpar in enumerate(active_secpars):
        gx = x0 + group_index * group_w + group_w / 2
        start_x = gx - total_bar_w / 2
        labels = []
        for scheme_index, (scheme_key, _scheme_label, variant_kind, _variant_label, color) in enumerate(active_series):
            key = (scheme_key, secpar, variant_kind)
            if key not in rows:
                continue
            bar_x = start_x + scheme_index * (bar_w + gap_w)
            value = rows[key][metric_key]
            bar_h = (value / y_max) * plot_h
            bar_y = y0 + plot_h - bar_h
            parts.append(
                f'<rect x="{bar_x:.2f}" y="{bar_y:.2f}" width="{bar_w:.2f}" '
                f'height="{bar_h:.2f}" fill="{color}"/>'
            )
            labels.append([bar_x + bar_w / 2, bar_y - 5, format_ms(value, suffix=True)])

        # De-overlap labels: sort by y descending (bottom first), enforce min gap
        labels.sort(key=lambda l: -l[1])
        min_label_gap = 12
        for i in range(1, len(labels)):
            if labels[i - 1][1] - labels[i][1] < min_label_gap:
                labels[i][1] = labels[i - 1][1] - min_label_gap
        for lbl in labels:
            lbl[1] = max(y0 - 5, lbl[1])
            parts.append(svg_text(lbl[0], lbl[1], lbl[2], 8))

        parts.append(svg_text(gx, y0 + plot_h + 24, str(secpar), 12))

    parts.append(svg_text(x0 + plot_w / 2, height - 28, "Security Parameter", 13))

    # Legend
    legend_cols = 2
    legend_rows = (n_series + legend_cols - 1) // legend_cols
    legend_x = margin_left + 10
    legend_y = 68
    col_w = 190
    for idx, (_scheme_key, scheme_label, _variant_kind, variant_label, color) in enumerate(active_series):
        col = idx // legend_rows
        row = idx % legend_rows
        lx = legend_x + col * col_w
        ly = legend_y + row * 22
        parts.append(f'<rect x="{lx}" y="{ly}" width="16" height="11" fill="{color}"/>')
        parts.append(svg_text(lx + 24, ly + 10, f"{scheme_label} {variant_label}", 11, "start"))

    parts.append("</svg>")
    output_path.write_text("\n".join(parts), encoding="utf-8")


def main():
    if len(sys.argv) != 3:
        print("usage: plot-bench-results.py <input.csv> <output_dir>", file=sys.stderr)
        return 1

    csv_path = Path(sys.argv[1]).resolve()
    output_stem = Path(sys.argv[2]).resolve()
    rows = load_rows(csv_path)

    for metric_key, metric_label in METRICS:
        suffix = metric_key
        output_path = output_stem.parent / f"{output_stem.name}_{suffix}.svg"
        write_svg(metric_key, metric_label, rows, output_path)
        print(output_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
