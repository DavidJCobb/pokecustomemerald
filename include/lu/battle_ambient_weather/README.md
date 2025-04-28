
# Battle ambient weather

When you're choosing an action to perform, the current weather animation now plays in a loop.

This isn't built on the battle animation system, because that system does not (as far as I know) give us a clean way to forcibly interrupt battle animations on demand. Rather, the new implementation is done entirely in C, mimicking the animation commands that the normal weather animations would execute.