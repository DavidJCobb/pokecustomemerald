#include "constants/global.h"
#include "constants/battle.h"
#include "constants/pokemon.h"
#include "constants/battle_arena.h"
#include "constants/battle_script_commands.h"
#include "constants/battle_anim.h"
#include "constants/battle_string_ids.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/game_stat.h"
#include "constants/trainers.h"
	.include "asm/macros.inc"
	.include "asm/macros/battle_script.inc"
	.include "constants/constants.inc"

	.section script_data, "aw", %progbits

.align 2
gBattleScriptsForMoveEffects::
	.4byte BattleScript_EffectHit						  @ EFFECT_HIT
	.4byte BattleScript_EffectSleep						@ EFFECT_SLEEP
	.4byte BattleScript_EffectPoisonHit				  @ EFFECT_POISON_HIT
	.4byte BattleScript_EffectAbsorb					  @ EFFECT_ABSORB
	.4byte BattleScript_EffectBurnHit					 @ EFFECT_BURN_HIT
	.4byte BattleScript_EffectFreezeHit				  @ EFFECT_FREEZE_HIT
	.4byte BattleScript_EffectParalyzeHit				@ EFFECT_PARALYZE_HIT
	.4byte BattleScript_EffectExplosion				  @ EFFECT_EXPLOSION
	.4byte BattleScript_EffectDreamEater				 @ EFFECT_DREAM_EATER
	.4byte BattleScript_EffectMirrorMove				 @ EFFECT_MIRROR_MOVE
	.4byte BattleScript_EffectAttackUp					@ EFFECT_ATTACK_UP
	.4byte BattleScript_EffectDefenseUp				  @ EFFECT_DEFENSE_UP
	.4byte BattleScript_EffectHit						  @ EFFECT_SPEED_UP
	.4byte BattleScript_EffectSpecialAttackUp		  @ EFFECT_SPECIAL_ATTACK_UP
	.4byte BattleScript_EffectHit						  @ EFFECT_SPECIAL_DEFENSE_UP
	.4byte BattleScript_EffectHit						  @ EFFECT_ACCURACY_UP
	.4byte BattleScript_EffectEvasionUp				  @ EFFECT_EVASION_UP
	.4byte BattleScript_EffectHit						  @ EFFECT_ALWAYS_HIT
	.4byte BattleScript_EffectAttackDown				 @ EFFECT_ATTACK_DOWN
	.4byte BattleScript_EffectDefenseDown				@ EFFECT_DEFENSE_DOWN
	.4byte BattleScript_EffectSpeedDown				  @ EFFECT_SPEED_DOWN
	.4byte BattleScript_EffectHit						  @ EFFECT_SPECIAL_ATTACK_DOWN
	.4byte BattleScript_EffectHit						  @ EFFECT_SPECIAL_DEFENSE_DOWN
	.4byte BattleScript_EffectAccuracyDown			  @ EFFECT_ACCURACY_DOWN
	.4byte BattleScript_EffectEvasionDown				@ EFFECT_EVASION_DOWN
	.4byte BattleScript_EffectHaze						 @ EFFECT_HAZE
	.4byte BattleScript_EffectBide						 @ EFFECT_BIDE
	.4byte BattleScript_EffectRampage					 @ EFFECT_RAMPAGE
	.4byte BattleScript_EffectRoar						 @ EFFECT_ROAR
	.4byte BattleScript_EffectMultiHit					@ EFFECT_MULTI_HIT
	.4byte BattleScript_EffectConversion				 @ EFFECT_CONVERSION
	.4byte BattleScript_EffectFlinchHit				  @ EFFECT_FLINCH_HIT
	.4byte BattleScript_EffectRestoreHp				  @ EFFECT_RESTORE_HP
	.4byte BattleScript_EffectToxic						@ EFFECT_TOXIC
	.4byte BattleScript_EffectPayDay					  @ EFFECT_PAY_DAY
	.4byte BattleScript_EffectLightScreen				@ EFFECT_LIGHT_SCREEN
	.4byte BattleScript_EffectTriAttack				  @ EFFECT_TRI_ATTACK
	.4byte BattleScript_EffectRest						 @ EFFECT_REST
	.4byte BattleScript_EffectOHKO						 @ EFFECT_OHKO
	.4byte BattleScript_EffectRazorWind				  @ EFFECT_RAZOR_WIND
	.4byte BattleScript_EffectSuperFang				  @ EFFECT_SUPER_FANG
	.4byte BattleScript_EffectDragonRage				 @ EFFECT_DRAGON_RAGE
	.4byte BattleScript_EffectTrap						 @ EFFECT_TRAP
	.4byte BattleScript_EffectHit						  @ EFFECT_HIGH_CRITICAL
	.4byte BattleScript_EffectDoubleHit				  @ EFFECT_DOUBLE_HIT
	.4byte BattleScript_EffectRecoilIfMiss			  @ EFFECT_RECOIL_IF_MISS
	.4byte BattleScript_EffectMist						 @ EFFECT_MIST
	.4byte BattleScript_EffectFocusEnergy				@ EFFECT_FOCUS_ENERGY
	.4byte BattleScript_EffectRecoil					  @ EFFECT_RECOIL
	.4byte BattleScript_EffectConfuse					 @ EFFECT_CONFUSE
	.4byte BattleScript_EffectAttackUp2				  @ EFFECT_ATTACK_UP_2
	.4byte BattleScript_EffectDefenseUp2				 @ EFFECT_DEFENSE_UP_2
	.4byte BattleScript_EffectSpeedUp2					@ EFFECT_SPEED_UP_2
	.4byte BattleScript_EffectSpecialAttackUp2		 @ EFFECT_SPECIAL_ATTACK_UP_2
	.4byte BattleScript_EffectSpecialDefenseUp2		@ EFFECT_SPECIAL_DEFENSE_UP_2
	.4byte BattleScript_EffectHit						  @ EFFECT_ACCURACY_UP_2
	.4byte BattleScript_EffectHit						  @ EFFECT_EVASION_UP_2
	.4byte BattleScript_EffectTransform				  @ EFFECT_TRANSFORM
	.4byte BattleScript_EffectAttackDown2				@ EFFECT_ATTACK_DOWN_2
	.4byte BattleScript_EffectDefenseDown2			  @ EFFECT_DEFENSE_DOWN_2
	.4byte BattleScript_EffectSpeedDown2				 @ EFFECT_SPEED_DOWN_2
	.4byte BattleScript_EffectHit						  @ EFFECT_SPECIAL_ATTACK_DOWN_2
	.4byte BattleScript_EffectSpecialDefenseDown2	 @ EFFECT_SPECIAL_DEFENSE_DOWN_2
	.4byte BattleScript_EffectHit						  @ EFFECT_ACCURACY_DOWN_2
	.4byte BattleScript_EffectHit						  @ EFFECT_EVASION_DOWN_2
	.4byte BattleScript_EffectReflect					 @ EFFECT_REFLECT
	.4byte BattleScript_EffectPoison					  @ EFFECT_POISON
	.4byte BattleScript_EffectParalyze					@ EFFECT_PARALYZE
	.4byte BattleScript_EffectAttackDownHit			 @ EFFECT_ATTACK_DOWN_HIT
	.4byte BattleScript_EffectDefenseDownHit			@ EFFECT_DEFENSE_DOWN_HIT
	.4byte BattleScript_EffectSpeedDownHit			  @ EFFECT_SPEED_DOWN_HIT
	.4byte BattleScript_EffectSpecialAttackDownHit	@ EFFECT_SPECIAL_ATTACK_DOWN_HIT
	.4byte BattleScript_EffectSpecialDefenseDownHit  @ EFFECT_SPECIAL_DEFENSE_DOWN_HIT
	.4byte BattleScript_EffectAccuracyDownHit		  @ EFFECT_ACCURACY_DOWN_HIT
	.4byte BattleScript_EffectHit						  @ EFFECT_EVASION_DOWN_HIT
	.4byte BattleScript_EffectSkyAttack				  @ EFFECT_SKY_ATTACK
	.4byte BattleScript_EffectConfuseHit				 @ EFFECT_CONFUSE_HIT
	.4byte BattleScript_EffectTwineedle				  @ EFFECT_TWINEEDLE
	.4byte BattleScript_EffectHit						  @ EFFECT_VITAL_THROW
	.4byte BattleScript_EffectSubstitute				 @ EFFECT_SUBSTITUTE
	.4byte BattleScript_EffectRecharge					@ EFFECT_RECHARGE
	.4byte BattleScript_EffectRage						 @ EFFECT_RAGE
	.4byte BattleScript_EffectMimic						@ EFFECT_MIMIC
	.4byte BattleScript_EffectMetronome				  @ EFFECT_METRONOME
	.4byte BattleScript_EffectLeechSeed				  @ EFFECT_LEECH_SEED
	.4byte BattleScript_EffectSplash					  @ EFFECT_SPLASH
	.4byte BattleScript_EffectDisable					 @ EFFECT_DISABLE
	.4byte BattleScript_EffectLevelDamage				@ EFFECT_LEVEL_DAMAGE
	.4byte BattleScript_EffectPsywave					 @ EFFECT_PSYWAVE
	.4byte BattleScript_EffectCounter					 @ EFFECT_COUNTER
	.4byte BattleScript_EffectEncore					  @ EFFECT_ENCORE
	.4byte BattleScript_EffectPainSplit				  @ EFFECT_PAIN_SPLIT
	.4byte BattleScript_EffectSnore						@ EFFECT_SNORE
	.4byte BattleScript_EffectConversion2				@ EFFECT_CONVERSION_2
	.4byte BattleScript_EffectLockOn					  @ EFFECT_LOCK_ON
	.4byte BattleScript_EffectSketch					  @ EFFECT_SKETCH
	.4byte BattleScript_EffectHit						  @ EFFECT_UNUSED_60
	.4byte BattleScript_EffectSleepTalk				  @ EFFECT_SLEEP_TALK
	.4byte BattleScript_EffectDestinyBond				@ EFFECT_DESTINY_BOND
	.4byte BattleScript_EffectFlail						@ EFFECT_FLAIL
	.4byte BattleScript_EffectSpite						@ EFFECT_SPITE
	.4byte BattleScript_EffectHit						  @ EFFECT_FALSE_SWIPE
	.4byte BattleScript_EffectHealBell					@ EFFECT_HEAL_BELL
	.4byte BattleScript_EffectHit						  @ EFFECT_QUICK_ATTACK
	.4byte BattleScript_EffectTripleKick				 @ EFFECT_TRIPLE_KICK
	.4byte BattleScript_EffectThief						@ EFFECT_THIEF
	.4byte BattleScript_EffectMeanLook					@ EFFECT_MEAN_LOOK
	.4byte BattleScript_EffectNightmare				  @ EFFECT_NIGHTMARE
	.4byte BattleScript_EffectMinimize					@ EFFECT_MINIMIZE
	.4byte BattleScript_EffectCurse						@ EFFECT_CURSE
	.4byte BattleScript_EffectHit						  @ EFFECT_UNUSED_6E
	.4byte BattleScript_EffectProtect					 @ EFFECT_PROTECT
	.4byte BattleScript_EffectSpikes					  @ EFFECT_SPIKES
	.4byte BattleScript_EffectForesight				  @ EFFECT_FORESIGHT
	.4byte BattleScript_EffectPerishSong				 @ EFFECT_PERISH_SONG
	.4byte BattleScript_EffectSandstorm				  @ EFFECT_SANDSTORM
	.4byte BattleScript_EffectEndure					  @ EFFECT_ENDURE
	.4byte BattleScript_EffectRollout					 @ EFFECT_ROLLOUT
	.4byte BattleScript_EffectSwagger					 @ EFFECT_SWAGGER
	.4byte BattleScript_EffectFuryCutter				 @ EFFECT_FURY_CUTTER
	.4byte BattleScript_EffectAttract					 @ EFFECT_ATTRACT
	.4byte BattleScript_EffectReturn					  @ EFFECT_RETURN
	.4byte BattleScript_EffectPresent					 @ EFFECT_PRESENT
	.4byte BattleScript_EffectFrustration				@ EFFECT_FRUSTRATION
	.4byte BattleScript_EffectSafeguard				  @ EFFECT_SAFEGUARD
	.4byte BattleScript_EffectThawHit					 @ EFFECT_THAW_HIT
	.4byte BattleScript_EffectMagnitude				  @ EFFECT_MAGNITUDE
	.4byte BattleScript_EffectBatonPass				  @ EFFECT_BATON_PASS
	.4byte BattleScript_EffectHit						  @ EFFECT_PURSUIT
	.4byte BattleScript_EffectRapidSpin				  @ EFFECT_RAPID_SPIN
	.4byte BattleScript_EffectSonicboom				  @ EFFECT_SONICBOOM
	.4byte BattleScript_EffectHit						  @ EFFECT_UNUSED_83
	.4byte BattleScript_EffectMorningSun				 @ EFFECT_MORNING_SUN
	.4byte BattleScript_EffectSynthesis				  @ EFFECT_SYNTHESIS
	.4byte BattleScript_EffectMoonlight				  @ EFFECT_MOONLIGHT
	.4byte BattleScript_EffectHiddenPower				@ EFFECT_HIDDEN_POWER
	.4byte BattleScript_EffectRainDance				  @ EFFECT_RAIN_DANCE
	.4byte BattleScript_EffectSunnyDay					@ EFFECT_SUNNY_DAY
	.4byte BattleScript_EffectDefenseUpHit			  @ EFFECT_DEFENSE_UP_HIT
	.4byte BattleScript_EffectAttackUpHit				@ EFFECT_ATTACK_UP_HIT
	.4byte BattleScript_EffectAllStatsUpHit			 @ EFFECT_ALL_STATS_UP_HIT
	.4byte BattleScript_EffectHit						  @ EFFECT_UNUSED_8D
	.4byte BattleScript_EffectBellyDrum				  @ EFFECT_BELLY_DRUM
	.4byte BattleScript_EffectPsychUp					 @ EFFECT_PSYCH_UP
	.4byte BattleScript_EffectMirrorCoat				 @ EFFECT_MIRROR_COAT
	.4byte BattleScript_EffectSkullBash				  @ EFFECT_SKULL_BASH
	.4byte BattleScript_EffectTwister					 @ EFFECT_TWISTER
	.4byte BattleScript_EffectEarthquake				 @ EFFECT_EARTHQUAKE
	.4byte BattleScript_EffectFutureSight				@ EFFECT_FUTURE_SIGHT
	.4byte BattleScript_EffectGust						 @ EFFECT_GUST
	.4byte BattleScript_EffectStomp						@ EFFECT_FLINCH_MINIMIZE_HIT
	.4byte BattleScript_EffectSolarBeam				  @ EFFECT_SOLAR_BEAM
	.4byte BattleScript_EffectThunder					 @ EFFECT_THUNDER
	.4byte BattleScript_EffectTeleport					@ EFFECT_TELEPORT
	.4byte BattleScript_EffectBeatUp					  @ EFFECT_BEAT_UP
	.4byte BattleScript_EffectSemiInvulnerable		 @ EFFECT_SEMI_INVULNERABLE
	.4byte BattleScript_EffectDefenseCurl				@ EFFECT_DEFENSE_CURL
	.4byte BattleScript_EffectSoftboiled				 @ EFFECT_SOFTBOILED
	.4byte BattleScript_EffectFakeOut					 @ EFFECT_FAKE_OUT
	.4byte BattleScript_EffectUproar					  @ EFFECT_UPROAR
	.4byte BattleScript_EffectStockpile				  @ EFFECT_STOCKPILE
	.4byte BattleScript_EffectSpitUp					  @ EFFECT_SPIT_UP
	.4byte BattleScript_EffectSwallow					 @ EFFECT_SWALLOW
	.4byte BattleScript_EffectHit						  @ EFFECT_UNUSED_A3
	.4byte BattleScript_EffectHail						 @ EFFECT_HAIL
	.4byte BattleScript_EffectTorment					 @ EFFECT_TORMENT
	.4byte BattleScript_EffectFlatter					 @ EFFECT_FLATTER
	.4byte BattleScript_EffectBurn				  @ EFFECT_WILL_O_WISP
	.4byte BattleScript_EffectMemento					 @ EFFECT_MEMENTO
	.4byte BattleScript_EffectFacade					  @ EFFECT_FACADE
	.4byte BattleScript_EffectFocusPunch				 @ EFFECT_FOCUS_PUNCH
	.4byte BattleScript_EffectSmellingsalt			  @ EFFECT_SMELLINGSALT
	.4byte BattleScript_EffectFollowMe					@ EFFECT_FOLLOW_ME
	.4byte BattleScript_EffectNaturePower				@ EFFECT_NATURE_POWER
	.4byte BattleScript_EffectCharge					  @ EFFECT_CHARGE
	.4byte BattleScript_EffectTaunt						@ EFFECT_TAUNT
	.4byte BattleScript_EffectHelpingHand				@ EFFECT_HELPING_HAND
	.4byte BattleScript_EffectTrick						@ EFFECT_TRICK
	.4byte BattleScript_EffectRolePlay					@ EFFECT_ROLE_PLAY
	.4byte BattleScript_EffectWish						 @ EFFECT_WISH
	.4byte BattleScript_EffectAssist					  @ EFFECT_ASSIST
	.4byte BattleScript_EffectIngrain					 @ EFFECT_INGRAIN
	.4byte BattleScript_EffectSuperpower				 @ EFFECT_SUPERPOWER
	.4byte BattleScript_EffectMagicCoat				  @ EFFECT_MAGIC_COAT
	.4byte BattleScript_EffectRecycle					 @ EFFECT_RECYCLE
	.4byte BattleScript_EffectRevenge					 @ EFFECT_REVENGE
	.4byte BattleScript_EffectBrickBreak				 @ EFFECT_BRICK_BREAK
	.4byte BattleScript_EffectYawn						 @ EFFECT_YAWN
	.4byte BattleScript_EffectKnockOff					@ EFFECT_KNOCK_OFF
	.4byte BattleScript_EffectEndeavor					@ EFFECT_ENDEAVOR
	.4byte BattleScript_EffectEruption					@ EFFECT_ERUPTION
	.4byte BattleScript_EffectSkillSwap				  @ EFFECT_SKILL_SWAP
	.4byte BattleScript_EffectImprison					@ EFFECT_IMPRISON
	.4byte BattleScript_EffectRefresh					 @ EFFECT_REFRESH
	.4byte BattleScript_EffectGrudge					  @ EFFECT_GRUDGE
	.4byte BattleScript_EffectSnatch					  @ EFFECT_SNATCH
	.4byte BattleScript_EffectLowKick					 @ EFFECT_LOW_KICK
	.4byte BattleScript_EffectSecretPower				@ EFFECT_SECRET_POWER
	.4byte BattleScript_EffectDoubleEdge				 @ EFFECT_DOUBLE_EDGE
	.4byte BattleScript_EffectTeeterDance				@ EFFECT_TEETER_DANCE
	.4byte BattleScript_EffectBurnHit					 @ EFFECT_BLAZE_KICK
	.4byte BattleScript_EffectHalveTypeDamage					@ EFFECT_MUD_SPORT
	.4byte BattleScript_EffectPoisonFang				 @ EFFECT_POISON_FANG
	.4byte BattleScript_EffectWeatherBall				@ EFFECT_WEATHER_BALL
	.4byte BattleScript_EffectOverheat					@ EFFECT_OVERHEAT
	.4byte BattleScript_EffectTickle					  @ EFFECT_TICKLE
	.4byte BattleScript_EffectCosmicPower				@ EFFECT_COSMIC_POWER
	.4byte BattleScript_EffectSkyUppercut				@ EFFECT_SKY_UPPERCUT
	.4byte BattleScript_EffectBulkUp					  @ EFFECT_BULK_UP
	.4byte BattleScript_EffectPoisonHit				  @ EFFECT_POISON_TAIL
	.4byte BattleScript_EffectHalveTypeDamage				 @ EFFECT_WATER_SPORT
	.4byte BattleScript_EffectCalmMind					@ EFFECT_CALM_MIND
	.4byte BattleScript_EffectDragonDance				@ EFFECT_DRAGON_DANCE
	.4byte BattleScript_EffectCamouflage				 @ EFFECT_CAMOUFLAGE

