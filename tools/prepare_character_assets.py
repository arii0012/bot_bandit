#!/usr/bin/env python3
"""Convert Square Character .tif textures to HD PNG and copy meshes for Raylib."""
import os
import sys
import shutil

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "assets", "characters", "square")
os.makedirs(OUT, exist_ok=True)

try:
    from PIL import Image, ImageFilter, ImageEnhance
except ImportError:
    Image = None

def enhance_to_png(src, dst, scale=2):
    im = Image.open(src)
    if im.mode not in ("RGB", "RGBA"):
        im = im.convert("RGBA")
    w, h = im.size
    im = im.resize((max(1, w * scale), max(1, h * scale)), Image.Resampling.LANCZOS)
    im = ImageEnhance.Sharpness(im).enhance(1.25)
    im = ImageEnhance.Contrast(im).enhance(1.08)
    detail = im.filter(ImageFilter.DETAIL)
    im = Image.blend(im, detail, 0.15)
    im.save(dst, "PNG", optimize=True)
    print("wrote", dst)

def placeholders():
    if Image is None:
        for name in ["face", "torso", "arm", "leg", "neck", "face_alt"]:
            open(os.path.join(OUT, name + ".png"), "wb").close()
        return
    for name, col in [
        ("face", (210, 168, 140, 255)),
        ("face_alt", (180, 200, 160, 255)),
        ("torso", (48, 50, 68, 255)),
        ("arm", (210, 168, 140, 255)),
        ("leg", (30, 30, 42, 255)),
        ("neck", (210, 168, 140, 255)),
    ]:
        Image.new("RGBA", (256, 256), col).save(os.path.join(OUT, name + ".png"))
    print("placeholders ->", OUT)

def main():
    src_root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "third_party", "Square_Character")
    if not os.path.isdir(src_root):
        for cand in ("Square Character", "Square_Character", "square_character"):
            p = os.path.join(os.path.dirname(src_root), cand)
            if os.path.isdir(p):
                src_root = p
                break
    if not os.path.isdir(src_root):
        print("Source not found:", src_root)
        placeholders()
        return
    mapping = {
        "Default Face.tif": "face.png",
        "Zombie Face.tif": "face_alt.png",
        "Default Torso.tif": "torso.png",
        "Default Arm.tif": "arm.png",
        "Default Leg.tif": "leg.png",
        "Default Neck.tif": "neck.png",
    }
    for root, _, files in os.walk(src_root):
        for f in files:
            full = os.path.join(root, f)
            low = f.lower()
            if f in mapping and Image is not None:
                enhance_to_png(full, os.path.join(OUT, mapping[f]), 2)
            elif low.endswith(".obj"):
                shutil.copy2(full, os.path.join(OUT, "character.obj"))
            elif low.endswith(".mtl"):
                shutil.copy2(full, os.path.join(OUT, "character.mtl"))
            elif low.endswith(".fbx"):
                shutil.copy2(full, os.path.join(OUT, "character.fbx"))
    if not any(fn.endswith(".png") for fn in os.listdir(OUT)):
        placeholders()
    print("Done", OUT)

if __name__ == "__main__":
    main()
