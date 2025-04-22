[link-bg-color-depth][#Background%20color%20depth]
[link-bg-mode][#Background%20mode]

# `bg.h`

Game Freak's helper library for working with VRAM background layers.

## Concepts

### Background color depth

A background tile can use a 16-color (4bpp) palette or a 256-color (8bpp) palette. A 4bpp tile graphic consumes 0x20 bytes in memory, while an 8bpp tile graphic consumes 0x40 bytes in memory.

Note that the background can only load up to 256 colors at a time (segmented into sixteen 16-color palettes). However, you can still display a 256-color layer alongside 4bpp layers, because a 256-color layer doesn't actually have to *define and use* all 256 colors. Think of the palette memory as a union:

```c
union {
   u16 colors[256];      // 256-color palette
   u16 palettes[16][16]; // 16-color palettes
} paletteData;
```

### Background mode

There are three possible background modes:

* **Text mode** is used to display traditional tile-based graphics.

* **Affine mode** is similar to text mode, except that the background layer can be translated, rotated, and scaled.

* **Bitmap mode** displays a pixel-based image.

Background modes *affect* layers but cannot be set *per* layer. Rather, there is an overall background mode value which indicates what layers are in what modes. The possibilities are:

| Value | BG0 | BG1 | BG2 | BG3 | Notes |
| :- | :-: | :-: | :-: | :-: | :- |
| `DISPCNT_MODE_0` | Text | Text | Text | Text |
| `DISPCNT_MODE_1` | Text | Text | Affine | Disabled |
| `DISPCNT_MODE_2` | Text | Text | Affine | Affine |
| `DISPCNT_MODE_3` | Disabled | Disabled | Bitmap | Disabled | 240x160px bitmap; 15-bit colors. |
| `DISPCNT_MODE_4` | Disabled | Disabled | Bitmap | Disabled | 240x160px bitmap; 256-color palette. |
| `DISPCNT_MODE_5` | Disabled | Disabled | Bitmap | Disabled | Two 160x128px bitmaps; 15-bit colors. `DISPCNT` flag 0x10 (1 << 4) indicates whether to display the second bitmap. |

## Structs

### `BgTemplate`

A simple definition for a background layer. The following fields are present:

<dl>
   <dt><code>bg</code></dt>
      <dd><p>Indicates which BG layer this template defines. A value in the range [0, 3].</p></dd>
   <dt><code>charBaseIndex</code></dt>
      <dd><p>Indicates where in VRAM this layer's tile graphics are stored. A background layer can access tiles located at any offset that is a multiple of 0x4000, so this value is the zeroth tile's location in VRAM, divided by 0x4000.</p></dd>
   <dt><code>mapBaseIndex</code></dt>
      <dd><p>Indicates where in VRAM this layer's tilemap is stored. Tilemap addresses can be a multiple of 0x800, so this value is the tilemap's location in VRAM, divided by 0x800.</p></dd>
   <dt><code>screenSize</code></dt>
      <dd>
         <p>The meaning of this value varies depending on whether this is a text-mode layer or an affine layer.</p>
      </dd>
   <dt><code>paletteMode</code></dt>
<dd><p>

This layer's [color depth][link-bg-color-depth]. Specify 0 for a 16-color palette, or 1 for a 256-color palette.

</p></dd>
   <dt><code>priority</code></dt>
      <dd><p>Indicates which layers render overtop each other. Layers with a lower priority render above layers with a higher priority.</p></dd>
   <dt><code>baseTile</code></dt>
      <dd><p>A convenience value that gets used by <code>LoadBgTiles</code>, but not by other means of loading tiles into VRAM.</p></dd>
</dl>

## Functions

### `InitBgsFromTemplates`

Given a background mode and a list of `BgTemplate`s, this initializes all background layers' states.

**Arguments:**

<dl>
   <dt><code>u8 mode</code></dt>
<dd><p>

A `DISPCNT_MODE_` constant from `gba/io_reg.h`, setting the [background mode][link-bg-mode].

</p></dd>
   <dt><code>const struct BgTemplate* templates</code></dt>
      <dd><p>A pointer to the background layer templates to load.</p></dd>
   <dt><code>u8 count</code></dt>
      <dd><p>The number of background layer templates to load.</p></dd>
</dl>

### `SetBgAffine`

Sets a given BG layer as affine. This is dependent on the current background mode; if the chosen layer cannot be made affine within the current [background mode][link-bg-mode], then the call does nothing.

**Arguments:**

<dl>
   <dt><code>u8 bg</code></dt>
      <dd><p>The layer.</p></dd>
   <dt><code>s32 srcCenterX</code></dt>
      <dd><p>Set this value to <code>-x * 256</code> given some desired X-coordinate to display the layer at.</p></dd>
   <dt><code>s32 srcCenterY</code></dt>
      <dd><p>Set this value to <code>-y * 256</code> given some desired Y-coordinate to display the layer at.</p></dd>
   <dt><code>s32 dispCenterX</code></dt>
      <dd><p>Unclear. Something to do with rotations, no doubt. Zero is a safe value to pass if you're not interested in rotating the layer.</p></dd>
   <dt><code>s32 dispCenterY</code></dt>
      <dd><p>Unclear. Something to do with rotations, no doubt. Zero is a safe value to pass if you're not interested in rotating the layer.</p></dd>
   <dt><code>scaleX</code></dt>
      <dd><p>Set this value to <code>x * 256</code> given a desired X-scale. For 100% scale, pass 256.</p></dd>
   <dt><code>scaleY</code></dt>
      <dd><p>Set this value to <code>y * 256</code> given a desired Y-scale. For 100% scale, pass 256.</p></dd>
   <dt><code>rotationAngle</code></dt>
      <dd><p>Zero is a safe value to pass if you're not interested in rotating the layer.</p></dd>
</dl>

### `ShowBg`

Shows a given BG layer.

**Arguments:**

<dl>
   <dt><code>u8 bg</code></dt>
      <dd><p>The layer.</p></dd>
</dl>

## Tricks

* The Pokemon Storage System UI uses `IsDma3ManagerBusyWithBgCopy()` when changing a box's wallpaper, to poll for when the wallpaper graphics have finished loading. They load and decompress the wallpaper tiles into a heap-allocated buffer, call `LoadBgTiles` to queue a copy into VRAM, and then free the buffer once the copy is done.

## See also

* [palette.h](#palette.md)