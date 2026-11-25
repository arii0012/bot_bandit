#!/usr/bin/env python3
"""Convert Square Character textures (.tif) to HD PNG + copy mesh for Raylib.
Run: python tools/prepare_character_assets.py [path_to_extracted_Square_Character]
"""
import os
import sys
import shutil

OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   "assets", "characters", "square")
os.makedirs(OUT, exist_ok=True)

try:
    from PIL import Image, ImageFilter, ImageEnhance
except ImportError:
    print("Pillow required: pip install Pillow")
    sys.exit(0)

def enhance_to_png(src, dst, scale=2):
    im = Image.open(src)
    if im.mode not in ("RGB", "RGBA"):
        im = im.convert("RGBA")
    w, h = im.size
    im = im.resize((w * scale, h * scale), Image.Resampling.LANCZOS)
    im = ImageEnhance.Sharpness(im).enhance(1.25)
    im = ImageEnhance.Contrast(im).enhance(1.08)
    # soft fabric micro detail
    detail = im.filter(ImageFilter.DETAIL)
    im = Image.blend(im, detail, 0.15)
    im.save(dst, "PNG", optimize=True)
    print("wrote", dst)

def main():
    src_root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "third_party", "Square_Character")
    if not os.path.isdir(src_root):
        # search common names
        for cand in ["Square Character", "Square_Character", "square_character"]:
            p = os.path.join(os.path.dirname(src_root), cand)
            if os.path.isdir(p):
                src_root = p
                break
    if not os.path.isdir(src_root):
        print("Source folder not found:", src_root)
        print("Place extracted Square_Character files and re-run.")
        # generate placeholder textures so build never fails
        for name in ["face", "torso", "arm", "leg", "neck"]:
            im = Image.new("RGBA", (256, 256), (210, 168, 140, 255))
            im.save(os.path.join(OUT, name + ".png"))
        print("Wrote placeholder PNGs to", OUT)
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
            low = f.lower()
            full = os.path.join(root, f)
            if f in mapping:
                enhance_to_png(full, os.path.join(OUT, mapping[f]), scale=2)
            elif low.endswith(".obj"):
                shutil.copy2(full, os.path.join(OUT, "character.obj"))
                print("copied obj")
            elif low.endswith(".mtl"):
                shutil.copy2(full, os.path.join(OUT, "character.mtl"))
            elif low.endswith(".fbx"):
                shutil.copy2(full, os.path.join(OUT, "character.fbx"))
            elif low.endswith((".png", ".jpg", ".jpeg")):
                dst = os.path.join(OUT, f.replace(" ", "_").lower())
                if not os.path.exists(dst):
                    try:
                        enhance_to_png(full, dst, scale=1)
                    except Exception:
                        shutil.copy2(full, dst)

    print("Done. Assets in", OUT)

if __name__ == "__main__":
    main()
