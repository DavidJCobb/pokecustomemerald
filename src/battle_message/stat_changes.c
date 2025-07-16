#include "battle_message/stat_changes.h"
#include "global.h" // dependency for battle.h
#include "battle.h"
#include "battle_message.h"
#include "string_util.h"
#include "text.h"
#include "constants/characters.h"
#include "lu/string_wrap.h"

//
// In the strings below, index 0x01 is a sentinel for the subject, and 0x00 for the cause.
//
// We'll copy the strings into a local buffer and replace the indices as needed.
//
#pragma region Strings
   #define CAUSE_NAME      "{B_IDX_NAME_WITH_PREFIX}{B_IDX_00}"
   #define SUBJECT_NAME    "{B_IDX_NAME_WITH_PREFIX}{B_IDX_01}"
   #define CAUSE_ABILITY   "{B_IDX_ABILITY}{B_IDX_00}"
   #define SUBJECT_ABILITY "{B_IDX_ABILITY}{B_IDX_01}"
   #define STATS_LIST      "{B_BUFF1}"
   #define STATS_CHANGE    "{B_BUFF3}"
   
   static const u8 sStatChange_General[] = _(SUBJECT_NAME "'s " STATS_LIST " " STATS_CHANGE "!");
   static const u8 sStatChange_Item[]    = _("Using {B_LAST_ITEM}, "SUBJECT_NAME"'s "STATS_LIST" "STATS_CHANGE"!");
   
   static const u8 sStatChange_Loss_FoeAbility[] = _(CAUSE_NAME"'s "CAUSE_ABILITY" cuts "SUBJECT_NAME"'s "STATS_LIST"!");
   static const u8 sStatChange_Loss_OwnAbility[] = _(CAUSE_NAME"'s "CAUSE_ABILITY" cuts {B_ACTIVE_PRONOUN}{PRONOUN_FORM_POSSESSIVE} "STATS_LIST"!");
   
   static const u8 sStatChange_UnilateralFailure_Ability[] = _(SUBJECT_NAME"'s "SUBJECT_ABILITY" prevents stat loss!");
   static const u8 sStatChange_UnilateralFailure_AbilityBlocksAbility[] = _(SUBJECT_NAME"'s "SUBJECT_ABILITY" prevented "CAUSE_NAME"'s "CAUSE_ABILITY" from working!");
   static const u8 sStatChange_UnilateralFailure_Mist[] = _(SUBJECT_NAME" is protected by MIST!");
   static const u8 sStatChange_UnilateralFailure_Protect[] = _(SUBJECT_NAME" protected {B_ACTIVE_PRONOUN}{PRONOUN_FORM_REFLEXIVE}!");
   
   static const u8 sStatChange_PerStatFailure_Ability[] = _(SUBJECT_NAME"'s "SUBJECT_ABILITY" prevents "STATS_LIST" loss!");
   
   static const u8 sStatChange_PerStatFailure_Bounded_Gain[] = _(SUBJECT_NAME"'s "STATS_LIST" won't go higher!");
   static const u8 sStatChange_PerStatFailure_Bounded_Loss[] = _(SUBJECT_NAME"'s "STATS_LIST" won't go lower!");
   
   #undef CAUSE_NAME
   #undef SUBJECT_NAME
   #undef CAUSE_ABILITY
   #undef SUBJECT_ABILITY
   #undef STATS_LIST
#pragma endregion

void BufferStatList(u8* battle_text_buffer, u8 stats) {
   battle_text_buffer[0] = B_BUFF_PLACEHOLDER_BEGIN;
   battle_text_buffer[1] = B_BUFF_STAT_LIST;
   battle_text_buffer[2] = stats;
   battle_text_buffer[3] = B_BUFF_EOS;
}

static const u8 sStatMagnitude_Gain1[] = _("rose$");
static const u8 sStatMagnitude_Gain2[] = _("sharply rose$");
static const u8 sStatMagnitude_Loss1[] = _("fell$");
static const u8 sStatMagnitude_Loss2[] = _("harshly fell$");
u8* WriteStatMagnitudeInto(u8* dst, s8 magnitude) {
   const u8* format;
   if (magnitude < 0) {
      format = sStatMagnitude_Loss1;
      if (magnitude < -1)
         format = sStatMagnitude_Loss2;
   } else {
      format = sStatMagnitude_Gain1;
      if (magnitude > 1)
         format = sStatMagnitude_Gain2;
   }
   return StringCopy(dst, format);
}

