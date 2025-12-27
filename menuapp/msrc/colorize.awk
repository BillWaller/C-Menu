#!/usr/bin/awk -f
# colorize.awk
# Bill Waller 2025
# an AWK script to colorize hex color codes in text output
function hex_to_dec(hex_str) {
    return strtonum("0x" hex_str)
}

function ansi_color(hex_color) {
    if (substr(hex_color, 1, 1) == "#") {
        hex_color = substr(hex_color, 2)
    }
    r = hex_to_dec(substr(hex_color, 1, 2))
    g = hex_to_dec(substr(hex_color, 3, 2))
    b = hex_to_dec(substr(hex_color, 5, 2))
    r /= 255 
    g /= 255
    b /= 255
    red = (r <= 0.0031308) ? (r * 12.92) : (1.055 * (r ^ (1 / 2.4)) - 0.055)
    green = (g <= 0.0031308) ? (g * 12.92) : (1.055 * (g ^ (1 / 2.4)) - 0.055)
    blue = (b <= 0.0031308) ? (b * 12.92) : (1.055 * (b ^ (1 / 2.4)) - 0.055)
    r = int(red * 255 + 0.5)
    g = int(green * 255 + 0.5)
    b = int(blue * 255 + 0.5)
    l = 0.2126 * r + 0.7152 * g + 0.0722 * b
    fg = (l > 128) ? "\x1b[38;2;0;0;0m" : "\x1b[38;2;255;255;255m"
    return sprintf("%s\x1b[48;2;%d;%d;%dm", fg, r, g, b)
}

function process_line() {
    line = $0
    if (match($0, /#[0-f]{6}/)) {
        color = substr(line, RSTART, RLENGTH) 
        ansi = ansi_color(color)
        p1 = substr(line, 1, RSTART - 1)
        p2 = substr(line, RSTART, RLENGTH)
        p3 = substr(line, RSTART + RLENGTH)
    }
}

BEGIN {
    ansioff = "\x1b[0m"
    while (getline > 0) {
        line = $0
        lineout = $0
        while (match(line, /#[0-f]{6}/)) {
            if (RLENGTH > 0) {
                color = substr(line, RSTART, RLENGTH)
                ansi = ansi_color(color)
                p1 = substr(line, 1, RSTART - 1)
                p2 = substr(line, RSTART, RLENGTH)
                p3 = substr(line, RSTART + RLENGTH)
                sprintf(lineout, "%s%s%s%s", p1, ansi, p2, ansioff)
                if (length(p3) > 0) {
                    line = p3
                    continue
                }
                break
            }
        }
        printf("%s\n", lineout)
    }
}
