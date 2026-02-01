#!/usr/bin/env python3
"""
Скрипт для генерации визуальных превью стилей отображения метеочасов
Воспроизводит логику отрисовки из display_original.h и display_compact.h

Запуск из корня проекта (нужен .venv с Pillow в корне):
    .venv/bin/python scripts/generate_previews.py

Или:
    source .venv/bin/activate   # Windows: .venv\\Scripts\\activate
    python scripts/generate_previews.py
"""

import sys
import os

# Корень проекта (родитель папки scripts)
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMG_DIR = os.path.join(PROJECT_ROOT, 'img')

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("ОШИБКА: Модуль PIL (Pillow) не найден!")
    print("\nУстановите Pillow одним из способов:")
    print("  1. pip3 install Pillow --user")
    print("  2. python3 -m pip install Pillow --user")
    print("  3. Или создайте venv:")
    print("     python3 -m venv venv")
    print("     source venv/bin/activate")
    print("     pip install Pillow")
    sys.exit(1)

# Размеры экрана ILI9341 (вертикальная ориентация)
SCREEN_WIDTH = 240
SCREEN_HEIGHT = 320

# Константы цветов ILI9341 (RGB565 -> RGB)
ILI9341_BLACK = (0, 0, 0)
ILI9341_DARKGREEN = (0, 62, 0)
ILI9341_PURPLE = (120, 15, 120)
ILI9341_BLUE = (0, 31, 255)
ILI9341_RED = (248, 0, 0)
ILI9341_YELLOW = (255, 224, 0)
ILI9341_WHITE = (255, 255, 255)
ILI9341_DARKGRAY = (16, 16, 16)  # Примерно 0x1082

# Константы символов
SYMBOL_W = 5
SYMBOL_H = 7
SPACE_W = 1

# Пиксельный шрифт 5x7 (как в Adafruit GFX)
# Каждый символ представлен как список из 7 байт (по 5 бит каждый, старший бит слева)
PIXEL_FONT_5x7 = {
    '0': [0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E],  # 01110 10001 10011 10101 11001 10001 01110
    '1': [0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E],  # 00100 01100 00100 00100 00100 00100 01110
    '2': [0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F],  # 01110 10001 00001 00010 00100 01000 11111
    '3': [0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E],  # 01110 10001 00001 00110 00001 10001 01110
    '4': [0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02],  # 00010 00110 01010 10010 11111 00010 00010
    '5': [0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E],  # 11111 10000 11110 00001 00001 10001 01110
    '6': [0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E],  # 00110 01000 10000 11110 10001 10001 01110
    '7': [0x1F, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04],  # 11111 00001 00010 00100 00100 00100 00100
    '8': [0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E],  # 01110 10001 10001 01110 10001 10001 01110
    '9': [0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C],  # 01110 10001 10001 01111 00001 00010 01100
    ':': [0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00],  # 00000 00000 00100 00000 00100 00000 00000
    '+': [0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00],  # 00000 00100 00100 11111 00100 00100 00000
    '%': [0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03],  # 11000 11001 00010 00100 01000 10011 00011
    '-': [0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00],  # 00000 00000 00000 11111 00000 00000 00000
    '.': [0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C],  # 00000 00000 00000 00000 00000 01100 01100
    ' ': [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],  # 00000 00000 00000 00000 00000 00000 00000
    't': [0x04, 0x04, 0x1F, 0x04, 0x04, 0x05, 0x02],  # 00100 00100 11111 00100 00100 00101 00010
    'C': [0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E],  # 01110 10001 10000 10000 10000 10001 01110
    'c': [0x00, 0x00, 0x0E, 0x11, 0x10, 0x11, 0x0E],  # 00000 00000 01110 10001 10000 10001 01110
    'o': [0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E],  # 00000 00000 01110 10001 10001 10001 01110
    'O': [0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E],  # 01110 10001 10001 10001 10001 10001 01110
    'M': [0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11],  # 10001 11011 10101 10001 10001 10001 10001
    'P': [0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10],  # 11110 10001 10001 11110 10000 10000 10000
    'B': [0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E],  # 11110 10001 10001 11110 10001 10001 11110
}