.include "data/battle_scripts/battle_arena.inc"
.include "data/battle_scripts/battle_win_loss.inc"
.include "data/battle_scripts/but_it_failed.inc"
.include "data/battle_scripts/flee.inc"
.include "data/battle_scripts/handle_fainted.inc"
.include "data/battle_scripts/level_up.inc"
.include "data/battle_scripts/move_selection.inc"
.include "data/battle_scripts/stat_change_various.inc"
.include "data/battle_scripts/stat_decrease_single.inc"
.include "data/battle_scripts/stat_increase_single.inc"
.include "data/battle_scripts/switch_out.inc"
.include "data/battle_scripts/weather.inc"

.include "data/battle_scripts/status_handlers/confused.inc"

.include "data/battle_scripts/shared_move_effects/bind.inc"
.include "data/battle_scripts/shared_move_effects/friendship_based_damage.inc"
.include "data/battle_scripts/shared_move_effects/halve_type_damage.inc"
.include "data/battle_scripts/shared_move_effects/heal_from_sunlight.inc"
.include "data/battle_scripts/shared_move_effects/heal_target.inc"
.include "data/battle_scripts/shared_move_effects/protect.inc"
.include "data/battle_scripts/shared_move_effects/recharge.inc"
.include "data/battle_scripts/shared_move_effects/recoil.inc"
.include "data/battle_scripts/shared_move_effects/recoil_if_miss.inc"
.include "data/battle_scripts/shared_move_effects/status_burn.inc"
.include "data/battle_scripts/shared_move_effects/status_confuse.inc"
.include "data/battle_scripts/shared_move_effects/status_paralyze.inc"
.include "data/battle_scripts/shared_move_effects/status_poison.inc"
.include "data/battle_scripts/shared_move_effects/status_sleep.inc"
.include "data/battle_scripts/shared_move_effects/use_level_as_damage.inc"

