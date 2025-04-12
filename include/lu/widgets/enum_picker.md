
# Enum picker

A widget that provides the visual effects for letting the user pick from an enumeration. Specifically, this widget consists of two sprites: outward-facing arrows, to be placed to either side of the currently displayed enum value. When the user changes the value, we play a sound and animate the arrows.

This widget doesn't handle displaying the enum's current value, nor applying changes to the value. It just handles the visual behaviors of the widget. You'll need to call the appropriate functions at the appropriate times.

The widget takes a base position and thereafter positions itself based on the current "row." Rows are assumed to be two tiles tall, and to consist of text in the normal Latin-encoding font.