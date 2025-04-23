#include "lu/pokemon_icon_pool.h"
#include <string.h> // memset
#include "gba/defines.h"
#include "malloc.h"
#include "sprite.h"
#include "pokemon_icon.h"
#include "gba/isagbprint.h"

/*

   MEMORY LAYOUT:
   
      PokemonSpeciesID[n]                 species_ids
      u8[n]                               sprite_ids
      u8[(n / 8) + ((n % 8) ? 1 : 0)]     delete_flags

*/

static u16 ComputeDeleteFlagsBufferSize(u8 count) {
   return sizeof(u8) * ((count / 8) + ((count % 8) ? 1 : 0));
}
static u16 ComputeBufferSize(u8 count) {
   return (
      sizeof(PokemonSpeciesID) * count +
      sizeof(u8) * count +
      ComputeDeleteFlagsBufferSize(count)
   );
}

void InitPokemonIconPool(struct PokemonIconPool* pool, u8 count) {
   if (pool->species_ids) {
      if (pool->count == count) {
         ClearPokemonIconPool(pool);
         return;
      }
      Free(pool->species_ids);
      pool->species_ids  = NULL;
      pool->sprite_ids   = NULL;
      pool->delete_flags = NULL;
   }
   pool->count        = count;
   pool->species_ids  = (PokemonSpeciesID*) AllocZeroed(ComputeBufferSize(count));
   pool->sprite_ids   = (u8*)pool->species_ids + (count * sizeof(PokemonSpeciesID));
   pool->delete_flags = (u8*)pool->sprite_ids + (count * sizeof(u8));
   for(int i = 0; i < pool->count; ++i) {
      pool->sprite_ids[i] = SPRITE_NONE;
   }
}
void ClearPokemonIconPool(struct PokemonIconPool* pool) {
   if (!pool->species_ids) {
      return;
   }
   for(int i = 0; i < pool->count; ++i) {
      pool->species_ids[i] = 0;
      
      u8 id = pool->sprite_ids[i];
      if (id < MAX_SPRITES) {
         DestroySprite(&gSprites[id]);
         pool->sprite_ids[i] = SPRITE_NONE;
      }
   }
   memset(
      pool->delete_flags,
      0,
      ComputeDeleteFlagsBufferSize(pool->count)
   );
}
void DestroyPokemonIconPool(struct PokemonIconPool* pool) {
   if (pool->species_ids) {
      for(int i = 0; i < pool->count; ++i) {
         u8 id = pool->sprite_ids[i];
         if (id < MAX_SPRITES) {
            DestroySprite(&gSprites[id]);
            pool->sprite_ids[i] = SPRITE_NONE;
         }
      }
      pool->sprite_ids   = NULL;
      pool->delete_flags = NULL;
      Free(pool->species_ids);
      pool->species_ids  = NULL;
   }
   pool->count = 0;
}

u16 FindPooledPokemonIconBySpecies(const struct PokemonIconPool* pool, PokemonSpeciesID species) {
   for(u8 i = 0; i < pool->count; ++i) {
      if (pool->species_ids[i] == species)
         return i;
   }
   return NO_POOLED_POKEMON_ICON;
}

static u16 FirstFreeIndex(const struct PokemonIconPool* pool) {
   for(u8 i = 0; i < pool->count; ++i) {
      if (pool->species_ids[i] == 0)
         return i;
   }
   return NO_POOLED_POKEMON_ICON;
}

