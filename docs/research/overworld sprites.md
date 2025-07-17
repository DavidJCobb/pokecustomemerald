
# Overworld sprites

When you're on the overworld, the game loads a fixed set of palettes for overworld sprites. All overworld sprites use these fixed palettes; there's no system for dynamically loading different palettes based on what sprites are on-screen or even on the current map.

Other Emerald forks and hacks have built systems for dynamically allocated overworld palettes. However, I'd like to go a step further, enabling not only dynamic palette allocation, but also things like skin tone variation. This leads to an interesting idea: what if we divide overworld sprites up, so that we can have one common palette with all skin tones, rather than having to replicate skin tones across each loaded palette?

## Palette notes

* Skin tone colors:
  * 255, 213, 180 (highlight)
  * 255, 197, 148 (midtone)
  * 222, 148, 115 (shadow)
  * 123,  65,  65 (outline)
  * Eyes often use (0, 0, 0).
  * Older male characters sometimes use (255, 255, 255) for small highlights on bald or balding heads. Generally, white is also used as another color, e.g. for a shirt, glasses, or elderly hair.
* Palette usage
  * 4 skin, 5 other
    * `swimmer_f`
  * 4 skin, 6 other
    * `swimmer_m`
  * 4 skin, 9 other
    * `teala`

### Specific desired palette swaps

* Skin colors, localized to specific OWs on the map
* Trainer classes
  * Ace Trainer
    * `man_3` gold -> green (hair)
    * `woman_5` brown -> green (hair)
  * PKMN Ranger
    * `camper` green -> orange
    * `picnicker` green -> orange
  * Ruin Maniac
    * `hiker` pink -> tan; blue -> tan; olive (legs) -> skin

### Palette colors by sprite

| Sprite | Palette | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | Clothing colors | Hair colors |
| :- | -: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | -: | -: |
| `artist` | NPC4 | skin light | skin mid | skin dark | skin line | | | drape shadow | hat light | hat mid | hat line | drape mid | drape line + hair mid | hair shadow | drape light | eyes | 7 | 2 |
| `beauty` | NPC3 | skin light | skin mid | skin dark | skin line | clothes mid | clothes shadow | clothes line | hair light | hair mid | hair line | | | | clothes light + earrings | eyes | 4 | 3 |
| `black_belt` | NPC3 | skin light | skin mid | skin dark | skin line | clothes mid | clothes shadow | clothes line | belt light | belt mid | belt line | hair light | hair mid | hair line + brows | clothes light | eyes | 7 | 3 |
| `boy_1` | NPC3 | skin light | skin mid | skin dark | skin line | clothes light | clothes mid | clothes dark | | shoes mid | shoes line | hair light | hair mid | hair dark + hair line | | eyes + shorts | 5 | 3 |
| `boy_2` | NPC1 | skin light | skin mid | skin dark | skin line | overalls light | overalls mid | overalls line | shirt light | shirt mid | shirt line | hair light | hair mid | hair dark + hair line | | eyes + legs line | 7 | 3 |
| `boy_3` | NPC4 | skin light | skin mid | skin dark | skin line | hair light + shirt light | hair mid + shirt mid + shoes mid | hair line + shirt line + shoes line | vest light | vest mid | vest dark | | | | | eyes | 6 | 3 |
| `bug_catcher` | NPC1 | skin light + hat light | skin mid + hat mid | skin dark + hat dark | skin line | | | | shirt light | shirt mid + hat accent mid | shirt dark + hat accent line | hat texture | hair mid + shoes mid | hair line + hat line + shoes line | | eyes + pants-to-legs line | 8 | 2 |
| `cameraman` | NPC4 |
| `camper` | NPC3 | skin light | skin mid | skin dark | skin line | shirt mid | shirt dark | shirt line | coat light + hat light | coat mid + hat mid | coat line + hat line | | hair mid | hair dark + hair line | shirt light | eyes |
| `contest_judge` | NPC3 |
| `cook` | NPC3 |
| `cycling_triathlete_f` | NPC1 |
| `cycling_triathlete_m` | NPC3 |
| `devon_employee` | NPC2 |
| `expert_f` | NPC4 |
| `expert_m` | NPC4 |
| `fat_man` | NPC1 |
| `fisherman` | NPC2 |
| `gameboy_kid` | NPC3 |
| `gentleman` | NPC3 |
| `girl_1` | NPC2 |
| `girl_2` | NPC3 |
| `girl_3` | NPC2 |
| `hex_maniac` | NPC4 |
| `hiker` | NPC1 |
| `hot_springs_old_woman` | NPC4 |
| `lass` | NPC4 |
| `leaf` |
| `link_receptionist` | NPC3 |
| `little_boy` | NPC4 |
| `little_girl` | NPC2 |
| `man_1` | NPC3 |
| `man_2` | NPC3 |
| `man_3` | NPC2 |
| `man_4` | NPC4 |
| `man_5` | NPC2 |
| `maniac` | NPC4 |
| `mart_employee` | NPC1 |
| `mauville_old_man_1` | NPC3 |
| `mauville_old_man_2` | NPC4 |
| `mom` | NPC4 |
| `mystery_event_deliveryman` | NPC1 |
| `ninja_boy` | NPC1 |
| `nurse` | NPC1 |
| `old_man` | NPC4 |
| `old_woman` | NPC3 |
| `picnicker` | NPC3 |
| `pokefan_f` | NPC2 |
| `pokefan_m` | NPC2 |
| `prof_birch` | NPC3 |
| `psychic_m` | NPC4 |
| `red` |
| `reporter_f` | NPC4 |
| `reporter_m` | NPC4 |
| `rich_boy` | NPC3 |
| `rooftop_sale_woman` | NPC1 |
| `rs_little_boy` | NPC1 |
| `running_triathlete_f` | NPC2 |
| `running_triathlete_m` | NPC4 |
| `sailor` | NPC1 |
| `school_kid_m` | NPC1 |
| `scientist_1` | NPC3 |
| `scientist_2` | NPC1 |
| `scott` | NPC1 |
| `steven` | NPC4 |
| `swimmer_f` | NPC2 |
| `swimmer_m` | NPC1 |
| `teala` | NPC1 |
| `tuber_f` | NPC1 | skin light | skin mid | skin dark | skin line | tube light | tube mid | tube line | | | | hair light | hair mid | hair line + eyes | tube shine | | 4 | 3 |
| `tuber_m` | NPC2 |
| `tuber_m_swimming` | NPC2 |
| `twin` | NPC2 |
| `union_room_attendant` | NPC3 |
| `unused_woman` | NPC1 |
| `wallace` | NPC4 |
| `wally` | NPC1 |
| `woman_1` | NPC1 |
| `woman_2` | NPC3 |
| `woman_3` | NPC2 |
| `woman_4` | NPC1 |
| `woman_5` | NPC2 |
| `youngster` | NPC1 |