.include "data/battle_scripts/abilities/weather_changing/drizzle.inc"
.include "data/battle_scripts/abilities/weather_changing/drought.inc"
.include "data/battle_scripts/abilities/intimidate.inc"

.include "data/battle_scripts/moves/weather_changing/hail.inc"
.include "data/battle_scripts/moves/weather_changing/rain_dance.inc"
.include "data/battle_scripts/moves/weather_changing/sandstorm.inc"
.include "data/battle_scripts/moves/weather_changing/sunny_day.inc"
.include "data/battle_scripts/moves/absorb.inc"
.include "data/battle_scripts/moves/assist.inc"
.include "data/battle_scripts/moves/attract.inc"
.include "data/battle_scripts/moves/baton_pass.inc"
.include "data/battle_scripts/moves/beat_up.inc"
.include "data/battle_scripts/moves/belly_drum.inc"
.include "data/battle_scripts/moves/bide.inc"
.include "data/battle_scripts/moves/brick_break.inc"
.include "data/battle_scripts/moves/bulk_up.inc"
.include "data/battle_scripts/moves/calm_mind.inc"
.include "data/battle_scripts/moves/camouflage.inc"
.include "data/battle_scripts/moves/charge.inc"
.include "data/battle_scripts/moves/conversion.inc"
.include "data/battle_scripts/moves/conversion_2.inc"
.include "data/battle_scripts/moves/cosmic_power.inc"
.include "data/battle_scripts/moves/curse.inc"
.include "data/battle_scripts/moves/defense_curl.inc"
.include "data/battle_scripts/moves/destiny_bond.inc"
.include "data/battle_scripts/moves/disable.inc"
.include "data/battle_scripts/moves/double_edge.inc"
.include "data/battle_scripts/moves/dragon_dance.inc"
.include "data/battle_scripts/moves/dragon_rage.inc"
.include "data/battle_scripts/moves/dream_eater.inc"
.include "data/battle_scripts/moves/earthquake.inc"
.include "data/battle_scripts/moves/encore.inc"
.include "data/battle_scripts/moves/endeavor.inc"
.include "data/battle_scripts/moves/eruption.inc"
.include "data/battle_scripts/moves/explosion.inc"
.include "data/battle_scripts/moves/facade.inc"
.include "data/battle_scripts/moves/fake_out.inc"
.include "data/battle_scripts/moves/flail.inc"
.include "data/battle_scripts/moves/flatter.inc"
.include "data/battle_scripts/moves/focus_energy.inc"
.include "data/battle_scripts/moves/focus_punch.inc"
.include "data/battle_scripts/moves/follow_me.inc"
.include "data/battle_scripts/moves/foresight.inc"
.include "data/battle_scripts/moves/fury_cutter.inc"
.include "data/battle_scripts/moves/future_sight.inc"
.include "data/battle_scripts/moves/grudge.inc"
.include "data/battle_scripts/moves/gust.inc"
.include "data/battle_scripts/moves/haze.inc"
.include "data/battle_scripts/moves/heal_bell.inc"
.include "data/battle_scripts/moves/helping_hand.inc"
.include "data/battle_scripts/moves/hidden_power.inc"
.include "data/battle_scripts/moves/imprison.inc"
.include "data/battle_scripts/moves/ingrain.inc"
.include "data/battle_scripts/moves/knock_off.inc"
.include "data/battle_scripts/moves/leech_seed.inc"
.include "data/battle_scripts/moves/light_screen.inc"
.include "data/battle_scripts/moves/lock_on.inc"
.include "data/battle_scripts/moves/low_kick.inc"
.include "data/battle_scripts/moves/magic_coat.inc"
.include "data/battle_scripts/moves/magnitude.inc"
.include "data/battle_scripts/moves/mean_look.inc"
.include "data/battle_scripts/moves/memento.inc"
.include "data/battle_scripts/moves/metronome.inc"
.include "data/battle_scripts/moves/mimic.inc"
.include "data/battle_scripts/moves/minimize.inc"
.include "data/battle_scripts/moves/mirror_coat.inc"
.include "data/battle_scripts/moves/mirror_move.inc"
.include "data/battle_scripts/moves/mist.inc"
.include "data/battle_scripts/moves/nature_power.inc"
.include "data/battle_scripts/moves/nightmare.inc"
.include "data/battle_scripts/moves/overheat.inc"
.include "data/battle_scripts/moves/pain_split.inc"
.include "data/battle_scripts/moves/perish_song.inc"
.include "data/battle_scripts/moves/poison_fang.inc"
.include "data/battle_scripts/moves/present.inc"
.include "data/battle_scripts/moves/psych_up.inc"
.include "data/battle_scripts/moves/psywave.inc"
.include "data/battle_scripts/moves/rage.inc"
.include "data/battle_scripts/moves/rapid_spin.inc"
.include "data/battle_scripts/moves/razor_wind.inc"
.include "data/battle_scripts/moves/recycle.inc"
.include "data/battle_scripts/moves/reflect.inc"
.include "data/battle_scripts/moves/refresh.inc"
.include "data/battle_scripts/moves/rest.inc"
.include "data/battle_scripts/moves/revenge.inc"
.include "data/battle_scripts/moves/roar.inc"
.include "data/battle_scripts/moves/role_play.inc"
.include "data/battle_scripts/moves/rollout.inc"
.include "data/battle_scripts/moves/safeguard.inc"
.include "data/battle_scripts/moves/secret_power.inc"
.include "data/battle_scripts/moves/sketch.inc"
.include "data/battle_scripts/moves/skill_swap.inc"
.include "data/battle_scripts/moves/skull_bash.inc"
.include "data/battle_scripts/moves/sky_attack.inc"
.include "data/battle_scripts/moves/sky_uppercut.inc"
.include "data/battle_scripts/moves/sleep_talk.inc"
.include "data/battle_scripts/moves/smelling_salts.inc"
.include "data/battle_scripts/moves/snatch.inc"
.include "data/battle_scripts/moves/snore.inc"
.include "data/battle_scripts/moves/softboiled.inc"
.include "data/battle_scripts/moves/solar_beam.inc"
.include "data/battle_scripts/moves/sonicboom.inc"
.include "data/battle_scripts/moves/spikes.inc"
.include "data/battle_scripts/moves/spite.inc"
.include "data/battle_scripts/moves/splash.inc"
.include "data/battle_scripts/moves/stockpile_and_friends.inc" @ Stockpile, Spit Up, Swallow
.include "data/battle_scripts/moves/stomp.inc"
.include "data/battle_scripts/moves/substitute.inc"
.include "data/battle_scripts/moves/super_fang.inc"
.include "data/battle_scripts/moves/superpower.inc"
.include "data/battle_scripts/moves/swagger.inc"
.include "data/battle_scripts/moves/taunt.inc"
.include "data/battle_scripts/moves/teeter_dance.inc"
.include "data/battle_scripts/moves/teleport.inc"
.include "data/battle_scripts/moves/thief.inc"
.include "data/battle_scripts/moves/thunder.inc"
.include "data/battle_scripts/moves/tickle.inc"
.include "data/battle_scripts/moves/torment.inc"
.include "data/battle_scripts/moves/transform.inc"
.include "data/battle_scripts/moves/trick.inc"
.include "data/battle_scripts/moves/triple_kick.inc"
.include "data/battle_scripts/moves/twineedle.inc"
.include "data/battle_scripts/moves/twister.inc"
.include "data/battle_scripts/moves/uproar.inc"
.include "data/battle_scripts/moves/weather_ball.inc"
.include "data/battle_scripts/moves/wish.inc"
.include "data/battle_scripts/moves/yawn.inc"

