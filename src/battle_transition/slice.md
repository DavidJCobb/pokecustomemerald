
# Slice transition

This transition pulls each scanline outward (i.e. one scanline is pulled leftward, and the next is pulled rightward, and on and on it goes), creating a widening gap at the center of the screen that eventually consumes the screen.

Scanlines are pulled at a speed measured in pixels per frame, with an acceleration factor measured in two-hundred-fifty-sixths of a pixel per frame. Every frame, the speed is increased by the acceleration, and then the acceleration is doubled. Both have maximums: the speed caps out at 16 pixels per frame, while the acceleration caps out at 128/256 pixels per frame. The transition will complete when each scanline is pulled fully off-screen (i.e. when they have shifted by `DISPLAY_WIDTH`, or 240px).

The relevant mathematical formulae are:

* $a\left(x\right)=\min\left(0.5,\ \frac{2^{x}}{256}\right)$
* $s\left(x\right)=\min\left(16,\ 1\ +\ a\left(x\right)\right)$
* $y=\sum_{n=0}^{x}s\left(x\right)$

Given *a(x)* as the acceleration on frame *x*, *s(x)* as the speed on frame *x*, and *y(x)* as the cumulative distance any scanline has moved on frame *x*, such that the animation should finish when *y(x) = 240* (i.e. roughly on frame 159, or 2.5 seconds in).

This assumes the default initial speed and acceleration of 1 and 1/256, respectively. In this codebase, we change the initial speed to 4 frames per second and the initial acceleration to 64/256 frames per second.