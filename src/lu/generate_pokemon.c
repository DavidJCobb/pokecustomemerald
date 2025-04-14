#include "lu/generate_pokemon.h"
#include "constants/items.h"
#include "global.h"
#include "pokemon.h"
#include "random.h"

extern void GeneratePokemon(struct Pokemon* mon, const struct GeneratePokemonRequest* req) {
   ZeroMonData(mon);
   
   u32 ot_id = req->ot_id;
   if (req->flags & GENERATE_POKEMON_WITH_OT_ID) {
      ;
   } else if (req->flags & GENERATE_POKEMON_WITH_OT_ID_OF_PLAYER) {
      ot_id = gSaveBlock2Ptr->playerTrainerId[0]
              | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
              | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
              | (gSaveBlock2Ptr->playerTrainerId[3] << 24);
   } else {
      ot_id = Random32();
   }
   
   bool8 forced_personality = FALSE;
   u32   personality = req->personality;
   if (req->flags & GENERATE_POKEMON_WITH_PERSONALITY) {
      forced_personality = TRUE;
   } else {
      u8 p_flags = req->flags & (
         GENERATE_POKEMON_WITH_GENDER |
         GENERATE_POKEMON_WITH_NATURE |
         GENERATE_POKEMON_WITH_SHININESS_SET |
         GENERATE_POKEMON_WITH_UNOWN_LETTER
      );
      if (p_flags) {
         if (p_flags & GENERATE_POKEMON_WITH_GENDER) {
            switch (gSpeciesInfo[req->species].genderRatio) {
               case MON_MALE:
               case MON_FEMALE:
               case MON_GENDERLESS:
                  //
                  // Fixed-gender species. Forcing any particular gender 
                  // is not possible.
                  //
                  p_flags &= ~GENERATE_POKEMON_WITH_GENDER;
                  break;
            }
         }
         if (p_flags != 0) {
            forced_personality = TRUE;
            do {
               personality = Random32();
               if (p_flags & GENERATE_POKEMON_WITH_GENDER) {
                  if (GetGenderFromSpeciesAndPersonality(req->species, personality) != req->gender)
                     continue;
               }
               if (p_flags & GENERATE_POKEMON_WITH_NATURE) {
                  if (GetNatureFromPersonality(personality) != req->nature)
                     continue;
               }
               if (p_flags & GENERATE_POKEMON_WITH_SHININESS_SET) {
                  bool8 is_shiny = GET_SHINY_VALUE(ot_id, personality) < SHINY_ODDS;
                  if (is_shiny != req->shiny)
                     continue;
               }
               if (p_flags & GENERATE_POKEMON_WITH_UNOWN_LETTER) {
                  u8 letter = GET_UNOWN_LETTER(personality);
                  if (letter != req->unown_letter)
                     continue;
               }
               break;
            } while (TRUE);
         }
      }
   }
   
   CreateMon(
      mon,
      req->species,
      req->level,
      req->fixed_iv,
      forced_personality,
      personality,
      OT_ID_PRESET,
      ot_id
   );
   if (req->held_item != ITEM_NONE) {
      u8 data[2];
      data[0] = req->held_item;
      data[1] = req->held_item >> 8;
      SetMonData(mon, MON_DATA_HELD_ITEM, &data);
   }
}