@----------------------------------------------------------------
@   Common
@----------------------------------------------------------------

BattleScript_EffectHit::
	jumpifnotmove MOVE_SURF, BattleScript_HitFromAtkCanceler
	jumpifnostatus3 BS_TARGET, STATUS3_UNDERWATER, BattleScript_HitFromAtkCanceler
	orword gHitMarker, HITMARKER_IGNORE_UNDERWATER
	setbyte sDMG_MULTIPLIER, 2
BattleScript_HitFromAtkCanceler::
	attackcanceler
BattleScript_HitFromAccCheck::
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
BattleScript_HitFromAtkString::
	ppreduce
BattleScript_HitFromCritCalc::
	critcalc
	damagecalc
	typecalc
	adjustnormaldamage
BattleScript_HitFromAtkAnimation::
   attackstringandanimation
	waitanimation
	effectivenesssound
	hitanimation BS_TARGET
	waitstate
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	critmessage
	waitmessage B_WAIT_TIME_LONG
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	seteffectwithchance
	tryfaintmon BS_TARGET
BattleScript_MoveEnd::
	moveendall
	end

@
@ Used for certain moves when the target has Substitute, when Substitute should 
@ block the move for gameplay reasons but it would not make sense in-universe 
@ for Substitute to physically block the move (i.e. Flatter and Swagger).
@
BattleScript_MakeMoveMissed::
	orbyte gMoveResultFlags, MOVE_RESULT_MISSED
BattleScript_PrintMoveMissed::
	ppreduce
BattleScript_PrintMoveMissed_NoPPReduce::
	attackstring
BattleScript_MoveMissedPause::
	pause B_WAIT_TIME_SHORT
BattleScript_MoveMissed::
	effectivenesssound
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_WasntAffected::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNWASNTAFFECTED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_PrintBankAbilityMadeIneffective::
	copybyte sBATTLER, sBATTLER_WITH_ABILITY
BattleScript_PrintAbilityMadeIneffective::
	attackstring
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXMADEITINEFFECTIVE
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

