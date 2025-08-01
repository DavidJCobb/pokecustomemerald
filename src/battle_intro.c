#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "scanline_effect.h"
#include "task.h"
#include "trig.h"
#include "constants/battle_intro.h"
#include "constants/trainers.h"

static EWRAM_DATA u16 sBgCnt = 0;

extern const u8 gBattleAnimBgCntSet[];
extern const u8 gBattleAnimBgCntGet[];

static void BattleIntroSlide1(u8);
static void BattleIntroSlide2(u8);
static void BattleIntroSlide3(u8);
static void BattleIntroSlideLink(u8);
static void BattleIntroSlidePartner(u8);

static const TaskFunc sBattleIntroSlideFuncs[] =
{
    [BATTLE_ENVIRONMENT_GRASS]      = BattleIntroSlide1,
    [BATTLE_ENVIRONMENT_LONG_GRASS] = BattleIntroSlide1,
    [BATTLE_ENVIRONMENT_SAND]       = BattleIntroSlide2,
    [BATTLE_ENVIRONMENT_UNDERWATER] = BattleIntroSlide2,
    [BATTLE_ENVIRONMENT_WATER]      = BattleIntroSlide2,
    [BATTLE_ENVIRONMENT_POND]       = BattleIntroSlide1,
    [BATTLE_ENVIRONMENT_MOUNTAIN]   = BattleIntroSlide1,
    [BATTLE_ENVIRONMENT_CAVE]       = BattleIntroSlide1,
    [BATTLE_ENVIRONMENT_BUILDING]   = BattleIntroSlide3,
    [BATTLE_ENVIRONMENT_PLAIN]      = BattleIntroSlide3,
};

#ifdef MODERN
   _Static_assert(BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_INITIAL >= 1, "the letterbox recede speed must be a positive non-zero number");
   _Static_assert(BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_BOOSTED >= 4, "the letterbox recede speed must be a positive non-zero number");
   //
   // See code comments in `BattleIntroSlide1`, below, for an explanation 
   // of the constraints below.
   //
   _Static_assert(0x20 % BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_INITIAL == 0, "the initial letterbox recede speed must multiply evenly into 0x20");
   //
   _Static_assert(0x30 % BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_BOOSTED == 0, "the boosted letterbox recede speed must multiply evenly into 0x30");
#endif
#define BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_INITIAL \
   (BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_INITIAL * 0x100) - BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_INITIAL
//
#define BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_BOOSTED \
   (BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_BOOSTED * 0x100) - BATTLE_INTRO_LETTERBOX_RECEDE_SPEED_BOOSTED

#ifdef MODERN
   _Static_assert(DISPLAY_WIDTH % BATTLE_INTRO_COMBATANT_SLIDE_IN_SPEED == 0, "the terrain slide speed must multiply evenly into the screen width");
#endif

void SetAnimBgAttribute(u8 bgId, u8 attributeId, u8 value)
{
    if (bgId < 4)
    {
        sBgCnt = GetGpuReg(gBattleAnimBgCntSet[bgId]);
        switch (attributeId)
        {
        case BG_ANIM_SCREEN_SIZE:
            ((struct BgCnt *)&sBgCnt)->screenSize = value;
            break;
        case BG_ANIM_AREA_OVERFLOW_MODE:
            ((struct BgCnt *)&sBgCnt)->areaOverflowMode = value;
            break;
        case BG_ANIM_MOSAIC:
            ((struct BgCnt *)&sBgCnt)->mosaic = value;
            break;
        case BG_ANIM_CHAR_BASE_BLOCK:
            ((struct BgCnt *)&sBgCnt)->charBaseBlock = value;
            break;
        case BG_ANIM_PRIORITY:
            ((struct BgCnt *)&sBgCnt)->priority = value;
            break;
        case BG_ANIM_PALETTES_MODE:
            ((struct BgCnt *)&sBgCnt)->palettes = value;
            break;
        case BG_ANIM_SCREEN_BASE_BLOCK:
            ((struct BgCnt *)&sBgCnt)->screenBaseBlock = value;
            break;
        }

        SetGpuReg(gBattleAnimBgCntSet[bgId], sBgCnt);
    }
}