static void FinalizeBuffering(
   const u8* format,
   const u8  battler_cause,
   const u8  battler_subject
) {
   if (!format) {
      u8 empty = EOS;
      BattleStringExpandPlaceholdersToDisplayedString(&empty);
      return;
   }
   
   #define BUFFER_SIZE 200
   u8 buffer[BUFFER_SIZE];
   StringCopy(buffer, format);
   for(u8 i = 0; i + 2 < BUFFER_SIZE; ++i) {
      if (buffer[i] != B_BUFF_PLACEHOLDER_BEGIN)
         continue;
      u8 placeholder = buffer[i + 1];
      u8 param       = buffer[i + 2];
      switch (placeholder) {
         case B_TXT_IDX_ABILITY:
         case B_TXT_IDX_NAME_WITH_PREFIX:
            i += 2;
            if (param == 0x00) {
               buffer[i] = battler_cause;
            } else if (param == 0x01) {
               buffer[i] = battler_subject;
            }
            break;
      }
   }
   #undef BUFFER_SIZE
   
   BattleStringExpandPlaceholdersToDisplayedString(buffer);
   lu_PrepStringWrap(B_WIN_MSG, FONT_NORMAL);
   lu_StringWrap(gDisplayedStringBattle);
}

void BufferStatChangeSuccessMessage(
   u8 battler_cause,
   u8 battler_affected,
   u8 change_cause,
   u8 stats,
   s8 magnitude
) {
   BufferStatList(gBattleTextBuff1, stats);
   WriteStatMagnitudeInto(gBattleTextBuff3, magnitude);
   
   const u8* format = sStatChange_General;
   if (change_cause == STATCHANGEMESSAGE_CAUSE_ITEM_HELD || change_cause == STATCHANGEMESSAGE_CAUSE_ITEM_USED) {
      format = sStatChange_Item;
   }
   if (magnitude < 0 && change_cause == STATCHANGEMESSAGE_CAUSE_ABILITY) {
      if (battler_cause == battler_affected) {
         format = sStatChange_Loss_OwnAbility;
      } else {
         format = sStatChange_Loss_FoeAbility;
      }
   }
   FinalizeBuffering(format, battler_cause, battler_affected);
}
void BufferStatChangeAllFailMessage(
   u8 battler_cause,
   u8 battler_affected,
   u8 change_cause,
   enum StatChangeMessage_AllStatsBlockedBy failure_cause
) {
   const u8* format = NULL;
   switch (failure_cause) {
      case STATCHANGEMESSAGE_ALL_STATS_NOT_BLOCKED:
         return;
      case STATCHANGEMESSAGE_ALLSTATSBLOCKEDBY_ABILITY:
         if (change_cause == STATCHANGEMESSAGE_CAUSE_ABILITY) {
            format = sStatChange_UnilateralFailure_AbilityBlocksAbility;
         } else {
            format = sStatChange_UnilateralFailure_Ability;
         }
         break;
      case STATCHANGEMESSAGE_ALLSTATSBLOCKEDBY_MIST:
         format = sStatChange_UnilateralFailure_Mist;
         break;
      case STATCHANGEMESSAGE_ALLSTATSBLOCKEDBY_PROTECT:
         format = sStatChange_UnilateralFailure_Protect;
         break;
   }
   FinalizeBuffering(format, battler_cause, battler_affected);
}
void BufferStatChangeAnyFailMessage(
   u8 battler_cause,
   u8 battler_affected,
   u8 change_cause,
   u8 stats,
   s8 magnitude,
   enum StatChangeMessage_AnyStatBlockedBy failure_cause
) {
   BufferStatList(gBattleTextBuff1, stats);
   const u8* format = NULL;
   switch (failure_cause) {
      case STATCHANGEMESSAGE_ANYSTATBLOCKEDBY_ABILITY:
         if (change_cause == STATCHANGEMESSAGE_CAUSE_ABILITY) {
            format = sStatChange_UnilateralFailure_AbilityBlocksAbility;
         } else {
            format = sStatChange_PerStatFailure_Ability;
         }
         break;
      case STATCHANGEMESSAGE_ANYSTATBLOCKEDBY_BOUNDED:
         if (magnitude < 0) {
            format = sStatChange_PerStatFailure_Bounded_Loss;
         } else {
            format = sStatChange_PerStatFailure_Bounded_Gain;
         }
         break;
   }
   FinalizeBuffering(format, battler_cause, battler_affected);
}