@----------------------------------------------------------------
@   Simple move effects
@----------------------------------------------------------------

BattleScript_EffectPoisonHit::
	setmoveeffect MOVE_EFFECT_POISON
	goto BattleScript_EffectHit

BattleScript_EffectBurnHit::
	setmoveeffect MOVE_EFFECT_BURN
	goto BattleScript_EffectHit

BattleScript_EffectFreezeHit::
	setmoveeffect MOVE_EFFECT_FREEZE
	goto BattleScript_EffectHit

BattleScript_EffectFlinchHit::
	setmoveeffect MOVE_EFFECT_FLINCH
	goto BattleScript_EffectHit

BattleScript_EffectParalyzeHit::
	setmoveeffect MOVE_EFFECT_PARALYSIS
	goto BattleScript_EffectHit

BattleScript_EffectPayDay::
	setmoveeffect MOVE_EFFECT_PAYDAY
	goto BattleScript_EffectHit

BattleScript_EffectTriAttack::
	setmoveeffect MOVE_EFFECT_TRI_ATTACK
	goto BattleScript_EffectHit

BattleScript_EffectAttackDownHit::
	setmoveeffect MOVE_EFFECT_ATK_MINUS_1
	goto BattleScript_EffectHit

BattleScript_EffectDefenseDownHit::
	setmoveeffect MOVE_EFFECT_DEF_MINUS_1
	goto BattleScript_EffectHit

BattleScript_EffectSpeedDownHit::
	setmoveeffect MOVE_EFFECT_SPD_MINUS_1
	goto BattleScript_EffectHit

BattleScript_EffectSpecialAttackDownHit::
	setmoveeffect MOVE_EFFECT_SP_ATK_MINUS_1
	goto BattleScript_EffectHit

BattleScript_EffectSpecialDefenseDownHit::
	setmoveeffect MOVE_EFFECT_SP_DEF_MINUS_1
	goto BattleScript_EffectHit

BattleScript_EffectAccuracyDownHit::
	setmoveeffect MOVE_EFFECT_ACC_MINUS_1
	goto BattleScript_EffectHit

BattleScript_EffectConfuseHit::
	setmoveeffect MOVE_EFFECT_CONFUSION
	goto BattleScript_EffectHit

BattleScript_EffectDefenseUpHit::
	setmoveeffect MOVE_EFFECT_DEF_PLUS_1 | MOVE_EFFECT_AFFECTS_USER
	goto BattleScript_EffectHit

BattleScript_EffectAttackUpHit::
	setmoveeffect MOVE_EFFECT_ATK_PLUS_1 | MOVE_EFFECT_AFFECTS_USER
	goto BattleScript_EffectHit

BattleScript_EffectAllStatsUpHit::
	setmoveeffect MOVE_EFFECT_ALL_STATS_UP | MOVE_EFFECT_AFFECTS_USER
	goto BattleScript_EffectHit

BattleScript_FlinchEffect::
	setmoveeffect MOVE_EFFECT_FLINCH
	goto BattleScript_EffectHit

@----------------------------------------------------------------
@   Other
@----------------------------------------------------------------

BattleScript_EffectRampage::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	jumpifstatus2 BS_ATTACKER, STATUS2_MULTIPLETURNS, BattleScript_EffectRampage2
	ppreduce
BattleScript_EffectRampage2:
	confuseifrepeatingattackends
	goto BattleScript_HitFromCritCalc

BattleScript_EffectMultiHit::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	setmultihitcounter 0
	initmultihitstring
	setbyte sMULTIHIT_EFFECT, 0
BattleScript_MultiHitLoop::
	jumpifhasnohp BS_ATTACKER, BattleScript_MultiHitEnd
	jumpifhasnohp BS_TARGET, BattleScript_MultiHitPrintStrings
	jumpifhalfword CMP_EQUAL, gChosenMove, MOVE_SLEEP_TALK, BattleScript_DoMultiHit
	jumpifstatus BS_ATTACKER, STATUS1_SLEEP, BattleScript_MultiHitPrintStrings
BattleScript_DoMultiHit::
	movevaluescleanup
	copybyte cEFFECT_CHOOSER, sMULTIHIT_EFFECT
	critcalc
	damagecalc
	typecalc
	jumpifmovehadnoeffect BattleScript_MultiHitNoMoreHits
	adjustnormaldamage
	attackanimation
	waitanimation
	effectivenesssound
	hitanimation BS_TARGET
	waitstate
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	critmessage
	waitmessage B_WAIT_TIME_LONG
	printstring STRINGID_EMPTYSTRING3
	waitmessage 1
	addbyte sMULTIHIT_STRING + 4, 1
	moveendto MOVEEND_NEXT_TARGET
	jumpifbyte CMP_COMMON_BITS, gMoveResultFlags, MOVE_RESULT_FOE_ENDURED, BattleScript_MultiHitPrintStrings
	decrementmultihit BattleScript_MultiHitLoop
	goto BattleScript_MultiHitPrintStrings
BattleScript_MultiHitNoMoreHits::
	pause B_WAIT_TIME_SHORT
BattleScript_MultiHitPrintStrings::
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	jumpifmovehadnoeffect BattleScript_MultiHitEnd
	copyarray gBattleTextBuff1, sMULTIHIT_STRING, 6
	printstring STRINGID_HITXTIMES
	waitmessage B_WAIT_TIME_LONG
BattleScript_MultiHitEnd::
	seteffectwithchance
	tryfaintmon BS_TARGET
	moveendcase MOVEEND_SYNCHRONIZE_TARGET
	moveendfrom MOVEEND_IMMUNITY_ABILITIES
	end

BattleScript_EffectRestoreHp::
	attackcanceler
	attackstring
	ppreduce
	tryhealhalfhealth BattleScript_AlreadyAtFullHp, BS_ATTACKER
	attackanimation
	waitanimation
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_PKMNREGAINEDHEALTH
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_EffectOHKO::
	attackcanceler
	attackstring
	ppreduce
	accuracycheck BattleScript_ButItFailed, NO_ACC_CALC_CHECK_LOCK_ON
	typecalc
	jumpifmovehadnoeffect BattleScript_HitFromAtkAnimation
	tryKO BattleScript_KOFail
	trysetdestinybondtohappen
	goto BattleScript_HitFromAtkAnimation
BattleScript_KOFail::
	pause B_WAIT_TIME_LONG
	printfromtable gKOFailedStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_TwoTurnMovesSecondTurn::
	attackcanceler
	setmoveeffect MOVE_EFFECT_CHARGING
	setbyte sB_ANIM_TURN, 1
	clearstatusfromeffect BS_ATTACKER
	orword gHitMarker, HITMARKER_NO_PPDEDUCT
	jumpifnotmove MOVE_SKY_ATTACK, BattleScript_HitFromAccCheck
	setmoveeffect MOVE_EFFECT_FLINCH
	goto BattleScript_HitFromAccCheck

BattleScriptFirstChargingTurn::
	attackcanceler
	printstring STRINGID_EMPTYSTRING3
	ppreduce
	attackanimation
	waitanimation
	orword gHitMarker, HITMARKER_CHARGING
	setmoveeffect MOVE_EFFECT_CHARGING | MOVE_EFFECT_AFFECTS_USER
	seteffectprimary
	copybyte cMULTISTRING_CHOOSER, sTWOTURN_STRINGID
	printfromtable gFirstTurnOfTwoStringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_EffectDoubleHit::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	setmultihitcounter 2
	initmultihitstring
	setbyte sMULTIHIT_EFFECT, 0
	goto BattleScript_MultiHitLoop

BattleScript_EffectCounter::
	attackcanceler
	counterdamagecalculator BattleScript_FailedFromAtkString
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	typecalc2
	adjustsetdamage
	goto BattleScript_HitFromAtkAnimation

BattleScript_EffectThawHit::
	setmoveeffect MOVE_EFFECT_BURN
	goto BattleScript_EffectHit

