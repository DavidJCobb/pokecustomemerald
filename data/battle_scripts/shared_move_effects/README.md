
This folder holds move effects that are shared by multiple moves.

## Status infliction 

Moves can apply statuses in one of two ways:

* Via the move script itself. All logic for applying the status &mdash; immunity checks, what have you &mdash; is carried out in script.
* Via a `MOVE_EFFECT` handler. These run hardcoded logic, but may also `call` into certain scripts, e.g. `BattleScript_MoveEffectBurn`.

The <code>status_<var>foo</var>.inc</code> files in this folder are move scripts.