def draw_pixel_char(draw, char, x, y, size, color):
    """Рисует один пиксельный символ 5x7 с масштабированием"""
    if char not in PIXEL_FONT_5x7:
        # Если символа нет, рисуем заглушку
        draw.rectangle([x, y, x + SYMBOL_W*size - 1, y + SYMBOL_H*size - 1], outline=color)
        return
    
    font_data = PIXEL_FONT_5x7[char]
    
    for row in range(7):
        byte_row = font_data[row]
        for col in range(5):
            # Проверяем бит (старший бит слева)
            if byte_row & (0x10 >> col):
                # Рисуем пиксель с масштабированием
                pixel_x = x + col * size
                pixel_y = y + row * size
                if size == 1:
                    draw.point((pixel_x, pixel_y), fill=color)
                else:
                    draw.rectangle([pixel_x, pixel_y, pixel_x+size-1, pixel_y+size-1], fill=color)

def print_text_pixel(draw, text, x, y, size, index, color, fill_color):
    """Пиксельная версия printText - рисует текст пиксельным шрифтом 5x7"""
    char_w = SYMBOL_W * size
    char_h = SYMBOL_H * size
    
    # Позиция с учетом index
    text_x = x + index * char_w + index * SPACE_W * size
    text_y = y
    
    # Заливка фона
    text_width = len(text) * char_w + (len(text) - 1) * SPACE_W * size
    draw.rectangle([text_x, text_y, text_x + text_width, text_y + char_h], fill=fill_color)
    
    # Рисуем каждый символ
    for i, char in enumerate(text):
        char_x = text_x + i * (char_w + SPACE_W * size)
        draw_pixel_char(draw, char, char_x, text_y, size, color)

# Тестовые данные
TEST_DATA = {
    'hours': 14,
    'minutes': 35,
    'seconds': 42,
    'day': 15,
    'month': 12,
    'dayOfWeek': 0,  # Воскресенье
    'temperature': 23.5,
    'humidity': 65,
    'pressure': 765,
    'co2': 485,
    'dotFlag': True
}


def rgb565_to_rgb(color565):
    """Конвертирует RGB565 в RGB"""
    r = ((color565 >> 11) & 0x1F) << 3
    g = ((color565 >> 5) & 0x3F) << 2
    b = (color565 & 0x1F) << 3
    return (r, g, b)


def print_text(draw, text, x, y, size, index, color, fill_color):
    """Имитирует printText из скетча - использует пиксельный шрифт"""
    # Используем пиксельный шрифт для всех символов
    print_text_pixel(draw, text, x, y, size, index, color, fill_color)


def draw_day_of_week_original(draw, x, y, day, font_medium):
    """Отрисовка дня недели (упрощенная версия) - использует латиницу"""
    # В оригинале дни недели рисуются латиницей (Bc, B, Cp и т.д.)
    days_latin = ["Bc", "Pn", "Bt", "Cp", "Ct", "Pt", "Cb"]
    draw.rectangle([x, y, x + SYMBOL_W*2*2 + SPACE_W*2, y + SYMBOL_H*2], fill=ILI9341_BLACK)
    if day < len(days_latin):
        # Используем пиксельный шрифт размером 2
        print_text_pixel(draw, days_latin[day], x, y, 2, 0, ILI9341_WHITE, ILI9341_BLACK)


def draw_arrow(draw, x, y):
    """Отрисовка одной стрелки вниз для давления (как в display_common.h)"""
    draw.rectangle([x+5, y, x+7, y+20], fill=ILI9341_WHITE)
    draw.polygon([(x+2, y+14), (x+7, y+19), (x+12, y+14)], fill=ILI9341_WHITE)


