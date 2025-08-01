
// Must be done before bitpacking attributes on savedata fields are parsed.
// (This spares us the burden of processing those attributes for every single 
// translation unit when we only need them here.)
#pragma lu_bitpack enable
#include "./lu/save-codegen-options.h"

#include "global.h"
#include "agb_flash.h"
#include "gba/flash_internal.h"
#include "fieldmap.h"
#include "save.h"
#include "save_encryption.h"
#include "task.h"
#include "decompress.h"
#include "load_save.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "main.h"
#include "trainer_hill.h"
#include "link.h"
#include "constants/game_stat.h"

#include "lu/bitpack_transformed_type_BoxPokemon.h"
#include "lu/custom_game_options.h"

// Generate the code for bitpacked savedata.
#include "./lu/save-codegen.h"

STATIC_ASSERT(sizeof(struct SaveSector) <= SECTOR_SIZE, SaveSectorSizeAssertion);

static u16 CalculateChecksum(void*, u16);
static void ReadFlashSector(u8, struct SaveSector*);
static bool32 SetDamagedSectorBits(u8 op, u8 sectorId);
static u8 TryWriteSector(u8, u8*);
static u8 TryWriteSectorFooter(u8 sector_index, const struct SaveSector*);

static u8 GetSaveValidStatus();

static u8 TryLoadSaveSlot();
static u8 CopySaveSlotData();

static u8 ReadSectorWithSize(u8 sectorId, u8 *data, u16 size);

static u8 OnBeginFullSlotSave(bool8 isIncremental);
static u8 OnFailedFullSlotSave();

static u8 SerializeToSlotOwnedSector(u8 sectorId, bool8 eraseFlashFirst);
static u8 WriteSlot(bool8 cycleSectors, bool8 savePokemonStorage, bool8 eraseFlashFirst);
static u8 WriteSectorWithSize(u8 sectorId, u8* src, u16 size);
static void WriteHallOfFame();

// Divide save blocks into individual chunks to be written to flash sectors

/*
 * Sector Layout:
 *
 * Sectors 0 - 13:      Save Slot 1
 * Sectors 14 - 27:     Save Slot 2
 * Sectors 28 - 29:     Hall of Fame
 * Sector 30:           Trainer Hill
 * Sector 31:           Recorded Battle
 *
 * There are two save slots for saving the player's game data. We alternate between
 * them each time the game is saved, so that if the current save slot is corrupt,
 * we can load the previous one. We also rotate the sectors in each save slot
 * so that the same data is not always being written to the same sector. This
 * might be done to reduce wear on the flash memory, but I'm not sure, since all
 * 14 sectors get written anyway.
 *
 * See SECTOR_ID_* constants in save.h
 */

COMMON_DATA u16 gLastWrittenSector = 0;
COMMON_DATA u32 gLastSaveCounter = 0;
COMMON_DATA u16 gLastKnownGoodSector = 0;
COMMON_DATA u32 gDamagedSaveSectors = 0;
COMMON_DATA u32 gSaveCounter = 0;
COMMON_DATA struct SaveSector *gReadWriteSector = NULL; // Pointer to a buffer for reading/writing a sector
COMMON_DATA u16 gIncrementalSectorId = 0;
COMMON_DATA u16 gSaveUnusedVar = 0;
COMMON_DATA u16 gSaveFileStatus = 0;
COMMON_DATA void (*gGameContinueCallback)(void) = NULL;
COMMON_DATA struct SaveSectorLocation gRamSaveSectorLocations[NUM_SECTORS_PER_SLOT] = {0};
COMMON_DATA u16 gSaveUnusedVar2 = 0;
COMMON_DATA u16 gSaveAttemptStatus = 0;

EWRAM_DATA struct SaveSector gSaveDataBuffer = {0}; // Buffer used for reading/writing sectors
EWRAM_DATA static u8 sUnusedVar = 0;


static u16 CalculateChecksum(void* data, u16 size) {
   u16 i;
   u32 checksum = 0;

   for (i = 0; i < (size / 4); i++) {
      checksum += *((u32*)data);
      data     += sizeof(u32);
   }

   return ((checksum >> 16) + checksum);
}
static void ReadFlashSector(u8 sectorId, struct SaveSector* dst) {
   ReadFlash(sectorId, 0, dst->data, SECTOR_SIZE);
}

static bool32 SetDamagedSectorBits(u8 op, u8 sectorId) {
   bool32 retVal = FALSE;

   switch (op) {
      case ENABLE:
         #ifndef NDEBUG
         if (!(gDamagedSaveSectors & (1 << sectorId))) {
            DebugPrintf("[Savedata] Sector ID %u tests as damaged!", sectorId);
         }
         #endif
         gDamagedSaveSectors |= (1 << sectorId);
         break;
      case DISABLE:
         gDamagedSaveSectors &= ~(1 << sectorId);
         break;
      case CHECK: // unused
         if (gDamagedSaveSectors & (1 << sectorId))
            retVal = TRUE;
         break;
   }

   return retVal;
}

