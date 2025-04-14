#ifndef GUARD_STARTER_CHOOSE_H
#define GUARD_STARTER_CHOOSE_H

#include "lu/game_typedefs.h"

extern const u16 gBirchBagGrass_Pal[];
extern const u32 gBirchBagTilemap[];
extern const u32 gBirchGrassTilemap[];
extern const u32 gBirchBagGrass_Gfx[];
extern const u32 gPokeballSelection_Gfx[];

PokemonSpeciesID GetStarterPokemon(u16 chosenStarterId);
void CB2_ChooseStarter(void);

#endif // GUARD_STARTER_CHOOSE_H
