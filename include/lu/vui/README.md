
# VUI

A widget-based UI framework, wherein UI widgets have simple V-tables.

All UIs exist within a `VUIContext`, which can be thought of as an *abstract grid*. A UI element takes up a rectangular area of grid tiles, and the D-Pad can be used to move from one widget to any directly adjacent widget.

Widgets can optionally contain an *abstract subgrid*, consisting of a *size* and a *focus position*. If a widget contains an abstract subgrid, then these two properties influence directional navigation from that widget to an adjacent widget.

To understand the intended use case for this, consider the vanilla naming screen: a four-row keyboard next to three rows of buttons. You can move a cursor within the keyboard, and if you move it outside of either edge of the keyboard, it'll be placed on one of the buttons. The game dynamically maps from the four-row coordinate space into the three-row coordinate space. Within VUI, you could implement the keyboard as a widget with a 1x3 size and an 8x4 subgrid, and then place the button widgets in the grid column to the keyboard widget's right:

<table>
<tbody>
<tr>
<td rowSpan="3">
Keyboard
<table>
<tbody>
<tr><td>A</td><td>B</td><td>C</td><td>D</td><td>E</td><td>F</td><td></td><td>.</td></tr>
<tr><td>G</td><td>H</td><td>I</td><td>J</td><td>K</td><td>L</td><td></td><td>,</td></tr>
<tr><td>M</td><td>N</td><td>O</td><td>P</td><td>Q</td><td>R</td><td>S</td><td></td></tr>
<tr><td>T</td><td>U</td><td>V</td><td>W</td><td>X</td><td>Y</td><td>Z</td><td></td></tr>
</tbody>
</table>
</td>
<td>OK</td>
</tr>
<tr><td>Backspace</td></tr>
<tr><td>Cancel</td></tr>
</tbody>
</table>

Then, the keyboard widget could use the subgrid focus position as its cursor position, such that when you move the cursor rightward out of the widget, we'll move widget focus to the correct button based on where the cursor was within the keyboard.

## Classes

### `VUIContext`

A container for `VUIWidget`s. A context manages widget focus and directional navigation between widgets.

You can optionally (on a per-axis basis) enable wraparound on the context. If you do, you must define the context's size (`w` and `h`). If you `allow_wraparound_x`, then it'll become possible to navigate between any widget whose X-extent includes the `0` coordinate, and any widget whose X-extent includes the `w - 1` coordinate; thus, also, for the Y-axis.

### `VUIWidget`

Base class for VUI widgets.

| Field group | Fields | Description |
| :- | :- | :- |
| Header | `functions` | Pointer to a v-table-like structure. |
| Flags | `disabled`<br/>`focusable`<br/>`has_subgrid` |
| Grid metrics | `pos`<br/>`size` | Fields describing the area that the widget takes up within the containing context's grid. |
| Subgrid | `subgrid_size`<br/>`subgrid_focus` | If the widget `has_subgrid`, then these fields indicate the size of the widget's subgrid, and the position of the focus position within that subgrid. |

A widget's size within the context grid must be non-zero on both axes.

If the subgrid is enabled but its size is 0 along any axis, then the size that is used is the widget's size within the context grid.