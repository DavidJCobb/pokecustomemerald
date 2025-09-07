#include "custom_game_options/handlers/starter.h"
#include "constants/items.h"
#include "global.h"
#include "pokedex.h"
#include "pokemon.h"
#include "custom_game_options/options.h"
#include "lu/generate_pokemon.h"

void GenerateAndGivePlayerStarter(PokemonSpeciesID species) {
   struct Pokemon mon;
   {
      struct GeneratePokemonRequest params = {
         .flags     = GENERATE_POKEMON_WITH_OT_ID_OF_PLAYER,
         .gender    = 0,
         .held_item = ITEM_NONE,
         .level     = gCustomGameOptions.starters.level,
         .species   = species,
      };
      if (gCustomGameOptions.starters.force_gender != CGO_STARTERPOKEMONGENDER_RANDOM) {
         params.flags |= GENERATE_POKEMON_WITH_GENDER;
         switch (gCustomGameOptions.starters.force_gender) {
            case CGO_STARTERPOKEMONGENDER_MALE:
               params.gender = MON_MALE;
               break;
            case CGO_STARTERPOKEMONGENDER_FEMALE:
               params.gender = MON_FEMALE;
               break;
            default:
               params.gender = MON_GENDERLESS;
               break;
         }
      }
      GeneratePokemon(&mon, &params);
   }
   
   int sentToPC = GiveMonToPlayer(&mon);
   
   PokemonSpeciesID nationalDexNum = SpeciesToNationalPokedexNum(species);
   switch (sentToPC) {
      case MON_CANT_GIVE:
         break;
      default:
         GetSetPokedexFlag(nationalDexNum, FLAG_SET_SEEN);
         GetSetPokedexFlag(nationalDexNum, FLAG_SET_CAUGHT);
         break;
    }
}