def draw_temp_celsius_label(draw, top_y, left_x, right_x, text_color, fill_color):
    """Подпись t°C: по вертикали от top_y, по горизонтали по центру между left_x и right_x."""
    label_w = 42
    x = left_x + (right_x - left_x - label_w) // 2
    print_text(draw, "t", x, top_y, 3, 0, text_color, fill_color)
    print_text(draw, "o", x+16, top_y-5, 2, 0, text_color, fill_color)
    print_text(draw, "C", x+27, top_y, 3, 0, text_color, fill_color)


def draw_co2_label(draw, top_y, left_x, right_x, text_color, fill_color):
    """Подпись CO2: по вертикали от top_y, по горизонтали по центру между left_x и right_x."""
    label_w = 45
    x = left_x + (right_x - left_x - label_w) // 2
    print_text(draw, "CO", x, top_y, 3, 0, text_color, fill_color)
    print_text(draw, "2", x+35, top_y+9, 2, 0, text_color, fill_color)


def draw_droplet(draw, top_y, left_x, right_x):
    """Капля влажности: верх (остриё) в top_y, по горизонтали по центру между left_x и right_x."""
    x = (left_x + right_x) // 2
    y = top_y + 15
    draw.ellipse([x-5, y-5, x+5, y+5], fill=ILI9341_WHITE)
    draw.polygon([(x, top_y), (x-5, y-3), (x+5, y-3)], fill=ILI9341_WHITE)


def draw_pressure_arrows(draw, top_y, left_x, right_x):
    """Две стрелки давления: от top_y, по центру между left_x и right_x."""
    arrow_w, gap = 13, 16
    total_w = arrow_w + gap + arrow_w
    x = left_x + (right_x - left_x - total_w) // 2
    draw_arrow(draw, x, top_y)
    draw_arrow(draw, x + arrow_w + gap, top_y)


# Константы оригинального стиля (display_original.h)
ORIG_HEADER_BOTTOM = 44
ORIG_QUAD_W = 120
ORIG_QUAD_H = 138
ORIG_SENSOR_LABEL_MARGIN = 40
ORIG_CLOCK_X, ORIG_CLOCK_Y = 10, 10
ORIG_DATE_Y = SYMBOL_H + 10