BattleScript_MoveWeatherChange::
	attackanimation
	waitanimation
	printfromtable gMoveWeatherChangeStringIds
	waitmessage B_WAIT_TIME_LONG
	call BattleScript_WeatherFormChanges
	goto BattleScript_MoveEnd

BattleScript_EffectSemiInvulnerable::
	jumpifstatus2 BS_ATTACKER, STATUS2_MULTIPLETURNS, BattleScript_SecondTurnSemiInvulnerable
	jumpifword CMP_COMMON_BITS, gHitMarker, HITMARKER_NO_ATTACKSTRING, BattleScript_SecondTurnSemiInvulnerable
	jumpifmove MOVE_FLY, BattleScript_FirstTurnFly
	jumpifmove MOVE_DIVE, BattleScript_FirstTurnDive
	jumpifmove MOVE_BOUNCE, BattleScript_FirstTurnBounce
	setbyte sTWOTURN_STRINGID, B_MSG_TURN1_DIG
	goto BattleScript_FirstTurnSemiInvulnerable

BattleScript_FirstTurnBounce::
	setbyte sTWOTURN_STRINGID, B_MSG_TURN1_BOUNCE
	goto BattleScript_FirstTurnSemiInvulnerable

BattleScript_FirstTurnDive::
	setbyte sTWOTURN_STRINGID, B_MSG_TURN1_DIVE
	goto BattleScript_FirstTurnSemiInvulnerable

BattleScript_FirstTurnFly::
	setbyte sTWOTURN_STRINGID, B_MSG_TURN1_FLY
BattleScript_FirstTurnSemiInvulnerable::
	call BattleScriptFirstChargingTurn
	setsemiinvulnerablebit
	goto BattleScript_MoveEnd

BattleScript_SecondTurnSemiInvulnerable::
	attackcanceler
	setmoveeffect MOVE_EFFECT_CHARGING
	setbyte sB_ANIM_TURN, 1
	clearstatusfromeffect BS_ATTACKER
	orword gHitMarker, HITMARKER_NO_PPDEDUCT
	jumpifnotmove MOVE_BOUNCE, BattleScript_SemiInvulnerableTryHit
	setmoveeffect MOVE_EFFECT_PARALYSIS
BattleScript_SemiInvulnerableTryHit::
	accuracycheck BattleScript_SemiInvulnerableMiss, ACC_CURR_MOVE
	clearsemiinvulnerablebit
	goto BattleScript_HitFromAtkString

BattleScript_SemiInvulnerableMiss::
	clearsemiinvulnerablebit
	goto BattleScript_PrintMoveMissed

BattleScript_AlreadyAtFullHp::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNHPFULL
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_NotAffectedWithAtkString::
	attackstring
BattleScript_NotAffected::
	pause B_WAIT_TIME_SHORT
	orbyte gMoveResultFlags, MOVE_RESULT_DOESNT_AFFECT_FOE
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_FaintAttacker::
	playfaintcry BS_ATTACKER
	pause B_WAIT_TIME_LONG
	dofaintanimation BS_ATTACKER
	cleareffectsonfaint BS_ATTACKER
	printstring STRINGID_ATTACKERFAINTED
	return

BattleScript_FaintTarget::
	playfaintcry BS_TARGET
	pause B_WAIT_TIME_LONG
	dofaintanimation BS_TARGET
	cleareffectsonfaint BS_TARGET
	printstring STRINGID_TARGETFAINTED
	return

BattleScript_GiveExp::
	setbyte sGIVEEXP_STATE, 0
	getexp BS_TARGET
	end2

BattleScript_PrintFullBox::
	printselectionstring STRINGID_BOXISFULL
	endselectionscript

BattleScript_Pausex20::
	pause B_WAIT_TIME_SHORT
	return

BattleScript_SideStatusWoreOff::
	printstring STRINGID_PKMNSXWOREOFF
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_SelectingDisabledMoveInPalace::
	printstring STRINGID_PKMNMOVEISDISABLED
BattleScript_SelectingUnusableMoveInPalace::
	moveendto MOVEEND_NEXT_TARGET
	end

BattleScript_WrapFree::
	printstring STRINGID_PKMNGOTFREE
	waitmessage B_WAIT_TIME_LONG
	copybyte gBattlerTarget, sBATTLER
	return

BattleScript_SpikesFree::
	printstring STRINGID_PKMNBLEWAWAYSPIKES
	waitmessage B_WAIT_TIME_LONG
	return

@ Edge-case: a faster Pokemon used a PP-reducing move on a slower Pokemon,
@ fully draining the PP of the move that the latter chose to use on that 
@ turn and thereby leaving the latter unable to act.
@
BattleScript_NoPPForMove::
	attackstring
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_BUTNOPPLEFT
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_MoveUsedIsTormented::
	printstring STRINGID_PKMNCANTUSEMOVETORMENT
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_SelectingTormentedMoveInPalace::
	printstring STRINGID_PKMNCANTUSEMOVETORMENT
	goto BattleScript_SelectingUnusableMoveInPalace

BattleScript_MoveUsedIsTaunted::
	printstring STRINGID_PKMNCANTUSEMOVETAUNT
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_SelectingNotAllowedMoveTauntInPalace::
	printstring STRINGID_PKMNCANTUSEMOVETAUNT
	goto BattleScript_SelectingUnusableMoveInPalace

BattleScript_MoveUsedIsImprisoned::
	printstring STRINGID_PKMNCANTUSEMOVESEALED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_SelectingImprisonedMove::
	printselectionstring STRINGID_PKMNCANTUSEMOVESEALED
	endselectionscript

BattleScript_SelectingImprisonedMoveInPalace::
	printstring STRINGID_PKMNCANTUSEMOVESEALED
	goto BattleScript_SelectingUnusableMoveInPalace

BattleScript_EnduredMsg::
	printstring STRINGID_PKMNENDUREDHIT
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_OneHitKOMsg::
	printstring STRINGID_ONEHITKO
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_FocusPunchSetUp::
	printstring STRINGID_EMPTYSTRING3
	waitmessage 1
	playanimation BS_ATTACKER, B_ANIM_FOCUS_PUNCH_SETUP
	printstring STRINGID_PKMNTIGHTENINGFOCUS
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_MoveUsedIsAsleep::
	printstring STRINGID_PKMNFASTASLEEP
	waitmessage B_WAIT_TIME_LONG
	statusanimation BS_ATTACKER
	goto BattleScript_MoveEnd

BattleScript_MoveUsedWokeUp::
	bicword gHitMarker, HITMARKER_WAKE_UP_CLEAR
	printfromtable gWokeUpStringIds
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_ATTACKER
	return

BattleScript_MonWokeUpInUproar::
	printstring STRINGID_PKMNWOKEUPINUPROAR
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_ATTACKER
	end2

BattleScript_PoisonTurnDmg::
	printstring STRINGID_PKMNHURTBYPOISON
	waitmessage B_WAIT_TIME_LONG
BattleScript_DoStatusTurnDmg::
	statusanimation BS_ATTACKER
BattleScript_DoTurnDmg::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	tryfaintmon BS_ATTACKER
	checkteamslost BattleScript_DoTurnDmgEnd
BattleScript_DoTurnDmgEnd::
	end2

BattleScript_BurnTurnDmg::
	printstring STRINGID_PKMNHURTBYBURN
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_DoStatusTurnDmg

BattleScript_MoveUsedIsFrozen::
	printstring STRINGID_PKMNISFROZEN
	waitmessage B_WAIT_TIME_LONG
	statusanimation BS_ATTACKER
	goto BattleScript_MoveEnd

BattleScript_MoveUsedUnfroze::
	printfromtable gGotDefrostedStringIds
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_ATTACKER
	return

BattleScript_DefrostedViaFireMove::
	printstring STRINGID_PKMNWASDEFROSTED
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_TARGET
	return

BattleScript_MoveUsedIsParalyzed::
	printstring STRINGID_PKMNISPARALYZED
	waitmessage B_WAIT_TIME_LONG
	statusanimation BS_ATTACKER
	cancelmultiturnmoves BS_ATTACKER
	goto BattleScript_MoveEnd