static u8 TryWriteSector(u8 sector, u8 *data) {
   u32 result = ProgramFlashSectorAndVerify(sector, data);
   if (result) { // is damaged?
      // Failed
      SetDamagedSectorBits(ENABLE, sector);
      #ifndef NDEBUG
         DebugPrintf("[Savedata] Damage detected at address %08X while writing sector data.", result);
      #endif
      return SAVE_STATUS_ERROR;
   } else {
      // Succeeded
      SetDamagedSectorBits(DISABLE, sector);
      return SAVE_STATUS_OK;
   }
}
static u8 TryWriteSectorFooter(u8 sector_index, const struct SaveSector* sector_data) {
   u8  status;
   u16 i;
   
   status = SAVE_STATUS_OK;
   for (i = 0; i < SECTOR_FOOTER_SIZE; i++) {
      if (ProgramFlashByte(sector_index, SECTOR_DATA_SIZE + i, ((u8*)sector_data)[SECTOR_DATA_SIZE + i])) {
         status = SAVE_STATUS_ERROR;
         break;
      }
   }

   if (status == SAVE_STATUS_ERROR) {
      // Writing signature/counter failed
      SetDamagedSectorBits(ENABLE, sector_index);
      #ifndef NDEBUG
         DebugPrintf("[Savedata] Damage detected while writing the sector footer (byte %u).", i);
      #endif
      return SAVE_STATUS_ERROR;
   }
   
   // Succeeded
   SetDamagedSectorBits(DISABLE, sector_index);
   return SAVE_STATUS_OK;
}


// This is not actually part of the save process. Rather, it reads directly into flash 
// memory and pulls out SaveBlock2 in order to read the trainer ID. This is fed into 
// Game Freak's ASLR code as part of the offset it shifts data by. The reason this has 
// to read into flash memory (i.e. the reason it exists at all) is because it's run 
// before you actually load your save file.
u8 ExtractRawSaveBlockByte(void* buffer, u32 offset) {
   struct lu_BitstreamState bs_state;
   lu_BitstreamInitialize(&bs_state, buffer);
   bs_state.target += offset / 8;
   bs_state.shift   = offset % 8;
   return lu_BitstreamRead_u8(&bs_state, 6);
}
u16 GetSaveBlocksPointersBaseOffset(void) {
   u16 i, slotOffset;
   u16 summed_id;
   u8  found_id_parts;
   struct SaveSector* sector;
   
   #pragma lu_bitpack serialized_sector_id_to_constant sector_of_trainer_id_0 gSaveBlock2Ptr->playerTrainerId[0]
   #pragma lu_bitpack serialized_offset_to_constant    offset_of_trainer_id_0 gSaveBlock2Ptr->playerTrainerId[0]
   #pragma lu_bitpack serialized_sector_id_to_constant sector_of_trainer_id_1 gSaveBlock2Ptr->playerTrainerId[1]
   #pragma lu_bitpack serialized_offset_to_constant    offset_of_trainer_id_1 gSaveBlock2Ptr->playerTrainerId[1]
   #pragma lu_bitpack serialized_sector_id_to_constant sector_of_trainer_id_2 gSaveBlock2Ptr->playerTrainerId[2]
   #pragma lu_bitpack serialized_offset_to_constant    offset_of_trainer_id_2 gSaveBlock2Ptr->playerTrainerId[2]
   #pragma lu_bitpack serialized_sector_id_to_constant sector_of_trainer_id_3 gSaveBlock2Ptr->playerTrainerId[3]
   #pragma lu_bitpack serialized_offset_to_constant    offset_of_trainer_id_3 gSaveBlock2Ptr->playerTrainerId[3]

   sector = gReadWriteSector = &gSaveDataBuffer;
   if (gFlashMemoryPresent != TRUE)
      return 0;
   {
      u8 status = GetSaveValidStatus();
      if (status == SAVE_STATUS_EMPTY || status == SAVE_STATUS_CORRUPT)
         return 0;
   }
   slotOffset     = NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);
   summed_id      = 0;
   found_id_parts = 0;
   for (i = 0; i < NUM_SECTORS_PER_SLOT; i++) {
      ReadFlashSector(i + slotOffset, gReadWriteSector);
      
      if (gReadWriteSector->id == sector_of_trainer_id_0) {
         found_id_parts |= (1 << 0);
         summed_id += ExtractRawSaveBlockByte(sector->data, offset_of_trainer_id_0);
      } else if (gReadWriteSector->id == sector_of_trainer_id_1) {
         found_id_parts |= (1 << 1);
         summed_id += ExtractRawSaveBlockByte(sector->data, offset_of_trainer_id_1);
      } else if (gReadWriteSector->id == sector_of_trainer_id_2) {
         found_id_parts |= (1 << 2);
         summed_id += ExtractRawSaveBlockByte(sector->data, offset_of_trainer_id_2);
      } else if (gReadWriteSector->id == sector_of_trainer_id_3) {
         found_id_parts |= (1 << 3);
         summed_id += ExtractRawSaveBlockByte(sector->data, offset_of_trainer_id_3);
      } else {
         continue;
      }
      if (found_id_parts == 15)
         return summed_id;
   }
   return 0;
}