def draw_original_style():
    """Отрисовка оригинального стиля с 4 квадрантами (соответствует display_original.h)"""
    img = Image.new('RGB', (SCREEN_WIDTH, SCREEN_HEIGHT), ILI9341_BLACK)
    draw = ImageDraw.Draw(img)
    
    try:
        font_small = ImageFont.load_default()
        font_medium = ImageFont.load_default()
        font_large = ImageFont.load_default()
        try:
            font_large = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 48)
            font_medium = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 24)
            font_small = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 12)
        except Exception:
            try:
                font_large = ImageFont.truetype("arial.ttf", 48)
                font_medium = ImageFont.truetype("arial.ttf", 24)
                font_small = ImageFont.truetype("arial.ttf", 12)
            except Exception:
                pass
    except Exception:
        font_small = font_medium = font_large = ImageFont.load_default()
    
    # Шапка: белая полоса
    draw.rectangle([0, ORIG_HEADER_BOTTOM - 3, SCREEN_WIDTH, ORIG_HEADER_BOTTOM], fill=ILI9341_WHITE)
    
    # Температура (левый верхний квадрант)
    draw.rectangle([0, ORIG_HEADER_BOTTOM, ORIG_QUAD_W, ORIG_HEADER_BOTTOM + ORIG_QUAD_H], fill=ILI9341_RED)
    draw_temp_celsius_label(draw, ORIG_HEADER_BOTTOM + ORIG_SENSOR_LABEL_MARGIN, 0, ORIG_QUAD_W, ILI9341_WHITE, ILI9341_RED)
    
    # Влажность (правый верхний квадрант)
    draw.rectangle([ORIG_QUAD_W, ORIG_HEADER_BOTTOM, SCREEN_WIDTH, ORIG_HEADER_BOTTOM + ORIG_QUAD_H], fill=ILI9341_BLUE)
    draw_droplet(draw, ORIG_HEADER_BOTTOM + ORIG_SENSOR_LABEL_MARGIN, ORIG_QUAD_W, SCREEN_WIDTH)
    
    # Давление (левый нижний квадрант)
    draw.rectangle([0, ORIG_HEADER_BOTTOM + ORIG_QUAD_H, ORIG_QUAD_W, SCREEN_HEIGHT], fill=ILI9341_DARKGREEN)
    draw_pressure_arrows(draw, ORIG_HEADER_BOTTOM + ORIG_QUAD_H + ORIG_SENSOR_LABEL_MARGIN, 0, ORIG_QUAD_W)
    
    # CO2 (правый нижний квадрант)
    draw.rectangle([ORIG_QUAD_W, ORIG_HEADER_BOTTOM + ORIG_QUAD_H, SCREEN_WIDTH, SCREEN_HEIGHT], fill=ILI9341_PURPLE)
    draw_co2_label(draw, ORIG_HEADER_BOTTOM + ORIG_QUAD_H + ORIG_SENSOR_LABEL_MARGIN, ORIG_QUAD_W, SCREEN_WIDTH, ILI9341_WHITE, ILI9341_PURPLE)
    
    # Часы
    clock_str = f"{TEST_DATA['hours']:02d}:{TEST_DATA['minutes']:02d}"
    print_text(draw, clock_str, ORIG_CLOCK_X, ORIG_CLOCK_Y, 3, 0, ILI9341_YELLOW, ILI9341_BLACK)
    
    # Дата и день недели
    draw_day_of_week_original(draw, 140, ORIG_CLOCK_Y + 7, TEST_DATA['dayOfWeek'], font_medium)
    day_str = f"{TEST_DATA['day']:02d}"
    month_str = f"{TEST_DATA['month']:02d}"
    print_text(draw, day_str, 17, ORIG_DATE_Y, 2, 13, ILI9341_WHITE, ILI9341_BLACK)
    print_text(draw, ".", 17, ORIG_DATE_Y, 2, 15, ILI9341_WHITE, ILI9341_BLACK)
    if TEST_DATA['month'] < 10:
        print_text(draw, "0", 17, ORIG_DATE_Y, 2, 16, ILI9341_WHITE, ILI9341_BLACK)
        print_text(draw, month_str, 17, ORIG_DATE_Y, 2, 17, ILI9341_WHITE, ILI9341_BLACK)
    else:
        print_text(draw, month_str, 17, ORIG_DATE_Y, 2, 16, ILI9341_WHITE, ILI9341_BLACK)
    
    # Значения датчиков
    val_y_top = ORIG_HEADER_BOTTOM + ORIG_QUAD_H // 2
    temp_str = f"+{int(TEST_DATA['temperature'])}"
    print_text(draw, temp_str, 34, val_y_top, 3, 0, ILI9341_YELLOW, ILI9341_RED)
    hum_str = f"{TEST_DATA['humidity']}%"
    print_text(draw, hum_str, ORIG_QUAD_W + 34, val_y_top, 3, 0, ILI9341_YELLOW, ILI9341_BLUE)
    
    val_y_bot = ORIG_HEADER_BOTTOM + ORIG_QUAD_H + ORIG_QUAD_H // 2
    pres_str = f"{TEST_DATA['pressure']}"
    print_text(draw, pres_str, 20, val_y_bot, 3, 0, ILI9341_YELLOW, ILI9341_DARKGREEN)
    print_text(draw, "MM", 75, val_y_bot + 7, 2, 0, ILI9341_YELLOW, ILI9341_DARKGREEN)
    co2_x = ORIG_QUAD_W + (25 if TEST_DATA['co2'] >= 1000 else 34)
    draw.rectangle([ORIG_QUAD_W, val_y_bot, SCREEN_WIDTH, val_y_bot + SYMBOL_H*3], fill=ILI9341_PURPLE)
    print_text(draw, f"{TEST_DATA['co2']}", co2_x, val_y_bot, 3, 0, ILI9341_YELLOW, ILI9341_PURPLE)
    
    return img