int GetAnimBgAttribute(u8 bgId, u8 attributeId)
{
    u16 bgCnt;

    if (bgId < 4)
    {
        bgCnt = GetGpuReg(gBattleAnimBgCntGet[bgId]);
        switch (attributeId)
        {
        case BG_ANIM_SCREEN_SIZE:
            return ((struct BgCnt *)&bgCnt)->screenSize;
        case BG_ANIM_AREA_OVERFLOW_MODE:
            return ((struct BgCnt *)&bgCnt)->areaOverflowMode;
        case BG_ANIM_MOSAIC:
            return ((struct BgCnt *)&bgCnt)->mosaic;
        case BG_ANIM_CHAR_BASE_BLOCK:
            return ((struct BgCnt *)&bgCnt)->charBaseBlock;
        case BG_ANIM_PRIORITY:
            return ((struct BgCnt *)&bgCnt)->priority;
        case BG_ANIM_PALETTES_MODE:
            return ((struct BgCnt *)&bgCnt)->palettes;
        case BG_ANIM_SCREEN_BASE_BLOCK:
            return ((struct BgCnt *)&bgCnt)->screenBaseBlock;
        }
    }

    return 0;
}

#define tState data[0]
#define tEnvironment data[1]

void HandleIntroSlide(u8 environment)
{
    u8 taskId;

    if ((gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER) && gPartnerTrainerId != TRAINER_STEVEN_PARTNER)
    {
        taskId = CreateTask(BattleIntroSlidePartner, 0);
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        taskId = CreateTask(BattleIntroSlideLink, 0);
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
    {
        taskId = CreateTask(BattleIntroSlide3, 0);
    }
    else if ((gBattleTypeFlags & BATTLE_TYPE_KYOGRE_GROUDON) && gGameVersion != VERSION_RUBY)
    {
        environment = BATTLE_ENVIRONMENT_UNDERWATER;
        taskId = CreateTask(BattleIntroSlide2, 0);
    }
    else
    {
        taskId = CreateTask(sBattleIntroSlideFuncs[environment], 0);
    }

    gTasks[taskId].tState = 0;
    gTasks[taskId].tEnvironment = environment;
    gTasks[taskId].data[2] = 0;
    gTasks[taskId].data[3] = 0;
    gTasks[taskId].data[4] = 0;
    gTasks[taskId].data[5] = 0;
    gTasks[taskId].data[6] = 0;
}

static void BattleIntroSlideEnd(u8 taskId)
{
    DestroyTask(taskId);
    gBattle_BG1_X = 0;
    gBattle_BG1_Y = 0;
    gBattle_BG2_X = 0;
    gBattle_BG2_Y = 0;
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
    SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR | WININ_WIN1_BG_ALL | WININ_WIN1_OBJ | WININ_WIN1_CLR);
    SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR | WINOUT_WINOBJ_BG_ALL | WINOUT_WINOBJ_OBJ | WINOUT_WINOBJ_CLR);
}