static u8 GetSaveValidStatus() {
   u8  slot_idx;
   u16 sector_idx;
   
   u8  slot_statuses[NUM_SAVE_SLOTS];
   u32 slot_counters[NUM_SAVE_SLOTS];
   
   for(slot_idx = 0; slot_idx < NUM_SAVE_SLOTS; ++slot_idx) {
      bool8 any_valid_signature = FALSE;
      u32   valid_sector_flags  = 0;
      
      for(sector_idx = 0; sector_idx < NUM_SECTORS_PER_SLOT; ++sector_idx) {
         ReadFlashSector(sector_idx + (slot_idx * NUM_SECTORS_PER_SLOT), gReadWriteSector);
         if (gReadWriteSector->signature == SECTOR_SIGNATURE) {
            u16 checksum;
            
            any_valid_signature = TRUE;
            checksum            = CalculateChecksum(gReadWriteSector->data, SECTOR_DATA_SIZE); // TODO: Could optimize by using only the bytespan of the serialized/bitpacked data.
            if (gReadWriteSector->checksum == checksum) {
               slot_counters[slot_idx] = gReadWriteSector->counter;
               #ifndef NDEBUG
                  if (valid_sector_flags & (1 << gReadWriteSector->id)) {
                     DebugPrintf("[Savedata][Validating] Slot %u sector ID %u was seen twice! Last seen at index %u.", slot_idx, gReadWriteSector->id, sector_idx);
                  }
               #endif
               valid_sector_flags |= 1 << gReadWriteSector->id;
            } else {
               #ifndef NDEBUG
                  DebugPrintf("[Savedata][Validating] Slot %u sector %u (ID %u) failed validation: checksum %04X differed from expected %04X.", slot_idx, sector_idx, gReadWriteSector->id, gReadWriteSector->checksum, checksum);
               #endif
            }
         }
      }
      if (any_valid_signature) {
         if (valid_sector_flags == (1 << NUM_SECTORS_PER_SLOT) - 1) // all flags set?
            slot_statuses[slot_idx] = SAVE_STATUS_OK;
         else {
            #ifndef NDEBUG
               DebugPrintf("[Savedata][Validating] Slot %u failed validation: the valid sector mask was %08X. If no sectors failed the checksum check, then perhaps a sector is missing?", slot_idx, valid_sector_flags);
            #endif
            slot_statuses[slot_idx] = SAVE_STATUS_ERROR;
         }
      } else {
         // No sectors in this slot have the correct signature; treat it as empty.
         slot_statuses[slot_idx] = SAVE_STATUS_EMPTY;
      }
   }
   
   if (slot_statuses[0] == SAVE_STATUS_OK && slot_statuses[1] == SAVE_STATUS_OK) {
      if (
         (slot_counters[0] == -1 && slot_counters[1] ==  0)
      || (slot_counters[0] ==  0 && slot_counters[1] == -1)
      ) {
         if ((unsigned)(slot_counters[0] + 1) < (unsigned)(slot_counters[1] + 1))
            gSaveCounter = slot_counters[1];
         else
            gSaveCounter = slot_counters[0];
      } else {
         if (slot_counters[0] < slot_counters[1])
            gSaveCounter = slot_counters[1];
         else
            gSaveCounter = slot_counters[0];
      }
      return SAVE_STATUS_OK;
   }
   
   //
   // One or both save slots are not OK.
   //
   
   {
      u8    i;
      bool8 all_empty;
      
      all_empty = TRUE;
      for(i = 0; i < NUM_SAVE_SLOTS; ++i) {
         all_empty &= (slot_statuses[i] == SAVE_STATUS_EMPTY);
         
         if (slot_statuses[i] == SAVE_STATUS_OK) {
            gSaveCounter = slot_counters[i];
            
            if (slot_statuses[i ? 0 : 1] == SAVE_STATUS_ERROR)
               return SAVE_STATUS_ERROR; // Other slot errored.
            return SAVE_STATUS_OK; // This slot is OK; other slot is empty.
         }
      }
      
      if (all_empty) {
         gSaveCounter       = 0;
         gLastWrittenSector = 0;
         return SAVE_STATUS_EMPTY;
      }
   }

   // Both slots errored.
   gSaveCounter       = 0;
   gLastWrittenSector = 0;
   return SAVE_STATUS_CORRUPT;
}

