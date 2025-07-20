
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

Base class for VUI widgets. Subclasses must have field lists that begin with `VUI_WIDGET_SUBCLASS_HEADER;`, and must not define a field named `base` (as the macro defines that field as

| Field group | Fields | Description |
| :- | :- | :- |
| Header | `functions` | Pointer to a v-table-like structure. |
| Flags | `disabled`<br/>`focusable`<br/>`has_subgrid` |
| Grid metrics | `pos`<br/>`size` | Fields describing the area that the widget takes up within the containing context's grid. |
| Subgrid | `subgrid_size`<br/>`subgrid_focus` | If the widget `has_subgrid`, then these fields indicate the size of the widget's subgrid, and the position of the focus position within that subgrid. |

A widget's size within the context grid must be non-zero on both axes.

If the subgrid is enabled but its size is 0 along any axis, then the size that is used is the widget's size within the context grid.

## Macros

### `VUI_WIDGET_TYPECHECK_AND_CALL`
Helper macro, used to define macro-based shims for `VUIWidget` functions. These shims type-check and adapt pointer-type arguments, such that you can pass a `VUIWidget*` pointer *or* a pointer of any subclass type, and the latter will be cast for you (since C doesn't formally *have* subclasses and we have to do shenanigans to recreate their effect).

For example:

```c
extern void VUIWidget_SetGridMetrics_(VUIWidget* this, u8 x, u8 y, u8 w, u8 h);
#define VUIWidget_SetGridMetrics(widget, x, y, w, h) \
   VUI_WIDGET_TYPECHECK_AND_CALL(VUIWidget_SetGridMetrics_, widget, x, y, w, h)
```

Here, `VUIWidget_SetGridMetrics_` is the real function, but the compiler will error if you pass a pointer of any type other than `VUIWidget*`. The `VUIWidget_SetGridMetrics` macro (note the lack of a trailing underscore) will accept a pointer to an instance of `VUIWidget` or any subclass thereof.

The way this macro works is by using `_Generic` to check for the presence of a `__is_vui_widget__[0]` member at the start of the pointed-to widget. Each widget class has, at the start of its member list, a macro that adds a zero-length array by that name. [Zero-length arrays are a GNU C extension](https://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html) which are guaranteed not to increase the size of a struct unless they cause the compiler to generate tail padding. This means that even if we have sub-sub-classes and deeper, giving each its own ZLA as a header field won't create "head padding" on the structs.

(A more portable equivalent to this would entail defining `VUI_WIDGET_TYPECHECK_AND_EXEC` so that it generates an anonymous union field like `union { int __is_vui_widget__[1]; VUIWidget base; }`, such that we have `__is_vui_widget__` as a referenceable identifier that doesn't actually add any size to the struct, by overlapping it with the `base` member that we also need. This would only allow subclasses to be passed in, but we could allow the `__is_vui_widget__` check to work on the base `VUIWidget` class by performing similar union trickery on its v-table pointer. Hm... Perhaps I should do exactly these things.)

### `VUI_WIDGET_TYPECHECK_AND_EXEC`
Similar to `VUI_WIDGET_TYPECHECK_AND_CALL`, but for expressions rather than function calls.