## Sprite usage

* The GBA can display 128 sprites on-screen at a time.
* Sprite maximum per scanline:
  * If "H-Blank Interval Free" in `DISPCNT` is set, then the GBA has 954 (`240*4-6`) sprite rendering cycles per scanline. Otherwise, it has 1210 (`304*4-6`) cycles.
  * A sprite whose width *w* is in the range [8px, 64px] takes *w* cycles to render if it isn't using affine transforms, or (10 + *n* * 2) cycles if it is.
  * If my math is right, then, the best-case scenario for 16px-wide overworld sprites is 75 sprites per scanline.
* Known sprite usage
  * Overworld sprites
  * World interactions
    * Footsteps
      * Bike tire tracks
      * Ripples and splashing in shallow water
      * Sand footprints
      * Short grass over feet (`FldEff_ShortGrass`)
      * Tall grass scattering (`FldEff_TallGrass`)
    * Hot springs water
    * Shadows (ledge jumps and Acro Bike wheelie hops) via `FldEff_Shadow`
    * Warp arrow sprite x1
      * `CreateWarpArrowSprite` and friends in `field_effect_helpers.c`
  * Attachments
    * Fly bird
    * Ninja NPC disguise standees
    * Surf blob
  * Berry trees
    * Growth sparkle
  * One sprite for all reflections shown on-screen
  * Battle transitions
  * Field effects
    * Field moves
      * Pokemon sprite (the Pokemon using the move for you)
    * PokeCenter healing (7)
      * Six sprites for Poke Balls
      * One sprite for the flashing glow/monitor
    * Pop out of ash (1)
      * `FldEff_AshPuff`
    * `FldEff_RayquazaSpotlight` (1)
    * `FldEff_NPCFlyOut` (1)
    * `CreateDeoxysRockFragments` (4)
    * Tile interactions
      * Lavaridge Gym basement lifts (1)
        * One sprite for the launch
  * Weathers