u8 LoadGameSave(u8 saveType) {
   u8 status;

   if (gFlashMemoryPresent != TRUE) {
      DebugPrintf("[Savedata] Requested savegame load, but there's no flash memory!", 0);
      gSaveFileStatus = SAVE_STATUS_NO_FLASH;
      return SAVE_STATUS_ERROR;
   }

   switch (saveType) {
      case SAVE_NORMAL:
      default:
         DebugPrintf("[Savedata] Performing full savegame load...", 0);
         status = TryLoadSaveSlot();
         CopyPartyAndObjectsFromSave();
         gSaveFileStatus = status;
         gGameContinueCallback = 0;
         DebugPrintf("[Savedata] Performed full savegame load.", 0);
         break;
      case SAVE_HALL_OF_FAME:
         status = ReadSectorWithSize(SECTOR_ID_HOF_1, gDecompressionBuffer, SECTOR_DATA_SIZE);
         if (status == SAVE_STATUS_OK) {
            status = ReadSectorWithSize(SECTOR_ID_HOF_2, &gDecompressionBuffer[SECTOR_DATA_SIZE], SECTOR_DATA_SIZE);
         }
         break;
   }

   return status;
}
static u8 TryLoadSaveSlot() {
   u8 status;
   
   gReadWriteSector = &gSaveDataBuffer;
   status = GetSaveValidStatus();
   #ifndef NDEBUG
      if (status == SAVE_STATUS_ERROR) {
         DebugPrintf("[Savedata] One of the two save slots is corrupt.");
      } else if (status == SAVE_STATUS_CORRUPT) {
         DebugPrintf("[Savedata] Both save slots (main and backup) are corrupt.");
      }
   #endif
   CopySaveSlotData();
   
   return status;
}
static u8 CopySaveSlotData() {
   u16 i;
   u16 checksum;
   u16 slotOffset = NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);
   u16 id;

   for (i = 0; i < NUM_SECTORS_PER_SLOT; i++) {
      ReadFlashSector(i + slotOffset, gReadWriteSector);

      id = gReadWriteSector->id;
      if (id == 0)
         gLastWrittenSector = i;

      checksum = CalculateChecksum(gReadWriteSector->data, SECTOR_DATA_SIZE); // TODO: Could optimize by using only the bytespan of the serialized/bitpacked data.

      // Only copy data for sectors whose signature and checksum fields are correct
      if (gReadWriteSector->signature == SECTOR_SIGNATURE && gReadWriteSector->checksum == checksum) {
         if (id < __lu_bitpack_sector_count) {
            ReadSavegameSector(gReadWriteSector->data, id);
         }
      }
   }
   
   // encrypt post-load:
   EncryptForSave();

   return SAVE_STATUS_OK;
}

// Used for data not considered part of one of the normal save slots, i.e. the Hall of Fame, Trainer Hill, or Recorded Battle
static u8 ReadSectorWithSize(u8 sectorId, u8* dst, u16 size) {
   u16 i;
   
   struct SaveSector* sector = &gSaveDataBuffer;
   ReadFlashSector(sectorId, sector);
   if (sector->signature == SECTOR_SIGNATURE) {
      u16 checksum = CalculateChecksum(sector->data, size);
      if (sector->id == checksum) {
         // Signature and checksum are correct, copy data
         for (i = 0; i < size; i++)
            dst[i] = sector->data[i];
         return SAVE_STATUS_OK;
      } else {
         // Incorrect checksum
         return SAVE_STATUS_CORRUPT;
      }
   } else {
        // Incorrect signature value
        return SAVE_STATUS_EMPTY;
   }
}

// Trainer Hill or Recorded Battle
u32 TryReadSpecialSaveSector(u8 sector, u8 *dst) {
   s32 i;
   s32 size;
   u8* savData;

   if (sector != SECTOR_ID_TRAINER_HILL && sector != SECTOR_ID_RECORDED_BATTLE)
      return SAVE_STATUS_ERROR;

   ReadFlash(sector, 0, (u8*)&gSaveDataBuffer, SECTOR_SIZE);
   if (*(u32*)(&gSaveDataBuffer.data[0]) != SPECIAL_SECTOR_SENTINEL)
      return SAVE_STATUS_ERROR;

   // Copies whole save sector except u32 counter
   i       = 0;
   size    = SECTOR_COUNTER_OFFSET - 1;
   savData = &gSaveDataBuffer.data[4]; // data[4] to skip past SPECIAL_SECTOR_SENTINEL
   for (; i <= size; i++)
      dst[i] = savData[i];
   
   return SAVE_STATUS_OK;
}