static void BattleIntroSlide1(u8 taskId)
{
    /*
        BG1 is the foreground/parallax decoration, e.g. spiky blades of grass 
        for battles in grass. Here, the parallax decoration moves rightward by 
        6 pixels per frame, and downward by 1 or 2 pixels per frame (2 if it's 
        "long grass").
        
        BG3 is the main background: the terrain visible below each combatant, 
        and the striped pattern and gradient behind that. The terrain graphics 
        are fixed in the positions they'll have for most of the battle; to 
        animate the player-side terrain from right to left, a scanline effect 
        is used.
        
        During the battle intro, all background layers are clipped to the 
        inside of screen window 0. This is how the game does the "receding 
        letterboxing" intro effect. The initial bounds of window 0 aren't set 
        here, but rather in battle_main.c's CB2_InitBattleInternal; they're 
        set so that the top edge of the window is halfway down the screen, and 
        the bottom edge is one pixel below that, setting the window up to be 
        one pixel tall initially. (The specific coordinates are 80px and 81px; 
        the I/O register value is thus 0x5051.)
    */
    int i;

    gBattle_BG1_X += 6;
    switch (gTasks[taskId].tState)
    {
    case 0:
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            gTasks[taskId].data[2] = 16;
            gTasks[taskId].tState++;
        }
        else
        {
            gTasks[taskId].data[2] = 1;
            gTasks[taskId].tState++;
        }
        break;
    case 1:
        if (--gTasks[taskId].data[2] == 0)
        {
            gTasks[taskId].tState++;
            //
            // Set up window 0 so that the background and sprites are only 
            // visible wherever they overlap it.
            //
            SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR);
        }
        break;
    case 2:
        //
        // The vanilla game animates (recedes) the black bars at the top and 
        // bottom of the screen using this expression:
        //
        //    gBattle_WIN0V -= 0xFF;
        //
        // This subtraction is a clever trick to expand the window along both 
        // the top and bottom directions. Recall that the window range is 
        // stored as a two-byte integer, with the high byte being the Y-coord 
        // of the window's top edge, and the low byte being the Y-coord of the 
        // window's bottom edge. It's initialized to 0x5051, which creates a 
        // one-pixel-tall window centered within the screen. Well, here's what 
        // happens when we subtract 0xFF:
        //
        //    0x5051
        //    0x4F52
        //    0x4E53
        //    0x4D54
        //
        // The top edge is pulled back, and the bottom edge is pushed down.
        //
        // The expression here is effectively (x - 0x100 + 1). If you want to 
        // change the rate at which the letterboxing recedes, then you can 
        // formulate a similar expression: (x - 0x200 + 2) == (x - 0x1FE), 
        // for example...
        //
        gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_INITIAL;
        //
        // ...But in order for the check below to pass, your chosen speed 
        // must multiply evenly into 0x20. That is: given a desired speed S, 
        // you must arrange it so that (0x20 % S) == 0.
        //
        if ((gBattle_WIN0V & 0xFF00) == 0x3000) // window 0 top edge is at 48px
        {
            gTasks[taskId].tState++;
            gTasks[taskId].data[2] = DISPLAY_WIDTH;
            gTasks[taskId].data[3] = 32;
            gIntroSlideFlags &= ~1;
        }
        break;
    case 3:
        if (gTasks[taskId].data[3])
        {
            gTasks[taskId].data[3]--;
        }
        else
        {
            if (gTasks[taskId].tEnvironment == BATTLE_ENVIRONMENT_LONG_GRASS)
            {
                if (gBattle_BG1_Y != (u16)(-80))
                    gBattle_BG1_Y -= 2;
            }
            else
            {
                if (gBattle_BG1_Y != (u16)(-56))
                    gBattle_BG1_Y -= 1;
            }
        }

        if (gBattle_WIN0V & 0xFF00) // window 0 top edge is inset by any amount
        {
            //
            // The vanilla game runs the following expression:
            //
            //    gBattle_WIN0V -= 0x3FC;
            //
            // This is the same basic principle as the subtraction used above: 
            // we're still receding the letterbox effect, but now at a faster 
            // rate. Consider that when we first reach this point, the window 
            // register should have the value 0x3071. Subtracting 0x3FC, then, 
            // we see this:
            //
            //    0x3071
            //    0x2C75
            //    0x2879
            //
            // The low byte makes it obvious: we're using the same trick, but 
            // now with a speed of 4, such that the vanilla subtraction value 
            // is (0x400 - 4) = 0x3FC.
            //
            // The caveat in this case is that the speed must multiply evenly 
            // into 0x30, or else we'll underflow rather than settling at a 
            // window top Y-coordinate of zero. That is: (0x30 % S) must equal 
            // zero in order to avoid missing our goal.
            //
            gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_BOOSTED;
        }

        if (gTasks[taskId].data[2])
            gTasks[taskId].data[2] -= BATTLE_INTRO_COMBATANT_SLIDE_IN_SPEED;

        // Scanline settings have already been set in CB2_InitBattleInternal()
        for (i = 0; i < DISPLAY_HEIGHT / 2; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = gTasks[taskId].data[2];

        for (; i < DISPLAY_HEIGHT; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = -gTasks[taskId].data[2];

        if (gTasks[taskId].data[2] == 0)
        {
            gScanlineEffect.state = 3;
            gTasks[taskId].tState++;
            CpuFill32(0, (void *)BG_SCREEN_ADDR(28), BG_SCREEN_SIZE);
            SetBgAttribute(1, BG_ATTR_CHARBASEINDEX, 0);
            SetBgAttribute(2, BG_ATTR_CHARBASEINDEX, 0);
            SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(28) | BGCNT_TXT256x512);
            SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(30) | BGCNT_TXT512x256);
        }
        break;
    case 4:
        BattleIntroSlideEnd(taskId);
        break;
    }
}