# Константы компактного стиля (display_compact.h)
COMPACT_CLOCK_ZONE_H = 128
COMPACT_QUAD_W = 120
COMPACT_QUAD_H = 96
COMPACT_SENSOR_LABEL_MARGIN = 20
COMPACT_DATE_TOP_MARGIN = 100
# Свой шрифт для крупных цифр времени: 7×11 px, масштаб 4
COMPACT_CLOCK_FONT_W = 7
COMPACT_CLOCK_FONT_H = 11
COMPACT_CLOCK_FONT_SCALE = 4
COMPACT_CLOCK_GLYPH_GAP = 2
COMPACT_CLOCK_TOP_MARGIN = 26

# Битмапы шрифта часов (0–9 и ':', индекс 10). Ряд = 7 бит, бит 6 = левый пиксель.
# Должно совпадать с clockFont в display_compact.h
CLOCK_FONT_7x11 = [
    (0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E),  # 0
    (0x0C, 0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F),  # 1
    (0x3E, 0x63, 0x03, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x63, 0x7F),  # 2
    (0x3E, 0x63, 0x03, 0x03, 0x1E, 0x03, 0x03, 0x03, 0x63, 0x63, 0x3E),  # 3
    (0x06, 0x0E, 0x1E, 0x36, 0x66, 0x66, 0x7F, 0x06, 0x06, 0x06, 0x06),  # 4
    (0x7F, 0x60, 0x60, 0x60, 0x3E, 0x03, 0x03, 0x03, 0x63, 0x63, 0x3E),  # 5
    (0x1E, 0x30, 0x60, 0x60, 0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E),  # 6
    (0x7F, 0x63, 0x03, 0x06, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x18, 0x18),  # 7
    (0x3E, 0x63, 0x63, 0x63, 0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E),  # 8
    (0x3E, 0x63, 0x63, 0x63, 0x63, 0x3F, 0x03, 0x03, 0x06, 0x0C, 0x38),  # 9
    (0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00),  # :
]


def draw_clock_glyph(draw, glyph_idx, x, y, scale, color, bg_color):
    """Рисует один символ шрифта часов 7×11 с масштабированием (как в display_compact.h)."""
    font = CLOCK_FONT_7x11
    if glyph_idx > len(font) - 1:
        return
    for row in range(COMPACT_CLOCK_FONT_H):
        line = font[glyph_idx][row]
        for col in range(COMPACT_CLOCK_FONT_W):
            c = color if (line >> (6 - col)) & 1 else bg_color
            px, py = x + col * scale, y + row * scale
            if scale == 1:
                draw.point((px, py), fill=c)
            else:
                draw.rectangle([px, py, px + scale - 1, py + scale - 1], fill=c)


def get_clock_digits_width(char_count):
    """Ширина строки из char_count символов шрифта часов (как в display_compact.h)."""
    return char_count * (COMPACT_CLOCK_FONT_W * COMPACT_CLOCK_FONT_SCALE + COMPACT_CLOCK_GLYPH_GAP) - COMPACT_CLOCK_GLYPH_GAP