//
// WRITING
//



static u8 OnBeginFullSlotSave(bool8 isIncremental) {
   gLastKnownGoodSector = gLastWrittenSector; // backup the current written sector before attempting to write.
   gLastSaveCounter     = gSaveCounter;
   gLastWrittenSector++;
   gLastWrittenSector = gLastWrittenSector % NUM_SECTORS_PER_SLOT;
   gSaveCounter++;
   if (isIncremental) {
      gIncrementalSectorId = 0;
   }
}
static u8 OnFailedFullSlotSave() {
   gLastWrittenSector = gLastKnownGoodSector;
   gSaveCounter       = gLastSaveCounter;
}

static u8 SerializeToSlotOwnedSector(u8 sectorId, bool8 eraseFlashFirst) {
   u16 sectorIndex;
   u8  status;
   
   // Adjust sector id for current save slot
   sectorIndex = sectorId + gLastWrittenSector;
   sectorIndex %= NUM_SECTORS_PER_SLOT;
   sectorIndex += NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);
   #ifndef NDEBUG
      DebugPrintf("[Savedata] Saving sector ID %u to sector index %u...", sectorId, sectorIndex);
   #endif

   // Clear temp save sector
   {
      u16 i;
      for (i = 0; i < SECTOR_SIZE; i++)
         ((u8*)gReadWriteSector)[i] = 0;
   }

   // Set footer data
   gReadWriteSector->saveVersion = SAVEDATA_SERIALIZATION_VERSION;
   gReadWriteSector->id          = sectorId;
   gReadWriteSector->signature   = SECTOR_SIGNATURE;
   gReadWriteSector->counter     = gSaveCounter;

   // Write current data to temp buffer for writing
   if (sectorId < __lu_bitpack_sector_count) {
      WriteSavegameSector(gReadWriteSector->data, sectorId);
      #ifndef NDEBUG
         if (gReadWriteSector->signature != SECTOR_SIGNATURE) {
            DebugPrintf("[Savedata] Failed to save sector ID %u to sector index %u: something went wrong with the bitpacking code generation, and we ended up overrunning the available space and corrupting the footer!", sectorId, sectorIndex);
         }
      #endif
   }

   gReadWriteSector->checksum = CalculateChecksum(gReadWriteSector->data, SECTOR_DATA_SIZE); // TODO: Could optimize by using only the bytespan of the serialized/bitpacked data.
   
   //
   // Commit the data to flash memory.
   //
   if (eraseFlashFirst) {
      EraseFlashSector(sectorIndex);
   }
   status = TryWriteSector(sectorIndex, gReadWriteSector->data);
   if (status == SAVE_STATUS_ERROR) {
      #ifndef NDEBUG
         DebugPrintf("[Savedata] Failed to save sector ID %u to sector index %u!", sectorId, sectorIndex);
      #endif
      return status;
   }
   return TryWriteSectorFooter(sectorIndex, gReadWriteSector);
}

