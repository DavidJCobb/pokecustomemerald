#include "global.h"
#include "battle.h"
#include "battle_transition.h"
#include "battle_transition_frontier.h"
#include "bg.h"
#include "decompress.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_effect.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "trig.h"
#include "util.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/rgb.h"

#include "battle_transition/common.h"
#include "battle_transition/common_intro.h"

#include "battle_transition/angled_wipes.h"
#include "battle_transition/aqua_magma.h"
#include "battle_transition/big_pokeball.h"
#include "battle_transition/blur.h"
#include "battle_transition/clockwise_wipe.h"
#include "battle_transition/grid_squares.h"
#include "battle_transition/mugshot_champion.h"
#include "battle_transition/mugshot_drake.h"
#include "battle_transition/mugshot_glacia.h"
#include "battle_transition/mugshot_phoebe.h"
#include "battle_transition/mugshot_sidney.h"
#include "battle_transition/regi.h"
#include "battle_transition/ripple.h"
#include "battle_transition/shuffle.h"
#include "battle_transition/slice.h"
#include "battle_transition/swirl.h"
#include "battle_transition/wave.h"

//
// Configuration.
//

#define SLICE_TRANSITION_INITIAL_SPEED 1 // vanilla: 1
#define SLICE_TRANSITION_INITIAL_ACCEL 1 // vanilla: 1



//
//
//

#define B_TRANS_DMA_FLAGS (1 | ((DMA_SRC_INC | DMA_DEST_FIXED | DMA_REPEAT | DMA_16BIT | DMA_START_HBLANK | DMA_ENABLE) << 16))

// Used by each transition task to determine which of its functions to call
#define tState          data[0]

#define SET_TILE(ptr, posY, posX, tile) \
{                                       \
    u32 index = (posY) * 32 + posX;     \
    ptr[index] = tile | (0xF0 << 8);    \
}

struct TransitionData
{
    vu8 VBlank_DMA;
    u16 WININ;
    u16 WINOUT;
    u16 WIN0H;
    u16 WIN0V;
    u16 unused1;
    u16 unused2;
    u16 BLDCNT;
    u16 BLDALPHA;
    u16 BLDY;
    s16 cameraX;
    s16 cameraY;
    s16 BG0HOFS_Lower;
    s16 BG0HOFS_Upper;
    s16 BG0VOFS; // used but not set
    s16 unused3;
    s16 counter;
    s16 unused4;
    s16 data[11];
};

struct RectangularSpiralLine
{
    u8 state;
    s16 position;
    u8 moveIndex;
    s16 reboundPosition;
    bool8 outward;
};

typedef bool8 (*TransitionStateFunc)(struct Task *task);
typedef bool8 (*TransitionSpriteCallback)(struct Sprite *sprite);

static bool8 Transition_StartIntro(struct Task *);
static bool8 Transition_WaitForIntro(struct Task *);
static bool8 Transition_StartMain(struct Task *);
static bool8 Transition_WaitForMain(struct Task *);

static void LaunchBattleTransitionTask(u8);
static void Task_BattleTransition(u8);
static void Task_Intro(u8);
static void Task_PokeballsTrail(u8);
static void Task_WhiteBarsFade(u8);
static void Task_Kyogre(u8);
static void Task_Groudon(u8);
static void Task_Rayquaza(u8);
static void Task_ShredSplit(u8);
static void Task_Blackhole(u8);
static void Task_BlackholePulsate(u8);
static void Task_RectangularSpiral(u8);
static void Task_FrontierLogoWiggle(u8);
static void Task_FrontierLogoWave(u8);
static void Task_FrontierSquares(u8);
static void Task_FrontierSquaresScroll(u8);
static void Task_FrontierSquaresSpiral(u8);
static void VBlankCB_BattleTransition(void);
static void VBlankCB_PatternWeave(void);
static void VBlankCB_CircularMask(void);
static void VBlankCB_FrontierLogoWave(void);
static void HBlankCB_FrontierLogoWave(void);
static void VBlankCB_WhiteBarsFade(void);
static void VBlankCB_WhiteBarsFade_Blend(void);
static void HBlankCB_WhiteBarsFade(void);
static void VBlankCB_Rayquaza(void);
static bool8 FramesCountdown(struct Task *);
static bool8 WeatherTrio_BgFadeBlack(struct Task *);
static bool8 WeatherTrio_WaitFade(struct Task *);
static bool8 Kyogre_Init(struct Task *);
static bool8 Kyogre_PaletteFlash(struct Task *);
static bool8 Kyogre_PaletteBrighten(struct Task *);
static bool8 Groudon_Init(struct Task *);
static bool8 Groudon_PaletteFlash(struct Task *);
static bool8 Groudon_PaletteBrighten(struct Task *);
static bool8 WeatherDuo_FadeOut(struct Task *);
static bool8 WeatherDuo_End(struct Task *);
static bool8 PatternWeave_Blend1(struct Task *);
static bool8 PatternWeave_Blend2(struct Task *);
static bool8 PatternWeave_FinishAppear(struct Task *);
static bool8 PatternWeave_CircularMask(struct Task *);
static bool8 PokeballsTrail_Init(struct Task *);
static bool8 PokeballsTrail_Main(struct Task *);
static bool8 PokeballsTrail_End(struct Task *);
static bool8 WhiteBarsFade_Init(struct Task *);
static bool8 WhiteBarsFade_StartBars(struct Task *);
static bool8 WhiteBarsFade_WaitBars(struct Task *);
static bool8 WhiteBarsFade_BlendToBlack(struct Task *);
static bool8 WhiteBarsFade_End(struct Task *);
static bool8 ShredSplit_Init(struct Task *);
static bool8 ShredSplit_Main(struct Task *);
static bool8 ShredSplit_BrokenCheck(struct Task *);
static bool8 ShredSplit_End(struct Task *);
static bool8 Blackhole_Init(struct Task *);
static bool8 Blackhole_Vibrate(struct Task *);
static bool8 Blackhole_GrowEnd(struct Task *);
static bool8 BlackholePulsate_Main(struct Task *);
static bool8 RectangularSpiral_Init(struct Task *);
static bool8 RectangularSpiral_Main(struct Task *);
static bool8 RectangularSpiral_End(struct Task *);
static bool8 FrontierLogoWiggle_Init(struct Task *);
static bool8 FrontierLogoWiggle_SetGfx(struct Task *);
static bool8 FrontierLogoWave_Init(struct Task *);
static bool8 FrontierLogoWave_SetGfx(struct Task *);
static bool8 FrontierLogoWave_InitScanline(struct Task *);
static bool8 FrontierLogoWave_Main(struct Task *);
static bool8 Rayquaza_Init(struct Task *);
static bool8 Rayquaza_SetGfx(struct Task *);
static bool8 Rayquaza_PaletteFlash(struct Task *);
static bool8 Rayquaza_FadeToBlack(struct Task *);
static bool8 Rayquaza_WaitFade(struct Task *);
static bool8 Rayquaza_SetBlack(struct Task *);
static bool8 Rayquaza_TriRing(struct Task *);
static bool8 FrontierSquares_Init(struct Task *);
static bool8 FrontierSquares_Draw(struct Task *);
static bool8 FrontierSquares_Shrink(struct Task *);
static bool8 FrontierSquares_End(struct Task *);
static bool8 FrontierSquaresSpiral_Init(struct Task *);
static bool8 FrontierSquaresSpiral_Outward(struct Task *);
static bool8 FrontierSquaresSpiral_SetBlack(struct Task *);
static bool8 FrontierSquaresSpiral_Inward(struct Task *);
static bool8 FrontierSquaresScroll_Init(struct Task *);
static bool8 FrontierSquaresScroll_Draw(struct Task *);
static bool8 FrontierSquaresScroll_SetBlack(struct Task *);
static bool8 FrontierSquaresScroll_Erase(struct Task *);
static bool8 FrontierSquaresScroll_End(struct Task *);
static void InitTransitionData(void);
static void FadeScreenBlack(void);
static void SetTrainerPicSlideDirection(s16, s16);
static void IncrementTrainerPicState(s16);
static s16 IsTrainerPicSlideDone(s16);
static bool16 UpdateRectangularSpiralLine(const s16 *const *, struct RectangularSpiralLine *);
static void SpriteCB_FldEffPokeballTrail(struct Sprite *);
static void SpriteCB_WhiteBarFade(struct Sprite *);
static void VBlankCB_ShredSplit(void);
static void HBlankCB_ShredSplit(void);

static s16 sDebug_RectangularSpiralData;
static u8 sTestingTransitionId;
static u8 sTestingTransitionState;
static struct RectangularSpiralLine sRectangularSpiralLines[4];

EWRAM_DATA static struct TransitionData *sTransitionData = NULL;

static const u32 sPokeballTrail_Tileset[] = INCBIN_U32("graphics/battle_transitions/pokeball_trail.4bpp");
static const u8 sPokeball_Gfx[] = INCBIN_U8("graphics/battle_transitions/pokeball.4bpp");
static const u32 sShrinkingBoxTileset[] = INCBIN_U32("graphics/battle_transitions/shrinking_box.4bpp");
static const u16 sUnused_Palette[] = INCBIN_U16("graphics/battle_transitions/unused.gbapal");
static const u32 sKyogre_Tileset[] = INCBIN_U32("graphics/battle_transitions/kyogre.4bpp.lz");
static const u32 sKyogre_Tilemap[] = INCBIN_U32("graphics/battle_transitions/kyogre.bin.lz");
static const u32 sGroudon_Tileset[] = INCBIN_U32("graphics/battle_transitions/groudon.4bpp.lz");
static const u32 sGroudon_Tilemap[] = INCBIN_U32("graphics/battle_transitions/groudon.bin.lz");
static const u16 sKyogre1_Palette[] = INCBIN_U16("graphics/battle_transitions/kyogre_pt1.gbapal");
static const u16 sKyogre2_Palette[] = INCBIN_U16("graphics/battle_transitions/kyogre_pt2.gbapal");
static const u16 sGroudon1_Palette[] = INCBIN_U16("graphics/battle_transitions/groudon_pt1.gbapal");
static const u16 sGroudon2_Palette[] = INCBIN_U16("graphics/battle_transitions/groudon_pt2.gbapal");
static const u16 sRayquaza_Palette[] = INCBIN_U16("graphics/battle_transitions/rayquaza.gbapal");
static const u32 sRayquaza_Tileset[] = INCBIN_U32("graphics/battle_transitions/rayquaza.4bpp");
static const u32 sRayquaza_Tilemap[] = INCBIN_U32("graphics/battle_transitions/rayquaza.bin");
static const u16 sFrontierLogo_Palette[] = INCBIN_U16("graphics/battle_transitions/frontier_logo.gbapal");
static const u32 sFrontierLogo_Tileset[] = INCBIN_U32("graphics/battle_transitions/frontier_logo.4bpp.lz");
static const u32 sFrontierLogo_Tilemap[] = INCBIN_U32("graphics/battle_transitions/frontier_logo.bin.lz");
static const u16 sFrontierSquares_Palette[] = INCBIN_U16("graphics/battle_transitions/frontier_squares_blanktiles.gbapal");
static const u32 sFrontierSquares_FilledBg_Tileset[] = INCBIN_U32("graphics/battle_transitions/frontier_square_1.4bpp.lz");
static const u32 sFrontierSquares_EmptyBg_Tileset[] = INCBIN_U32("graphics/battle_transitions/frontier_square_2.4bpp.lz");
static const u32 sFrontierSquares_Shrink1_Tileset[] = INCBIN_U32("graphics/battle_transitions/frontier_square_3.4bpp.lz");
static const u32 sFrontierSquares_Shrink2_Tileset[] = INCBIN_U32("graphics/battle_transitions/frontier_square_4.4bpp.lz");
static const u32 sFrontierSquares_Tilemap[] = INCBIN_U32("graphics/battle_transitions/frontier_squares.bin");