static void BattleIntroSlide2(u8 taskId)
{
    int i;

    switch (gTasks[taskId].tEnvironment)
    {
    case BATTLE_ENVIRONMENT_SAND:
    case BATTLE_ENVIRONMENT_WATER:
        gBattle_BG1_X += 8;
        break;
    case BATTLE_ENVIRONMENT_UNDERWATER:
        gBattle_BG1_X += 6;
        break;
    }

    if (gTasks[taskId].tEnvironment == BATTLE_ENVIRONMENT_WATER)
    {
        gBattle_BG1_Y = Cos2(gTasks[taskId].data[6]) / 512 - 8;
        if (gTasks[taskId].data[6] < 180)
            gTasks[taskId].data[6] += 4;
        else
            gTasks[taskId].data[6] += 6;

        if (gTasks[taskId].data[6] == 360)
            gTasks[taskId].data[6] = 0;
    }

    switch (gTasks[taskId].tState)
    {
    case 0:
        gTasks[taskId].data[4] = 16;
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            gTasks[taskId].data[2] = 16;
            gTasks[taskId].tState++;
        }
        else
        {
            gTasks[taskId].data[2] = 1;
            gTasks[taskId].tState++;
        }
        break;
    case 1:
        if (--gTasks[taskId].data[2] == 0)
        {
            gTasks[taskId].tState++;
            SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR);
        }
        break;
    case 2:
        gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_INITIAL;
        if ((gBattle_WIN0V & 0xFF00) == 0x3000)
        {
            gTasks[taskId].tState++;
            gTasks[taskId].data[2] = DISPLAY_WIDTH;
            gTasks[taskId].data[3] = 32;
            gTasks[taskId].data[5] = 1;
            gIntroSlideFlags &= ~1;
        }
        break;
    case 3:
        if (gTasks[taskId].data[3])
        {
            if (--gTasks[taskId].data[3] == 0)
            {
                SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG3 | BLDCNT_TGT2_OBJ);
                SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(15, 0));
                SetGpuReg(REG_OFFSET_BLDY, 0);
            }
        }
        else
        {
            if ((gTasks[taskId].data[4] & 0x1F) && --gTasks[taskId].data[5] == 0)
            {
                gTasks[taskId].data[4] += 0xFF;
                gTasks[taskId].data[5] = 4;
            }
        }

        if (gBattle_WIN0V & 0xFF00)
            gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_BOOSTED;

        if (gTasks[taskId].data[2])
            gTasks[taskId].data[2] -= BATTLE_INTRO_COMBATANT_SLIDE_IN_SPEED;

        // Scanline settings have already been set in CB2_InitBattleInternal()
        for (i = 0; i < DISPLAY_HEIGHT / 2; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = gTasks[taskId].data[2];

        for (; i < DISPLAY_HEIGHT; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = -gTasks[taskId].data[2];

        if (gTasks[taskId].data[2] == 0)
        {
            gScanlineEffect.state = 3;
            gTasks[taskId].tState++;
            CpuFill32(0, (void *)BG_SCREEN_ADDR(28), BG_SCREEN_SIZE);
            SetBgAttribute(1, BG_ATTR_CHARBASEINDEX, 0);
            SetBgAttribute(2, BG_ATTR_CHARBASEINDEX, 0);
            SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(28) | BGCNT_TXT256x512);
            SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(30) | BGCNT_TXT512x256);
        }
        break;
    case 4:
        BattleIntroSlideEnd(taskId);
        break;
    }

    if (gTasks[taskId].tState != 4)
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(gTasks[taskId].data[4], 0));
}