static u8 WriteSlot(bool8 cycleSectors, bool8 savePokemonStorage, bool8 eraseFlashFirst) {
   u8  i;
   u32 status;
   u32 encryptionKeyBackup;
   
   DebugPrintf("[Savedata] Performing full save...", 0);
   
   if (cycleSectors) {
      OnBeginFullSlotSave(FALSE);
   }
   status = SAVE_STATUS_OK;
   
   // decrypt pre-save:
   DecryptForSave();
   
   if (savePokemonStorage) {
      // SaveBlock1, SaveBlock2, and PokemonStorage
      int slot_status;
      
      for (i = 0; i < NUM_SECTORS_PER_SLOT; i++) {
         slot_status = SerializeToSlotOwnedSector(i, eraseFlashFirst);
         if (slot_status == SAVE_STATUS_ERROR)
            status = SAVE_STATUS_ERROR;
      }
      
      #ifndef NDEBUG
      if (status == SAVE_STATUS_ERROR) {
         DebugPrintf("[Savedata] Full save done, but a slot failed to save properly.");
      } else {
         DebugPrintf("[Savedata] Full save done. No errors detected yet (but we still need to check whether flash memory has an apparent issue).");
      }
      #endif
   } else {
      #pragma lu_bitpack serialized_sector_id_to_constant sector_start_pc (*gPokemonStoragePtr)
      // SaveBlock1, SaveBlock2
      for (i = 0; i < sector_start_pc; i++) {
         int slot_status = SerializeToSlotOwnedSector(i, eraseFlashFirst);
         if (slot_status == SAVE_STATUS_ERROR)
            status = SAVE_STATUS_ERROR;
      }
      
      #ifndef NDEBUG
      if (status == SAVE_STATUS_ERROR) {
         DebugPrintf("[Savedata] Partial save (no PC; sectors [%u, %u]) done, but a slot failed to save properly.", 0, sector_start_pc);
      } else {
         DebugPrintf("[Savedata] Partial save (no PC; sectors [%u, %u]) done. No errors detected yet (but we still need to check whether flash memory has an apparent issue).", 0, sector_start_pc);
      }
      #endif
   }
   
   // re-encrypt post-save:
   EncryptForSave();
   
   if (gDamagedSaveSectors) {
      if (cycleSectors) {
         // Revert cycling.
         OnFailedFullSlotSave();
      }
      status = SAVE_STATUS_ERROR;
   }
   
   return status;
}
static u8 WriteSectorWithSize(u8 sectorIndex, u8* src, u16 size) { // should only be used for non-slot sectors
   u16 i;
   u8  status;
   struct SaveSector* sector = &gSaveDataBuffer;

   // Clear temp save sector
   for (i = 0; i < SECTOR_SIZE; i++)
      ((u8*)sector)[i] = 0;

   sector->signature = SECTOR_SIGNATURE;

   // Copy data to temp buffer for writing
   for (i = 0; i < size; i++)
      sector->data[i] = src[i];

   // though this appears to be incorrect, it might be some sector checksum instead of a whole save checksum and only appears to be relevent to HOF data, if used.
   sector->id = CalculateChecksum(src, size);
   
   status = TryWriteSector(sectorIndex, sector->data);
   if (status == SAVE_STATUS_ERROR) {
      return status;
   }
   return TryWriteSectorFooter(sectorIndex, sector);
}
static void WriteHallOfFame() {
   u8* tempAddr = gDecompressionBuffer;
   WriteSectorWithSize(SECTOR_ID_HOF_1, tempAddr,                    SECTOR_DATA_SIZE);
   WriteSectorWithSize(SECTOR_ID_HOF_2, tempAddr + SECTOR_DATA_SIZE, SECTOR_DATA_SIZE);
}


u8 TrySavingData(u8 saveType) {
   DebugPrintf("[Savedata] Received request to save... Type is %d.", saveType);
   if (gFlashMemoryPresent != TRUE) {
      gSaveAttemptStatus = SAVE_STATUS_ERROR;
      return SAVE_STATUS_ERROR;
   }

   HandleSavingData(saveType);
   if (!gDamagedSaveSectors) {
      DebugPrintf("[Savedata] Request to save was successful (hopefully)!", 0);
      gSaveAttemptStatus = SAVE_STATUS_OK;
      return SAVE_STATUS_OK;
   } else {
      DebugPrintf("[Savedata] Request to save failed!", 0);
      DoSaveFailedScreen(saveType);
      gSaveAttemptStatus = SAVE_STATUS_ERROR;
      return SAVE_STATUS_ERROR;
   }
}
u8 HandleSavingData(u8 saveType) {
   u8   i;
   u32* backupVar = gTrainerHillVBlankCounter;

   gTrainerHillVBlankCounter = NULL;
   
   switch (saveType) {
      case SAVE_HALL_OF_FAME:
         if (GetGameStat(GAME_STAT_ENTERED_HOF) < 999)
            IncrementGameStat(GAME_STAT_ENTERED_HOF);

         // Write the full save slot first
         CopyPartyAndObjectsToSave();
         WriteSlot(TRUE, TRUE, FALSE);

         // Save the Hall of Fame
         WriteHallOfFame();
         break;
         
      case SAVE_OVERWRITE_DIFFERENT_FILE:
         // Erase non-slot sectors (Hall of Fame; Trainer Hill; Recorded Battle)
         for (i = SECTOR_ID_HOF_1; i < SECTORS_COUNT; i++) {
            EraseFlashSector(i);
         }
         //
         // fallthrough
         //
      case SAVE_NORMAL:
      default:
         CopyPartyAndObjectsToSave();
         WriteSlot(TRUE, TRUE, FALSE);
         break;
         
      case SAVE_LINK:
      case SAVE_EREADER: // Dummied, now duplicate of SAVE_LINK
         // Used by link / Battle Frontier
         // Write only SaveBlocks 1 and 2 (skips the PC)
         CopyPartyAndObjectsToSave();
         WriteSlot(FALSE, FALSE, TRUE);
         break;
   }
   
   gTrainerHillVBlankCounter = backupVar;
   
   return 0;
}

