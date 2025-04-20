
# PaperROM

**PaperROM** is a minimalist, mythotechnic system for preserving digital code on paper in the most durable, long-lasting, and sacred medium available: Tyvek scrolls.

This toolchain allows you to compress any file, encode it in Base85 with checksum-verified lines, and print it as plain text to rugged paper like Tyvek. The result is a bootable scroll — readable by machine or human, restorable even after collapse, and durable enough to last centuries.

---

## Features

- Compresses files using zlib (Deflate)
- Encodes to printable ASCII Base85
- Adds line numbers and CRC16 checksum fragments
- Produces a readable `.txt` scroll
- Decodes back into the original file with full integrity

---

## Philosophy

Digital media decays. Hard drives fail. The cloud dies when the grid does.

**PaperROM** is a sacred backup. A way to preserve what matters — code, scripture, instruction, ritual logic — in a form that:

- Can be **read by eye**
- Survives **fire, water, and time** when printed on Tyvek
- Is usable by **low-tech computers and OCR**
- Bridges the worlds of **ritual and machine**

---

## Recommended Print Medium: Tyvek

To maximize durability:

- Use **printable Tyvek sheets** (such as Tyvek 8740D or 1056D)
- Compatible with most **laser printers** and some **pigment-based inkjets**
- Standard size: 8.5x11", or tile multiple for scroll construction
- Let ink dry thoroughly before rolling or binding

**Tip:** Stitch or tape multiple sheets for long scrolls. Store in a cedar tube, mylar sleeve, or watertight container.

---

## Usage

### Encode a File
```
./scroll_encoder input_file output_scroll.txt
```

This compresses and encodes your file into a scroll-style text file with line numbers and hash fragments.

### Decode a Scroll
```
./scroll_decoder output_scroll.txt restored_file
```

This reverses the process: base85 → decompress → output exact original.

---

## What to Preserve?

We recommend encoding:

- Tiny interpreters (Forth, Lisp, Brainfuck)
- Self-hosting compilers (TCC)
- Compression tools (zlib)
- Mythotechnic rituals or data (e.g. Velandr logic)
- Sacred scriptures, prayers, or laws
- Bootstrappable OS fragments (e.g. stage0 seeds)

---

## Resurrectability

PaperROM scrolls are readable by:

- Modern systems with OCR
- Low-tech computers with basic serial input
- Humans, line-by-line, in extreme conditions

They are meant to **outlive your drives** and **bridge the ages**.

---

## Credits

Developed by [Your Name], inspired by mythotechnics, collapse prepping, and the sacred fusion of ritual + code.