static void BattleIntroSlide3(u8 taskId)
{
    int i;

    gBattle_BG1_X += 8;
    switch (gTasks[taskId].tState)
    {
    case 0:
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG3 | BLDCNT_TGT2_OBJ);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(8, 8));
        SetGpuReg(REG_OFFSET_BLDY, 0);
        gTasks[taskId].data[4] = BLDALPHA_BLEND(8, 8);
        if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
        {
            gTasks[taskId].data[2] = 16;
            gTasks[taskId].tState++;
        }
        else
        {
            gTasks[taskId].data[2] = 1;
            gTasks[taskId].tState++;
        }
        break;
    case 1:
        if (--gTasks[taskId].data[2] == 0)
        {
            gTasks[taskId].tState++;
            SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR);
        }
        break;
    case 2:
        gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_INITIAL;
        if ((gBattle_WIN0V & 0xFF00) == 0x3000)
        {
            gTasks[taskId].tState++;
            gTasks[taskId].data[2] = DISPLAY_WIDTH;
            gTasks[taskId].data[3] = 32;
            gTasks[taskId].data[5] = 1;
            gIntroSlideFlags &= ~1;
        }
        break;
    case 3:
        if (gTasks[taskId].data[3])
        {
            gTasks[taskId].data[3]--;
        }
        else
        {
            if ((gTasks[taskId].data[4] & 0xF) && --gTasks[taskId].data[5] == 0)
            {
                gTasks[taskId].data[4] += 0xFF;
                gTasks[taskId].data[5] = 6;
            }
        }

        if (gBattle_WIN0V & 0xFF00)
            gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_BOOSTED;

        if (gTasks[taskId].data[2])
            gTasks[taskId].data[2] -= BATTLE_INTRO_COMBATANT_SLIDE_IN_SPEED;

        // Scanline settings have already been set in CB2_InitBattleInternal()
        for (i = 0; i < DISPLAY_HEIGHT / 2; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = gTasks[taskId].data[2];

        for (; i < DISPLAY_HEIGHT; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = -gTasks[taskId].data[2];

        if (gTasks[taskId].data[2] == 0)
        {
            gScanlineEffect.state = 3;
            gTasks[taskId].tState++;
            CpuFill32(0, (void *)BG_SCREEN_ADDR(28), BG_SCREEN_SIZE);
            SetBgAttribute(1, BG_ATTR_CHARBASEINDEX, 0);
            SetBgAttribute(2, BG_ATTR_CHARBASEINDEX, 0);
            SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(28) | BGCNT_TXT256x512);
            SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(30) | BGCNT_TXT512x256);
        }
        break;
    case 4:
        BattleIntroSlideEnd(taskId);
        break;
    }

    if (gTasks[taskId].tState != 4)
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(gTasks[taskId].data[4], 0));
}

