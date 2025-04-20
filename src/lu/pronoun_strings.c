#include "lu/pronoun_strings.h"

#define MAKE_PRONOUN_LIST(type) \
   static const u8* const sPronouns_##type[] = { \
      sPronounString_##type##_Subject, \
      sPronounString_##type##_Object, \
      sPronounString_##type##_Possessive, \
      sPronounString_##type##_PossessiveInd, \
      sPronounString_##type##_Reflexive, \
      sPronounString_##type##_SubjectIs, \
   };

typedef const u8* const * pronoun_list_type;

static const u8 sPronounString_Masc_Subject[]       = _("he");
static const u8 sPronounString_Masc_Object[]        = _("him");
static const u8 sPronounString_Masc_Possessive[]    = _("his");
static const u8 sPronounString_Masc_PossessiveInd[] = _("his");
static const u8 sPronounString_Masc_Reflexive[]     = _("himself");
static const u8 sPronounString_Masc_SubjectIs[]     = _("he's");
MAKE_PRONOUN_LIST(Masc)

static const u8 sPronounString_MascCap_Subject[]       = _("He");
static const u8 sPronounString_MascCap_Object[]        = _("Him");
static const u8 sPronounString_MascCap_Possessive[]    = _("His");
static const u8 sPronounString_MascCap_PossessiveInd[] = _("His");
static const u8 sPronounString_MascCap_Reflexive[]     = _("Himself");
static const u8 sPronounString_MascCap_SubjectIs[]     = _("He's");
MAKE_PRONOUN_LIST(MascCap)

static const u8 sPronounString_Fem_Subject[]       = _("she");
static const u8 sPronounString_Fem_Object[]        = _("her");
static const u8 sPronounString_Fem_Possessive[]    = _("her");
static const u8 sPronounString_Fem_PossessiveInd[] = _("hers");
static const u8 sPronounString_Fem_Reflexive[]     = _("herself");
static const u8 sPronounString_Fem_SubjectIs[]     = _("she's");
MAKE_PRONOUN_LIST(Fem)

static const u8 sPronounString_FemCap_Subject[]       = _("She");
static const u8 sPronounString_FemCap_Object[]        = _("Her");
static const u8 sPronounString_FemCap_Possessive[]    = _("Her");
static const u8 sPronounString_FemCap_PossessiveInd[] = _("Hers");
static const u8 sPronounString_FemCap_Reflexive[]     = _("Herself");
static const u8 sPronounString_FemCap_SubjectIs[]     = _("She's");
MAKE_PRONOUN_LIST(FemCap)

static const u8 sPronounString_Neuter_Subject[]       = _("it");
static const u8 sPronounString_Neuter_Object[]        = _("it");
static const u8 sPronounString_Neuter_Possessive[]    = _("its");
static const u8 sPronounString_Neuter_PossessiveInd[] = _("its");
static const u8 sPronounString_Neuter_Reflexive[]     = _("itself");
static const u8 sPronounString_Neuter_SubjectIs[]     = _("it's");
MAKE_PRONOUN_LIST(Neuter)

static const u8 sPronounString_NeuterCap_Subject[]       = _("it");
static const u8 sPronounString_NeuterCap_Object[]        = _("it");
static const u8 sPronounString_NeuterCap_Possessive[]    = _("its");
static const u8 sPronounString_NeuterCap_PossessiveInd[] = _("its");
static const u8 sPronounString_NeuterCap_Reflexive[]     = _("itself");
static const u8 sPronounString_NeuterCap_SubjectIs[]     = _("it's");
MAKE_PRONOUN_LIST(NeuterCap)

static const u8 sPronounString_Epicene_Subject[]       = _("they");
static const u8 sPronounString_Epicene_Object[]        = _("them");
static const u8 sPronounString_Epicene_Possessive[]    = _("their");
static const u8 sPronounString_Epicene_PossessiveInd[] = _("theirs");
static const u8 sPronounString_Epicene_Reflexive[]     = _("themselves");
static const u8 sPronounString_Epicene_SubjectIs[]     = _("they're");
MAKE_PRONOUN_LIST(Epicene)

static const u8 sPronounString_EpiceneCap_Subject[]       = _("They");
static const u8 sPronounString_EpiceneCap_Object[]        = _("Them");
static const u8 sPronounString_EpiceneCap_Possessive[]    = _("Their");
static const u8 sPronounString_EpiceneCap_PossessiveInd[] = _("Theirs");
static const u8 sPronounString_EpiceneCap_Reflexive[]     = _("Themselves");
static const u8 sPronounString_EpiceneCap_SubjectIs[]     = _("They're");
MAKE_PRONOUN_LIST(EpiceneCap)

extern const u8* GetPronounString(enum PronounType type, enum PronounForm form) {
   pronoun_list_type list = sPronouns_Neuter;
   
   bool8 capitalized = (form & 0x80) != 0;
   form &= 0x7F;
   
   if (capitalized) {
      list = sPronouns_NeuterCap;
   }
   switch (type) {
      case PRONOUN_TYPE_EPICENE:
         list = sPronouns_Epicene;
         if (capitalized) {
            list = sPronouns_EpiceneCap;
         }
         break;
      case PRONOUN_TYPE_FEMININE:
         list = sPronouns_Fem;
         if (capitalized) {
            list = sPronouns_FemCap;
         }
         break;
      case PRONOUN_TYPE_MASCULINE:
         list = sPronouns_Masc;
         if (capitalized) {
            list = sPronouns_MascCap;
         }
         break;
      case PRONOUN_TYPE_NEUTER:
         break;
   }
   return list[form];
}