extern u16 AddPooledPokemonIcon(struct PokemonIconPool* pool, PokemonSpeciesID species, u8 x, u8 y) {
   if (species == 0) {
      return NO_POOLED_POKEMON_ICON;
   }
   
   u16 i = FirstFreeIndex(pool);
   if (i == NO_POOLED_POKEMON_ICON) {
      DebugPrintf("failed to add pooled icon: species %u (pool is full)", species);
      return i;
   }
   
   u8 id = pool->sprite_ids[i];
   if (id != SPRITE_NONE) {
      DestroySprite(&gSprites[id]);
      pool->sprite_ids[i] = SPRITE_NONE;
   }
   
   LoadMonIconPalette(species);
   
   u8 sprite_id = pool->sprite_ids[i] = CreateMonIconNoPersonality(
      species,
      SpriteCallbackDummy,
      x, // x
      y, // y
      0, // subpriority
      TRUE // handleDeoxys
   );
   if (sprite_id >= MAX_SPRITES) {
      DebugPrintf("failed to add pooled icon: species %u (no sprite available)", species);
      pool->sprite_ids[i] = SPRITE_NONE;
      return NO_POOLED_POKEMON_ICON;
   }
   pool->species_ids[i] = species;
   UnmarkPooledPokemonIconForDelete(pool, i);
   
   DebugPrintf("added pooled icon: species %u at index %u, sprite ID %u", species, i, pool->sprite_ids[i]);
   return i;
}
extern void RemovePooledPokemonIcon(struct PokemonIconPool* pool, u8 i) {
   {
      u8 id = pool->sprite_ids[i];
      if (id < MAX_SPRITES) {
         DestroySprite(&gSprites[id]);
         pool->sprite_ids[i] = SPRITE_NONE;
      }
   }
   pool->species_ids[i] = 0;
   UnmarkPooledPokemonIconForDelete(pool, i);
}

void MarkAllPooledPokemonIconsForDelete(struct PokemonIconPool* pool) {
   DebugPrintf("marking all pooled icons for delete");
   u8 wholes = pool->count / 8;
   for(int i = 0; i < wholes; ++i) {
      pool->delete_flags[i] = 0xFF;
   }
   u8 remaining = pool->count % 8;
   if (remaining) {
      pool->delete_flags[wholes] = 0;
      for(int i = 0; i < remaining; ++i) {
         pool->delete_flags[wholes] |= (1 << i);
      }
   }
}
void MarkPooledPokemonIconForDelete(struct PokemonIconPool* pool, u8 i) {
   DebugPrintf("marking pooled icon for delete: %u", i);
   pool->delete_flags[i / 8] |= 1 << (i % 8);
}
void UnmarkPooledPokemonIconForDelete(struct PokemonIconPool* pool, u8 i) {
   DebugPrintf("unmarking pooled icon for delete: %u", i);
   pool->delete_flags[i / 8] &= ~(1 << (i % 8));
}
void DeleteMarkedPooledPokemonIcons(struct PokemonIconPool* pool) {
   DebugPrintf("deleting all marked-for-delete pooled icons...");
   for(int i = 0; i < pool->count; ++i) {
      bool8 flag = (pool->delete_flags[i / 8] & (1 << (i % 8))) != 0;
      if (!flag)
         continue;
      u8 id = pool->sprite_ids[i];
      DebugPrintf(" - deleting pool index %u sprite ID %u, species %u", i, id, pool->species_ids[i]);
      pool->species_ids[i] = 0;
      if (id < MAX_SPRITES) {
         DestroySprite(&gSprites[id]);
      }
      pool->sprite_ids[i] = SPRITE_NONE;
   }
   DebugPrintf("deleted all marked-for-delete pooled icons");
   memset(
      pool->delete_flags,
      0,
      ComputeDeleteFlagsBufferSize(pool->count)
   );
}

void SetPooledPokemonIconsVisible(struct PokemonIconPool* pool, bool8 visible) {
   bool8 invisible = !visible;
   for(int i = 0; i < pool->count; ++i) {
      u8 id = pool->sprite_ids[i];
      if (id < MAX_SPRITES)
         gSprites[id].invisible = invisible;
   }
}

void SetPooledPokemonIconPosition(struct PokemonIconPool* pool, u8 index, u8 x, u8 y) {
   u8 id = pool->sprite_ids[index];
   if (id >= MAX_SPRITES)
      return;
   gSprites[id].x = x;
   gSprites[id].y = y;
}