static void BattleIntroSlideLink(u8 taskId)
{
    int i;

    if (gTasks[taskId].tState > 1 && !gTasks[taskId].data[4])
    {
        u16 var0 = gBattle_BG1_X & 0x8000;
        if (var0 || gBattle_BG1_X < 80)
        {
            gBattle_BG1_X += 3;
            gBattle_BG2_X -= 3;
        }
        else
        {
            CpuFill32(0, (void *)BG_SCREEN_ADDR(28), BG_SCREEN_SIZE);
            CpuFill32(0, (void *)BG_SCREEN_ADDR(30), BG_SCREEN_SIZE);
            gTasks[taskId].data[4] = 1;
        }
    }

    switch (gTasks[taskId].tState)
    {
    case 0:
        gTasks[taskId].data[2] = 32;
        gTasks[taskId].tState++;
        break;
    case 1:
        if (--gTasks[taskId].data[2] == 0)
        {
            gTasks[taskId].tState++;
            gSprites[gBattleStruct->linkBattleVsSpriteId_V].oam.objMode = ST_OAM_OBJ_WINDOW;
            gSprites[gBattleStruct->linkBattleVsSpriteId_V].callback = SpriteCB_VsLetterInit;
            gSprites[gBattleStruct->linkBattleVsSpriteId_S].oam.objMode = ST_OAM_OBJ_WINDOW;
            gSprites[gBattleStruct->linkBattleVsSpriteId_S].callback = SpriteCB_VsLetterInit;
            SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR);
            SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WINOBJ_BG_ALL | WINOUT_WINOBJ_OBJ | WINOUT_WINOBJ_CLR | WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2);
        }
        break;
    case 2:
        gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_INITIAL;
        if ((gBattle_WIN0V & 0xFF00) == 0x3000)
        {
            gTasks[taskId].tState++;
            gTasks[taskId].data[2] = DISPLAY_WIDTH;
            gTasks[taskId].data[3] = 32;
            gIntroSlideFlags &= ~1;
        }
        break;
    case 3:
        if (gBattle_WIN0V & 0xFF00)
            gBattle_WIN0V -= BATTLE_INTRO_LETTERBOX_RECEDE_SUBTRACTION_BOOSTED;

        if (gTasks[taskId].data[2])
            gTasks[taskId].data[2] -= BATTLE_INTRO_COMBATANT_SLIDE_IN_SPEED;

        // Scanline settings have already been set in CB2_InitBattleInternal()
        for (i = 0; i < DISPLAY_HEIGHT / 2; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = gTasks[taskId].data[2];

        for (; i < DISPLAY_HEIGHT; i++)
            gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer][i] = -gTasks[taskId].data[2];

        if (gTasks[taskId].data[2] == 0)
        {
            gScanlineEffect.state = 3;
            gTasks[taskId].tState++;
            SetBgAttribute(1, BG_ATTR_CHARBASEINDEX, 0);
            SetBgAttribute(2, BG_ATTR_CHARBASEINDEX, 0);
            SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(28) | BGCNT_TXT256x512);
            SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(30) | BGCNT_TXT512x256);
        }
        break;
    case 4:
        BattleIntroSlideEnd(taskId);
        break;
    }
}