def draw_compact_clock(draw, hours, minutes, show_colon, color, bg_color):
    """Рисует крупные часы шрифтом 7×11 (соответствует drawClock в display_compact.h)."""
    clock_str = f"{hours:02d}:{minutes:02d}"
    total_w = get_clock_digits_width(5)
    x = (SCREEN_WIDTH - total_w) // 2
    y = COMPACT_CLOCK_TOP_MARGIN
    step = COMPACT_CLOCK_FONT_W * COMPACT_CLOCK_FONT_SCALE + COMPACT_CLOCK_GLYPH_GAP
    scale = COMPACT_CLOCK_FONT_SCALE
    glyph_w = COMPACT_CLOCK_FONT_W * scale
    glyph_h = COMPACT_CLOCK_FONT_H * scale

    draw_clock_glyph(draw, ord(clock_str[0]) - ord('0'), x, y, scale, color, bg_color)
    draw_clock_glyph(draw, ord(clock_str[1]) - ord('0'), x + step, y, scale, color, bg_color)
    if show_colon:
        draw_clock_glyph(draw, 10, x + 2 * step, y, scale, color, bg_color)
    else:
        draw.rectangle([x + 2 * step, y, x + 2 * step + glyph_w - 1, y + glyph_h - 1], fill=bg_color)
    draw_clock_glyph(draw, ord(clock_str[3]) - ord('0'), x + 3 * step, y, scale, color, bg_color)
    draw_clock_glyph(draw, ord(clock_str[4]) - ord('0'), x + 4 * step, y, scale, color, bg_color)


