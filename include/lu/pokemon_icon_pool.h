#ifndef GUARD_LU_POKEMON_ICON_POOL_H
#define GUARD_LU_POKEMON_ICON_POOL_H

#include "gba/types.h"
#include "lu/game_typedefs.h"

struct PokemonIconPool {
   u8                count;
   PokemonSpeciesID* species_ids;
   u8*               sprite_ids;
   u8*               delete_flags;
};

#define NO_POOLED_POKEMON_ICON 0xFFFF

extern void InitPokemonIconPool(struct PokemonIconPool*, u8 count);
extern void ClearPokemonIconPool(struct PokemonIconPool*);
extern void DestroyPokemonIconPool(struct PokemonIconPool*);

extern u16 FindPooledPokemonIconBySpecies(const struct PokemonIconPool*, PokemonSpeciesID);

extern u16 AddPooledPokemonIcon(struct PokemonIconPool*, PokemonSpeciesID, u8 x, u8 y);
extern void RemovePooledPokemonIcon(struct PokemonIconPool*, u8 index);

extern void MarkAllPooledPokemonIconsForDelete(struct PokemonIconPool*);
extern void MarkPooledPokemonIconForDelete(struct PokemonIconPool*, u8 index);
extern void UnmarkPooledPokemonIconForDelete(struct PokemonIconPool*, u8 index);
extern void DeleteMarkedPooledPokemonIcons(struct PokemonIconPool*);

extern void SetPooledPokemonIconsVisible(struct PokemonIconPool*, bool8 visible);

extern void SetPooledPokemonIconPosition(struct PokemonIconPool*, u8 index, u8 x, u8 y);

#endif