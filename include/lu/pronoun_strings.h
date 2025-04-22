#ifndef GUARD_LU_PRONOUNS_H
#define GUARD_LU_PRONOUNS_H

#include "gba/types.h"

#define CAPITALIZE_PRONOUN_FORM 0x80

enum PronounType {
   PRONOUN_TYPE_EPICENE,   // they
   PRONOUN_TYPE_FEMININE,  // she
   PRONOUN_TYPE_MASCULINE, // he
   PRONOUN_TYPE_NEUTER,    // it
};
enum PronounForm {
   // These are also set in charmap.txt. Keep them in synch.
   PRONOUN_FORM_SUBJECT,
   PRONOUN_FORM_OBJECT,
   PRONOUN_FORM_POSSESSIVE,
   PRONOUN_FORM_POSSESSIVE_INDEPENDENT,
   PRONOUN_FORM_REFLEXIVE,
   PRONOUN_FORM_SUBJECT_IS,
   
   NUM_PRONOUN_FORMS,
   
   PRONOUN_FORM_CAPITALIZED_SUBJECT = PRONOUN_FORM_SUBJECT | CAPITALIZE_PRONOUN_FORM,
   PRONOUN_FORM_CAPITALIZED_OBJECT,
   PRONOUN_FORM_CAPITALIZED_POSSESSIVE,
   PRONOUN_FORM_CAPITALIZED_POSSESSIVE_INDEPENDENT,
   PRONOUN_FORM_CAPITALIZED_REFLEXIVE,
   PRONOUN_FORM_CAPITALIZED_SUBJECT_IS,
};

extern const u8* GetPronounString(enum PronounType, enum PronounForm);
extern const u8* GetPokemonGenderPronounString(u8 monGender, enum PronounForm);

extern enum PronounType PokemonGenderToPronounType(u8 monGender);

inline bool8 IsValidPronounForm(enum PronounForm form) {
   return (form & ~CAPITALIZE_PRONOUN_FORM) < NUM_PRONOUN_FORMS;
}

#endif