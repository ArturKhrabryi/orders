import argparse
import barcode
import sys
from barcode.writer import ImageWriter
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

def main() -> int:
    parser = argparse.ArgumentParser(description="Generate barcode with label.")
    parser.add_argument("-e", "--ean", required=True, help="EAN-13 code (13 digits)")
    parser.add_argument("-l", "--label", required=False, help="Label to display above the barcode. Generate without label if not passed")
    parser.add_argument("-f","--filename", required=True, help="Output file name")
    args = parser.parse_args()

    ean_code = args.ean
    ean = barcode.get("ean13", ean_code, writer=ImageWriter())
    options = {
        "quiet_zone": 0,
    }

    filename = Path(args.filename)
    res = ean.save(filename.with_suffix(''), options)

    if args.label is not None:
        img = Image.open(res)
        margin_top = 70
        new_img = Image.new("RGB", (img.width, img.height + margin_top), "white")
        new_img.paste(img, (0, margin_top))

        label = args.label
        draw = ImageDraw.Draw(new_img)
        font = ImageFont.truetype("calibri.ttf", size = 42)
        bbox = draw.textbbox((0, 0), label, font=font)
        text_width = bbox[2] - bbox[0]
        draw.text(((img.width - text_width) // 2, 30), label, fill="black", font=font)

        new_img.save(res)

if __name__ == "__main__":
    sys.exit(main())
