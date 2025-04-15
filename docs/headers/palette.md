# `palette.h`

Game Freak's helper library for working with VRAM color palettes. This library offers two core functions:

* Loading and modifying palette data.
* Fading palettes into or out of a desired color.

## Concepts

### Color depth

Palettized graphics on a GBA use 15-bit colors with five bits per channel. This means that R, G, and B values must be multiples of 8.

### Palette fades

There are four different kinds of fades:

* Normal fades
* Hardware fades
* Fast fades
* Gradual fades

The first three kinds of fade are mutually exclusive with one another: only one fade operation (applied to whichever palettes you specify) can be performed at a time. They all share a common control struct, `gPaletteFade`, to manage the state of the fade.

Multiple gradual fades can run alongside each other. They avoid `gPaletteFade` in favor of using one task per fade.

## Summarized contents

### Globals

| Name | Purpose |
| :- | :- |
| [`gPaletteFade`](#gPaletteFade) | Parameters for the ongoing non-gradual palette fade. |
| [`gPlttBufferFaded`](#gPlttBufferFaded) | Palette data being mirrored from RAM to VRAM (unless disabled via `gPaletteFade.bufferTransferDisabled`). |
| [`gPlttBufferUnfaded`](#gPlttBufferUnfaded) | Palette data without any fades applied. Can be used to recover original colors after fading. |

### Functions

#### Callbacks

| Name | Purpose |
| :- | :- |
| [`TransferPlttBuffer`](#TransferPlttBuffer) | Copies `gPlttBufferFaded` from RAM to VRAM (unless disabled via `gPaletteFade.bufferTransferDisabled`). |
| [`UpdatePaletteFade`](#UpdatePaletteFade) | Carries out the next frame of any ongoing non-gradual palette fade. |

#### Color utilities

| Name | Purpose |
| :- | :- |
| `SetBackdropFromColor` | Sets the backdrop color (BG palette 0 color 0). |
| `SetBackdropFromPalette` | Sets the backdrop color (BG palette 0 color 0) to color 0 from a color array. |
| [`TintPalette_CustomTone`](#TintPalette_CustomTone) | Converts a color array to shades of a single hue. |
| [`TintPalette_GrayScale`](#TintPalette_GrayScale) | Converts a color array to greyscale. |
| [`TintPalette_GrayScale2`](#TintPalette_GrayScale2) | Converts a color array to seven shades of grey. |
| [`TintPalette_SepiaTone`](#TintPalette_SepiaTone) | Converts a color array to sepia. |

#### Palette management

| Name | Purpose |
| :- | :- |
| [`BlendPalettes`](#BlendPalettes) | Blends the chosen palettes with a given color and opacity. |
| [`BlendPalettesUnfaded`](#BlendPalettesUnfaded) | Reverts all fades and other changes made to the palettes, and then calls `BlendPalettes`]. |
| [`FillPalette`](#FillPalette) | Fills a portion of palette memory with copies of a single color value. You need not fill an entire palette; you can fill a specific slots within any palette, if desired. |
| [`InvertPlttBuffer`](#InvertPlttBuffer) | Inverts the colors of the chosen palettes. |
| [`LoadCompressedPalette`](#LoadCompressedPalette) | Loads compressed color values into palette memory. |
| [`LoadPalette`](#LoadPalette) | Loads uncompressed color values into palette memory. You need not load an entire palette; you can load just a few colors into specific slots within any palette, if desired. |
| [`TintPlttBuffer`](#TintPlttBuffer) | Tints the colors of the chosen palettes. |
| [`UnfadePlttBuffer`](#UnfadePlttBuffer) | Reverts all fades and similar modifications applied to the chosen palettes. |

#### Palette fades

| Name | Purpose |
| :- | :- |
| [`BeginFastPaletteFade`](#BeginFastPaletteFade) | Begins an optimized palette fade to/from white/black. |
| [`BeginHardwarePaletteFade`](#BeginHardwarePaletteFade) | Begins a hardware-based palette fade to white or black, using I/O registers and screen windows. |
| [`BeginNormalPaletteFade`](#BeginNormalPaletteFade) | Begins a normal palette fade, with controllable speed and blend coefficients. |
| [`BlendPalettesGradually`](#BlendPalettesGradually) | Begins a software-based palette fade using a task. This is the only kind of fade that can run concurrently with other fades. |
| [`ResetPaletteFade`](#ResetPaletteFade) | Resets all ongoing palette fades. |

## Constants and macros

Only macros that are generally useful for external code will be listed here.

| Name | Purpose |
| :- | :- |
| `BG_PLTT_ID(n)` | Used to format a BG palette index <var>n</var> for use by general-purpose palette functions like `LoadPalette`, which can operate on both BG palettes and sprite palettes. |
| `OBJ_PLTT_ID(n)` | Used to format a sprite palette index <var>n</var> for use by general-purpose palette functions like `LoadPalette`, which can operate on both BG palettes and sprite palettes. |

## Globals

### `gPaletteFade`

A control struct that manages palette fades. It has the following members (sorted alphabetically here).

| Member | Purpose | Notes |
| :- | :- | :- |
| `active` | Indicates whether a palette fade is ongoing. | If you have UI that needs to wait for a fade-in or fade-out to complete, this is the variable you poll. |
| `blendColor` | A 15-bit color value. |
| `bufferTransferDisabled` | Disables the effects of `TransferPlttBuffer`, such that `gPlttBufferFaded` will no longer be copied into VRAM. This is orthogonal to whether a fade is actually ongoing. | UI code writes to this fairly often. |
| `delayCounter` | For normal fades, this counter tracks how much of the delay between blend steps has elapsed. |
| `deltaY` | Rate of change for the blend coefficient. |
| `hardwareFadeFinishing` |
| `mode` | Indicates whether a normal, fast, or hardware fade is ongoing. |
| `multipurpose1` |
| `multipurpose2` |
| `objPaletteToggle` | State variable for fast hardware fades. |
| `shouldResetBlendRegisters` | State variable for hardware fades. |
| `softwareFadeFinishing` |
| `softwareFadeFinishingCounter` |
| `targetY` | Target blend coefficient. |
| `y` | Current blend coefficient. |
| `yDec` | Whether the blend coefficient is decreasing. |

### `gPlttBufferFaded`

A copy of all palette data (BGs and sprites), stored in RAM as a flat `u16` array. BG colors are listed before sprite colors.

This variable is automatically updated when palettes are loaded. It represents all desired color palettes as modified by any ongoing palette fades.

**Notes:**

* The `TransferPlttBuffer` function, typically invoked by a main callback, will copy this buffer into VRAM unless palette fades are disabled.

* The *backdrop* color for the screen &mdash; the color shown behind all background layers, visible through transparent regions &mdash; is BG palette 0 color 0. Many parts of the game will write directly to `gPlttBufferFaded[0]` to set the "live" version of this color, though this depends on palette fading being enabled.

### `gPlttBufferUnfaded`

A copy of all palette data (BGs and sprites), stored in RAM as a flat `u16` array. BG colors are listed before sprite colors.

This variable is automatically updated when palettes are loaded. It represnts all desired color palettes without the effects of any ongoing palette fades.


## Functions

### `BeginFastPaletteFade`

Begins a fast fade &mdash; an optimized fade to or from white or black, with minimal configuration options.

These fades always adjust the red, green, and blue component of each color by 16 per step (with bounds-checking). One blend step is performed per frame, but each frame alternates between blending BG palettes and blending sprite palettes, so in practice, every palette is blended by 16 every other frame.

**Arguments:**

<dl>
   <dt><code>u8 submode</code></dt>
      <dd>
         <p>One of the following constants:</p>

* `FAST_FADE_IN_FROM_WHITE`
* `FAST_FADE_OUT_TO_WHITE`
* `FAST_FADE_IN_FROM_BLACK`
* `FAST_FADE_OUT_TO_BLACK`

</dd>
</dl>

### `BeginHardwarePaletteFade`

Begins a hardware-based fade. Hardware fades rely on a GBA I/O feature called "screen windows" (not to be confused with paint windows from `window.h`), which can be used to darken, lighten, or mask out background layers and sprites either across the whole screen or within specific bounding boxes.

This approach to fading will always blend by one unit (i.e. RGB 8) per step: you can make these blends slower by inserting a delay between blend steps, but you can't make it faster (by skipping blend steps, as other fades do).

**Arguments:**

<dl>
   <dt><code>u8 blendCnt</code></dt>
      <dd><p>Values for the <code>BLDCNT</code> I/O register. These control which layers are affected, whether the layers are darkened or brightened, and other associated effects.</p></dd>
   <dt><code>u8 delay</code></dt>
      <dd><p>The delay, in frames, between each blend step.</p></dd>
   <dt><code>u8 y</code></dt>
      <dd><p>The initial intensity of the blend effect. That is: the to-be-blended layers will be lightened or darkened by <code>y * 8</code> RGB.</p></dd>
   <dt><code>u8 targetY</code></dt>
      <dd><p>The final intensity of the blend effect.</p></dd>
   <dt><code>u8 shouldResetBlendRegisters</code></dt>
      <dd><p>If bit 0 is set, then the hardware I/O registers for screen blending will be reset when the fade is complete.</p></dd>
</dl>

### `BeginNormalPaletteFade`

**Arguments:**

<dl>
   <dt><code>u32 selectedPalettes</code></dt>
      <dd>
         <p>The palettes to blend. This is a bitmask with one bit per palette: palette 0 is (1 << 0); palette 1 is (1 << 1); palette 2 is (1 << 2); and so on.</p>
      </dd>
   <dt><code>s8 delay</code></dt>
      <dd>
         <p>The delay, in frames, between each blend step.</p>
         <p>By default, each fade step adjusts the blend coefficient by 2, with the requested delay in frames between each step. However, if the delay specified here is a negative number, then each fade step will adjust the blend coefficient by <code>2 - delay</code>, with no delay between steps.</p>
      </dd>
   <dt><code>u8 startY</code></dt>
      <dd><p>The amount by which the <code>blendColor</code> should be blended (i.e. the color's opacity) at the start of the fade operation.</p></dd>
   <dt><code>u8 targetY</code></dt>
      <dd><p>The amount by which the <code>blendColor</code> should be blended (i.e. the color's opacity) when the fade is complete.</p></dd>
   <dt><code>u16 blendColor</code></dt>
      <dd><p>The color to blend onto the palettes.</p></dd>
</dl>

### `BlendPalettes`

Blends the selected color palettes with the selected color, given the chosen blend coefficient for that color. This just calls `BlendPalette` under the hood.

**Arguments:**

<dl>
   <dt><code>u32 selectedPalettes</code></dt>
      <dd><p>The color palettes to blend. This is a bitmask with one bit per palette: palette 0 is (1 << 0); palette 1 is (1 << 1); palette 2 is (1 << 2); and so on.</p></dd>
   <dt><code>u8 coeff</code></dt>
      <dd><p>The blend coefficient.</p></dd>
   <dt><code>u16 color</code></dt>
      <dd><p>The color that the palettes will be blended with.</p></dd>
</dl>

### `BlendPalettesGradually`

Blend color palettes in a series of steps toward or away from a given color. The vanilla game only uses this for the Kyogre/Groudon fight scene, to flash the screen for lightning: they call it once to fade the BG from white, and another fades the two Pokemon from black.

This function creates a task with handler `Task_BlendPalettesGradually` to carry out the fade. This function doesn't rely on `gPaletteFade`; this means that multiple simultaneous fades are possible, but it also means that `gPaletteFade.active` won't test as true (unless any of the other fade types are running).

**Arguments:**

<dl>
   <dt><code>u32 selectedPalettes</code></dt>
      <dd><p>The color palettes to fade. This is a bitmask with one bit per palette: palette 0 is (1 << 0); palette 1 is (1 << 1); palette 2 is (1 << 2); and so on.</p></dd>
   <dt><code>s8 delay</code></dt>
      <dd><p>Extra frame delay between blend steps.</p></dd>
   <dt><code>u8 coeff</code></dt>
      <dd><p>...</p></dd>
   <dt><code>u8 coeffTarget</code></dt>
      <dd><p>...</p></dd>
   <dt><code>u16 color</code></dt>
      <dd><p>The color that the palette will be blended with.</p></dd>
   <dt><code>u8 priority</code></dt>
      <dd><p>...</p></dd>
   <dt><code>u8 id</code></dt>
      <dd><p>This was intended to let you uniquely identify a "blend palettes gradually" task, if you ran multiple such blends at a time. It allows the unused <code>IsBlendPalettesGraduallyTaskActive</code> function to tell each task apart.</p></dd>
</dl>

### `BlendPalettesUnfaded`

Reverts all existing palette fades, and then calls `BlendPalettes` for you.

**Arguments:** The same as those of `BlendPalettes`.

### `FillPalette`

Overwrites a portion of the CPU-side palette RAM (i.e. `gPlttBufferUnfaded` and `gPlttBufferFaded`) with the desired color.

**Arguments:**

<dl>
   <dt><code>u16 value</code></dt>
      <dd><p>The 15-bit color to write in.</p></dd>
   <dt><code>u16 offset</code></dt>
      <dd><p>

An offset into the palette buffers. This is the same argument as is passed to [`LoadPalette`](#LoadPalette).

</p></dd>
   <dt><code>u16 size</code></dt>
      <dd><p>

The size of the data to overwrite. This is the same argument as is passed to [`LoadPalette`](#LoadPalette).

</p></dd>
</dl>

### `InvertPlttBuffer`

Instantly inverts the colors in the chosen palettes, by overwriting values in `gPlttBufferFaded`.

**Arguments:**

<dl>
   <dt><code>u32 selectedPalettes</code></dt>
      <dd><p>The color palettes to invert. This is a bitmask with one bit per palette: palette 0 is (1 << 0); palette 1 is (1 << 1); palette 2 is (1 << 2); and so on.</p></dd>
</dl>

### `LoadCompressedPalette`

Decompresses palette data, and then loads it to the chosen destination.

Data is decompressed into the `gPaletteDecompressionBuffer` variable, and so cannot be larger than `PLTT_SIZE`, the total size of all loaded palette data in the GBA's memory.

**Arguments:**

<dl>
   <dt><code>u32* src</code></dt>
      <dd><p>The compressed color palette data to load.</p></dd>
   <dt><code>u16 dst</code></dt>
      <dd>
         <p>

An offset into the palette buffers. This is the same argument as is passed to [`LoadPalette`](#LoadPalette).

</p></dd>
   <dt><code>u16 size</code></dt>
      <dd>
         <p>

The decompressed size of the data to copy. This is the same argument as is passed to [`LoadPalette`](#LoadPalette).

</p>
      </dd>
</dl>

### `LoadPalette`

Loads palette data to the chosen destination. This is most often used to load whole color palettes, but it can as easily be used to overwrite single colors in a palette.

**Arguments:**

<dl>
   <dt><code>u16* src</code></dt>
      <dd><p>The color palette data to load.</p></dd>
   <dt><code>u16 dst</code></dt>
      <dd>
         <p>The destination palette (and, optionally, color index) to load to.</p>
         <p>To load data into BG color palette <var>n</var>, specify <code>BG_PLTT_ID(<var>n</var>)</code></p>
         <p>To load data into sprite color palette <var>n</var>, specify <code>OBJ_PLTT_ID(<var>n</var>)</code></p>
         <p>To load data to a specific color slot <var>s</var>, specify <code>BG_PLTT_ID(<var>n</var>) + <var>s</var></code>.</p>
      </dd>
   <dt><code>u16 size</code></dt>
      <dd>
         <p>The size of the data to copy &mdash; broadly, the number of colors times <code>sizeof(uint16_t)</code>, <em>or</em> the <code>sizeof</code> the actual palette data you're loading. To load a 16-color 4bpp palette (the usual), you can specify the convenience constant <code>PLTT_SIZE_4BPP</code>, defined in <code>gba/defines.h</code>.</p>
      </dd>
</dl>

#### Examples

##### Loading a single color palette

```c
static const u16 sMyPalette[] = INCBIN_U16("graphics/title_screen/unused.gbapal");

static void LoadMyPalette(void) {
   LoadPalette(sMyPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
}
```

##### Loading multiple palettes at a time

The build tools include `incbin`, which scans for uses of the `INCBIN_...` macros and replaces them with binary data concatenated into the source code. These macros can accept a single file path, or a comma-separated list of file paths; in the latter case, the binary data from each file will be included, one after the other. This makes it possible to concatenate multiple palette files into a single palette data variable, which in turn makes it possible to load all of those palettes with a single `LoadPalette` call.

```c
static const u16 sMyPalettes[] = INCBIN_U16(
   "graphics/some_screen/some_palette_1.gbapal",
   "graphics/some_screen/some_palette_2.gbapal"
);

static void LoadMyPalette(void) {
   LoadPalette(sMyPalette, BG_PLTT_ID(14), 2 * PLTT_SIZE_4BPP);
}
```

##### Overwriting a single color in a loaded palette

The vanilla title screen does this to animate the glowing markings on Rayquaza's body. (See `UpdateLegendaryMarkingColor` in `title_screen.c`.)

```c
void SetPaletteColor(u8 palette, u8 color_index, u8 r, u8 g, u8 b) {
   u16 color = RGB(r, g, b);
   LoadPalette(&color, BG_PLTT_ID(palette) + color_index, sizeof(color));
}
```

### `ResetPaletteFade`

Resets all ongoing palette fades.

Takes no arguments.

### `TintPalette_CustomTone`

A helper function that acts not on a palette in VRAM, but on an array of color values. This function converts the color values to greyscale, and then scales them by your chosen factors; essentially, this is a sepia filter, but with whatever color you want in place of brown.

**Arguments:**

<dl>
   <dt><code>u16* palette</code></dt>
      <dd><p>The color data to modify.</p></dd>
   <dt><code>u16 count</code></dd>
      <dd><p>The number of colors to modify.</p></dd>
   <dt><code>u16 rTone</code></dd>
      <dd><p>Scaling factor to apply to the red channel. This should generally be an argument of the form <code>Q_8_8(1.0)</code>, using the desired linear scale in place of <code>1.0</code>. The function guards against overflow, so multipliers above 1.0 are safe.</p></dd>
   <dt><code>u16 gTone</code></dd>
      <dd><p>Scaling factor to apply to the green channel.</p></dd>
   <dt><code>u16 bTone</code></dd>
      <dd><p>Scaling factor to apply to the blue channel.</p></dd>
</dl>

### `TintPalette_GrayScale`

A helper function that acts not on a palette in VRAM, but on an array of color values. This function converts the color values to greyscale, modifying them in place.

**Arguments:**

<dl>
   <dt><code>u16* palette</code></dt>
      <dd><p>The color data to modify.</p></dd>
   <dt><code>u16 count</code></dd>
      <dd><p>The number of colors to modify.</p></dd>
</dl>

### `TintPalette_GrayScale2`

A helper function that acts not on a palette in VRAM, but on an array of color values. This function converts the color values to greyscale, modifying them in place. This is a threshold blend: rather than displaying shades of grey anywhere in the GBA's visual range, it generates at most seven shades.

**Arguments:**

<dl>
   <dt><code>u16* palette</code></dt>
      <dd><p>The color data to modify.</p></dd>
   <dt><code>u16 count</code></dd>
      <dd><p>The number of colors to modify.</p></dd>
</dl>

### `TintPalette_SepiaTone`

A helper function that acts not on a palette in VRAM, but on an array of color values. This function converts the color values to sepia, modifying them in place. It does this by first converting them to greyscale, and then scaling the RGB components by factors of 1.2, 1.0, and 0.94, while guarding against overflow of the R component.

**Arguments:**

<dl>
   <dt><code>u16* palette</code></dt>
      <dd><p>The color data to modify.</p></dd>
   <dt><code>u16 count</code></dd>
      <dd><p>The number of colors to modify.</p></dd>
</dl>

### `TintPlttBuffer`

Instantly tints the colors in the chosen palettes, by overwriting values in `gPlttBufferFaded`.

**Arguments:**

<dl>
   <dt><code>u32 selectedPalettes</code></dt>
      <dd><p>The color palettes to tint. This is a bitmask with one bit per palette: palette 0 is (1 << 0); palette 1 is (1 << 1); palette 2 is (1 << 2); and so on.</p></dd>
   <dt><code>s8 r</code></dd>
      <dd><p>Delta for the R component divided by 8.</p></dd>
   <dt><code>s8 g</code></dd>
      <dd><p>Delta for the R component divided by 8.</p></dd>
   <dt><code>s8 b</code></dd>
      <dd><p>Delta for the R component divided by 8.</p></dd>
</dl>

### `TransferPlttBuffer`

If palette fades haven't been disabled (via `gPaletteFade.bufferTransferDisabled`), then this function overwrites palette data in VRAM with the contents of `gPlttBufferFaded`. Additionally, if a hardware fade is currently active, it also updates blend-related registers.

### `UnfadePlttBuffer`

Wholly reverts all fade effects applied to the chosen palettes, by overwriting values in `gPlttBufferFaded` with values from `gPlttBufferUnfaded`.

**Arguments:**

<dl>
   <dt><code>u32 selectedPalettes</code></dt>
      <dd><p>The color palettes to un-fade. This is a bitmask with one bit per palette: palette 0 is (1 << 0); palette 1 is (1 << 1); palette 2 is (1 << 2); and so on.</p></dd>
</dl>

### `UpdatePaletteFade`

Updates any ongoing non-gradual fade. Takes no arguments. Returns one of the following values:

* `PALETTE_FADE_STATUS_DELAY` (2)
* `PALETTE_FADE_STATUS_ACTIVE` (1)
* `PALETTE_FADE_STATUS_DONE` (0)
* `PALETTE_FADE_STATUS_LOADING` (0xFF)

It's common for menus to call this function blindly from main callback 2, without bothering to check whether a fade is ongoing and without checking the result. Some parts of the game take an alternative approach, calling this function from task handlers or callbacks when they know a fade should occur, and polling the return value to see when the fade is complete.

## Notes

* "Gradual" fades should probably be named "concurrent" fades, since the ability to run more than one concurrently is the main difference in functionality.

* The function names that begin with `TintPalette_` should probably be named `TintColorArray_...` instead.

* The vast majority of these functions copy data into palette-related buffers without bounds-checking. It may be prudent to ensure that your palette data won't write out of bounds, e.g. by `_Static_assert`ing that `INCBIN`'d data is the expected size (if you're passing the data's `sizeof` as an argument to `LoadPalette` and friends).

## See also

* [bg.h](#bg.md)
* [`PlttData`](./gba/types.md#PlttData), a helper struct for working with 15-bit colors