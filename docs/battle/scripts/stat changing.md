
## Stat-changing move scripts

There are a few basic approaches to changing stats in move scripts, assuming the vanilla command set.

* Check whether the stat change is possible via `jumpifstat`. If so, run `playstatchangeanimation` for every possible combination of stat changes you might cause, followed by an invocation of `setstatchanger` and `statbuffchange` for each possible stat.

  * You would also make `attackanimation` and friends conditional on `jumpifstat`.

* Check whether the stat change is possible via `jumpifstat`, and then run `statbuffchange` for each stat you wish to change. Since you already know the stat change succeeded, run `setgraphicalstatchangevalues` and `playanimation` to play the stat-change animation.

* Run `statbuffchange` blindly, and then check the multi-string chooser byte to see what message it wants to print. You can use this to tell whether the stat change failed (whether for any reason, or specifically because the stat is already at its minimum/maximum).

  * You would also make `attackanimation` and friends conditional on the result of `statbuffchange`.

### New commands

In this hack, we've removed `playstatchangeanimation`, `statbuffchange`, and `setgraphicalstatchangevalues`, and we've added two new commands: `trystatchange` and `showstatchange`.

The `trystatchange` command attempts to apply the stat change and, unless told not to, will send a battle controller message to display both the stat-change result text and the stat-change animation. You may want to suppress the visuals when a move script's control flow depends on whether the stat change is successful (i.e. if you only want to play the `attackanimation` if the stat change succeeds).

The `showstatchange` command sends the aforementioned battle controller message for the last stat change applied via `trystatchange` (or via a move effect; we've changed the native code for those).