static void BattleIntroSlidePartner(u8 taskId)
{
    switch (gTasks[taskId].tState)
    {
    case 0:
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].tState++;
        break;
    case 1:
        if (--gTasks[taskId].data[2] == 0)
        {
            gTasks[taskId].tState++;
            SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(2) | BGCNT_CHARBASE(2) | BGCNT_16COLOR | BGCNT_SCREENBASE(28) | BGCNT_TXT512x256);
            SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(2) | BGCNT_CHARBASE(2) | BGCNT_16COLOR | BGCNT_SCREENBASE(30) | BGCNT_TXT512x256);
            SetGpuReg(REG_OFFSET_DISPCNT, GetGpuReg(REG_OFFSET_DISPCNT) | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_WIN0_ON | DISPCNT_WIN1_ON | DISPCNT_OBJWIN_ON);
            SetGpuReg(REG_OFFSET_WININ, WININ_WIN1_BG1 | WININ_WIN1_BG2 | WININ_WIN1_BG3 | WININ_WIN1_OBJ | WININ_WIN1_CLR);
            SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR | WINOUT_WINOBJ_BG_ALL | WINOUT_WINOBJ_OBJ | WINOUT_WINOBJ_CLR);
            gBattle_BG0_Y = -48;
            gBattle_BG1_X = DISPLAY_WIDTH;
            gBattle_BG2_X = -DISPLAY_WIDTH;
        }
        break;
    case 2:
        gBattle_WIN0V += 0x100;
        if ((gBattle_WIN0V & 0xFF00) != 0x100)
            gBattle_WIN0V--;

        if ((gBattle_WIN0V & 0xFF00) == 0x2000)
        {
            gTasks[taskId].tState++;
            gTasks[taskId].data[2] = DISPLAY_WIDTH;
            gIntroSlideFlags &= ~1;
        }
        break;
    case 3:
        if ((gBattle_WIN0V & 0xFF00) != 0x4C00)
            gBattle_WIN0V += 0x3FC;

        if (gTasks[taskId].data[2])
            gTasks[taskId].data[2] -= 2;

        gBattle_BG1_X = gTasks[taskId].data[2];
        gBattle_BG2_X = -gTasks[taskId].data[2];
        if (gTasks[taskId].data[2] == 0)
            gTasks[taskId].tState++;
        break;
    case 4:
        gBattle_BG0_Y += 2;
        gBattle_BG2_Y += 2;
        if ((gBattle_WIN0V & 0xFF00) != 0x5000)
            gBattle_WIN0V += 0xFF;

        if (!gBattle_BG0_Y)
        {
            CpuFill32(0, (void *)BG_SCREEN_ADDR(28), BG_SCREEN_SIZE * 4);
            SetGpuReg(REG_OFFSET_DISPCNT, GetGpuReg(REG_OFFSET_DISPCNT) & ~DISPCNT_WIN1_ON);
            SetBgAttribute(1, BG_ATTR_CHARBASEINDEX, 0);
            SetBgAttribute(2, BG_ATTR_CHARBASEINDEX, 0);
            SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(28) | BGCNT_TXT256x512);
            SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_16COLOR | BGCNT_SCREENBASE(30) | BGCNT_TXT512x256);
            gScanlineEffect.state = 3;
            gTasks[taskId].tState++;
        }
        break;
    case 5:
        BattleIntroSlideEnd(taskId);
        break;
    }
}

void DrawBattlerOnBg(int bgId, u8 x, u8 y, u8 battlerPosition, u8 paletteId, u8 *tiles, u16 *tilemap, u16 tilesOffset)
{
    int i, j;
    u8 battler = GetBattlerAtPosition(battlerPosition);
    int offset = tilesOffset;
    CpuCopy16(gMonSpritesGfxPtr->sprites.ptr[battlerPosition] + BG_SCREEN_SIZE * gBattleMonForms[battler], tiles, BG_SCREEN_SIZE);
    LoadBgTiles(bgId, tiles, 0x1000, tilesOffset);
    for (i = y; i < y + 8; i++)
    {
        for (j = x; j < x + 8; j++)
        {
            tilemap[i * 32 + j] = offset | (paletteId << 12);
            offset++;
        }
    }
    LoadBgTilemap(bgId, tilemap, BG_SCREEN_SIZE, 0);
}

static void UNUSED DrawBattlerOnBgDMA(u8 x, u8 y, u8 battlerPosition, u8 arg3, u8 paletteId, u16 arg5, u8 arg6, u8 arg7)
{
    int i, j, offset;

    DmaCopy16(3, gMonSpritesGfxPtr->sprites.ptr[battlerPosition] + BG_SCREEN_SIZE * arg3, (void *)BG_SCREEN_ADDR(0) + arg5, BG_SCREEN_SIZE);
    offset = (arg5 >> 5) - (arg7 << 9);
    for (i = y; i < y + 8; i++)
    {
        for (j = x; j < x + 8; j++)
        {
            *((u16 *)(BG_VRAM) + (i * 32) + (j + (arg6 << 10))) = offset | (paletteId << 12);
            offset++;
        }
    }
}
