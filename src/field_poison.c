#include "global.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "field_message_box.h"
#include "field_poison.h"
#include "fldeff_misc.h"
#include "frontier_util.h"
#include "party_menu.h"
#include "pokenav.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "trainer_hill.h"
#include "constants/field_poison.h"
#include "constants/party_menu.h"

#include "lu/custom_game_options.h"

static bool32 IsMonValidSpecies(struct Pokemon *pokemon)
{
    u16 species = GetMonData(pokemon, MON_DATA_SPECIES_OR_EGG);
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return FALSE;

    return TRUE;
}

static bool32 AllMonsFainted(void)
{
    int i;
    struct Pokemon *pokemon = gPlayerParty;

    for (i = 0; i < PARTY_SIZE; i++, pokemon++)
    {
        if (IsMonValidSpecies(pokemon) && GetMonData(pokemon, MON_DATA_HP) != 0)
            return FALSE;
    }
    return TRUE;
}

static void FaintFromFieldPoison(u8 partyIdx)
{
    struct Pokemon *pokemon = &gPlayerParty[partyIdx];
    u32 status = STATUS1_NONE;

    AdjustFriendship(pokemon, FRIENDSHIP_EVENT_FAINT_FIELD_PSN);
    SetMonData(pokemon, MON_DATA_STATUS, &status);
    GetMonData(pokemon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
}

static void RecoverFromFieldPoison(u8 partyIdx) {
   struct Pokemon *pokemon = &gPlayerParty[partyIdx];
   u32 status = STATUS1_NONE;
   
   AdjustFriendship(pokemon, FRIENDSHIP_EVENT_FAINT_FIELD_PSN);
   SetMonData(pokemon, MON_DATA_STATUS, &status);
   GetMonData(pokemon, MON_DATA_NICKNAME, gStringVar1);
   StringGet_Nickname(gStringVar1);
}

static bool32 MonFaintedFromPoison(u8 partyIdx)
{
    struct Pokemon *pokemon = &gPlayerParty[partyIdx];
    if (IsMonValidSpecies(pokemon) && GetMonData(pokemon, MON_DATA_HP) == 0 && GetAilmentFromStatus(GetMonData(pokemon, MON_DATA_STATUS)) == AILMENT_PSN)
        return TRUE;

    return FALSE;
}

static bool8 MonCanBeCuredOfPoison(u8 partyIdx) {
   struct Pokemon *pokemon = &gPlayerParty[partyIdx];
   if (IsMonValidSpecies(pokemon) && GetMonData(pokemon, MON_DATA_HP) == 1 && GetAilmentFromStatus(GetMonData(pokemon, MON_DATA_STATUS)) == AILMENT_PSN)
      return TRUE;

    return FALSE;
}

#define tState    data[0]
#define tPartyIdx data[1]

#include "strings/field_poison.h"

static void Task_TryFieldPoisonWhiteOut(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:
        for (; tPartyIdx < PARTY_SIZE; tPartyIdx++)
        {
            if (gCustomGameOptions.overworld_poison.faint) {
               if (MonFaintedFromPoison(tPartyIdx)) {
                  FaintFromFieldPoison(tPartyIdx);
                  ShowFieldMessage(gText_PkmnFainted_FldPsn);
                  tState++;
                  return;
               }
            } else {
               if (MonCanBeCuredOfPoison(tPartyIdx)) {
                  RecoverFromFieldPoison(tPartyIdx);
                  ShowFieldMessage(gText_FieldPoison_EnduredAndCured);
                  tState++;
                  return;
               }
            }
        }
        tState = 2; // Finished checking party
        break;
    case 1:
        // Wait for "{mon} fainted" message, then return to party loop
        if (IsFieldMessageBoxHidden())
            tState--;
        break;
    case 2:
        if (AllMonsFainted())
        {
            // Battle facilities have their own white out script to handle the challenge loss
#ifdef BUGFIX
            if (CurrentBattlePyramidLocation() || InBattlePike() || InTrainerHillChallenge())
#else
            if (CurrentBattlePyramidLocation() | InBattlePike() || InTrainerHillChallenge())
#endif
                gSpecialVar_Result = FLDPSN_FRONTIER_WHITEOUT;
            else
                gSpecialVar_Result = FLDPSN_WHITEOUT;
        }
        else
        {
            gSpecialVar_Result = FLDPSN_NO_WHITEOUT;
        }
        ScriptContext_Enable();
        DestroyTask(taskId);
        break;
    }
}

#undef tState
#undef tPartyIdx

void TryFieldPoisonWhiteOut(void)
{
    CreateTask(Task_TryFieldPoisonWhiteOut, 80);
    ScriptContext_Stop();
}

s32 DoPoisonFieldEffect(void)
{
    int i;
    u32 hp;
    struct Pokemon *pokemon = gPlayerParty;
    u32 numPoisoned = 0;
    u32 numFainted = 0;
    u32 numCured = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 status = GetMonData(pokemon, MON_DATA_STATUS);
        if (GetMonData(pokemon, MON_DATA_SANITY_HAS_SPECIES) && GetAilmentFromStatus(status) == AILMENT_PSN)
        {
            // Apply poison damage
            hp = GetMonData(pokemon, MON_DATA_HP);
            if (hp == 0) {
               numFainted++;
            } else {
               u16 damage = gCustomGameOptions.overworld_poison.damage; // vanilla: 1
               if (hp <= damage) {
                  hp = 0;
                  if (gCustomGameOptions.overworld_poison.faint) {
                     numFainted++;
                  } else {
                     hp = 1;
                     numCured++;
                  }
               } else {
                  hp -= damage;
                  if (hp == 1 && !gCustomGameOptions.overworld_poison.faint) {
                     numCured++;
                  }
               }
            }

            SetMonData(pokemon, MON_DATA_HP, &hp);
            numPoisoned++;
        }
        pokemon++;
    }

    // Do screen flash effect
    if (numFainted != 0 || numPoisoned != 0)
        FldEffPoison_Start();

    if (numFainted != 0)
        return FLDPSN_FNT;

    if (numCured != 0)
        return FLDPSN_HEAL;

    if (numPoisoned != 0)
        return FLDPSN_PSN;

    return FLDPSN_NONE;
}
