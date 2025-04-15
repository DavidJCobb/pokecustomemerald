
# `gba/types.h`

A header that defines various helper types for working with the GBA.

## Integer typedefs

| Typedef | Synonym of |
| :- | :- |
| `bool8`  | `uint8_t` (via `u8`) |
| `bool16` | `uint16_t` (via `u16`) |
| `bool32` | `uint32_t` (via `u32`) |
| `s8`  | `int8_t` |
| `s16` | `int16_t` |
| `s32` | `int32_t` |
| `s64` | `int64_t` |
| `u8`  | `uint8_t` |
| `u16` | `uint16_t` |
| `u32` | `uint32_t` |
| `u64` | `uint64_t` |
| `vbool8`  | `volatile uint8_t` (via `vu8`) |
| `vbool16` | `volatile uint16_t` (via `vu16`) |
| `vbool32` | `volatile uint32_t` (via `vu32`) |
| `vs8`  | `volatile int8_t` |
| `vs16` | `volatile int16_t` |
| `vs32` | `volatile int32_t` |
| `vs64` | `volatile int64_t` |
| `vu8`  | `volatile uint8_t` |
| `vu16` | `volatile uint16_t` |
| `vu32` | `volatile uint32_t` |
| `vu64` | `volatile uint64_t` |
| `f32` | `float` |
| `f64` | `double` |

## Structs

### `BgAffineSrcData`

A helper struct that can be used to define affine transformation parameters for a BG layer. Syscalls exist to copy and convert data from this struct into a `BgAffineDstData`.

```c
struct BgAffineSrcData {
   s32 texX;
   s32 texY;
   s16 scrX;
   s16 scrY;
   s16 sx;
   s16 sy;
   u16 alpha;
};
```

### `BgAffineDstData`

A helper struct that represents affine transformation parameters for a BG layer, as those parameters exist within I/O registers.

```c
struct BgAffineDstData {
   s16 pa;
   s16 pb;
   s16 pc;
   s16 pd;
   s32 dx;
   s32 dy;
};
```

### `BgCnt`

A helper struct that represents the value of the `BG0CNT` through `BG3CNT` I/O registers.

```c
struct BgCnt {
   u16 priority         : 2;
   u16 charBaseBlock    : 2;
   u16 dsCharBaseBlock  : 2;
   u16 mosaic           : 1;
   u16 palettes         : 1;
   u16 screenBaseBlock  : 5;
   u16 areaOverflowMode : 1;
   u16 screenSize       : 2;
};
```

### `OamData`

A helper struct that represents the OAM data for a single sprite in VRAM.

```c
struct {
   /*0x00*/ u32 y          :  8;
   /*0x01*/ u32 affineMode :  2; // 0x1, 0x2 -> 0x4
            u32 objMode    :  2; // 0x4, 0x8 -> 0xC
            u32 mosaic     :  1; // 0x10
            u32 bpp        :  1; // 0x20
            u32 shape      :  2; // 0x40, 0x80 -> 0xC0
            
   /*0x02*/ u32 x          :  9;
            u32 matrixNum  :  5; // bits 3/4 are h-flip/v-flip if not in affine mode
            u32 size       :  2; // 0x4000, 0x8000 -> 0xC000

   /*0x04*/ u16 tileNum    : 10; // 0x3FF
            u16 priority   :  2; // 0x400, 0x800 -> 0xC00
            u16 paletteNum :  4;
            
   /*0x06*/ u16 affineParam;
};
```

**Members:**

<dl>
   <dt><code>y</code></dt>
      <dd><p>The Y-coordinate, in pixels.</p></dd>
   <dt><code>affineMode</code></dt>
      <dd>

One of the following values:

| Name | Meaning |
| :- | :- |
| `ST_OAM_AFFINE_OFF`    | Disables affine mode. |
| `ST_OAM_AFFINE_NORMAL` | Enables affine mode. |
| `ST_OAM_AFFINE_ERASE`  | Disables affine mode, and sets the sprite as invisible. |
| `ST_OAM_AFFINE_DOUBLE` | Indicates that the sprite should be copied to an off-screen canvas double its size, and that affine transformations should be performed there. You would use this for things like square sprites whose corners would be clipped if rotated in place. |

</dd>
   <dt><code>objMode</code></dt>
      <dd>

One of the following values:

| Name | Meaning |
| :- | :- |
| `ST_OAM_OBJ_NORMAL` | The sprite behaves as normal. |
| `ST_OAM_OBJ_BLEND`  | The sprite is semi-transparent. Essentially, this overrides the behavior of the `BLDCNT` I/O register for the pixel beneath the sprite, treating the sprite as the source pixel. |
| `ST_OAM_OBJ_WINDOW` | The sprite will not be displayed, but its non-transparent pixels will be considered part of the "OBJ Window" screen region. |

