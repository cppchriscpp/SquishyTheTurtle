#include <gb/gb.h>

#define SPRITE_SMALL 0
#define SPRITE_BIG 4

#define PLAYER_MAX_HEALTH 5U

#define BLANK_TILE 0U
#define HEART_TILE 96U

#define PLAYER_ANIM_INTERVAL 0x08U // %00000100
#define PLAYER_ANIM_SHIFT 3U
#define PLAYER_MOVE_DISTANCE 2U

#define MAP_TILES_ACROSS 10U
#define MAP_TILES_DOWN 8U
#define MAP_TILE_ROW_WIDTH 100U
#define MAP_TILE_ROW_HEIGHT 8U 

#define SCREEN_WIDTH 160U
#define SCREEN_HEIGHT 140U
#define STATUS_BAR_HEIGHT 16U

#define SPRITE_OFFSCREEN 0xEF

extern UBYTE base_tiles[];
extern UBYTE base_sprites[];
extern UBYTE world_0[];

extern UBYTE buffer[20U];

extern UINT16 sys_time;

INT16 get_map_tile_base_position();


// Yep... these are all globals. Limited memory, gameboy, poor compiler support for local vars, etc.
// If this is grossing you out, please stop thinking about it like C. Think about it like the assembly behind it,
// and also keep in mind that we're working in a very limited system. Tricks will be necessary.
extern UBYTE i, j;
extern UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, isMiniMode, spriteSize, playerHealth;