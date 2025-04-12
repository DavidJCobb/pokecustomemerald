#ifndef GUARD_LU_PICK_SPECIES_MENU
#define GUARD_LU_PICK_SPECIES_MENU

enum PickSpeciesMenu_ZeroType {
   PICKSPECIESMENU_ZEROTYPE_DISALLOWED,
   PICKSPECIESMENU_ZEROTYPE_DEFAULT,
   PICKSPECIESMENU_ZEROTYPE_NONE,
};

struct PickSpeciesMenuParams {
   void (*callback)(u16);
   const u8* header_text;
   enum PickSpeciesMenu_ZeroType zero_type;
};

#define PICKSPECIESMENU_RESULT_CANCELED 0xFFFF

extern void ShowPickSpeciesMenu(const struct PickSpeciesMenuParams*);

#endif