// All battle transitions use the same intro
static const TaskFunc sTasks_Intro[B_TRANSITION_COUNT] =
{
    [0 ... B_TRANSITION_COUNT - 1] = &Task_Intro
};

// After the intro each transition has a unique main task.
// This task will call the functions that do the transition effects.
static const TaskFunc sTasks_Main[B_TRANSITION_COUNT] =
{
    [B_TRANSITION_BLUR]            = BattleTransitionTaskHandler_Blur,
    [B_TRANSITION_SWIRL]           = BattleTransitionTaskHandler_Swirl,
    [B_TRANSITION_SHUFFLE]         = BattleTransitionTaskHandler_Shuffle,
    [B_TRANSITION_BIG_POKEBALL]    = BattleTransitionTaskHandler_BigPokeball,
    [B_TRANSITION_POKEBALLS_TRAIL] = Task_PokeballsTrail,
    [B_TRANSITION_CLOCKWISE_WIPE]  = BattleTransitionTaskHandler_ClockwiseWipe,
    [B_TRANSITION_RIPPLE]          = BattleTransitionTaskHandler_Ripple,
    [B_TRANSITION_WAVE]            = BattleTransitionTaskHandler_Wave,
    [B_TRANSITION_SLICE]           = BattleTransitionTaskHandler_Slice,
    [B_TRANSITION_WHITE_BARS_FADE] = Task_WhiteBarsFade,
    [B_TRANSITION_GRID_SQUARES]    = BattleTransitionTaskHandler_GridSquares,
    [B_TRANSITION_ANGLED_WIPES]    = BattleTransitionTaskHandler_AngledWipes,
    [B_TRANSITION_SIDNEY]          = BattleTransitionTaskHandler_MugshotSidney,
    [B_TRANSITION_PHOEBE]          = BattleTransitionTaskHandler_MugshotPhoebe,
    [B_TRANSITION_GLACIA]          = BattleTransitionTaskHandler_MugshotGlacia,
    [B_TRANSITION_DRAKE]           = BattleTransitionTaskHandler_MugshotDrake,
    [B_TRANSITION_CHAMPION]        = BattleTransitionTaskHandler_MugshotChampion,
    [B_TRANSITION_AQUA]            = BattleTransitionTaskHandler_Aqua,
    [B_TRANSITION_MAGMA]           = BattleTransitionTaskHandler_Magma,
    [B_TRANSITION_REGICE]          = BattleTransitionTaskHandler_Regice,
    [B_TRANSITION_REGISTEEL]       = BattleTransitionTaskHandler_Registeel,
    [B_TRANSITION_REGIROCK]        = BattleTransitionTaskHandler_Regirock,
    [B_TRANSITION_KYOGRE]          = Task_Kyogre,
    [B_TRANSITION_GROUDON]         = Task_Groudon,
    [B_TRANSITION_RAYQUAZA]        = Task_Rayquaza,
    [B_TRANSITION_SHRED_SPLIT]     = Task_ShredSplit,
    [B_TRANSITION_BLACKHOLE]       = Task_Blackhole,
    [B_TRANSITION_BLACKHOLE_PULSATE] = Task_BlackholePulsate,
    [B_TRANSITION_RECTANGULAR_SPIRAL] = Task_RectangularSpiral,
    [B_TRANSITION_FRONTIER_LOGO_WIGGLE] = Task_FrontierLogoWiggle,
    [B_TRANSITION_FRONTIER_LOGO_WAVE] = Task_FrontierLogoWave,
    [B_TRANSITION_FRONTIER_SQUARES] = Task_FrontierSquares,
    [B_TRANSITION_FRONTIER_SQUARES_SCROLL] = Task_FrontierSquaresScroll,
    [B_TRANSITION_FRONTIER_SQUARES_SPIRAL] = Task_FrontierSquaresSpiral,
    [B_TRANSITION_FRONTIER_CIRCLES_MEET] = Task_FrontierCirclesMeet,
    [B_TRANSITION_FRONTIER_CIRCLES_CROSS] = Task_FrontierCirclesCross,
    [B_TRANSITION_FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL] = Task_FrontierCirclesAsymmetricSpiral,
    [B_TRANSITION_FRONTIER_CIRCLES_SYMMETRIC_SPIRAL] = Task_FrontierCirclesSymmetricSpiral,
    [B_TRANSITION_FRONTIER_CIRCLES_MEET_IN_SEQ] = Task_FrontierCirclesMeetInSeq,
    [B_TRANSITION_FRONTIER_CIRCLES_CROSS_IN_SEQ] = Task_FrontierCirclesCrossInSeq,
    [B_TRANSITION_FRONTIER_CIRCLES_ASYMMETRIC_SPIRAL_IN_SEQ] = Task_FrontierCirclesAsymmetricSpiralInSeq,
    [B_TRANSITION_FRONTIER_CIRCLES_SYMMETRIC_SPIRAL_IN_SEQ] = Task_FrontierCirclesSymmetricSpiralInSeq,
};

static const TransitionStateFunc sTaskHandlers[] =
{
    &Transition_StartIntro,
    &Transition_WaitForIntro,
    &Transition_StartMain,
    &Transition_WaitForMain
};

static const TransitionStateFunc sKyogre_Funcs[] =
{
    WeatherTrio_BgFadeBlack,
    WeatherTrio_WaitFade,
    Kyogre_Init,
    Kyogre_PaletteFlash,
    Kyogre_PaletteBrighten,
    FramesCountdown,
    WeatherDuo_FadeOut,
    WeatherDuo_End
};

static const TransitionStateFunc sPokeballsTrail_Funcs[] =
{
    PokeballsTrail_Init,
    PokeballsTrail_Main,
    PokeballsTrail_End
};

#define NUM_POKEBALL_TRAILS 5
static const s16 sPokeballsTrail_StartXCoords[2] = { -16, DISPLAY_WIDTH + 16 };
static const s16 sPokeballsTrail_Delays[NUM_POKEBALL_TRAILS] = {0, 32, 64, 18, 48};
static const s16 sPokeballsTrail_Speeds[2] = {8, -8};

static const TransitionStateFunc sShredSplit_Funcs[] =
{
    ShredSplit_Init,
    ShredSplit_Main,
    ShredSplit_BrokenCheck,
    ShredSplit_End
};

static const u8 sShredSplit_SectionYCoords[] = {39, DISPLAY_HEIGHT - 41};
static const s16 sShredSplit_SectionMoveDirs[] = {1, -1};

static const TransitionStateFunc sBlackhole_Funcs[] =
{
    Blackhole_Init,
    Blackhole_Vibrate,
    Blackhole_GrowEnd
};

static const TransitionStateFunc sBlackholePulsate_Funcs[] =
{
    Blackhole_Init,
    BlackholePulsate_Main
};

// Blackhole rapidly alternates adding these values to the radius,
// resulting in a vibrating shrink/grow effect.
static const s16 sBlackhole_Vibrations[] = {-6, 4};

static const TransitionStateFunc sRectangularSpiral_Funcs[] =
{
    RectangularSpiral_Init,
    RectangularSpiral_Main,
    RectangularSpiral_End
};

#define SPIRAL_END (-1)
#define SPIRAL_REBOUND (-2)

// Note that the directions are inverted for the lines originating at the bottom.
// i.e. MOVE_RIGHT is a right move for the top lines and a left move for the inverted bottom lines.
enum {
    MOVE_RIGHT = 1,
    MOVE_LEFT,
    MOVE_UP,
    MOVE_DOWN,
};

// Offsets of the movement data for spiraling in either direction.
#define SPIRAL_INWARD_START  0
#define SPIRAL_INWARD_END    3
#define SPIRAL_OUTWARD_START 4
#define SPIRAL_OUTWARD_END   7

static const s16 sRectangularSpiral_Major_InwardRight[]  = {MOVE_RIGHT,  27, 275, SPIRAL_END};
static const s16 sRectangularSpiral_Major_InwardLeft[]   = {MOVE_LEFT,  486, SPIRAL_END};
static const s16 sRectangularSpiral_Major_InwardUp[]     = {MOVE_UP,    262, SPIRAL_END};
static const s16 sRectangularSpiral_Major_InwardDown[]   = {MOVE_DOWN,  507, SPIRAL_REBOUND};

static const s16 sRectangularSpiral_Minor_InwardRight[]  = {MOVE_RIGHT, 213, SPIRAL_END};
static const s16 sRectangularSpiral_Minor_InwardLeft[]   = {MOVE_LEFT,  548, SPIRAL_REBOUND};
static const s16 sRectangularSpiral_Minor_InwardUp[]     = {MOVE_UP,    196, SPIRAL_END};
static const s16 sRectangularSpiral_Minor_InwardDown[]   = {MOVE_DOWN,  573, 309, SPIRAL_END};

static const s16 sRectangularSpiral_Minor_OutwardRight[] = {MOVE_RIGHT, 474, SPIRAL_END};
static const s16 sRectangularSpiral_Minor_OutwardLeft[]  = {MOVE_LEFT,  295, 32, SPIRAL_END};
static const s16 sRectangularSpiral_Minor_OutwardUp[]    = {MOVE_UP,     58, SPIRAL_END};
static const s16 sRectangularSpiral_Minor_OutwardDown[]  = {MOVE_DOWN,  455, SPIRAL_END};

static const s16 sRectangularSpiral_Major_OutwardRight[] = {MOVE_RIGHT, 540, SPIRAL_END};
static const s16 sRectangularSpiral_Major_OutwardLeft[]  = {MOVE_LEFT,  229, SPIRAL_END};
static const s16 sRectangularSpiral_Major_OutwardUp[]    = {MOVE_UP,    244, 28, SPIRAL_END};
static const s16 sRectangularSpiral_Major_OutwardDown[]  = {MOVE_DOWN,  517, SPIRAL_END};

// Move data for spiral lines starting in the top left / bottom right
static const s16 *const sRectangularSpiral_MoveDataTable_MajorDiagonal[] =
{
    [SPIRAL_INWARD_START] =
    sRectangularSpiral_Major_InwardRight,
    sRectangularSpiral_Major_InwardDown,
    sRectangularSpiral_Major_InwardLeft,
    sRectangularSpiral_Major_InwardUp,

    [SPIRAL_OUTWARD_START] =
    sRectangularSpiral_Major_OutwardUp,
    sRectangularSpiral_Major_OutwardLeft,
    sRectangularSpiral_Major_OutwardDown,
    sRectangularSpiral_Major_OutwardRight
};

// Move data for spiral lines starting in the top right / bottom left
static const s16 *const sRectangularSpiral_MoveDataTable_MinorDiagonal[] =
{
    [SPIRAL_INWARD_START] =
    sRectangularSpiral_Minor_InwardDown,
    sRectangularSpiral_Minor_InwardLeft,
    sRectangularSpiral_Minor_InwardUp,
    sRectangularSpiral_Minor_InwardRight,

    [SPIRAL_OUTWARD_START] =
    sRectangularSpiral_Minor_OutwardLeft,
    sRectangularSpiral_Minor_OutwardDown,
    sRectangularSpiral_Minor_OutwardRight,
    sRectangularSpiral_Minor_OutwardUp
};

static const s16 *const *const sRectangularSpiral_MoveDataTables[] =
{
    sRectangularSpiral_MoveDataTable_MajorDiagonal,
    sRectangularSpiral_MoveDataTable_MinorDiagonal
};

static const TransitionStateFunc sGroudon_Funcs[] =
{
    WeatherTrio_BgFadeBlack,
    WeatherTrio_WaitFade,
    Groudon_Init,
    Groudon_PaletteFlash,
    Groudon_PaletteBrighten,
    FramesCountdown,
    WeatherDuo_FadeOut,
    WeatherDuo_End
};

