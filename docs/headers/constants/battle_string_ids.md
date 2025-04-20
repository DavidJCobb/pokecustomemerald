# Battle strings

Most battle strings are named based on their content, rather than their purpose or context. This table lists the strings and any available information about how they're used, and what tokens are available for substitution.

| `STRINGID_...` constant | Variable | Category | Subcategory | Description |
| :- | :- | :-: | :-: | :- |
| `STRINGID_INTROMSG` |
| `STRINGID_INTROSENDOUT` |
| `STRINGID_RETURNMON` |
| `STRINGID_SWITCHINMON` |
| `STRINGID_USEDMOVE` |
| `STRINGID_BATTLEEND` |
| 6 | | unused |
| 7 | | unused |
| 8 | | unused |
| 9 | | unused |
| 10 | | unused |
| 11 | | unused |
| `STRINGID_TRAINER1LOSETEXT` |
| `STRINGID_PKMNGAINEDEXP` | | system | EXP | **Buffer 1** gained **Buffer 3** EXP points. **Buffer 2** may hold an adjective (if the Pokemon's EXP gain was boosted by it being traded), and will otherwise be an empty string. |
| `STRINGID_PKMNGREWTOLV` | | system | EXP | **Buffer 1** grew to Level **Buffer 2**.
| `STRINGID_PKMNLEARNEDMOVE` | | system | learning moves | **Buffer 1** learned **Buffer 2** after leveling up. |
| `STRINGID_TRYTOLEARNMOVE1` | | system | learning moves | **Buffer 1** is trying to learn **Buffer 2** after leveling up... |
| `STRINGID_TRYTOLEARNMOVE2` | | system | learning moves | ...but **Buffer 1** can't learn more than four moves. |
| `STRINGID_TRYTOLEARNMOVE3` | | system | learning moves | Delete a move to make room for **Buffer 2**? |
| `STRINGID_PKMNFORGOTMOVE` | | system | learning moves | **Buffer 1** forgot **Buffer 2**. |
| `STRINGID_STOPLEARNINGMOVE` | | system | learning moves | Prompts the player as to whether they want to stop learning **Buffer 2**. |
| `STRINGID_DIDNOTLEARNMOVE` | | system | learning moves | Confirms the player's refusal to teach **Buffer 1** the **Buffer 2** move. |
| `STRINGID_PKMNLEARNEDMOVE2` | | system | learning moves | Confirms the player's decision to teach **Buffer 1** the **Buffer 2** move. |
| `STRINGID_ATTACKMISSED` | | common move results | | **Attacker**'s attack missed. |
| `STRINGID_PKMNPROTECTEDITSELF` | | moves | Protect | **Defender** protected itself. |
| `STRINGID_STATSWONTINCREASE2` | | stats | failure |
| `STRINGID_AVOIDEDDAMAGE` | | common move results | failure | **Defender** avoided damage using **Defender Ability**. |
| `STRINGID_ITDOESNTAFFECT` | | common move results | type effectiveness | **Attacker**'s move doesn't affect **Defender**. |
| `STRINGID_ATTACKERFAINTED` | | common move results | faint | **Attacker** fainted. |
| `STRINGID_TARGETFAINTED` | | common move results | faint | **Defender** fainted. |
| `STRINGID_PLAYERGOTMONEY` | | battle outcome | victory | The player received **Buffer 1** money. |
| `STRINGID_PLAYERWHITEOUT` | | battle outcome | defeat | States that the player has no usable Pokemon left. |
| `STRINGID_PLAYERWHITEOUT2` | | battle outcome | defeat | States that the player "whited out." |
| `STRINGID_PREVENTSESCAPE` |
| `STRINGID_HITXTIMES` | | common move results | multi-hit |
| `STRINGID_PKMNFELLASLEEP` | | status | sleep |
| `STRINGID_PKMNMADESLEEP` | | status | sleep |
| `STRINGID_PKMNALREADYASLEEP` | | status | sleep | **Defender** cannot become asleep, because they are already asleep.
| `STRINGID_PKMNALREADYASLEEP2` | | status | sleep | **Attacker** cannot become asleep, because they are already asleep. |
| `STRINGID_PKMNWASNTAFFECTED` |
| `STRINGID_PKMNWASPOISONED` | | status | poison |
| `STRINGID_PKMNPOISONEDBY` | | status | poison |
| `STRINGID_PKMNHURTBYPOISON` | | status | poison | **Attacker** is hurt by its poison status. |
| `STRINGID_PKMNALREADYPOISONED` | | status | poison | **Defender** cannot become poisoned, because they are already poisoned. |
| `STRINGID_PKMNBADLYPOISONED` | | status | poison |
| `STRINGID_PKMNENERGYDRAINED` | | | | **Defender**'s energy was drained. |
| `STRINGID_PKMNWASBURNED` | | status | burn |
| `STRINGID_PKMNBURNEDBY` | | status | burn |
| `STRINGID_PKMNHURTBYBURN` | | status | burn |
| `STRINGID_PKMNWASFROZEN` | | status | freeze |
| `STRINGID_PKMNFROZENBY` | | status | freeze |
| `STRINGID_PKMNISFROZEN` | | status | freeze |
| `STRINGID_PKMNWASDEFROSTED` | | status | freeze | **Defender** was defrosted. |
| `STRINGID_PKMNWASDEFROSTED2` | | status | freeze | **Attacker** was defrosted. |
| `STRINGID_PKMNWASDEFROSTEDBY` | | status | freeze | **Attacker** was defrosted by **Current Move**. |
| `STRINGID_PKMNWASPARALYZED` | | status | paralysis |
| `STRINGID_PKMNWASPARALYZEDBY` | | status | paralysis |
| `STRINGID_PKMNISPARALYZED` | | status | paralysis |
| `STRINGID_PKMNISALREADYPARALYZED` | | status | paralysis | **Defender** cannot become paralyzed, because they are already paralyzed. |
| `STRINGID_PKMNHEALEDPARALYSIS` | | status | paralysis | **Defender** was healed of paralysis. |
| `STRINGID_PKMNDREAMEATEN` | | moves | Dream Eater |
| `STRINGID_STATSWONTINCREASE` | | stats | failure | **Attacker**'s **Buffer 1** stat cannot be increased any further. |
| `STRINGID_STATSWONTDECREASE` | | stats | failure | **Defender**'s **Buffer 1** stat cannot be decreased any further. |
| `STRINGID_TEAMSTOPPEDWORKING` |
| `STRINGID_FOESTOPPEDWORKING` |
| `STRINGID_PKMNISCONFUSED` | | status | confusion | **Attacker** is confused. Displayed before the attempted or failed attack. |
| `STRINGID_PKMNHEALEDCONFUSION` | | status | confusion | **Attacker**'s confusion has spontaneously ended. Displayed before their attack. |
| `STRINGID_PKMNWASCONFUSED` | | status | confusion |
| `STRINGID_PKMNALREADYCONFUSED` | | status | confusion | **Defender** cannot become confused, because they are already confused. |
| `STRINGID_PKMNFELLINLOVE` | | status | infatuation |
| `STRINGID_PKMNINLOVE` | | status | infatuation |
| `STRINGID_PKMNIMMOBILIZEDBYLOVE` | | status | infatuation | **Attacker** has failed to attack due to the status condition. |
| `STRINGID_PKMNBLOWNAWAY` |
| `STRINGID_PKMNCHANGEDTYPE` |
| `STRINGID_PKMNFLINCHED` | | common move results | failure | **Attacker** has failed to attack, because they flinched. |
| `STRINGID_PKMNREGAINEDHEALTH` |
| `STRINGID_PKMNHPFULL` |
| `STRINGID_PKMNRAISEDSPDEF` | | stats | increase |
| `STRINGID_PKMNRAISEDDEF` | | stats | increase |
| `STRINGID_PKMNCOVEREDBYVEIL` |
| `STRINGID_PKMNUSEDSAFEGUARD` |
| `STRINGID_PKMNSAFEGUARDEXPIRED` |
| `STRINGID_PKMNWENTTOSLEEP` |
| `STRINGID_PKMNSLEPTHEALTHY` |
| `STRINGID_PKMNWHIPPEDWHIRLWIND` |
| `STRINGID_PKMNTOOKSUNLIGHT` |
| `STRINGID_PKMNLOWEREDHEAD` |
| `STRINGID_PKMNISGLOWING` |
| `STRINGID_PKMNFLEWHIGH` | | moves | Fly | **Attacker** has just used Fly, and entered its semi-invulnerable state. |
| `STRINGID_PKMNDUGHOLE` | | moves | Dig | **Attacker** has just used Dig, and entered its semi-invulnerable state. |
| `STRINGID_PKMNSQUEEZEDBYBIND` | | paired statuses | wrap | **Defender** has been wrapped by **Attacker** using Bind.[^wrap-status-onset] |
| `STRINGID_PKMNTRAPPEDINVORTEX` |
| `STRINGID_PKMNWRAPPEDBY` | | paired statuses | wrap | **Defender** has been wrapped by **Attacker** using Wrap.[^wrap-status-onset] |
| `STRINGID_PKMNCLAMPED` | | paired statuses | wrap | **Defender** has been wrapped by **Attacker** using Clamp.[^wrap-status-onset] |
| `STRINGID_PKMNHURTBY` | | paired statuses | wrap | **Attacker** is hurt by **Buffer 1**, the move with which the victim was wrapped. |
| `STRINGID_PKMNFREEDFROM` | | paired statuses | wrap | **Attacker** is no longer affected by **Buffer 1**, the move with which they were previously wrapped. |
| `STRINGID_PKMNCRASHED` |
| `STRINGID_PKMNSHROUDEDINMIST` |
| `STRINGID_PKMNPROTECTEDBYMIST` |
| `STRINGID_PKMNGETTINGPUMPED` |
| `STRINGID_PKMNHITWITHRECOIL` |
| `STRINGID_PKMNPROTECTEDITSELF2` | | moves | Protect |
| `STRINGID_PKMNBUFFETEDBYSANDSTORM` | | weather | damage |
| `STRINGID_PKMNPELTEDBYHAIL` | | weather | damage |
| `STRINGID_PKMNSEEDED` | | moves | Leech Seed |
| `STRINGID_PKMNEVADEDATTACK` |
| `STRINGID_PKMNSAPPEDBYLEECHSEED` | | moves | Leech Seed |
| `STRINGID_PKMNFASTASLEEP` | | status | sleep |
| `STRINGID_PKMNWOKEUP` | | status | sleep |
| `STRINGID_PKMNUPROARKEPTAWAKE` | | moves | Uproar |
| `STRINGID_PKMNWOKEUPINUPROAR` | | moves | Uproar |
| `STRINGID_PKMNCAUSEDUPROAR` | | moves | Uproar |
| `STRINGID_PKMNMAKINGUPROAR` | | moves | Uproar |
| `STRINGID_PKMNCALMEDDOWN` | | moves | Uproar |
| `STRINGID_PKMNCANTSLEEPINUPROAR` | | moves | Uproar |
| `STRINGID_PKMNSTOCKPILED` | | moves | Stockpile |
| `STRINGID_PKMNCANTSTOCKPILE` | | moves | Stockpile |
| `STRINGID_PKMNCANTSLEEPINUPROAR2` | | moves | Uproar |
| `STRINGID_UPROARKEPTPKMNAWAKE` | | moves | Uproar |
| `STRINGID_PKMNSTAYEDAWAKEUSING` |
| `STRINGID_PKMNSTORINGENERGY` |
| `STRINGID_PKMNUNLEASHEDENERGY` |
| `STRINGID_PKMNFATIGUECONFUSION` |
| `STRINGID_PLAYERPICKEDUPMONEY` |
| `STRINGID_PKMNUNAFFECTED` |
| `STRINGID_PKMNTRANSFORMEDINTO` | | moves | Transform |
| `STRINGID_PKMNMADESUBSTITUTE` | | moves | Substitute |
| `STRINGID_PKMNHASSUBSTITUTE` | | moves | Substitute |
| `STRINGID_SUBSTITUTEDAMAGED` | | moves | Substitute |
| `STRINGID_PKMNSUBSTITUTEFADED` | | moves | Substitute |
| `STRINGID_PKMNMUSTRECHARGE` |
| `STRINGID_PKMNRAGEBUILDING` | | moves | Rage |
| `STRINGID_PKMNMOVEWASDISABLED` | | moves | Disable |
| `STRINGID_PKMNMOVEISDISABLED` | | moves | Disable |
| `STRINGID_PKMNMOVEDISABLEDNOMORE` | | moves | Disable |
| `STRINGID_PKMNGOTENCORE` | | moves | Encore |
| `STRINGID_PKMNENCOREENDED` | | moves | Encore |
| `STRINGID_PKMNTOOKAIM` |
| `STRINGID_PKMNSKETCHEDMOVE` | | moves | Sketch |
| `STRINGID_PKMNTRYINGTOTAKEFOE` |
| `STRINGID_PKMNTOOKFOE` |
| `STRINGID_PKMNREDUCEDPP` |
| `STRINGID_PKMNSTOLEITEM` |
| `STRINGID_TARGETCANTESCAPENOW` |
| `STRINGID_PKMNFELLINTONIGHTMARE` | | moves | Nightmare |
| `STRINGID_PKMNLOCKEDINNIGHTMARE` | | moves | Nightmare |
| `STRINGID_PKMNLAIDCURSE` | | moves | Curse |
| `STRINGID_PKMNAFFLICTEDBYCURSE` | | moves | Curse |
| `STRINGID_SPIKESSCATTERED` | | moves | Spikes |
| `STRINGID_PKMNHURTBYSPIKES` | | moves | Spikes |
| `STRINGID_PKMNIDENTIFIED` |
| `STRINGID_PKMNPERISHCOUNTFELL` | | moves | Perish Song |
| `STRINGID_PKMNBRACEDITSELF` |
| `STRINGID_PKMNENDUREDHIT` |
| `STRINGID_MAGNITUDESTRENGTH` | | moves | Magnitude |
| `STRINGID_PKMNCUTHPMAXEDATTACK` |
| `STRINGID_PKMNCOPIEDSTATCHANGES` |
| `STRINGID_PKMNGOTFREE` |
| `STRINGID_PKMNSHEDLEECHSEED` | | moves | Leech Seed |
| `STRINGID_PKMNBLEWAWAYSPIKES` | | moves | Spikes |
| `STRINGID_PKMNFLEDFROMBATTLE` |
| `STRINGID_PKMNFORESAWATTACK` |
| `STRINGID_PKMNTOOKATTACK` |
| `STRINGID_PKMNATTACK` |
| `STRINGID_PKMNCENTERATTENTION` |
| `STRINGID_PKMNCHARGINGPOWER` |
| `STRINGID_NATUREPOWERTURNEDINTO` | | moves | Nature Power |
| `STRINGID_PKMNSTATUSNORMAL` |
| `STRINGID_PKMNHASNOMOVESLEFT` | | system | |
| `STRINGID_PKMNSUBJECTEDTOTORMENT` |
| `STRINGID_PKMNCANTUSEMOVETORMENT` |
| `STRINGID_PKMNTIGHTENINGFOCUS` |
| `STRINGID_PKMNFELLFORTAUNT` | | moves | Taunt |
| `STRINGID_PKMNCANTUSEMOVETAUNT` | | moves | Taunt |
| `STRINGID_PKMNREADYTOHELP` |
| `STRINGID_PKMNSWITCHEDITEMS` |
| `STRINGID_PKMNCOPIEDFOE` |
| `STRINGID_PKMNMADEWISH` | | moves | Wish |
| `STRINGID_PKMNWISHCAMETRUE` | | moves | Wish |
| `STRINGID_PKMNPLANTEDROOTS` |
| `STRINGID_PKMNABSORBEDNUTRIENTS` |
| `STRINGID_PKMNANCHOREDITSELF` |
| `STRINGID_PKMNWASMADEDROWSY` |
| `STRINGID_PKMNKNOCKEDOFF` |
| `STRINGID_PKMNSWAPPEDABILITIES` |
| `STRINGID_PKMNSEALEDOPPONENTMOVE` |
| `STRINGID_PKMNCANTUSEMOVESEALED` |
| `STRINGID_PKMNWANTSGRUDGE` |
| `STRINGID_PKMNLOSTPPGRUDGE` |
| `STRINGID_PKMNSHROUDEDITSELF` |
| `STRINGID_PKMNMOVEBOUNCED` |
| `STRINGID_PKMNWAITSFORTARGET` |
| `STRINGID_PKMNSNATCHEDMOVE` |
| `STRINGID_PKMNMADEITRAIN` | | weather | onset |
| `STRINGID_PKMNRAISEDSPEED` |
| `STRINGID_PKMNPROTECTEDBY` |
| `STRINGID_PKMNPREVENTSUSAGE` |
| `STRINGID_PKMNRESTOREDHPUSING` |
| `STRINGID_PKMNCHANGEDTYPEWITH` |
| `STRINGID_PKMNPREVENTSPARALYSISWITH` |
| `STRINGID_PKMNPREVENTSROMANCEWITH` |
| `STRINGID_PKMNPREVENTSPOISONINGWITH` |
| `STRINGID_PKMNPREVENTSCONFUSIONWITH` |
| `STRINGID_PKMNRAISEDFIREPOWERWITH` |
| `STRINGID_PKMNANCHORSITSELFWITH` |
| `STRINGID_PKMNCUTSATTACKWITH` |
| `STRINGID_PKMNPREVENTSSTATLOSSWITH` |
| `STRINGID_PKMNHURTSWITH` |
| `STRINGID_PKMNTRACED` |
| `STRINGID_STATSHARPLY` | | stats | increase |
| `STRINGID_STATROSE` | | stats | increase |
| `STRINGID_STATHARSHLY` | | stats | decrease |
| `STRINGID_STATFELL` | | stats | decrease |
| `STRINGID_ATTACKERSSTATROSE` | | stats | increase |
| `STRINGID_DEFENDERSSTATROSE` | | stats | increase |
| `STRINGID_ATTACKERSSTATFELL` | | stats | decrease |
| `STRINGID_DEFENDERSSTATFELL` | | stats | decrease |
| `STRINGID_CRITICALHIT` | | common move results | critical hits |
| `STRINGID_ONEHITKO` | | common move results | OHKO |
| `STRINGID_123POOF` | | system | learning moves |
| `STRINGID_ANDELLIPSIS` |
| `STRINGID_NOTVERYEFFECTIVE` | | common move results | type effectiveness |
| `STRINGID_SUPEREFFECTIVE` | | common move results | type effectiveness |
| `STRINGID_GOTAWAYSAFELY` | | system | fleeing | The player has successfully fled the battle. |
| `STRINGID_WILDPKMNFLED` | | system | fleeing |
| `STRINGID_NORUNNINGFROMTRAINERS` | | system | fleeing | The player has failed to flee, because this is a trainer battle. |
| `STRINGID_CANTESCAPE` | | system | fleeing |
| `STRINGID_DONTLEAVEBIRCH` | | system | fleeing | The player has failed to flee, because this is the battle in which they rescue Prof. Birch. |
| `STRINGID_BUTNOTHINGHAPPENED` |
| `STRINGID_BUTITFAILED` |
| `STRINGID_ITHURTCONFUSION` | | status | confusion | **Attacker** hurt itself as a result of being confused. |
| `STRINGID_MIRRORMOVEFAILED` | | moves | Mirror Move |
| `STRINGID_STARTEDTORAIN` | | weather | onset |
| `STRINGID_DOWNPOURSTARTED` | | weather | onset |
| `STRINGID_RAINCONTINUES` | | weather | ongoing |
| `STRINGID_DOWNPOURCONTINUES` | | weather | ongoing |
| `STRINGID_RAINSTOPPED` | | weather | ending |
| `STRINGID_SANDSTORMBREWED` | | weather | onset |
| `STRINGID_SANDSTORMRAGES` | | weather | ongoing |
| `STRINGID_SANDSTORMSUBSIDED` | | weather | ending |
| `STRINGID_SUNLIGHTGOTBRIGHT` | | weather | onset |
| `STRINGID_SUNLIGHTSTRONG` | | weather | ongoing |
| `STRINGID_SUNLIGHTFADED` | | weather | ending |
| `STRINGID_STARTEDHAIL` | | weather | onset |
| `STRINGID_HAILCONTINUES` | | weather | ongoing |
| `STRINGID_HAILSTOPPED` | | weather | ending |
| `STRINGID_FAILEDTOSPITUP` |
| `STRINGID_FAILEDTOSWALLOW` |
| `STRINGID_WINDBECAMEHEATWAVE` |
| `STRINGID_STATCHANGESGONE` |
| `STRINGID_COINSSCATTERED` |
| `STRINGID_TOOWEAKFORSUBSTITUTE` | | moves | Substitute | The attacker has too little HP to create a Substitute. |
| `STRINGID_SHAREDPAIN` |
| `STRINGID_BELLCHIMED` |
| `STRINGID_FAINTINTHREE` |
| `STRINGID_NOPPLEFT` |
| `STRINGID_BUTNOPPLEFT` | | common move results | failure | The attacker's chosen move had its PP reduced *after* they chose it but before they used it, such that they failed to do anything on this turn. |
| `STRINGID_PLAYERUSEDITEM` | | system | items |
| `STRINGID_WALLYUSEDITEM` | | system | items |
| `STRINGID_TRAINERBLOCKEDBALL` | | system | items |
| `STRINGID_DONTBEATHIEF` | | system | items |
| `STRINGID_ITDODGEDBALL` | `sText_ItDodgedBall` | system | catching | **Defender**, a wild Pokemon, is scripted to be impossible to catch, and the player's attempt to do so just failed. This message should tell the player that **Defender** can't be caught. This is a leftover from FireRed: there, the `handleballthrow` script command checks if the player is battling a disguised ghost or the vengeful Marowak and, if so, it invokes a script which prints this message. This is done after the attacker and defender variables are set up. |
| `STRINGID_YOUMISSEDPKMN` | | system | catching | Unused. Presumably, thrown Poke Balls once had a random chance to miss. |
| `STRINGID_PKMNBROKEFREE` | | system | catching | The player failed to catch **Defender**. It broke out of the Poke Ball immediately. |
| `STRINGID_ITAPPEAREDCAUGHT` | | system | catching | The player failed to catch **Defender**. It broke out of the Poke Ball after the ball shook once. |
| `STRINGID_AARGHALMOSTHADIT` | | system | catching | The player failed to catch **Defender**. It broke out of the Poke Ball after the ball shook twice. |
| `STRINGID_SHOOTSOCLOSE` | | system | catching | The player failed to catch **Defender**. It broke out of the Poke Ball after the ball shook thrice. |
| `STRINGID_GOTCHAPKMNCAUGHTPLAYER` | | system | catching | **Opponent 1** has been caught. |
| `STRINGID_GOTCHAPKMNCAUGHTWALLY` | | system | catching | **Opponent 1** has been caught. |
| `STRINGID_GIVENICKNAMECAPTURED` | | system | catching | Player is being prompted to nickname **Opponent 1** after having caught it. |
| `STRINGID_PKMNSENTTOPC` | | system | catching | **Opponent 1** has been sent to the PC after being caught, because the player's party is full. |
| `STRINGID_PKMNDATAADDEDTODEX` | | system | catching | **Opponent 1**'s data has been added to the Pokedex, now that it has been caught. This string assumes that the Pokemon has no nickname. |
| `STRINGID_ITISRAINING` | | weather | battle start[^weather-from-overworld] | It was raining on the overworld, and so is raining at the start of the battle. |
| `STRINGID_SANDSTORMISRAGING` | | weather | battle start[^weather-from-overworld] | A sandstorm was raging on the overworld, and so a sandstorm is ongoing at the start of the battle. |
| `STRINGID_CANTESCAPE2` |
| `STRINGID_PKMNIGNORESASLEEP` | | system | disobedience |
| `STRINGID_PKMNIGNOREDORDERS` | | system | disobedience |
| `STRINGID_PKMNBEGANTONAP` | | system | disobedience |
| `STRINGID_PKMNLOAFING` | | system | disobedience |
| `STRINGID_PKMNWONTOBEY` | | system | disobedience |
| `STRINGID_PKMNTURNEDAWAY` | | system | disobedience |
| `STRINGID_PKMNPRETENDNOTNOTICE` | | system | disobedience |
| `STRINGID_ENEMYABOUTTOSWITCHPKMN` | | system | battle flow |
| `STRINGID_CREPTCLOSER` | | safari zone | |
| `STRINGID_CANTGETCLOSER` | | safari zone | |
| `STRINGID_PKMNWATCHINGCAREFULLY` | | safari zone | |
| `STRINGID_PKMNCURIOUSABOUTX` | | safari zone | |
| `STRINGID_PKMNENTHRALLEDBYX` | | safari zone | |
| `STRINGID_PKMNIGNOREDX` | | safari zone | |
| `STRINGID_THREWPOKEBLOCKATPKMN` | | safari zone | |
| `STRINGID_OUTOFSAFARIBALLS` | | safari zone | |
| `STRINGID_PKMNSITEMCUREDPARALYSIS` | | hold items | status cure |
| `STRINGID_PKMNSITEMCUREDPOISON` | | hold items | status cure |
| `STRINGID_PKMNSITEMHEALEDBURN` | | hold items | status cure |
| `STRINGID_PKMNSITEMDEFROSTEDIT` | | hold items | status cure |
| `STRINGID_PKMNSITEMWOKEIT` | | hold items | status cure |
| `STRINGID_PKMNSITEMSNAPPEDOUT` | | hold items | status cure |
| `STRINGID_PKMNSITEMCUREDPROBLEM` | | hold items | status cure |
| `STRINGID_PKMNSITEMRESTOREDHEALTH` | | hold items | restoration |
| `STRINGID_PKMNSITEMRESTOREDPP` | | hold items | restoration |
| `STRINGID_PKMNSITEMRESTOREDSTATUS` | | hold items | |
| `STRINGID_PKMNSITEMRESTOREDHPALITTLE` | | hold items | restoration |
| `STRINGID_ITEMALLOWSONLYYMOVE` |
| `STRINGID_PKMNHUNGONWITHX` |
| `STRINGID_EMPTYSTRING3` |
| `STRINGID_PKMNSXPREVENTSBURNS` |
| `STRINGID_PKMNSXBLOCKSY` |
| `STRINGID_PKMNSXRESTOREDHPALITTLE2` |
| `STRINGID_PKMNSXWHIPPEDUPSANDSTORM` |
| `STRINGID_PKMNSXPREVENTSYLOSS` |
| `STRINGID_PKMNSXINFATUATEDY` |
| `STRINGID_PKMNSXMADEYINEFFECTIVE` |
| `STRINGID_PKMNSXCUREDYPROBLEM` |
| `STRINGID_ITSUCKEDLIQUIDOOZE` |
| `STRINGID_PKMNTRANSFORMED` |
| `STRINGID_ELECTRICITYWEAKENED` |
| `STRINGID_FIREWEAKENED` |
| `STRINGID_PKMNHIDUNDERWATER` |
| `STRINGID_PKMNSPRANGUP` |
| `STRINGID_HMMOVESCANTBEFORGOTTEN` |
| `STRINGID_XFOUNDONEY` |
| `STRINGID_PLAYERDEFEATEDTRAINER1` |
| `STRINGID_SOOTHINGAROMA` |
| `STRINGID_ITEMSCANTBEUSEDNOW` |
| `STRINGID_FORXCOMMAYZ` |
| `STRINGID_USINGITEMSTATOFPKMNROSE` |
| `STRINGID_PKMNUSEDXTOGETPUMPED` |
| `STRINGID_PKMNSXMADEYUSELESS` |
| `STRINGID_PKMNTRAPPEDBYSANDTOMB` |
| `STRINGID_EMPTYSTRING4` |
| `STRINGID_ABOOSTED` |
| `STRINGID_PKMNSXINTENSIFIEDSUN` |
| `STRINGID_PKMNMAKESGROUNDMISS` |
| `STRINGID_YOUTHROWABALLNOWRIGHT` |
| `STRINGID_PKMNSXTOOKATTACK` |
| `STRINGID_PKMNCHOSEXASDESTINY` |
| `STRINGID_PKMNLOSTFOCUS` |
| `STRINGID_USENEXTPKMN` |
| `STRINGID_PKMNFLEDUSINGITS` | | system | fleeing |
| `STRINGID_PKMNFLEDUSING` | | system | fleeing |
| `STRINGID_PKMNWASDRAGGEDOUT` |
| `STRINGID_PREVENTEDFROMWORKING` |
| `STRINGID_PKMNSITEMNORMALIZEDSTATUS` |
| `STRINGID_TRAINER1USEDITEM` | | system | items | **Trainer 1** used **Last Item**. |
| `STRINGID_BOXISFULL` | | system | catching |
| `STRINGID_PKMNAVOIDEDATTACK` |
| `STRINGID_PKMNSXMADEITINEFFECTIVE` |
| `STRINGID_PKMNSXPREVENTSFLINCHING` |
| `STRINGID_PKMNALREADYHASBURN` | | status | burn |
| `STRINGID_STATSWONTDECREASE2` |
| `STRINGID_PKMNSXBLOCKSY2` |
| `STRINGID_PKMNSXWOREOFF` |
| `STRINGID_PKMNRAISEDDEFALITTLE` |
| `STRINGID_PKMNRAISEDSPDEFALITTLE` |
| `STRINGID_THEWALLSHATTERED` |
| `STRINGID_PKMNSXPREVENTSYSZ` |
| `STRINGID_PKMNSXCUREDITSYPROBLEM` |
| `STRINGID_ATTACKERCANTESCAPE` |
| `STRINGID_PKMNOBTAINEDX` |
| `STRINGID_PKMNOBTAINEDX2` |
| `STRINGID_PKMNOBTAINEDXYOBTAINEDZ` |
| `STRINGID_BUTNOEFFECT` |
| `STRINGID_PKMNSXHADNOEFFECTONY` |
| `STRINGID_TWOENEMIESDEFEATED` |
| `STRINGID_TRAINER2LOSETEXT` |
| `STRINGID_PKMNINCAPABLEOFPOWER` |
| `STRINGID_GLINTAPPEARSINEYE` |
| `STRINGID_PKMNGETTINGINTOPOSITION` |
| `STRINGID_PKMNBEGANGROWLINGDEEPLY` |
| `STRINGID_PKMNEAGERFORMORE` |
| `STRINGID_DEFEATEDOPPONENTBYREFEREE` |
| `STRINGID_LOSTTOOPPONENTBYREFEREE` |
| `STRINGID_TIEDOPPONENTBYREFEREE` |
| `STRINGID_QUESTIONFORFEITMATCH` |
| `STRINGID_FORFEITEDMATCH` |
| `STRINGID_PKMNTRANSFERREDSOMEONESPC` | | system | catching |
| `STRINGID_PKMNTRANSFERREDLANETTESPC` | | system | catching |
| `STRINGID_PKMNBOXSOMEONESPCFULL` | | system | catching |
| `STRINGID_PKMNBOXLANETTESPCFULL` | | system | catching |
| `STRINGID_TRAINER1WINTEXT` |
| `STRINGID_TRAINER2WINTEXT` |

[^weather-from-overworld]: The `gWeatherStartsStringIds` table maps weather conditions (`WEATHER_...`) to battle string IDs, to be displayed when weather is "inherited" from the overworld.

[^wrap-status-onset]: Several moves, like Bind and Wrap, afflict a common "wrap" status. They share many of their strings, but the onset of the status uses different strings for each move, indexing into the `gWrappedStringIds` table.