BattleScript_MoveUsedFlinched::
	printstring STRINGID_PKMNFLINCHED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_PrintUproarOverTurns::
	printfromtable gUproarOverTurnStringIds
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_ThrashConfuses::
	chosenstatus2animation BS_ATTACKER, STATUS2_CONFUSION
	printstring STRINGID_PKMNFATIGUECONFUSION
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_PrintPayDayMoneyString::
	printstring STRINGID_PLAYERPICKEDUPMONEY
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_WrapTurnDmg::
	playanimation BS_ATTACKER, B_ANIM_TURN_TRAP, sB_ANIM_ARG1
	printstring STRINGID_PKMNHURTBY
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_DoTurnDmg

BattleScript_WrapEnds::
	printstring STRINGID_PKMNFREEDFROM
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_MoveUsedIsInLove::
	printstring STRINGID_PKMNINLOVE
	waitmessage B_WAIT_TIME_LONG
	status2animation BS_ATTACKER, STATUS2_INFATUATION
	return

BattleScript_MoveUsedIsInLoveCantAttack::
	printstring STRINGID_PKMNIMMOBILIZEDBYLOVE
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_NightmareTurnDmg::
	printstring STRINGID_PKMNLOCKEDINNIGHTMARE
	waitmessage B_WAIT_TIME_LONG
	status2animation BS_ATTACKER, STATUS2_NIGHTMARE
	goto BattleScript_DoTurnDmg

BattleScript_CurseTurnDmg::
	printstring STRINGID_PKMNAFFLICTEDBYCURSE
	waitmessage B_WAIT_TIME_LONG
	status2animation BS_ATTACKER, STATUS2_CURSED
	goto BattleScript_DoTurnDmg

BattleScript_TargetPRLZHeal::
	printstring STRINGID_PKMNHEALEDPARALYSIS
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_TARGET
	return

BattleScript_UpdateEffectStatusIconRet::
	updatestatusicon BS_EFFECT_BATTLER
	waitstate
	return

BattleScript_MoveEffectPoison::
	statusanimation BS_EFFECT_BATTLER
	printfromtable gGotPoisonedStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet

BattleScript_MoveEffectBurn::
	statusanimation BS_EFFECT_BATTLER
	printfromtable gGotBurnedStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet

BattleScript_MoveEffectFreeze::
	statusanimation BS_EFFECT_BATTLER
	printfromtable gGotFrozenStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet

BattleScript_MoveEffectParalysis::
	statusanimation BS_EFFECT_BATTLER
	printfromtable gGotParalyzedStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet

BattleScript_MoveEffectUproar::
	printstring STRINGID_PKMNCAUSEDUPROAR
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_MoveEffectSleep::
	statusanimation BS_EFFECT_BATTLER
	printfromtable gFellAsleepStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet

BattleScript_MoveEffectToxic::
	statusanimation BS_EFFECT_BATTLER
	printstring STRINGID_PKMNBADLYPOISONED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet

BattleScript_MoveEffectPayDay::
	printstring STRINGID_COINSSCATTERED
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_MoveEffectWrap::
	printfromtable gWrappedStringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_MoveEffectRecoil::
	jumpifmove MOVE_STRUGGLE, BattleScript_DoRecoil
	jumpifability BS_ATTACKER, ABILITY_ROCK_HEAD, BattleScript_RecoilEnd
BattleScript_DoRecoil::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_PKMNHITWITHRECOIL
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
BattleScript_RecoilEnd::
	return

BattleScript_MoveEffectConfusion::
	chosenstatus2animation BS_EFFECT_BATTLER, STATUS2_CONFUSION
	printstring STRINGID_PKMNWASCONFUSED
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_ItemSteal::
	playanimation BS_TARGET, B_ANIM_ITEM_STEAL
	printstring STRINGID_PKMNSTOLEITEM
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_SpeedBoostActivates::
	playanimation BS_ATTACKER, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printstring STRINGID_PKMNRAISEDSPEED
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_TraceActivates::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNTRACED
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_RainDishActivates::
	printstring STRINGID_PKMNSXRESTOREDHPALITTLE2
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	end3

BattleScript_SandstreamActivates::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXWHIPPEDUPSANDSTORM
	waitstate
	playanimation BS_BATTLER_0, B_ANIM_SANDSTORM_CONTINUES
	call BattleScript_WeatherFormChanges
	end3

BattleScript_ShedSkinActivates::
	printstring STRINGID_PKMNSXCUREDYPROBLEM
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_ATTACKER
	end3

BattleScript_WeatherFormChanges::
	setbyte sBATTLER, 0
BattleScript_WeatherFormChangesLoop::
	trycastformdatachange
	addbyte sBATTLER, 1
	jumpifbytenotequal sBATTLER, gBattlersCount, BattleScript_WeatherFormChangesLoop
	return

BattleScript_CastformChange::
	call BattleScript_DoCastformChange
	end3

BattleScript_DoCastformChange::
	docastformchangeanimation
	waitstate
	printstring STRINGID_PKMNTRANSFORMED
	waitmessage B_WAIT_TIME_LONG
	return


BattleScript_TookAttack::
	attackstring
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXTOOKATTACK
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_ATTACKSTRING_PRINTED
	return

BattleScript_SturdyPreventsOHKO::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNPROTECTEDBY
	pause B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_DampStopsExplosion::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNPREVENTSUSAGE
	pause B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_MoveHPDrain_PPLoss::
	ppreduce
BattleScript_MoveHPDrain::
	attackstring
	pause B_WAIT_TIME_SHORT
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	printstring STRINGID_PKMNRESTOREDHPUSING
	waitmessage B_WAIT_TIME_LONG
	orbyte gMoveResultFlags, MOVE_RESULT_DOESNT_AFFECT_FOE
	goto BattleScript_MoveEnd

BattleScript_MonMadeMoveUseless_PPLoss::
	ppreduce
BattleScript_MonMadeMoveUseless::
	attackstring
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXMADEYUSELESS
	waitmessage B_WAIT_TIME_LONG
	orbyte gMoveResultFlags, MOVE_RESULT_DOESNT_AFFECT_FOE
	goto BattleScript_MoveEnd

BattleScript_FlashFireBoost_PPLoss::
	ppreduce
BattleScript_FlashFireBoost::
	attackstring
	pause B_WAIT_TIME_SHORT
	printfromtable gFlashFireStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_AbilityNoStatLoss::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNPREVENTSSTATLOSSWITH
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_BRNPrevention::
	pause B_WAIT_TIME_SHORT
	printfromtable gBRNPreventionStringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_PRLZPrevention::
	pause B_WAIT_TIME_SHORT
	printfromtable gPRLZPreventionStringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_PSNPrevention::
	pause B_WAIT_TIME_SHORT
	printfromtable gPSNPreventionStringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_ObliviousPreventsAttraction::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNPREVENTSROMANCEWITH
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_FlinchPrevention::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXPREVENTSFLINCHING
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_SoundproofProtected::
	attackstring
	ppreduce
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXBLOCKSY
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_AbilityNoSpecificStatLoss::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXPREVENTSYLOSS
	waitmessage B_WAIT_TIME_LONG
	setbyte cMULTISTRING_CHOOSER, B_MSG_STAT_FELL_EMPTY
	return

BattleScript_StickyHoldActivates::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXMADEYINEFFECTIVE
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_ColorChangeActivates::
	printstring STRINGID_PKMNCHANGEDTYPEWITH
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_RoughSkinActivates::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_DAMAGE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_PKMNHURTSWITH
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	return

BattleScript_CuteCharmActivates::
	status2animation BS_ATTACKER, STATUS2_INFATUATION
	printstring STRINGID_PKMNSXINFATUATEDY
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_ApplySecondaryEffect::
	waitstate
	seteffectsecondary
	return

BattleScript_SynchronizeActivates::
	waitstate
	seteffectprimary
	return

