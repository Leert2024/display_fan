# -*- coding: utf-8 -*-
"""把 pattern.png 转换为 LED 点阵数组，并输出可视化图片。

圆被 divide_num 条半径等分，每条半径上有 led_num 个 LED（含圆心）。
数组大小为 divide_num * led_num * 3，排列为 [半径序号][LED 序号][RGB]，
从 x 轴开始逆时针填充，输出到 pattern.c。
"""
import json
import math
import sys

from PIL import Image, ImageDraw

if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8")  # Windows 控制台默认 GBK，统一为 UTF-8

CFG = "config.json"
SRC_PNG = "pics/you.png"      # 输入图片
OUT_PNG = "pattern_out.png"  # 可视化输出图片
OUT_C = "../keil_project/Core/BUPT_RobotTeam_Libraries/hardware/fan/pattern.c"   # 点阵数组输出.c文件
OUT_H = "../keil_project/Core/BUPT_RobotTeam_Libraries/hardware/fan/pattern.h"   # 点阵数组输出.h文件

BLACK = (0, 0, 0)
GRAY = (40, 40, 40)     # 无效区域颜色
CENTER = (210, 210)     # 圆心（图片中心）
RADIUS = 210            # 圆半径（直径 420）
DOT_R = 1.5             # 可视化时每个 LED 点的半径

DIM_FACTOR = 0.5    # 圆心处亮度系数（靠中心的点密集，调暗）
BRIGHT_FACTOR = 1.2  # 边缘处亮度系数（靠边缘的点稀疏，调亮）


def load_config():
    with open(CFG, encoding="utf-8") as f:
        cfg = json.load(f)
    return cfg["led_num"], cfg["divide_num"]


def led_xy(i, j, led_num, divide_num):
    """第 i 条半径上第 j 个 LED 在图片中的坐标 (x, y)。

    i=0 对应 x 轴，逆时针递增；j=0 为圆心，j=led_num-1 为圆周。
    图片 y 轴向下，故 y = cy - r*sin(theta)。
    """
    cx, cy = CENTER
    theta = 2 * math.pi * i / divide_num
    r = j * RADIUS / (led_num - 1) if led_num > 1 else 0
    return cx + r * math.cos(theta), cy - r * math.sin(theta)


def region_color(px, w, h, x, y):
    """3*3 区域 -> 颜色：取以 (x, y) 为中心的 3*3 区域的平均颜色。

    专用接口，当前实现暂为取平均值，便于后续优化为加权/插值等算法。
    """
    xi, yi = int(round(x)), int(round(y))
    rs = gs = bs = 0
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            sx = min(max(xi + dx, 0), w - 1)
            sy = min(max(yi + dy, 0), h - 1)
            r, g, b = px[sx, sy]
            rs += r
            gs += g
            bs += b
    return rs // 9, gs // 9, bs // 9


def adjust_color(rgb, dist):
    """(原颜色, 离中心距离) -> 最终颜色：近中心调暗，近边缘调亮。

    专用接口，当前实现为亮度系数随距离线性变化，便于后续优化。
    """
    t = dist / RADIUS  # 0 为圆心，1 为圆周
    factor = DIM_FACTOR + (BRIGHT_FACTOR - DIM_FACTOR) * t
    return tuple(min(max(int(round(c * factor)), 0), 255) for c in rgb)


def remove_invalid(img):
    """把无效区域 (127, 127, 127) 灰改为全黑，避免其被采样进点阵。"""
    px = img.load()
    for y in range(img.height):
        for x in range(img.width):
            if px[x, y] == GRAY:
                px[x, y] = (0, 0, 0)
    return px


def main():
    led_num, divide_num = load_config()
    img = Image.open(SRC_PNG).convert("RGB")
    px = remove_invalid(img)

    # 采样生成点阵数组
    rows = []  # [半径序号][LED 序号] = (r, g, b)
    for i in range(divide_num):
        row = []
        for j in range(led_num):
            x, y = led_xy(i, j, led_num, divide_num)
            color = region_color(px, img.width, img.height, x, y)
            dist = math.hypot(x - CENTER[0], y - CENTER[1])
            row.append(adjust_color(color, dist))
        rows.append(row)

    # 可视化转换结果
    base = Image.new("RGB", img.size, (0, 0, 0))
    draw = ImageDraw.Draw(base)
    for i, row in enumerate(rows):
        for j, color in enumerate(row):
            x, y = led_xy(i, j, led_num, divide_num)
            draw.ellipse((x - DOT_R, y - DOT_R, x + DOT_R, y + DOT_R), fill=color)
    mask = Image.new("L", img.size, 0)
    ImageDraw.Draw(mask).ellipse((0, 0, img.width - 1, img.height - 1), fill=255)
    out = Image.composite(base, Image.new("RGB", img.size, BLACK), mask)
    out.save(OUT_PNG)

    # 输出C数组
    with open(OUT_C, "w", encoding="utf-8") as f:
        f.write('# include "pattern.h"\n\n')
        f.write("const uint8_t pattern[DIVIDE_NUM][LED_NUM][3] = {\n")
        for row in rows:
            entries = ", ".join("{%d, %d, %d}" % c for c in row)
            f.write("    {" + entries + "},\n")
        f.write("};\n")

    # 输出数组长度等信息到h文件
    with open(OUT_H, "w", encoding="utf-8") as f:
        f.write(
            f"""# ifndef __PATTERN_H__
# define __PATTERN_H__

# include <stdint.h>

# define LED_NUM {led_num}
# define DIVIDE_NUM {divide_num}

extern const uint8_t pattern[DIVIDE_NUM][LED_NUM][3];

# endif
"""
        )

    print(f"led_num={led_num}, divide_num={divide_num}, "
          f"数组长度={divide_num * led_num * 3}")
    print(f"已生成: {OUT_C}, {OUT_PNG}")


if __name__ == "__main__":
    main()