// Trainer Hill or Recorded Battle
// This is almost the same as our `WriteSectorWithSize` save for the presence of a sentinel u32 placed 
// inside the sector before the data to be written.
u32 TryWriteSpecialSaveSector(u8 sector, u8* src) {
   s32   i;
   s32   size;
   u8*   savData;
   void* savDataBuffer;

   if (sector != SECTOR_ID_TRAINER_HILL && sector != SECTOR_ID_RECORDED_BATTLE)
      return SAVE_STATUS_ERROR;

   savDataBuffer = &gSaveDataBuffer;
   *(u32*)(savDataBuffer) = SPECIAL_SECTOR_SENTINEL;

   // Copies whole save sector except u32 counter
   i    = 0;
   size = SECTOR_COUNTER_OFFSET - 1;
   savData = &gSaveDataBuffer.data[4]; // data[4] to skip past SPECIAL_SECTOR_SENTINEL
   for (; i <= size; i++)
      savData[i] = src[i];
   
   if (ProgramFlashSectorAndVerify(sector, savDataBuffer) != 0)
      return SAVE_STATUS_ERROR;
   return SAVE_STATUS_OK;
}

// TODO: Rename to DoLinkIncrementalPartialSave_PartA
u8 WriteSaveBlock2(void) {
   if (gFlashMemoryPresent != TRUE)
      return TRUE;

   CopyPartyAndObjectsToSave();
   {  // This is the same as `OnBeginFullSlotSave`, except we don't cycle to the next slot.
      gReadWriteSector     = &gSaveDataBuffer;
      gLastKnownGoodSector = gLastWrittenSector;
      gLastSaveCounter     = gSaveCounter;
      gIncrementalSectorId = 0;
      gDamagedSaveSectors  = 0;
   }
   
   {
      #pragma lu_bitpack serialized_sector_id_to_constant start_of_sb1 (*gSaveBlock1Ptr)
      int i = 0;
      for(; i < start_of_sb1; ++i) {
         SerializeToSlotOwnedSector(i, TRUE);
      }
      gIncrementalSectorId = i;
   }
   if (gDamagedSaveSectors) {
      // This is the same as `OnFailedFullSlotSave`.
      gLastWrittenSector = gLastKnownGoodSector;
      gSaveCounter       = gLastSaveCounter;
   }
   
   return FALSE;
}

// TODO: Rename to DoLinkIncrementalPartialSave_PartB
//
// Used in conjunction with WriteSaveBlock2 to write both for certain link saves.
// This will be called repeatedly in a task, writing each sector of SaveBlock1 incrementally.
// It returns TRUE when finished.
bool8 WriteSaveBlock1Sector(void) {
   #pragma lu_bitpack serialized_sector_id_to_constant start_of_pss (*gPokemonStoragePtr)
   
   u8  finished = FALSE;
   u16 sectorId = ++gIncrementalSectorId; // Because WriteSaveBlock2 will have been called prior, this will be SECTOR_ID_SAVEBLOCK1_START
   if (sectorId < start_of_pss) {
      SerializeToSlotOwnedSector(sectorId, TRUE);
   } else {
      finished = TRUE;
   }

   if (gDamagedSaveSectors) {
      DoSaveFailedScreen(SAVE_LINK);
   }

   return finished;
}

#define tState         data[0]
#define tTimer         data[1]
#define tInBattleTower data[2]
//
// Note that this is very different from TrySavingData(SAVE_LINK).
// Most notably it does save the PC data.
//
void Task_LinkFullSave(u8 taskId) {
   s16 *data = gTasks[taskId].data;

   switch (tState) {
      case 0:
         gSoftResetDisabled = TRUE;
         tState = 1;
         break;
      case 1:
         SetLinkStandbyCallback();
         tState = 2;
         break;
      case 2:
         if (IsLinkTaskFinished()) {
            if (!tInBattleTower)
                SaveMapView();
            tState = 3;
         }
         break;
      case 3:
         if (!tInBattleTower)
            SetContinueGameWarpStatusToDynamicWarp();
         LinkFullSave_Init();
         tState = 4;
         break;
      case 4: // 5-frame wait.
         if (++tTimer == 5) {
            tTimer = 0;
            tState = 5;
         }
         break;
      case 5:
         if (LinkFullSave_WriteSector())
            tState = 6;
         else
            tState = 4; // Not finished, delay again
         break;
      case 6:
         LinkFullSave_ReplaceLastSector();
         tState = 7;
         break;
      case 7:
         if (!tInBattleTower)
            ClearContinueGameWarpStatus2();
         SetLinkStandbyCallback();
         tState = 8;
         break;
      case 8:
         if (IsLinkTaskFinished()) {
            LinkFullSave_SetLastSectorSignature();
            tState = 9;
         }
         break;
      case 9:
         SetLinkStandbyCallback();
         tState = 10;
         break;
      case 10:
         if (IsLinkTaskFinished())
            tState++;
         break;
      case 11:
         if (++tTimer > 5) {
            gSoftResetDisabled = FALSE;
            DestroyTask(taskId);
         }
         break;
   }
}
//
#undef tState
#undef tTimer
#undef tInBattleTower