BattleScript_NoItemSteal::
	pause B_WAIT_TIME_SHORT
	printstring STRINGID_PKMNSXMADEYINEFFECTIVE
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_AbilityCuredStatus::
	printstring STRINGID_PKMNSXCUREDITSYPROBLEM
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_SCRIPTING
	return

BattleScript_IgnoresWhileAsleep::
	printstring STRINGID_PKMNIGNORESASLEEP
	waitmessage B_WAIT_TIME_LONG
	moveendto MOVEEND_NEXT_TARGET
	end

BattleScript_IgnoresAndUsesRandomMove::
	printstring STRINGID_PKMNIGNOREDORDERS
	waitmessage B_WAIT_TIME_LONG
	jumptocalledmove FALSE

BattleScript_MoveUsedLoafingAround::
	@ Skip ahead if not the Battle Palace message
	jumpifbyte CMP_NOT_EQUAL, cMULTISTRING_CHOOSER, B_MSG_INCAPABLE_OF_POWER, BattleScript_MoveUsedLoafingAroundMsg
	setbyte gBattleCommunication, 0
	palacetryescapestatus BS_ATTACKER
	setbyte cMULTISTRING_CHOOSER, B_MSG_INCAPABLE_OF_POWER
BattleScript_MoveUsedLoafingAroundMsg::
	printfromtable gInobedientStringIds
	waitmessage B_WAIT_TIME_LONG
	moveendto MOVEEND_NEXT_TARGET
	end

BattleScript_IgnoresAndFallsAsleep::
	printstring STRINGID_PKMNBEGANTONAP
	waitmessage B_WAIT_TIME_LONG
	setmoveeffect MOVE_EFFECT_SLEEP | MOVE_EFFECT_AFFECTS_USER
	seteffectprimary
	moveendto MOVEEND_NEXT_TARGET
	end

BattleScript_IgnoresAndHitsItself::
	printstring STRINGID_PKMNWONTOBEY
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_DoSelfConfusionDmg

BattleScript_SubstituteFade::
	playanimation BS_TARGET, B_ANIM_SUBSTITUTE_FADE
	printstring STRINGID_PKMNSUBSTITUTEFADED
	return

BattleScript_BerryCurePrlzEnd2::
	call BattleScript_BerryCureParRet
	end2

BattleScript_BerryCureParRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMCUREDPARALYSIS
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_SCRIPTING
	removeitem BS_SCRIPTING
	return

BattleScript_BerryCurePsnEnd2::
	call BattleScript_BerryCurePsnRet
	end2

BattleScript_BerryCurePsnRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMCUREDPOISON
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_SCRIPTING
	removeitem BS_SCRIPTING
	return

BattleScript_BerryCureBrnEnd2::
	call BattleScript_BerryCureBrnRet
	end2

BattleScript_BerryCureBrnRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMHEALEDBURN
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_SCRIPTING
	removeitem BS_SCRIPTING
	return

BattleScript_BerryCureFrzEnd2::
	call BattleScript_BerryCureFrzRet
	end2

BattleScript_BerryCureFrzRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMDEFROSTEDIT
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_SCRIPTING
	removeitem BS_SCRIPTING
	return

BattleScript_BerryCureSlpEnd2::
	call BattleScript_BerryCureSlpRet
	end2

BattleScript_BerryCureSlpRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMWOKEIT
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_SCRIPTING
	removeitem BS_SCRIPTING
	return

BattleScript_BerryCureConfusionEnd2::
	call BattleScript_BerryCureConfusionRet
	end2

BattleScript_BerryCureConfusionRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMSNAPPEDOUT
	waitmessage B_WAIT_TIME_LONG
	removeitem BS_SCRIPTING
	return

BattleScript_BerryCureChosenStatusEnd2::
	call BattleScript_BerryCureChosenStatusRet
	end2

BattleScript_BerryCureChosenStatusRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printfromtable gBerryEffectStringIds
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_SCRIPTING
	removeitem BS_SCRIPTING
	return

BattleScript_WhiteHerbEnd2::
	call BattleScript_WhiteHerbRet
	end2

BattleScript_WhiteHerbRet::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMRESTOREDSTATUS
	waitmessage B_WAIT_TIME_LONG
	removeitem BS_SCRIPTING
	return

BattleScript_ItemHealHP_RemoveItem::
	playanimation BS_ATTACKER, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMRESTOREDHEALTH
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	removeitem BS_ATTACKER
	end2

BattleScript_BerryPPHealEnd2::
	playanimation BS_ATTACKER, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMRESTOREDPP
	waitmessage B_WAIT_TIME_LONG
	removeitem BS_ATTACKER
	end2

BattleScript_ItemHealHP_End2::
	call BattleScript_ItemHealHP_Ret
	end2

BattleScript_ItemHealHP_Ret::
	playanimation BS_ATTACKER, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMRESTOREDHPALITTLE
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	return

BattleScript_SelectingNotAllowedMoveChoiceItem::
	printselectionstring STRINGID_ITEMALLOWSONLYYMOVE
	endselectionscript

BattleScript_FocusBandActivates::
	playanimation BS_TARGET, B_ANIM_FOCUS_BAND
	printstring STRINGID_PKMNHUNGONWITHX
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_BerryConfuseHealEnd2::
	playanimation BS_ATTACKER, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNSITEMRESTOREDHEALTH
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_FORXCOMMAYZ
	waitmessage B_WAIT_TIME_LONG
	setmoveeffect MOVE_EFFECT_CONFUSION | MOVE_EFFECT_AFFECTS_USER
	seteffectprimary
	removeitem BS_ATTACKER
	end2

BattleScript_BerryStatRaiseEnd2::
	playanimation BS_ATTACKER, B_ANIM_HELD_ITEM_EFFECT
	trystatchange 0, (STAT_CHANGE_USE_QUEUED_CHANGE | MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_SUPPRESS_ANIMATIONS), 0, STAT_CHANGE_CAUSE_ITEM_HELD, 0
	removeitem BS_ATTACKER
	end2

BattleScript_BerryFocusEnergyEnd2::
	playanimation BS_ATTACKER, B_ANIM_HELD_ITEM_EFFECT
	printstring STRINGID_PKMNUSEDXTOGETPUMPED
	waitmessage B_WAIT_TIME_LONG
	removeitem BS_ATTACKER
	end2

BattleScript_ActionSelectionItemsCantBeUsed::
	printselectionstring STRINGID_ITEMSCANTBEUSEDNOW
	endselectionscript

BattleScript_FlushMessageBox::
	printstring STRINGID_EMPTYSTRING3
	return

BattleScript_PalacePrintFlavorText::
	setbyte gBattleCommunication + 1, 0
BattleScript_PalaceTryBattlerFlavorText::
	palaceflavortext BS_ATTACKER @ BS_ATTACKER here overwritten by gBattleCommunication + 1
	jumpifbyte CMP_NOT_EQUAL, gBattleCommunication, TRUE, BattleScript_PalaceEndFlavorText
	printfromtable gBattlePalaceFlavorTextTable
	waitmessage B_WAIT_TIME_LONG
BattleScript_PalaceEndFlavorText::
	addbyte gBattleCommunication + 1, 1
	jumpifbytenotequal gBattleCommunication + 1, gBattlersCount, BattleScript_PalaceTryBattlerFlavorText
	setbyte gBattleCommunication, 0
	setbyte gBattleCommunication + 1, 0
	end2


BattleScript_AskIfWantsToForfeitMatch::
	printselectionstring STRINGID_QUESTIONFORFEITMATCH
	forfeityesnobox BS_ATTACKER
	endselectionscript

BattleScript_PrintPlayerForfeited::
	printstring STRINGID_FORFEITEDMATCH
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_PlayerForfeitedNonBattleFacilityBattle::
	printstring STRINGID_FORFEITEDMATCH
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_LocalBattleLostPrintWhiteOutSecondHalfOnly

BattleScript_PrintPlayerForfeitedLinkBattle::
	printstring STRINGID_FORFEITEDMATCH
	waitmessage B_WAIT_TIME_LONG
	endlinkbattle
	waitmessage B_WAIT_TIME_LONG
	end2