def draw_compact_style():
    """Отрисовка компактного стиля: 40% для часов, 60% для датчиков в квадрантах (соответствует display_compact.h)"""
    img = Image.new('RGB', (SCREEN_WIDTH, SCREEN_HEIGHT), ILI9341_BLACK)
    draw = ImageDraw.Draw(img)
    
    try:
        font_small = ImageFont.load_default()
        font_medium = ImageFont.load_default()
        font_large = ImageFont.load_default()
        try:
            font_large = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 48)
            font_medium = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 24)
            font_small = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 12)
        except Exception:
            try:
                font_large = ImageFont.truetype("arial.ttf", 48)
                font_medium = ImageFont.truetype("arial.ttf", 24)
                font_small = ImageFont.truetype("arial.ttf", 12)
            except Exception:
                pass
    except Exception:
        font_small = font_medium = font_large = ImageFont.load_default()
    
    sensor_zone_top = COMPACT_CLOCK_ZONE_H
    
    # Разделительные линии между часами и датчиками
    draw.rectangle([0, sensor_zone_top - 2, SCREEN_WIDTH, sensor_zone_top], fill=ILI9341_WHITE)
    
    # Крупные часы по центру (свой шрифт 7×11, масштаб 4)
    draw_compact_clock(draw, TEST_DATA['hours'], TEST_DATA['minutes'], TEST_DATA['dotFlag'], ILI9341_YELLOW, ILI9341_BLACK)
    
    # Дата и день недели
    draw_day_of_week_original(draw, 140, COMPACT_DATE_TOP_MARGIN, TEST_DATA['dayOfWeek'], font_medium)
    day_str = f"{TEST_DATA['day']:02d}"
    month_str = f"{TEST_DATA['month']:02d}"
    print_text(draw, day_str, 17, COMPACT_DATE_TOP_MARGIN, 2, 13, ILI9341_WHITE, ILI9341_BLACK)
    print_text(draw, ".", 17, COMPACT_DATE_TOP_MARGIN, 2, 15, ILI9341_WHITE, ILI9341_BLACK)
    if TEST_DATA['month'] < 10:
        print_text(draw, "0", 17, COMPACT_DATE_TOP_MARGIN, 2, 16, ILI9341_WHITE, ILI9341_BLACK)
        print_text(draw, month_str, 17, COMPACT_DATE_TOP_MARGIN, 2, 17, ILI9341_WHITE, ILI9341_BLACK)
    else:
        print_text(draw, month_str, 17, COMPACT_DATE_TOP_MARGIN, 2, 16, ILI9341_WHITE, ILI9341_BLACK)
    
    # Температура — левый верхний квадрант
    draw.rectangle([0, sensor_zone_top, COMPACT_QUAD_W, sensor_zone_top + COMPACT_QUAD_H], fill=ILI9341_RED)
    draw_temp_celsius_label(draw, sensor_zone_top + COMPACT_SENSOR_LABEL_MARGIN, 0, COMPACT_QUAD_W, ILI9341_WHITE, ILI9341_RED)
    
    # Влажность — правый верхний квадрант
    draw.rectangle([COMPACT_QUAD_W, sensor_zone_top, SCREEN_WIDTH, sensor_zone_top + COMPACT_QUAD_H], fill=ILI9341_BLUE)
    draw_droplet(draw, sensor_zone_top + COMPACT_SENSOR_LABEL_MARGIN, COMPACT_QUAD_W, SCREEN_WIDTH)
    
    # Давление — левый нижний квадрант
    draw.rectangle([0, sensor_zone_top + COMPACT_QUAD_H, COMPACT_QUAD_W, SCREEN_HEIGHT], fill=ILI9341_DARKGREEN)
    draw_pressure_arrows(draw, sensor_zone_top + COMPACT_QUAD_H + COMPACT_SENSOR_LABEL_MARGIN, 0, COMPACT_QUAD_W)
    
    # CO2 — правый нижний квадрант
    draw.rectangle([COMPACT_QUAD_W, sensor_zone_top + COMPACT_QUAD_H, SCREEN_WIDTH, SCREEN_HEIGHT], fill=ILI9341_PURPLE)
    draw_co2_label(draw, sensor_zone_top + COMPACT_QUAD_H + COMPACT_SENSOR_LABEL_MARGIN, COMPACT_QUAD_W, SCREEN_WIDTH, ILI9341_WHITE, ILI9341_PURPLE)
    
    # Значения датчиков
    val_y_top = sensor_zone_top + COMPACT_QUAD_H // 2
    temp_str = f"+{int(TEST_DATA['temperature'])}"
    print_text(draw, temp_str, 34, val_y_top, 3, 0, ILI9341_YELLOW, ILI9341_RED)
    hum_str = f"{TEST_DATA['humidity']}%"
    print_text(draw, hum_str, COMPACT_QUAD_W + 34, val_y_top, 3, 0, ILI9341_YELLOW, ILI9341_BLUE)
    
    val_y_bot = sensor_zone_top + COMPACT_QUAD_H + COMPACT_QUAD_H // 2
    pres_str = f"{TEST_DATA['pressure']}"
    print_text(draw, pres_str, 20, val_y_bot, 3, 0, ILI9341_YELLOW, ILI9341_DARKGREEN)
    print_text(draw, "MM", 75, val_y_bot + 7, 2, 0, ILI9341_YELLOW, ILI9341_DARKGREEN)
    co2_x = COMPACT_QUAD_W + (25 if TEST_DATA['co2'] >= 1000 else 34)
    draw.rectangle([COMPACT_QUAD_W, val_y_bot, SCREEN_WIDTH, val_y_bot + SYMBOL_H*3], fill=ILI9341_PURPLE)
    print_text(draw, f"{TEST_DATA['co2']}", co2_x, val_y_bot, 3, 0, ILI9341_YELLOW, ILI9341_PURPLE)
    
    return img


def main():
    """Главная функция для генерации превью"""
    print("Генерация превью стилей отображения...")
    
    if not os.path.exists(IMG_DIR):
        os.makedirs(IMG_DIR)
    
    preview_original = os.path.join(IMG_DIR, 'preview_original.png')
    preview_compact = os.path.join(IMG_DIR, 'preview_compact.png')
    
    print("  - Отрисовка оригинального стиля...")
    img_original = draw_original_style()
    img_original.save(preview_original)
    print("    ✓ Сохранено в img/preview_original.png")
    
    print("  - Отрисовка компактного стиля...")
    img_compact = draw_compact_style()
    img_compact.save(preview_compact)
    print("    ✓ Сохранено в img/preview_compact.png")
    
    print("\n✓ Все превью успешно созданы!")
    print("   Размер экрана: 240x320 пикселей (ILI9341, вертикальная ориентация)")


if __name__ == '__main__':
    main()
