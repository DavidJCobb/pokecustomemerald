
# `VUIFrame`

A helper struct that can be used to draw a frame of background tiles at a given region. Pass it as an argument to `VUIFrame_Draw`.

A frame definition consists of:

* Four corners. Each corner has:
  * A size, in tiles.
  * Tile info. This is either a pointer to an array of tile IDs (stored in row-major order), or a sentinel constant indicating that this corner is a mirrored version of one of the other corners.
* Four edges. Each edge can specify a single tile, or can optionally specify that it is a mirrored version of the opposite edge. You can also optionally flip the tile whether or not it's mirrored.

Additionally, specifying `VUIFRAME_TILEID_UNCHANGED` as a tile ID means that we won't draw a tile to that area.

An example of a frame definition:

```c
static const u16 sChromeFrameCornerTileIDs_Upper[] = {
   COMMONTILE_CHROME_CORNER_UPPER_A,
   COMMONTILE_CHROME_CORNER_UPPER_B
};
static const u16 sChromeFrameCornerTileIDs_Lower[] = {
   COMMONTILE_CHROME_CORNER_LOWER_A,
   COMMONTILE_CHROME_CORNER_LOWER_B
};
static const VUIFrame sChromeFrame = {
   .corners = {
      .sizes = {
         .tl = { 1, 2 },
         .bl = { 1, 2 },
      },
      .tiles = {
         .tl = { .ids    = sChromeFrameCornerTileIDs_Upper },
         .tr = { .mirror = VUIFRAME_CORNER_MIRROR_X },
         .bl = { .ids    = sChromeFrameCornerTileIDs_Lower },
         .br = { .mirror = VUIFRAME_CORNER_MIRROR_X },
      },
   },
   .edges = {
      .left = {
         .tile_id = COMMONTILE_CHROME_EDGE_L,
      },
      .right = {
         .mirror = TRUE,
      },
      .top = {
         .tile_id = COMMONTILE_CHROME_EDGE_TOP,
      },
      .bottom = {
         .tile_id = COMMONTILE_CHROME_EDGE_BOT,
      },
   },
};
```

Here, we define the top-left and bottom-left corners as 1x2-tile arrangements. We then specify that the top-right an bottom-right corners look identical to the left-side counterparts save for being horizontally flipped. We handle the left- and right-side edges similarly; however, the top and bottom edges are both defined (neither mirrors the other).