static const TransitionStateFunc sRayquaza_Funcs[] =
{
    WeatherTrio_BgFadeBlack,
    WeatherTrio_WaitFade,
    Rayquaza_Init,
    Rayquaza_SetGfx,
    Rayquaza_PaletteFlash,
    Rayquaza_FadeToBlack,
    Rayquaza_WaitFade,
    Rayquaza_SetBlack,
    Rayquaza_TriRing,
    Blackhole_Vibrate,
    Blackhole_GrowEnd
};

static const TransitionStateFunc sWhiteBarsFade_Funcs[] =
{
    WhiteBarsFade_Init,
    WhiteBarsFade_StartBars,
    WhiteBarsFade_WaitBars,
    WhiteBarsFade_BlendToBlack,
    WhiteBarsFade_End
};

#define NUM_WHITE_BARS 8
static const s16 sWhiteBarsFade_StartDelays[NUM_WHITE_BARS] = {0, 20, 15, 40, 10, 25, 35, 5};

static const struct SpriteFrameImage sSpriteImage_Pokeball[] =
{
    {sPokeball_Gfx, sizeof(sPokeball_Gfx)}
};

static const union AnimCmd sSpriteAnim_Pokeball[] =
{
    ANIMCMD_FRAME(0, 1),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_Pokeball[] =
{
    sSpriteAnim_Pokeball
};

static const union AffineAnimCmd sSpriteAffineAnim_Pokeball1[] =
{
    AFFINEANIMCMD_FRAME(0, 0, -4, 1),
    AFFINEANIMCMD_JUMP(0)
};

static const union AffineAnimCmd sSpriteAffineAnim_Pokeball2[] =
{
    AFFINEANIMCMD_FRAME(0, 0, 4, 1),
    AFFINEANIMCMD_JUMP(0)
};

static const union AffineAnimCmd *const sSpriteAffineAnimTable_Pokeball[] =
{
    sSpriteAffineAnim_Pokeball1,
    sSpriteAffineAnim_Pokeball2
};

static const struct SpriteTemplate sSpriteTemplate_Pokeball =
{
    .tileTag = TAG_NONE,
    .paletteTag = FLDEFF_PAL_TAG_POKEBALL_TRAIL,
    .oam = &gObjectEventBaseOam_32x32,
    .anims = sSpriteAnimTable_Pokeball,
    .images = sSpriteImage_Pokeball,
    .affineAnims = sSpriteAffineAnimTable_Pokeball,
    .callback = SpriteCB_FldEffPokeballTrail
};

static const u16 sFieldEffectPal_Pokeball[] = INCBIN_U16("graphics/field_effects/palettes/pokeball.gbapal");

const struct SpritePalette gSpritePalette_Pokeball = {sFieldEffectPal_Pokeball, FLDEFF_PAL_TAG_POKEBALL_TRAIL};

static const TransitionStateFunc sFrontierLogoWiggle_Funcs[] =
{
    FrontierLogoWiggle_Init,
    FrontierLogoWiggle_SetGfx,
    PatternWeave_Blend1,
    PatternWeave_Blend2,
    PatternWeave_FinishAppear,
    PatternWeave_CircularMask
};

static const TransitionStateFunc sFrontierLogoWave_Funcs[] =
{
    FrontierLogoWave_Init,
    FrontierLogoWave_SetGfx,
    FrontierLogoWave_InitScanline,
    FrontierLogoWave_Main
};

static const TransitionStateFunc sFrontierSquares_Funcs[] =
{
    FrontierSquares_Init,
    FrontierSquares_Draw,
    FrontierSquares_Shrink,
    FrontierSquares_End
};

static const TransitionStateFunc sFrontierSquaresSpiral_Funcs[] =
{
    FrontierSquaresSpiral_Init,
    FrontierSquaresSpiral_Outward,
    FrontierSquaresSpiral_SetBlack,
    FrontierSquaresSpiral_Inward,
    FrontierSquares_End
};

static const TransitionStateFunc sFrontierSquaresScroll_Funcs[] =
{
    FrontierSquaresScroll_Init,
    FrontierSquaresScroll_Draw,
    FrontierSquaresScroll_SetBlack,
    FrontierSquaresScroll_Erase,
    FrontierSquaresScroll_End
};

#define SQUARE_SIZE 4
#define MARGIN_SIZE 1 // Squares do not fit evenly across the width, so there is a margin on either side.
#define NUM_SQUARES_PER_ROW ((DISPLAY_WIDTH - (MARGIN_SIZE * 8 * 2)) / (SQUARE_SIZE * 8))
#define NUM_SQUARES_PER_COL (DISPLAY_HEIGHT / (SQUARE_SIZE * 8))
#define NUM_SQUARES         (NUM_SQUARES_PER_ROW * NUM_SQUARES_PER_COL)

// The order in which the squares should appear/disappear to create
// the spiral effect. Spiraling inward starts with the first element,
// and spiraling outward starts with the last. The positions are the
// squares numbered left-to-right top-to-bottom.
static const u8 sFrontierSquaresSpiral_Positions[NUM_SQUARES] = {
    28, 29, 30, 31, 32, 33, 34,
    27, 20, 13,  6,  5,  4,  3,
     2,  1,  0,  7, 14, 21, 22,
    23, 24, 25, 26, 19, 12, 11,
    10,  9,  8, 15, 16, 17, 18
};

// In the scrolling version the squares appear/disappear in a "random" order
// dictated by the list below.
static const u8 sFrontierSquaresScroll_Positions[] = {
     0, 16, 41, 22, 44,  2, 43, 21,
    46, 27,  9, 48, 38,  5, 57, 59,
    12, 63, 35, 28, 10, 53,  7, 49,
    39, 23, 55,  1, 62, 17, 61, 30,
     6, 34, 15, 51, 32, 58, 13, 45,
    37, 52, 11, 24, 60, 19, 56, 33,
    29, 50, 40, 54, 14,  3, 47, 20,
    18, 25,  4, 36, 26, 42, 31,  8
};

//---------------------------
// Main transition functions
//---------------------------

static void CB2_TestBattleTransition(void)
{
    switch (sTestingTransitionState)
    {
    case 0:
        LaunchBattleTransitionTask(sTestingTransitionId);
        sTestingTransitionState++;
        break;
    case 1:
        if (IsBattleTransitionDone())
        {
            sTestingTransitionState = 0;
            SetMainCallback2(CB2_ReturnToField);
        }
        break;
    }

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

void BattleTransition_StartTest(u8 transitionId)
{
    sTestingTransitionId = transitionId;
    SetMainCallback2(CB2_TestBattleTransition);
}

void BattleTransition_StartOnField(u8 transitionId)
{
    gMain.callback2 = CB2_OverworldBasic;
    LaunchBattleTransitionTask(transitionId);
}

void BattleTransition_Start(u8 transitionId)
{
    LaunchBattleTransitionTask(transitionId);
}

// main task that launches sub-tasks for phase1 and phase2
#define tTransitionId   data[1]
#define tTransitionDone data[15]

bool8 IsBattleTransitionDone(void)
{
    u8 taskId = FindTaskIdByFunc(Task_BattleTransition);
    if (gTasks[taskId].tTransitionDone)
    {
        DestroyTask(taskId);
        FREE_AND_SET_NULL(sTransitionData);
        gBattleTransitionData = NULL;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void LaunchBattleTransitionTask(u8 transitionId)
{
    u8 taskId = CreateTask(Task_BattleTransition, 2);
    gTasks[taskId].tTransitionId = transitionId;
    sTransitionData = AllocZeroed(sizeof(*sTransitionData));
    gBattleTransitionData = (struct BattleTransitionData*)sTransitionData;
}

static void Task_BattleTransition(u8 taskId)
{
    while (sTaskHandlers[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Transition_StartIntro(struct Task *task)
{
    SetWeatherScreenFadeOut();
    CpuCopy32(gPlttBufferFaded, gPlttBufferUnfaded, PLTT_SIZE);
    if (sTasks_Intro[task->tTransitionId] != NULL)
    {
        CreateTask(sTasks_Intro[task->tTransitionId], 4);
        task->tState++;
        return FALSE;
    }
    else
    {
        task->tState = 2;
        return TRUE;
    }
}

static bool8 Transition_WaitForIntro(struct Task *task)
{
    if (FindTaskIdByFunc(sTasks_Intro[task->tTransitionId]) == TASK_NONE)
    {
        task->tState++;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool8 Transition_StartMain(struct Task *task)
{
    CreateTask(sTasks_Main[task->tTransitionId], 0);
    task->tState++;
    return FALSE;
}

static bool8 Transition_WaitForMain(struct Task *task)
{
    task->tTransitionDone = FALSE;
    if (FindTaskIdByFunc(sTasks_Main[task->tTransitionId]) == TASK_NONE)
        task->tTransitionDone = TRUE;
    return FALSE;
}

#undef tTransitionId
#undef tTransitionDone

static void Task_Intro(u8 taskId)
{
    if (gTasks[taskId].tState == 0)
    {
        gTasks[taskId].tState++;
        BattleTransitionIntro_CreateTask();
    }
    else if (BattleTransitionIntro_IsTaskDone())
    {
        DestroyTask(taskId);
    }
}

//------------------------------------------------------------------------
// B_TRANSITION_BIG_POKEBALL, B_TRANSITION_AQUA, B_TRANSITION_MAGMA,
// B_TRANSITION_REGICE, B_TRANSITION_REGISTEEL, B_TRANSITION_REGIROCK
// and B_TRANSITION_KYOGRE.
//
// With the exception of B_TRANSITION_KYOGRE, all of the above transitions
// use the same weave effect (see the PatternWeave functions).
// Unclear why Kyogre's was grouped here and not with Groudon/Rayquaza's.
//------------------------------------------------------------------------

#define tBlendTarget1 data[1]
#define tBlendTarget2 data[2]
#define tBlendDelay   data[3]

// Data 1-3 change purpose for PatternWeave_CircularMask
#define tRadius      data[1]
#define tRadiusDelta data[2]
#define tVBlankSet   data[3]

#define tSinIndex     data[4]
#define tAmplitude    data[5]
#define tEndDelay     data[8]

static void Task_Kyogre(u8 taskId)
{
    while (sKyogre_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static void InitPatternWeaveTransition(struct Task *task)
{
    s32 i;

    InitTransitionData();
    ScanlineEffect_Clear();

    task->tBlendTarget1 = 16;
    task->tBlendTarget2 = 0;
    task->tSinIndex = 0;
    task->tAmplitude = 0x4000;
    sTransitionData->WININ = WININ_WIN0_ALL;
    sTransitionData->WINOUT = 0;
    sTransitionData->WIN0H = DISPLAY_WIDTH;
    sTransitionData->WIN0V = DISPLAY_HEIGHT;
    sTransitionData->BLDCNT = BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL;
    sTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);

    for (i = 0; i < DISPLAY_HEIGHT; i++)
        gScanlineEffectRegBuffers[1][i] = DISPLAY_WIDTH;

    SetVBlankCallback(VBlankCB_PatternWeave);
}

#define tTimer data[1]

static bool8 Kyogre_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    CpuFill16(0, tilemap, BG_SCREEN_SIZE);
    LZ77UnCompVram(sKyogre_Tileset, tileset);
    LZ77UnCompVram(sKyogre_Tilemap, tilemap);

    task->tState++;
    return FALSE;
}

static bool8 Kyogre_PaletteFlash(struct Task *task)
{
    if (task->tTimer % 3 == 0)
    {
        u16 offset = task->tTimer % 30;
        offset /= 3;
        LoadPalette(&sKyogre1_Palette[offset * 16], BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    }
    if (++task->tTimer > 58)
    {
        task->tState++;
        task->tTimer = 0;
    }

    return FALSE;
}

static bool8 Kyogre_PaletteBrighten(struct Task *task)
{
    if (task->tTimer % 5 == 0)
    {
        s16 offset = task->tTimer / 5;
        LoadPalette(&sKyogre2_Palette[offset * 16], BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    }
    if (++task->tTimer > 68)
    {
        task->tState++;
        task->tTimer = 0;
        task->tEndDelay = 30;
    }

    return FALSE;
}

static bool8 WeatherDuo_FadeOut(struct Task *task)
{
    BeginNormalPaletteFade(PALETTES_OBJECTS | (1 << 15), 1, 0, 16, RGB_BLACK);
    task->tState++;
    return FALSE;
}

static bool8 WeatherDuo_End(struct Task *task)
{
    if (!gPaletteFade.active)
    {
        DmaStop(0);
        FadeScreenBlack();
        DestroyTask(FindTaskIdByFunc(task->func));
    }
    return FALSE;
}

#undef tTimer

// The PatternWeave_ functions are used by several different transitions.
// They create an effect where a pattern/image (such as the Magma emblem) is
// formed by a shimmering weave effect.
static bool8 PatternWeave_Blend1(struct Task *task)
{
    sTransitionData->VBlank_DMA = FALSE;
    if (task->tBlendDelay == 0 || --task->tBlendDelay == 0)
    {
        task->tBlendTarget2++;
        task->tBlendDelay = 2;
    }
    sTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);
    if (task->tBlendTarget2 > 15)
        task->tState++;
    task->tSinIndex += 8;
    task->tAmplitude -= 256;

    BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude >> 8, DISPLAY_HEIGHT);

    sTransitionData->VBlank_DMA++;
    return FALSE;
}

static bool8 PatternWeave_Blend2(struct Task *task)
{
    sTransitionData->VBlank_DMA = FALSE;
    if (task->tBlendDelay == 0 || --task->tBlendDelay == 0)
    {
        task->tBlendTarget1--;
        task->tBlendDelay = 2;
    }
    sTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);
    if (task->tBlendTarget1 == 0)
        task->tState++;
    task->tSinIndex += 8;
    task->tAmplitude -= 256;

    BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude >> 8, DISPLAY_HEIGHT);

    sTransitionData->VBlank_DMA++;
    return FALSE;
}

static bool8 PatternWeave_FinishAppear(struct Task *task)
{
    sTransitionData->VBlank_DMA = FALSE;
    task->tSinIndex += 8;
    task->tAmplitude -= 256;

    BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude >> 8, DISPLAY_HEIGHT);

    if (task->tAmplitude <= 0)
    {
        task->tState++;
        task->tRadius = DISPLAY_HEIGHT;
        task->tRadiusDelta = 1 << 8;
        task->tVBlankSet = FALSE;
    }

    sTransitionData->VBlank_DMA++;
    return FALSE;
}

static bool8 FramesCountdown(struct Task *task)
{
    if (--task->tEndDelay == 0)
        task->tState++;
    return FALSE;
}

static bool8 WeatherTrio_BgFadeBlack(struct Task *task)
{
    BeginNormalPaletteFade(PALETTES_BG, 1, 0, 16, RGB_BLACK);
    task->tState++;
    return FALSE;
}

static bool8 WeatherTrio_WaitFade(struct Task *task)
{
    if (!gPaletteFade.active)
        task->tState++;
    return FALSE;
}

// Do a shrinking circular mask to go to a black screen after the pattern appears.
static bool8 PatternWeave_CircularMask(struct Task *task)
{
    sTransitionData->VBlank_DMA = FALSE;
    if (task->tRadiusDelta < (4 << 8))
        task->tRadiusDelta += 128; // 256 is 1 unit of speed. Speed up every other frame (128 / 256)
    if (task->tRadius != 0)
    {
        task->tRadius -= task->tRadiusDelta >> 8;
        if (task->tRadius < 0)
            task->tRadius = 0;
    }
    BattleTransitionCommon_SetCircularMask(gScanlineEffectRegBuffers[0], DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, task->tRadius);
    if (task->tRadius == 0)
    {
        SetVBlankCallback(NULL);
        DmaStop(0);
        FadeScreenBlack();
        DestroyTask(FindTaskIdByFunc(task->func));
    }
    else
    {
        if (!task->tVBlankSet)
        {
            task->tVBlankSet++;
            SetVBlankCallback(VBlankCB_CircularMask);
        }
        sTransitionData->VBlank_DMA++;
    }
    return FALSE;
}

static void VBlankCB_SetWinAndBlend(void)
{
    DmaStop(0);
    VBlankCB_BattleTransition();
    if (sTransitionData->VBlank_DMA)
        DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
    REG_WININ = sTransitionData->WININ;
    REG_WINOUT = sTransitionData->WINOUT;
    REG_WIN0V = sTransitionData->WIN0V;
    REG_BLDCNT = sTransitionData->BLDCNT;
    REG_BLDALPHA = sTransitionData->BLDALPHA;
}

static void VBlankCB_PatternWeave(void)
{
    VBlankCB_SetWinAndBlend();
    DmaSet(0, gScanlineEffectRegBuffers[1], &REG_BG0HOFS, B_TRANS_DMA_FLAGS);
}

static void VBlankCB_CircularMask(void)
{
    VBlankCB_SetWinAndBlend();
    DmaSet(0, gScanlineEffectRegBuffers[1], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}

#undef tAmplitude
#undef tSinIndex
#undef tBlendTarget1
#undef tBlendTarget2
#undef tRadius
#undef tRadiusDelta
#undef tVBlankSet

//------------------------------
// B_TRANSITION_POKEBALLS_TRAIL
//------------------------------
// Vanilla: used for trainer encounters in most locations, when the foe is stronger than the player.

#define sSide  data[0]
#define sDelay data[1]
#define sPrevX data[2]

static void Task_PokeballsTrail(u8 taskId)
{
    while (sPokeballsTrail_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 PokeballsTrail_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    CpuSet(sPokeballTrail_Tileset, tileset, 0x20);
    CpuFill32(0, tilemap, BG_SCREEN_SIZE);
    LoadPalette(sFieldEffectPal_Pokeball, BG_PLTT_ID(15), sizeof(sFieldEffectPal_Pokeball));

    task->tState++;
    return FALSE;
}

static bool8 PokeballsTrail_Main(struct Task *task)
{
    s16 i;
    s16 side;
    s16 startX[ARRAY_COUNT(sPokeballsTrail_StartXCoords)];
    s16 delays[ARRAY_COUNT(sPokeballsTrail_Delays)];
    memcpy(startX, sPokeballsTrail_StartXCoords, sizeof(sPokeballsTrail_StartXCoords));
    memcpy(delays, sPokeballsTrail_Delays, sizeof(sPokeballsTrail_Delays));

    // Randomly pick which side the first ball should start on.
    // The side is then flipped for each subsequent ball.
    side = Random() & 1;
    for (i = 0; i < NUM_POKEBALL_TRAILS; i++, side ^= 1)
    {
        gFieldEffectArguments[0] = startX[side];   // x
        gFieldEffectArguments[1] = (i * 32) + 16;  // y
        gFieldEffectArguments[2] = side;
        gFieldEffectArguments[3] = delays[i];
        FieldEffectStart(FLDEFF_POKEBALL_TRAIL);
    }

    task->tState++;
    return FALSE;
}

static bool8 PokeballsTrail_End(struct Task *task)
{
    if (!FieldEffectActiveListContains(FLDEFF_POKEBALL_TRAIL))
    {
        FadeScreenBlack();
        DestroyTask(FindTaskIdByFunc(Task_PokeballsTrail));
    }
    return FALSE;
}

bool8 FldEff_PokeballTrail(void)
{
    u8 spriteId = CreateSpriteAtEnd(&sSpriteTemplate_Pokeball, gFieldEffectArguments[0], gFieldEffectArguments[1], 0);
    gSprites[spriteId].oam.priority = 0;
    gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
    gSprites[spriteId].sSide = gFieldEffectArguments[2];
    gSprites[spriteId].sDelay = gFieldEffectArguments[3];
    gSprites[spriteId].sPrevX = -1;
    InitSpriteAffineAnim(&gSprites[spriteId]);
    StartSpriteAffineAnim(&gSprites[spriteId], gFieldEffectArguments[2]);
    return FALSE;
}

static void SpriteCB_FldEffPokeballTrail(struct Sprite *sprite)
{
    s16 speeds[ARRAY_COUNT(sPokeballsTrail_Speeds)];
    memcpy(speeds, sPokeballsTrail_Speeds, sizeof(sPokeballsTrail_Speeds));

    if (sprite->sDelay != 0)
    {
        sprite->sDelay--;
    }
    else
    {
        if (sprite->x >= 0 && sprite->x <= DISPLAY_WIDTH)
        {
            // Set Pokéball position
            s16 posX = sprite->x >> 3;
            s16 posY = sprite->y >> 3;

            // If Pokéball moved forward clear trail behind it
            if (posX != sprite->sPrevX)
            {
                u32 var;
                u16 *ptr;

                sprite->sPrevX = posX;
                var = ((REG_BG0CNT >> 8) & 0x1F) << 11;
                ptr = (u16 *)(BG_VRAM + var);

                SET_TILE(ptr, posY - 2, posX, 1);
                SET_TILE(ptr, posY - 1, posX, 1);
                SET_TILE(ptr, posY - 0, posX, 1);
                SET_TILE(ptr, posY + 1, posX, 1);
            }
        }
        sprite->x += speeds[sprite->sSide];
        if (sprite->x < -15 || sprite->x > DISPLAY_WIDTH + 15)
            FieldEffectStop(sprite, FLDEFF_POKEBALL_TRAIL);
    }
}

#undef sSide
#undef sDelay
#undef sPrevX

//--------------------------
// B_TRANSITION_SHRED_SPLIT
//--------------------------

// Data starts at 4. Possible it shared data
// with Slice above, which ends at 3.
#define tDelayTimer data[4]
#define tExtent     data[5]
#define tDelay      data[6]

static void Task_ShredSplit(u8 taskId)
{
    while (sShredSplit_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 ShredSplit_Init(struct Task *task)
{
    u16 i;

    InitTransitionData();
    ScanlineEffect_Clear();

    sTransitionData->WININ = WININ_WIN0_ALL;
    sTransitionData->WINOUT = 0;
    sTransitionData->WIN0V = DISPLAY_HEIGHT;

    for (i = 0; i < DISPLAY_HEIGHT; i++)
    {
        gScanlineEffectRegBuffers[1][i] = sTransitionData->cameraX;
        gScanlineEffectRegBuffers[1][DISPLAY_HEIGHT + i] = DISPLAY_WIDTH;
        gScanlineEffectRegBuffers[0][i] = sTransitionData->cameraX;
        gScanlineEffectRegBuffers[0][DISPLAY_HEIGHT + i] = DISPLAY_WIDTH;
        gScanlineEffectRegBuffers[0][DISPLAY_HEIGHT * 2 + i] = 0;
        gScanlineEffectRegBuffers[0][DISPLAY_HEIGHT * 3 + i] = 256;
        gScanlineEffectRegBuffers[0][DISPLAY_HEIGHT * 4 + i] = 1;
    }

    task->tDelayTimer = 0;
    task->tExtent = 0;
    task->tDelay = 7;

    EnableInterrupts(INTR_FLAG_HBLANK);

    SetVBlankCallback(VBlankCB_ShredSplit);
    SetHBlankCallback(HBlankCB_ShredSplit);

    task->tState++;
    return TRUE;
}

static void VBlankCB_ShredSplit(void) {
   DmaStop(0);
   BattleTransitionCommon_VBlankCB();
   REG_WININ  = WININ_WIN0_ALL;
   REG_WINOUT = 0;
   REG_WIN0V  = DISPLAY_HEIGHT;
   if (gBattleTransitionData->VBlank_DMA)
      DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 4);
   DmaSet(0, &gScanlineEffectRegBuffers[1][DISPLAY_HEIGHT], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}

static void HBlankCB_ShredSplit(void) {
   if (REG_VCOUNT < DISPLAY_HEIGHT) {
      u16 var = gScanlineEffectRegBuffers[1][REG_VCOUNT];
      REG_BG1HOFS = var;
      REG_BG2HOFS = var;
      REG_BG3HOFS = var;
   }
}

static bool8 ShredSplit_Main(struct Task *task)
{
    u16 i, j, k;
    u8 baseY[ARRAY_COUNT(sShredSplit_SectionYCoords)];
    s16 moveDirs[ARRAY_COUNT(sShredSplit_SectionMoveDirs)];
    u8 linesFinished;
    u16 *ptr4, *ptr3, *ptr1, *ptr2;
    s16 y;

    memcpy(baseY, sShredSplit_SectionYCoords, sizeof(baseY));
    memcpy(moveDirs, sShredSplit_SectionMoveDirs, sizeof(moveDirs));

    sTransitionData->VBlank_DMA = FALSE;
    linesFinished = 0;

    for (i = 0; i <= task->tExtent; i++)
    {
        // Slide half of the pixel rows (alternating) right
        for (j = 0; j < 2; j++)
        {
            for (k = 0; k < 2; k++)
            {
                y = baseY[j] + (moveDirs[k] * -i * 2);
                if (y >= 0 && (y != DISPLAY_HEIGHT / 2 - 1 || j != 1))
                {
                    ptr4 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT * 2];
                    ptr3 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT * 3];
                    ptr1 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT * 4];
                    if (*ptr4 >= DISPLAY_WIDTH)
                    {
                        *ptr4 = DISPLAY_WIDTH;
                        linesFinished++;
                    }
                    else
                    {
                        *ptr4 += (*ptr3 >> 8);
                        if (*ptr1 <= 0x7F)
                            *ptr1 *= 2;
                        if (*ptr3 <= 0xFFF)
                            *ptr3 += *ptr1;
                    }
                    ptr2 = &gScanlineEffectRegBuffers[0][y];
                    ptr3 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT];
                    *ptr2 = sTransitionData->cameraX + *ptr4;
                    *ptr3 = DISPLAY_WIDTH - *ptr4;

                    if (i == 0)
                        break;
                }
            }
        }

        // Slide the other half of the rows left
        for (j = 0; j < 2; j++)
        {
            for (k = 0; k < 2; k++)
            {
                y = baseY[j] + 1 + (moveDirs[k] * -i * 2);
                if (y <= DISPLAY_HEIGHT && (y != DISPLAY_HEIGHT / 2 || j != 1))
                {
                    ptr4 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT * 2];
                    ptr3 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT * 3];
                    ptr1 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT * 4];
                    if (*ptr4 >= DISPLAY_WIDTH)
                    {
                        *ptr4 = DISPLAY_WIDTH;
                        linesFinished++;
                    }
                    else
                    {
                        *ptr4 += (*ptr3 >> 8);
                        if (*ptr1 <= 0x7F)
                            *ptr1 *= 2;
                        if (*ptr3 <= 0xFFF)
                            *ptr3 += *ptr1;
                    }
                    ptr2 = &gScanlineEffectRegBuffers[0][y];
                    ptr3 = &gScanlineEffectRegBuffers[0][y + DISPLAY_HEIGHT];
                    *ptr2 = sTransitionData->cameraX - *ptr4;
                    *ptr3 = (*ptr4 << 8) | (DISPLAY_WIDTH + 1);

                    if (i == 0)
                        break;
                }
            }
        }
    }

    // Count down to next move
    if (--task->tDelayTimer < 0)
        task->tDelayTimer = 0;

    // Try increase effect's extent
    if (task->tDelayTimer <= 0 && task->tExtent + 1 <= DISPLAY_HEIGHT / 8)
    {
        task->tDelayTimer = task->tDelay;
        task->tExtent++;
    }

    // All lines have reached screen width, move on.
    if (linesFinished >= DISPLAY_HEIGHT)
        task->tState++;

    sTransitionData->VBlank_DMA++;
    return FALSE;
}

// This function never increments the state counter, because the loop condition
// is always false, resulting in the game being stuck in an infinite loop.
// It's possible this transition is only partially
// done and the second part was left out.
// In any case removing or bypassing this state allows the transition to finish.
static bool8 ShredSplit_BrokenCheck(struct Task *task)
{
    u16 i;
    bool32 done = TRUE;
    u16 checkVar2 = 0xFF10;

    for (i = 0; i < DISPLAY_HEIGHT; i++)
    {
        if (gScanlineEffectRegBuffers[1][i] != DISPLAY_WIDTH && gScanlineEffectRegBuffers[1][i] != checkVar2)
            done = FALSE;
    }

    if (done == TRUE)
        task->tState++;

    return FALSE;
}

static bool8 ShredSplit_End(struct Task *task)
{
    DmaStop(0);
    FadeScreenBlack();
    DestroyTask(FindTaskIdByFunc(Task_ShredSplit));
    return FALSE;
}

#undef tDelayTimer
#undef tExtent
#undef tDelay

//-----------------------------------------------------------
// B_TRANSITION_BLACKHOLE and B_TRANSITION_BLACKHOLE_PULSATE
//-----------------------------------------------------------

#define tRadius    data[1]
#define tGrowSpeed data[2]
#define tSinIndex  data[5]
#define tVibrateId data[6]
#define tAmplitude data[6] // Used differently by the two transitions
#define tFlag      data[7] // Used generally to indicate an action has taken place.

static void Task_Blackhole(u8 taskId)
{
    while (sBlackhole_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static void Task_BlackholePulsate(u8 taskId)
{
    while (sBlackholePulsate_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

// Init is shared by both transitions
static bool8 Blackhole_Init(struct Task *task)
{
    s32 i;

    InitTransitionData();
    ScanlineEffect_Clear();

    sTransitionData->WININ = 0;
    sTransitionData->WINOUT = WINOUT_WIN01_ALL;
    sTransitionData->WIN0H = DISPLAY_WIDTH;
    sTransitionData->WIN0V = DISPLAY_HEIGHT;

    for (i = 0; i < DISPLAY_HEIGHT; i++)
        gScanlineEffectRegBuffers[1][i] = 0;

    SetVBlankCallback(VBlankCB_CircularMask);

    task->tState++;
    task->tRadius = 1;
    task->tGrowSpeed = 1 << 8;
    task->tFlag = FALSE;

    return FALSE;
}

static bool8 Blackhole_GrowEnd(struct Task *task)
{
    if (task->tFlag == TRUE)
    {
        DmaStop(0);
        SetVBlankCallback(NULL);
        DestroyTask(FindTaskIdByFunc(task->func));
    }
    else
    {
        sTransitionData->VBlank_DMA = FALSE;
        if (task->tGrowSpeed < 1024)
            task->tGrowSpeed += 128;
        if (task->tRadius < DISPLAY_HEIGHT)
            task->tRadius += task->tGrowSpeed >> 8;
        if (task->tRadius > DISPLAY_HEIGHT)
            task->tRadius = DISPLAY_HEIGHT;
        BattleTransitionCommon_SetCircularMask(gScanlineEffectRegBuffers[0], DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, task->tRadius);
        if (task->tRadius == DISPLAY_HEIGHT)
        {
            task->tFlag = TRUE;
            FadeScreenBlack();
        }
        else
        {
            sTransitionData->VBlank_DMA++;
        }
    }

    return FALSE;
}

static bool8 Blackhole_Vibrate(struct Task *task)
{
    sTransitionData->VBlank_DMA = FALSE;
    if (task->tFlag == FALSE)
    {
        task->tFlag++;
        task->tRadius = 48;
        task->tVibrateId = 0;
    }
    task->tRadius += sBlackhole_Vibrations[task->tVibrateId];
    task->tVibrateId = (task->tVibrateId + 1) % (int)ARRAY_COUNT(sBlackhole_Vibrations);
    BattleTransitionCommon_SetCircularMask(gScanlineEffectRegBuffers[0], DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, task->tRadius);
    if (task->tRadius < 9)
    {
        task->tState++;
        task->tFlag = FALSE;
    }

    sTransitionData->VBlank_DMA++;
    return FALSE;
}

static bool8 BlackholePulsate_Main(struct Task *task)
{
    u16 index; // should be s16 I think
    s16 amplitude;

    sTransitionData->VBlank_DMA = FALSE;
    if (task->tFlag == FALSE)
    {
        task->tFlag++;
        task->tSinIndex = 2;
        task->tAmplitude = 2;
    }

    if (task->tRadius > DISPLAY_HEIGHT)
        task->tRadius = DISPLAY_HEIGHT;

    BattleTransitionCommon_SetCircularMask(gScanlineEffectRegBuffers[0], DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, task->tRadius);
    if (task->tRadius == DISPLAY_HEIGHT)
    {
        DmaStop(0);
        FadeScreenBlack();
        DestroyTask(FindTaskIdByFunc(task->func));
    }

    index = task->tSinIndex;
    if ((task->tSinIndex & 0xFF) <= 128)
    {
        amplitude = task->tAmplitude;
        task->tSinIndex += 8;
    }
    else
    {
        amplitude = task->tAmplitude - 1;
        task->tSinIndex += 16;
    }
    task->tRadius += Sin(index & 0xFF, amplitude);

    if (task->tRadius <= 0)
        task->tRadius = 1;

    if (task->tSinIndex >= 0xFF)
    {
        task->tSinIndex >>= 8;
        task->tAmplitude++;
    }

    sTransitionData->VBlank_DMA++;
    return FALSE;
}

#undef tRadius
#undef tGrowSpeed
#undef tSinIndex
#undef tVibrateId
#undef tAmplitude
#undef tFlag

//---------------------------------
// B_TRANSITION_RECTANGULAR_SPIRAL
//---------------------------------

static void Task_RectangularSpiral(u8 taskId)
{
    while (sRectangularSpiral_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 RectangularSpiral_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    CpuCopy16(sShrinkingBoxTileset, tileset, 0x20);
    CpuCopy16(&sShrinkingBoxTileset[0x70], &tileset[0x20], 0x20);
    CpuFill16(0xF0 << 8, tilemap, BG_SCREEN_SIZE);
    LoadPalette(sFieldEffectPal_Pokeball, BG_PLTT_ID(15), sizeof(sFieldEffectPal_Pokeball));

    task->data[3] = 1;
    task->tState++;

    // Line starting in top left
    sRectangularSpiralLines[0].state = SPIRAL_INWARD_START;
    sRectangularSpiralLines[0].position = -1;
    sRectangularSpiralLines[0].moveIndex = 1;
    sRectangularSpiralLines[0].reboundPosition = 308;
    sRectangularSpiralLines[0].outward = FALSE;

    // Line starting in bottom right
    sRectangularSpiralLines[1].state = SPIRAL_INWARD_START;
    sRectangularSpiralLines[1].position = -1;
    sRectangularSpiralLines[1].moveIndex = 1;
    sRectangularSpiralLines[1].reboundPosition = 308;
    sRectangularSpiralLines[1].outward = FALSE;

    // Line starting in top right
    sRectangularSpiralLines[2].state = SPIRAL_INWARD_START;
    sRectangularSpiralLines[2].position = -3;
    sRectangularSpiralLines[2].moveIndex = 1;
    sRectangularSpiralLines[2].reboundPosition = 307;
    sRectangularSpiralLines[2].outward = FALSE;

    // Line starting in bottom left
    sRectangularSpiralLines[3].state = SPIRAL_INWARD_START;
    sRectangularSpiralLines[3].position = -3;
    sRectangularSpiralLines[3].moveIndex = 1;
    sRectangularSpiralLines[3].reboundPosition = 307;
    sRectangularSpiralLines[3].outward = FALSE;

    return FALSE;
}

static bool8 RectangularSpiral_Main(struct Task *task)
{
    u16 *tilemap, *tileset;
    u8 i;
    u16 j;
    bool32 done = TRUE;

    GetBg0TilesDst(&tilemap, &tileset);

    // Draw 2 tiles at a time for each spiral line
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < ARRAY_COUNT(sRectangularSpiralLines); j++)
        {
            s16 position = 0;
            s16 x = 0, y = 0;

            if (UpdateRectangularSpiralLine(sRectangularSpiral_MoveDataTables[j / 2], &sRectangularSpiralLines[j]))
            {
                // The line moved to a new position, draw the tile.
                done = FALSE;
                position = sRectangularSpiralLines[j].position;

                // Invert position for the two lines that start at the bottom.
                if ((j % 2) == 1)
                    position = 637 - position;

                x = position % 32;
                y = position / 32;

                SET_TILE(tilemap, y, x, 2);
            }
        }
    }

    if (done == TRUE)
        task->tState++;
    return FALSE;
}

static bool8 RectangularSpiral_End(struct Task *task)
{
    DmaStop(0);
    FadeScreenBlack();
    DestroyTask(FindTaskIdByFunc(task->func));
    return FALSE;
}

// Returns TRUE if a tile should be drawn, FALSE otherwise
static bool16 UpdateRectangularSpiralLine(const s16 *const *moveDataTable, struct RectangularSpiralLine *line)
{
    const s16 *moveData = moveDataTable[line->state];

    // Has spiral finished?
    // Note that most move data arrays endsin SPIRAL_END but it is
    // only ever reached on the final array of spiraling outward.
    if (moveData[line->moveIndex] == SPIRAL_END)
        return FALSE;

    // Presumably saving data for debug.
    sDebug_RectangularSpiralData = moveData[0];
    sDebug_RectangularSpiralData = moveData[1];
    sDebug_RectangularSpiralData = moveData[2];
    sDebug_RectangularSpiralData = moveData[3];

    // Note that for the two lines originating at the bottom the
    // position is inverted, so the directions are flipped.
    // i.e. position += 1 is right for the top lines and left
    // for their inverted partners on the bottom.
    switch (moveData[0])
    {
    case MOVE_RIGHT:
        line->position += 1;
        break;
    case MOVE_LEFT:
        line->position -= 1;
        break;
    case MOVE_UP:
        line->position -= 32;
        break;
    case MOVE_DOWN:
        line->position += 32;
        break;
    }

    // Below check is never true.
    // SPIRAL_END was already checked, and position is never >= 640
    if (line->position >= 640 || moveData[line->moveIndex] == SPIRAL_END)
        return FALSE;

    if (!line->outward && moveData[line->moveIndex] == SPIRAL_REBOUND)
    {
        // Line has reached the final point of spiraling inward.
        // Time to flip and start spiraling outward.
        line->outward = TRUE;
        line->moveIndex = 1;
        line->position = line->reboundPosition;
        line->state = SPIRAL_OUTWARD_START;
    }

    // Reached move target, advance to next movement.
    if (line->position == moveData[line->moveIndex])
    {
        line->state++;
        if (line->outward == TRUE)
        {
            if (line->state > SPIRAL_OUTWARD_END)
            {
                // Still spiraling outward, loop back to the first state
                // but use the second set of move targets.
                // For example, the 28 in sRectangularSpiral_Major_OutwardUp
                line->moveIndex++;
                line->state = SPIRAL_OUTWARD_START;
            }
        }
        else
        {
            if (line->state > SPIRAL_INWARD_END)
            {
                // Still spiraling inward, loop back to the first state
                // but use the second set of move targets.
                // For example, the 275 in sRectangularSpiral_Major_InwardRight
                line->moveIndex++;
                line->state = SPIRAL_INWARD_START;
            }
        }
    }
    return TRUE;
}

//----------------------
// B_TRANSITION_GROUDON
//----------------------

#define tTimer data[1]

static void Task_Groudon(u8 taskId)
{
    while (sGroudon_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Groudon_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    CpuFill16(0, tilemap, BG_SCREEN_SIZE);
    LZ77UnCompVram(sGroudon_Tileset, tileset);
    LZ77UnCompVram(sGroudon_Tilemap, tilemap);

    task->tState++;
    task->tTimer = 0;
    return FALSE;
}

static bool8 Groudon_PaletteFlash(struct Task *task)
{
    if (task->tTimer % 3 == 0)
    {
        u16 offset = (task->tTimer % 30) / 3;
        LoadPalette(&sGroudon1_Palette[offset * 16], BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    }
    if (++task->tTimer > 58)
    {
        task->tState++;
        task->tTimer = 0;
    }

    return FALSE;
}

static bool8 Groudon_PaletteBrighten(struct Task *task)
{
    if (task->tTimer % 5 == 0)
    {
        s16 offset = task->tTimer / 5;
        LoadPalette(&sGroudon2_Palette[offset * 16], BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    }
    if (++task->tTimer > 68)
    {
        task->tState++;
        task->tTimer = 0;
        task->tEndDelay = 30;
    }

    return FALSE;
}

#undef tTimer
#undef tEndDelay

//-----------------------
// B_TRANSITION_RAYQUAZA
//-----------------------

#define tTimer     data[1]
#define tGrowSpeed data[2] // Shared from B_TRANSITION_BLACKHOLE
#define tFlag      data[7] // Shared from B_TRANSITION_BLACKHOLE

static void Task_Rayquaza(u8 taskId)
{
    while (sRayquaza_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 Rayquaza_Init(struct Task *task)
{
    u16 *tilemap, *tileset;
    u16 i;

    InitTransitionData();
    ScanlineEffect_Clear();

    SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_CHARBASE(2) | BGCNT_SCREENBASE(26) | BGCNT_TXT256x512);
    GetBg0TilesDst(&tilemap, &tileset);
    CpuFill16(0, tilemap, BG_SCREEN_SIZE);
    CpuCopy16(sRayquaza_Tileset, tileset, 0x2000);

    sTransitionData->counter = 0;
    task->tState++;
    LoadPalette(&sRayquaza_Palette[80], BG_PLTT_ID(15), PLTT_SIZE_4BPP);

    for (i = 0; i < DISPLAY_HEIGHT; i++)
    {
        gScanlineEffectRegBuffers[0][i] = 0;
        gScanlineEffectRegBuffers[1][i] = 0x100;
    }

    SetVBlankCallback(VBlankCB_Rayquaza);
    return FALSE;
}

static bool8 Rayquaza_SetGfx(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    CpuCopy16(sRayquaza_Tilemap, tilemap, sizeof(sRayquaza_Tilemap));
    task->tState++;
    return FALSE;
}

static bool8 Rayquaza_PaletteFlash(struct Task *task)
{
    if ((task->tTimer % 4) == 0)
    {
        u16 value = task->tTimer / 4;
        const u16 *palPtr = &sRayquaza_Palette[(value + 5) * 16];
        LoadPalette(palPtr, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    }
    if (++task->tTimer > 40)
    {
        task->tState++;
        task->tTimer = 0;
    }

    return FALSE;
}

static bool8 Rayquaza_FadeToBlack(struct Task *task)
{
    if (++task->tTimer > 20)
    {
        task->tState++;
        task->tTimer = 0;
        BeginNormalPaletteFade(PALETTES_OBJECTS | (1 << 15), 2, 0, 16, RGB_BLACK);
    }

    return FALSE;
}

static bool8 Rayquaza_WaitFade(struct Task *task)
{
    if (!gPaletteFade.active)
    {
        sTransitionData->counter = 1;
        task->tState++;
    }
    return FALSE;
}

static bool8 Rayquaza_SetBlack(struct Task *task)
{
    BlendPalettes(PALETTES_BG & ~(1 << 15), 8, RGB_BLACK);
    BlendPalettes(PALETTES_OBJECTS | (1 << 15), 0, RGB_BLACK);

    task->tState++;
    return FALSE;
}

static bool8 Rayquaza_TriRing(struct Task *task)
{
    if ((task->tTimer % 3) == 0)
    {
        u16 value = task->tTimer / 3;
        const u16 *palPtr = &sRayquaza_Palette[(value + 0) * 16];
        LoadPalette(palPtr, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    }
    if (++task->tTimer >= 40)
    {
        u16 i;

        sTransitionData->WININ = 0;
        sTransitionData->WINOUT = WINOUT_WIN01_ALL;
        sTransitionData->WIN0H = DISPLAY_WIDTH;
        sTransitionData->WIN0V = DISPLAY_HEIGHT;

        for (i = 0; i < DISPLAY_HEIGHT; i++)
            gScanlineEffectRegBuffers[1][i] = 0;

        SetVBlankCallback(VBlankCB_CircularMask);
        task->tState++;
        task->tGrowSpeed = 1 << 8;
        task->tFlag = FALSE;
        ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_BG0_ON);
    }

    return FALSE;
}

static void VBlankCB_Rayquaza(void)
{
    void *dmaSrc;

    DmaStop(0);
    VBlankCB_BattleTransition();

    if (sTransitionData->counter == 0)
        dmaSrc = gScanlineEffectRegBuffers[0];
    else if (sTransitionData->counter == 1)
        dmaSrc = gScanlineEffectRegBuffers[1];
    else
        dmaSrc = gScanlineEffectRegBuffers[0];

    DmaSet(0, dmaSrc, &REG_BG0VOFS, B_TRANS_DMA_FLAGS);
}

#undef tTimer
#undef tGrowSpeed
#undef tFlag

//------------------------------
// B_TRANSITION_WHITE_BARS_FADE
//------------------------------

#define sFade            data[0]
#define sFinished        data[1]
#define sDestroyAttempts data[2]
#define sDelay           data[5]
#define sIsMainSprite    data[6]

#define FADE_TARGET (16 << 8)

static void Task_WhiteBarsFade(u8 taskId)
{
    while (sWhiteBarsFade_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 WhiteBarsFade_Init(struct Task *task)
{
    u16 i;

    InitTransitionData();
    ScanlineEffect_Clear();

    sTransitionData->BLDCNT = BLDCNT_TGT1_ALL | BLDCNT_EFFECT_LIGHTEN;
    sTransitionData->BLDY = 0;
    sTransitionData->WININ = WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_BG3 | WININ_WIN0_OBJ;
    sTransitionData->WINOUT = WINOUT_WIN01_ALL;
    sTransitionData->WIN0V = DISPLAY_HEIGHT;

    for (i = 0; i < DISPLAY_HEIGHT; i++)
    {
        gScanlineEffectRegBuffers[1][i] = 0;
        gScanlineEffectRegBuffers[1][i + DISPLAY_HEIGHT] = DISPLAY_WIDTH;
    }

    EnableInterrupts(INTR_FLAG_HBLANK);
    SetHBlankCallback(HBlankCB_WhiteBarsFade);
    SetVBlankCallback(VBlankCB_WhiteBarsFade);

    task->tState++;
    return FALSE;
}

static bool8 WhiteBarsFade_StartBars(struct Task *task)
{
    s16 i, posY;
    s16 delays[ARRAY_COUNT(sWhiteBarsFade_StartDelays)];
    struct Sprite *sprite;
    memcpy(delays, sWhiteBarsFade_StartDelays, sizeof(sWhiteBarsFade_StartDelays));

    for (i = 0, posY = 0; i < NUM_WHITE_BARS; i++, posY += DISPLAY_HEIGHT / NUM_WHITE_BARS)
    {
        sprite = &gSprites[CreateInvisibleSprite(SpriteCB_WhiteBarFade)];
        sprite->x = DISPLAY_WIDTH;
        sprite->y = posY;
        sprite->sDelay = delays[i];
    }

    // Set on one sprite only. This one will enable the DMA
    // copy in VBlank and wait for the others to destroy.
    sprite->sIsMainSprite++;

    task->tState++;
    return FALSE;
}

static bool8 WhiteBarsFade_WaitBars(struct Task *task)
{
    sTransitionData->VBlank_DMA = 0;
    if (sTransitionData->counter >= NUM_WHITE_BARS)
    {
        BlendPalettes(PALETTES_ALL, 16, RGB_WHITE);
        task->tState++;
    }
    return FALSE;
}

static bool8 WhiteBarsFade_BlendToBlack(struct Task *task)
{
    sTransitionData->VBlank_DMA = 0;

    DmaStop(0);
    SetVBlankCallback(0);
    SetHBlankCallback(0);

    sTransitionData->WIN0H = DISPLAY_WIDTH;
    sTransitionData->BLDY = 0;
    sTransitionData->BLDCNT = 0xFF;
    sTransitionData->WININ = WININ_WIN0_ALL;

    SetVBlankCallback(VBlankCB_WhiteBarsFade_Blend);

    task->tState++;
    return FALSE;
}

static bool8 WhiteBarsFade_End(struct Task *task)
{
   if (++sTransitionData->BLDY > 16)
   {
       FadeScreenBlack();
       DestroyTask(FindTaskIdByFunc(Task_WhiteBarsFade));
   }
   return FALSE;
}

static void VBlankCB_WhiteBarsFade(void)
{
    DmaStop(0);
    VBlankCB_BattleTransition();
    REG_BLDCNT = sTransitionData->BLDCNT;
    REG_WININ = sTransitionData->WININ;
    REG_WINOUT = sTransitionData->WINOUT;
    REG_WIN0V = sTransitionData->WIN0V;
    if (sTransitionData->VBlank_DMA)
        DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 4);
    DmaSet(0, &gScanlineEffectRegBuffers[1][DISPLAY_HEIGHT], &REG_WIN0H, B_TRANS_DMA_FLAGS);
}

static void VBlankCB_WhiteBarsFade_Blend(void)
{
    VBlankCB_BattleTransition();
    REG_BLDY = sTransitionData->BLDY;
    REG_BLDCNT = sTransitionData->BLDCNT;
    REG_WININ = sTransitionData->WININ;
    REG_WINOUT = sTransitionData->WINOUT;
    REG_WIN0H = sTransitionData->WIN0H;
    REG_WIN0V = sTransitionData->WIN0V;
}

static void HBlankCB_WhiteBarsFade(void)
{
    REG_BLDY = gScanlineEffectRegBuffers[1][REG_VCOUNT];
}

static void SpriteCB_WhiteBarFade(struct Sprite *sprite)
{
    if (sprite->sDelay)
    {
        sprite->sDelay--;
        if (sprite->sIsMainSprite)
            sTransitionData->VBlank_DMA = 1;
    }
    else
    {
        u16 i;
        u16 *ptr1 = &gScanlineEffectRegBuffers[0][sprite->y];
        u16 *ptr2 = &gScanlineEffectRegBuffers[0][sprite->y + DISPLAY_HEIGHT];
        for (i = 0; i < DISPLAY_HEIGHT / NUM_WHITE_BARS; i++)
        {
            ptr1[i] = sprite->sFade >> 8;
            ptr2[i] = (u8)sprite->x;
        }
        if (sprite->x == 0 && sprite->sFade == FADE_TARGET)
            sprite->sFinished = TRUE;

        sprite->x -= 16;
        sprite->sFade += FADE_TARGET / 32;

        if (sprite->x < 0)
            sprite->x = 0;
        if (sprite->sFade > FADE_TARGET)
            sprite->sFade = FADE_TARGET;

        if (sprite->sIsMainSprite)
            sTransitionData->VBlank_DMA = 1;

        if (sprite->sFinished)
        {
            // If not the main sprite, destroy self. Otherwise, wait until the
            // others have destroyed themselves, or until enough time has elapsed.
            if (!sprite->sIsMainSprite || (sTransitionData->counter >= NUM_WHITE_BARS - 1 && sprite->sDestroyAttempts++ > 7))
            {
                sTransitionData->counter++;
                DestroySprite(sprite);
            }
        }
    }
}

#undef sFade
#undef sFinished
#undef sDestroyAttempts
#undef sDelay
#undef sIsMainSprite

//-----------------------------------
// General transition functions
//-----------------------------------

static void InitTransitionData(void)
{
    memset(sTransitionData, 0, sizeof(*sTransitionData));
    GetCameraOffsetWithPan(&sTransitionData->cameraX, &sTransitionData->cameraY);
}

static void VBlankCB_BattleTransition(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void GetBg0TilesDst(u16 **tilemap, u16 **tileset)
{
    u16 screenBase = REG_BG0CNT >> 8;
    u16 charBase = REG_BG0CNT >> 2;

    screenBase <<= 11;
    charBase <<= 14;

    *tilemap = (u16 *)(BG_VRAM + screenBase);
    *tileset = (u16 *)(BG_VRAM + charBase);
}

static void FadeScreenBlack(void)
{
    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
}

//-----------------------------------
// B_TRANSITION_FRONTIER_LOGO_WIGGLE
//-----------------------------------

#define tSinIndex  data[4]
#define tAmplitude data[5]

static bool8 FrontierLogoWiggle_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    InitPatternWeaveTransition(task);
    GetBg0TilesDst(&tilemap, &tileset);
    CpuFill16(0, tilemap, BG_SCREEN_SIZE);
    LZ77UnCompVram(sFrontierLogo_Tileset, tileset);
    LoadPalette(sFrontierLogo_Palette, BG_PLTT_ID(15), sizeof(sFrontierLogo_Palette));

    task->tState++;
    return FALSE;
}

static bool8 FrontierLogoWiggle_SetGfx(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    LZ77UnCompVram(sFrontierLogo_Tilemap, tilemap);
    BattleTransitionCommon_SetSinWave(gScanlineEffectRegBuffers[0], 0, task->tSinIndex, 132, task->tAmplitude, DISPLAY_HEIGHT);

    task->tState++;
    return TRUE;
}

static void Task_FrontierLogoWiggle(u8 taskId)
{
    while (sFrontierLogoWiggle_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

#undef tSinIndex
#undef tAmplitude

//---------------------------------
// B_TRANSITION_FRONTIER_LOGO_WAVE
//---------------------------------

#define tSinVal       data[1]
#define tAmplitudeVal data[2]
#define tTimer        data[3]
#define tStartedFade  data[4]
#define tBlendTarget2 data[5]
#define tBlendTarget1 data[6]
#define tSinDecrement data[7]

static void Task_FrontierLogoWave(u8 taskId)
{
    while (sFrontierLogoWave_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 FrontierLogoWave_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    InitTransitionData();
    ScanlineEffect_Clear();
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON);
    task->tAmplitudeVal = 32 << 8;
    task->tSinVal = 0x7FFF;
    task->tBlendTarget2 = 0;
    task->tBlendTarget1 = 16;
    task->tSinDecrement = 2560;
    sTransitionData->BLDCNT = BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL;
    sTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);
    REG_BLDCNT = sTransitionData->BLDCNT;
    REG_BLDALPHA = sTransitionData->BLDALPHA;
    GetBg0TilesDst(&tilemap, &tileset);
    CpuFill16(0, tilemap, BG_SCREEN_SIZE);
    LZ77UnCompVram(sFrontierLogo_Tileset, tileset);
    LoadPalette(sFrontierLogo_Palette, BG_PLTT_ID(15), sizeof(sFrontierLogo_Palette));
    sTransitionData->cameraY = 0;

    task->tState++;
    return FALSE;
}

static bool8 FrontierLogoWave_SetGfx(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    LZ77UnCompVram(sFrontierLogo_Tilemap, tilemap);

    task->tState++;
    return TRUE;
}

static bool8 FrontierLogoWave_InitScanline(struct Task *task)
{
    u8 i;

    for (i = 0; i < DISPLAY_HEIGHT; i++)
        gScanlineEffectRegBuffers[1][i] = sTransitionData->cameraY;

    SetVBlankCallback(VBlankCB_FrontierLogoWave);
    SetHBlankCallback(HBlankCB_FrontierLogoWave);
    EnableInterrupts(INTR_FLAG_HBLANK);

    task->tState++;
    return TRUE;
}

static bool8 FrontierLogoWave_Main(struct Task *task)
{
    u8 i;
    u16 sinVal, amplitude, sinSpread;

    sTransitionData->VBlank_DMA = FALSE;

    amplitude = task->tAmplitudeVal >> 8;
    sinVal = task->tSinVal;
    sinSpread = 384;

    task->tSinVal -= task->tSinDecrement;

    if (task->tTimer >= 70)
    {
        // Decrease amount of logo movement and distortion
        // until it rests normally in the middle of the screen.
        if (task->tAmplitudeVal - 384 >= 0)
            task->tAmplitudeVal -= 384;
        else
            task->tAmplitudeVal = 0;
    }

    if (task->tTimer >= 0 && task->tTimer % 3 == 0)
    {
        // Blend logo into view
        if (task->tBlendTarget2 < 16)
            task->tBlendTarget2++;
        else if (task->tBlendTarget1 > 0)
            task->tBlendTarget1--;

        sTransitionData->BLDALPHA = BLDALPHA_BLEND(task->tBlendTarget2, task->tBlendTarget1);
    }

    // Move logo up and down and distort it
    for (i = 0; i < DISPLAY_HEIGHT; i++, sinVal += sinSpread)
    {
        s16 index = sinVal / 256;
        gScanlineEffectRegBuffers[0][i] = sTransitionData->cameraY + Sin(index & 0xff, amplitude);
    }

    if (++task->tTimer == 101)
    {
        task->tStartedFade++;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    }

    if (task->tStartedFade && !gPaletteFade.active)
        DestroyTask(FindTaskIdByFunc(Task_FrontierLogoWave));

    task->tSinDecrement -= 17;
    sTransitionData->VBlank_DMA++;
    return FALSE;
}

static void VBlankCB_FrontierLogoWave(void)
{
    VBlankCB_BattleTransition();
    REG_BLDCNT = sTransitionData->BLDCNT;
    REG_BLDALPHA = sTransitionData->BLDALPHA;

    if (sTransitionData->VBlank_DMA)
        DmaCopy16(3, gScanlineEffectRegBuffers[0], gScanlineEffectRegBuffers[1], DISPLAY_HEIGHT * 2);
}

static void HBlankCB_FrontierLogoWave(void)
{
    u16 var = gScanlineEffectRegBuffers[1][REG_VCOUNT];
    REG_BG0VOFS = var;
}

#undef tSinVal
#undef tAmplitudeVal
#undef tTimer
#undef tStartedFade
#undef tBlendTarget2
#undef tBlendTarget1
#undef tSinDecrement

//----------------------------------------------------------------------
// B_TRANSITION_FRONTIER_SQUARES, B_TRANSITION_FRONTIER_SQUARES_SCROLL,
// and B_TRANSITION_FRONTIER_SQUARES_SPIRAL
//----------------------------------------------------------------------

#define tPosX             data[2]
#define tPosY             data[3]
#define tRowPos           data[4]
#define tShrinkState      data[5]
#define tShrinkDelayTimer data[6]
#define tShrinkDelay      data[7]

static void Task_FrontierSquares(u8 taskId)
{
    while (sFrontierSquares_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static void Task_FrontierSquaresSpiral(u8 taskId)
{
    while (sFrontierSquaresSpiral_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static void Task_FrontierSquaresScroll(u8 taskId)
{
    while (sFrontierSquaresScroll_Funcs[gTasks[taskId].tState](&gTasks[taskId]));
}

static bool8 FrontierSquares_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    LZ77UnCompVram(sFrontierSquares_FilledBg_Tileset, tileset);

    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect(0, 1, 0, 0, MARGIN_SIZE, 32, 15);
    FillBgTilemapBufferRect(0, 1, 30 - MARGIN_SIZE, 0, MARGIN_SIZE, 32, 15);
    CopyBgTilemapBufferToVram(0);
    LoadPalette(sFrontierSquares_Palette, BG_PLTT_ID(15), sizeof(sFrontierSquares_Palette));

    task->tPosX = MARGIN_SIZE;
    task->tPosY = 0;
    task->tRowPos = 0;
    task->tShrinkDelay = 10;

    task->tState++;
    return FALSE;
}

static bool8 FrontierSquares_Draw(struct Task *task)
{
    CopyRectToBgTilemapBufferRect(0, sFrontierSquares_Tilemap, 0, 0,
                                  SQUARE_SIZE, SQUARE_SIZE,
                                  task->tPosX, task->tPosY,
                                  SQUARE_SIZE, SQUARE_SIZE,
                                  15, 0, 0);
    CopyBgTilemapBufferToVram(0);

    task->tPosX += SQUARE_SIZE;
    if (++task->tRowPos == NUM_SQUARES_PER_ROW)
    {
        task->tPosX = MARGIN_SIZE;
        task->tPosY += SQUARE_SIZE;
        task->tRowPos = 0;
        if (task->tPosY >= NUM_SQUARES_PER_COL * SQUARE_SIZE)
            task->tState++;
    }

    return FALSE;
}

static bool8 FrontierSquares_Shrink(struct Task *task)
{
    u8 i;
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    if (task->tShrinkDelayTimer++ >= task->tShrinkDelay)
    {
        switch (task->tShrinkState)
        {
        case 0:
            for (i = BG_PLTT_ID(15) + 10; i < BG_PLTT_ID(15) + 15; i++)
            {
                gPlttBufferUnfaded[i] = RGB_BLACK;
                gPlttBufferFaded[i] = RGB_BLACK;
            }
            break;
        case 1:
            BlendPalettes(PALETTES_ALL & ~(1 << 15), 16, RGB_BLACK);
            LZ77UnCompVram(sFrontierSquares_EmptyBg_Tileset, tileset);
            break;
        case 2:
            LZ77UnCompVram(sFrontierSquares_Shrink1_Tileset, tileset);
            break;
        case 3:
            LZ77UnCompVram(sFrontierSquares_Shrink2_Tileset, tileset);
            break;
        default:
            FillBgTilemapBufferRect_Palette0(0, 1, 0, 0, 32, 32);
            CopyBgTilemapBufferToVram(0);
            task->tState++;
            return FALSE;
        }

        task->tShrinkDelayTimer = 0;
        task->tShrinkState++;
    }

    return FALSE;
}

#undef tPosX
#undef tPosY
#undef tRowPos
#undef tShrinkState
#undef tShrinkDelayTimer
#undef tShrinkDelay

#define tSquareNum data[2]
#define tFadeFlag  data[3]

static bool8 FrontierSquaresSpiral_Init(struct Task *task)
{
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    LZ77UnCompVram(sFrontierSquares_FilledBg_Tileset, tileset);

    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect(0, 1, 0, 0, MARGIN_SIZE, 32, 15);
    FillBgTilemapBufferRect(0, 1, 30 - MARGIN_SIZE, 0, MARGIN_SIZE, 32, 15);
    CopyBgTilemapBufferToVram(0);
    LoadPalette(sFrontierSquares_Palette, BG_PLTT_ID(14), sizeof(sFrontierSquares_Palette));
    LoadPalette(sFrontierSquares_Palette, BG_PLTT_ID(15), sizeof(sFrontierSquares_Palette));
    BlendPalette(BG_PLTT_ID(14), 16, 8, RGB_BLACK);

    task->tSquareNum = NUM_SQUARES - 1;
    task->tFadeFlag = 0;

    task->tState++;
    return FALSE;
}

static bool8 FrontierSquaresSpiral_Outward(struct Task *task)
{
    u8 pos = sFrontierSquaresSpiral_Positions[task->tSquareNum];
    u8 x = pos % NUM_SQUARES_PER_ROW;
    u8 y = pos / NUM_SQUARES_PER_ROW;
    CopyRectToBgTilemapBufferRect(0, sFrontierSquares_Tilemap, 0, 0,
                                  SQUARE_SIZE, SQUARE_SIZE,
                                  SQUARE_SIZE * x + MARGIN_SIZE, SQUARE_SIZE * y,
                                  SQUARE_SIZE, SQUARE_SIZE,
                                  15, 0, 0);
    CopyBgTilemapBufferToVram(0);

    if (--task->tSquareNum < 0)
        task->tState++;
    return FALSE;
}

// Now that the overworld is completely covered by the squares,
// set it to black so it's not revealed when the squares are removed.
static bool8 FrontierSquaresSpiral_SetBlack(struct Task *task)
{
    BlendPalette(BG_PLTT_ID(14), 16, 3, RGB_BLACK);
    BlendPalettes(PALETTES_ALL & ~(1 << 15 | 1 << 14), 16, RGB_BLACK);

    task->tSquareNum = 0;
    task->tFadeFlag = 0;

    task->tState++;
    return FALSE;
}

// Spiral inward erasing the squares
static bool8 FrontierSquaresSpiral_Inward(struct Task *task)
{
    // Each square is faded first, then the one that was faded last move is erased.
    if (task->tFadeFlag ^= 1)
    {
        // Shade square
        CopyRectToBgTilemapBufferRect(0, sFrontierSquares_Tilemap, 0, 0,
                                      SQUARE_SIZE, SQUARE_SIZE,
                                      SQUARE_SIZE * (sFrontierSquaresSpiral_Positions[task->tSquareNum] % NUM_SQUARES_PER_ROW) + MARGIN_SIZE,
                                      SQUARE_SIZE * (sFrontierSquaresSpiral_Positions[task->tSquareNum] / NUM_SQUARES_PER_ROW),
                                      SQUARE_SIZE, SQUARE_SIZE,
                                      14, 0, 0);
    }
    else
    {
        if (task->tSquareNum > 0)
        {
            // Erase square
            FillBgTilemapBufferRect(0, 1,
                                    SQUARE_SIZE * (sFrontierSquaresSpiral_Positions[task->tSquareNum - 1] % NUM_SQUARES_PER_ROW) + MARGIN_SIZE,
                                    SQUARE_SIZE * (sFrontierSquaresSpiral_Positions[task->tSquareNum - 1] / NUM_SQUARES_PER_ROW),
                                    SQUARE_SIZE, SQUARE_SIZE,
                                    15);
        }
        task->tSquareNum++;
    }

    if (task->tSquareNum >= NUM_SQUARES)
        task->tState++;

    CopyBgTilemapBufferToVram(0);
    return FALSE;
}

static bool8 FrontierSquares_End(struct Task *task)
{
    FillBgTilemapBufferRect_Palette0(0, 1, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(0);
    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
    DestroyTask(FindTaskIdByFunc(task->func));
    return FALSE;
}

#undef tSquareNum
#undef tFadeFlag

#define tScrollXDir       data[0]
#define tScrollYDir       data[1]
#define tScrollUpdateFlag data[2]

#define tSquareNum        data[2]

static void Task_ScrollBg(u8 taskId)
{
    if (!(gTasks[taskId].tScrollUpdateFlag ^= 1))
    {
        SetGpuReg(REG_OFFSET_BG0VOFS, gBattle_BG0_X);
        SetGpuReg(REG_OFFSET_BG0HOFS, gBattle_BG0_Y);
        gBattle_BG0_X += gTasks[taskId].tScrollXDir;
        gBattle_BG0_Y += gTasks[taskId].tScrollYDir;
    }
}

static bool8 FrontierSquaresScroll_Init(struct Task *task)
{
    u8 taskId = 0;
    u16 *tilemap, *tileset;

    GetBg0TilesDst(&tilemap, &tileset);
    LZ77UnCompVram(sFrontierSquares_FilledBg_Tileset, tileset);
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(0);
    LoadPalette(sFrontierSquares_Palette, BG_PLTT_ID(15), sizeof(sFrontierSquares_Palette));

    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    SetGpuReg(REG_OFFSET_BG0VOFS, gBattle_BG0_X);
    SetGpuReg(REG_OFFSET_BG0HOFS, gBattle_BG0_Y);

    task->tSquareNum = 0;

    // Start scrolling bg in a random direction.
    taskId = CreateTask(Task_ScrollBg, 1);
    switch (Random() % 4)
    {
    case 0: // Down/right
        gTasks[taskId].tScrollXDir = 1;
        gTasks[taskId].tScrollYDir = 1;
        break;
    case 1: // Up/left
        gTasks[taskId].tScrollXDir = -1;
        gTasks[taskId].tScrollYDir = -1;
        break;
    case 2: // Up/right
        gTasks[taskId].tScrollXDir = 1;
        gTasks[taskId].tScrollYDir = -1;
        break;
    default: // Down/left
        gTasks[taskId].tScrollXDir = -1;
        gTasks[taskId].tScrollYDir = 1;
        break;
    }

    task->tState++;
    return FALSE;
}

static bool8 FrontierSquaresScroll_Draw(struct Task *task)
{
    u8 pos = sFrontierSquaresScroll_Positions[task->tSquareNum];
    u8 x = pos / (NUM_SQUARES_PER_ROW + 1); // +1 because during scroll an additional column covers the margin.
    u8 y = pos % (NUM_SQUARES_PER_ROW + 1);

    CopyRectToBgTilemapBufferRect(0, &sFrontierSquares_Tilemap, 0, 0,
                                  SQUARE_SIZE, SQUARE_SIZE,
                                  SQUARE_SIZE * x + MARGIN_SIZE, SQUARE_SIZE * y,
                                  SQUARE_SIZE, SQUARE_SIZE,
                                  15, 0, 0);
    CopyBgTilemapBufferToVram(0);

    if (++task->tSquareNum >= (int)ARRAY_COUNT(sFrontierSquaresScroll_Positions))
        task->tState++;
    return 0;
}

// Now that the overworld is completely covered by the squares,
// set it to black so it's not revealed when the squares are removed.
static bool8 FrontierSquaresScroll_SetBlack(struct Task *task)
{
    BlendPalettes(PALETTES_ALL & ~(1 << 15), 16, RGB_BLACK);

    task->tSquareNum = 0;

    task->tState++;
    return FALSE;
}

static bool8 FrontierSquaresScroll_Erase(struct Task *task)
{
    u8 pos = sFrontierSquaresScroll_Positions[task->tSquareNum];
    u8 x = pos / (NUM_SQUARES_PER_ROW + 1);
    u8 y = pos % (NUM_SQUARES_PER_ROW + 1);

    FillBgTilemapBufferRect(0, 1,
                            SQUARE_SIZE * x + MARGIN_SIZE, SQUARE_SIZE * y,
                            SQUARE_SIZE, SQUARE_SIZE,
                            15);
    CopyBgTilemapBufferToVram(0);

    if (++task->tSquareNum >= (int)ARRAY_COUNT(sFrontierSquaresScroll_Positions))
    {
        DestroyTask(FindTaskIdByFunc(Task_ScrollBg));
        task->tState++;
    }

    return FALSE;
}

static bool8 FrontierSquaresScroll_End(struct Task *task)
{
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, gBattle_BG0_Y);

    FillBgTilemapBufferRect_Palette0(0, 1, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(0);
    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);

    DestroyTask(FindTaskIdByFunc(task->func));
    task->tState++; // Changing value of a destroyed task

    return FALSE;
}

#undef tScrollXDir
#undef tScrollYDir
#undef tScrollUpdateFlag
#undef tSquareNum