</dd>
   <dt><code>mosaic</code></dt>
      <dd><p>A boolean.</p></dd>
   <dt><code>bpp</code></dt>
      <dd><p>Either <code>ST_OAM_4BPP</code> or <code>ST_OAM_8BPP</code>.</p></dd>
   <dt><code>shape</code></dt>
      <dd><p>Either <code>ST_OAM_SQUARE</code>, <code>ST_OAM_H_RECTANGLE</code>, or <code>ST_OAM_V_RECTANGLE</code>.</p></dd>
   <dt><code>x</code></dt>
      <dd><p>The X-coordinate, in pixels.</p></dd>
   <dt><code>matrixNum</code></dt>
      <dd><p>If the sprite is in affine mode, then this is the index of an affine transformation matrix stored in VRAM. Otherwise, this is zero bitwise-OR'd with any combination of the flags <code>ST_OAM_HFLIP</code> and <code>ST_OAM_VFLIP</code>, indicating that the sprite should be mirrored along either or both axes.</p></dd>
   <dt><code>size</code></dt>
      <dd>

The meaning of this value depends on the sprite's shape.

| Value | `ST_OAM_SQUARE` | `ST_OAM_H_RECTANGLE` | `ST_OAM_V_RECTANGLE` |
| :- | :- | :- | :- |
| `ST_OAM_SIZE_0` |   8x8 | 16x8  |  8x16 |
| `ST_OAM_SIZE_1` | 16x16 | 32x8  |  8x32 |
| `ST_OAM_SIZE_2` | 32x32 | 32x16 | 16x32 |
| `ST_OAM_SIZE_3` | 64x64 | 64x32 | 32x64 |

</dd>
   <dt><code>tileNum</code></dt>
      <dd><p>The sprite's tile number in VRAM; a value in the range [0, 0x3FF].</p></dd>
   <dt><code>priority</code></dt>
      <dd><p>The sprite's priority, indicating which BG layers it draws in front of. Given a sprite and a BG layer with the same priority, the sprite draws on top.</p></dd>
   <dt><code>paletteNum</code></dt>
      <dd><p>Which palette index the sprite uses. This is unused if the sprite uses 8bpp colors; in that case, the sprite's pixel data treats the usual 16 palettes as one 256-color palette.</p></dd>
   <dt><code>affineParam</code></dt>
      <dd>
         <p>This value is not meaningful on its own.</p>
         <p>Sprites in affine mode can use affine transformation matrices stored in VRAM. Each matrix has four 16-bit parameters. These parameters are intermingled into the OAM data: the <code>affineParam</code> values for OAM entries 0, 1, 2, and 3 constitute the first such matrix; the values for OAM entries 4, 5, 6, and 7 constitute the second such matrix; and so on.</p>
      </dd>
</dl>

### `ObjAffineSrcData`

```c
struct ObjAffineSrcData {
   s16 xScale;
   s16 yScale;
   u16 rotation;
};
```

### `PlttData`

A helper struct that represents a single 15-bit color.

```c
struct {
   u16 r : 5;
   u16 g : 5;
   u16 b : 5;
   u16 unused : 1;
};
```

### `SioMultiCnt`

The SIO Control Structure for multiplayer links.

```c
struct SioMultiCnt {
   u16 baudRate    : 2; // baud rate
   u16 si          : 1; // SI terminal
   u16 sd          : 1; // SD terminal
   u16 id          : 2; // ID
   u16 error       : 1; // error flag
   u16 enable      : 1; // SIO enable
   u16 unused_11_8 : 4;
   u16 mode        : 2; // communication mode (should equal 2)
   u16 intrEnable  : 1; // IRQ enable
   u16 unused_15   : 1;
   u16 data;            // data
};
```

**Members:**

<dl>
   <dt><code>baudRate</code></dt>
      <dd><p>One of the following constants, defining the data transfer speed in bits per second: <code>ST_SIO_9600_BPS</code>; <code>ST_SIO_38400_BPS</code>; <code>ST_SIO_57600_BPS</code>; or <code>ST_SIO_115200_BPS</code>.</p></dd>
   <dt><code>si</code></dt>
      <dd><p>Read-only status bool. False if this is a parent connection, or true if this is a child connection.</p></dd>
   <dt><code>sd</code></dt>
      <dd><p>Read-only status bool. False indicates a bad connection; true indicates that all GBAs are ready.</p></dd>
   <dt><code>id</code></dt>
      <dd><p>Read-only. Indicates which participant in the link this console is: 0 if it's the parent, or 1, 2, or 3 if it's one of the children.</p></dd>
   <dt><code>error</code></dt>
      <dd><p>Read-only status bool indicating whether an error has occurred.</p></dd>
   <dt><code>enable</code></dt>
      <dd><p>Read-only for all consoles except the connection host. Indicates whether the connection is inactive (0), or started/busy (1).</p></dd>
   <dt><code>mode</code></dt>
      <dd><p>Must be <code>ST_SIO_MULTI_MODE</code> for multiplayer mode.</p></dd>
   <dt><code>intrEnable</code></dt>
      <dd><p>If set, this bool indicates that an IRQ will occur upon link completion.</p></dd>
   <dt><code>data</code></dt>
      <dd><p>The outgoing data to send via <code>SIOMLT_SEND</code>.</p></dd>
</dl>