// Code for incremental saves during a link.
//
// Not sure why this exists... Maybe to prevent the game from hanging during 
// a link save? Does the game do *that* much extra processing when linked?
//
// What's especially vile is that outside code has direct access to the save 
// process and its innards. There isn't just an "init" call and a "do next" 
// call; outside code calls into functions that manage sector signatures and 
// similar.
//
// The typical flow is:
//  - Init
//  - Write sectors in loop
//  - Replace last sector
//  - Make savegame edits
//  - Wait for link to end
//  - Set last sector signature
//
// In most cases, this is called exclusively through Task_LinkFullSave, but 
// there's one exception: trading, which has its own "link full save" task 
// that also calls these functions directly.

bool8 LinkFullSave_Init(void) {
   if (gFlashMemoryPresent != TRUE)
      return TRUE;
   CopyPartyAndObjectsToSave();
   OnBeginFullSlotSave(TRUE);
   return FALSE;
}
bool8 LinkFullSave_WriteSector(void) { // incremental save step; return TRUE on complete or error; FALSE if not yet done
   // TODO: Investigate renaming this to `LinkFullSave_DoNext`.
   u8 status;
   if (gIncrementalSectorId < NUM_SECTORS_PER_SLOT - 1) {
      status = SAVE_STATUS_OK;
      
      SerializeToSlotOwnedSector(gIncrementalSectorId, FALSE);
      gIncrementalSectorId++;
      if (gDamagedSaveSectors) {
         status = SAVE_STATUS_ERROR;
         OnFailedFullSlotSave();
      }
   } else {
      // Exceeded max sector, finished
      status = SAVE_STATUS_ERROR;
   }
   if (gDamagedSaveSectors)
      DoSaveFailedScreen(SAVE_NORMAL);

   // In this case "error" either means that an actual error was encountered
   // or that the given max sector has been reached (meaning it has finished successfully).
   // If there was an actual error the save failed screen above will also be shown.
   if (status == SAVE_STATUS_ERROR)
      return TRUE;
   else
      return FALSE;
}
bool8 LinkFullSave_ReplaceLastSector(void) {
   // TODO: Investigate renaming this to `LinkFullSave_Finalize`.
   // Alternatively, consider integrating this into the "do next" function, since the game is 
   // willing to pop a save-failed screen at any point in the process.
   
   //HandleReplaceSectorAndVerify(NUM_SECTORS_PER_SLOT, gRamSaveSectorLocations);
   //
   // I'm honestly not sure this was ever needed. I think what happened was that Game Freak's 
   // original `LinkFullSave_WriteSector` used `HandleWriteIncrementalSector`, which in turn 
   // used `HandleWriteSetor`, which did a full sector write but didn't call Game Freak's 
   // equivalent to our `OnFailedFullSlotSave`. Then, they used `HandleReplaceSectorAndVerify` 
   // to redundantly re-write the first sector, because that one *does* call Game Freak's 
   // equivalent to our `OnFailedFullSlotSave` at the end.
   
   if (gDamagedSaveSectors)
      DoSaveFailedScreen(SAVE_NORMAL);
   return FALSE;
}

bool8 LinkFullSave_SetLastSectorSignature(void) {
   // TODO: Investigate renaming this to `LinkFullSave_DoPostSaveBehaviors`.
   
   //CopySectorSignatureByte(NUM_SECTORS_PER_SLOT, gRamSaveSectorLocations);
   //
   // Should hopefully no longer be needed given our changes to the save process.
    
   if (gDamagedSaveSectors)
      DoSaveFailedScreen(SAVE_NORMAL);
   return FALSE;
}

// End of link incremental save code.






void ClearSaveData(void) {
   u16 i;

   // Clear the full save two sectors at a time
   for (i = 0; i < SECTORS_COUNT / 2; i++) {
      EraseFlashSector(i);
      EraseFlashSector(i + SECTORS_COUNT / 2);
   }
}
void Save_ResetSaveCounters(void) {
   gSaveCounter        = 0;
   gLastWrittenSector  = 0;
   gDamagedSaveSectors = 0;
}
