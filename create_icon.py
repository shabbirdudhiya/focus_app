#!/usr/bin/env python3
"""
Simple script to create a basic icon file for FocusApp.
Requires Pillow library: pip install pillow
"""

from PIL import Image, ImageDraw
import os

# Create a simple icon
size = 256
icon = Image.new('RGBA', (size, size), (0, 0, 0, 0))
draw = ImageDraw.Draw(icon)

# Draw a simple clock/timer icon
# Outer circle
center = size // 2
radius = size // 2 - 20
draw.ellipse([center - radius, center - radius, center + radius, center + radius], 
             fill=(70, 130, 180), outline=(255, 255, 255), width=8)

# Inner circle
draw.ellipse([center - radius + 30, center - radius + 30, 
             center + radius - 30, center + radius - 30], 
             fill=(255, 255, 255))

# Clock hands
draw.line([center, center, center, center - radius + 40], fill=(0, 0, 0), width=6)
draw.line([center, center, center + radius // 2, center], fill=(0, 0, 0), width=4)

# Save as PNG first
icon.save('app_icon.png')
print("Created app_icon.png")

# Try to create ICO (requires Pillow 9.0.0+)
try:
    icon.save('app_icon.ico', format='ICO', sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    print("Created app_icon.ico")
except Exception as e:
    print(f"Could not create ICO: {e}")
    print("Please use an online converter to create .ico from app_icon.png")
