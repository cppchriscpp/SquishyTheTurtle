#include "main.h"
#include "helper_1.h"
#include "title.h"
#include "sprite.h"
#include "graphics/world_0_sprites.h"

#include <gb/gb.h>
#include <rand.h>

// TODO LIST
// 1. Figure out the write exception on every startup in bgb. (Saving grace: Real hardware doesn't seem to care.)
// 2. Title this thing. Stretchy the turtle??
// 5. Make it not look stupid when you grow next to a wall.

#define BANK_GRAPHICS 1U
#define BANK_WORLD_DATA 2U
#define BANK_HELPER_1 3U
#define BANK_TITLE 4U
#define BANK_SPRITE_DATA 5U

// This won't get confusing. Honest. I swear. &@#*!
UBYTE i, j;

UBYTE isMiniMode;
UBYTE temp1, temp2, temp3, temp4, temp5;
UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, spriteSize, gameState, playerVelocityLock, cycleCounter;
UBYTE playerHealth;
UBYTE buffer[20U];
UINT16 playerWorldTileStart, temp16;
UBYTE* currentMap;
UBYTE* tempPointer; 
// Pointers to pointers to pointers to pointers to pointers to pointers to pointers to pointers to pointers to pointers to...
UBYTE* * * currentMapSprites;

struct SPRITE sprites[6];
void init_vars() {
	isMiniMode = 1U;
	playerWorldPos = 0U;
	playerWorldTileStart = get_map_tile_base_position();
	currentMap = world_0;
	currentMapSprites = world_0_sprites;
	btns = oldBtns = 0U;
	playerXVel = playerYVel = 0U;
	spriteSize = 8U;
	
	playerX = playerY = 36U;
	playerHealth = 5U;
	gameState = GAME_STATE_RUNNING;
	playerVelocityLock = 0U;
}

void load_map() {
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);
	playerWorldTileStart = get_map_tile_base_position();
	
	// This is efficient. I swear! NOT AT ALL AWFUL. IN ANY WAY. NOPE.
	for (i = 0U; i != MAP_TILES_DOWN; i++) {
		for (j = 0U; j != MAP_TILES_ACROSS; j++) {
			buffer[j*2U] = currentMap[playerWorldTileStart + j] * 4U; // TODO: Bit shifts > multiplication. Our compiler's not smart enough to convert for us..
			buffer[j*2U+1U] = buffer[j*2U]+2U;
		}
		set_bkg_tiles(0U, i*2U, 20U, 1U, buffer);
		
		for (j = 0U; j != MAP_TILES_ACROSS*2; j++) {
			buffer[j]++;
		}
		set_bkg_tiles(0U, i*2U+1U, 20U, 1U, buffer);
		playerWorldTileStart += MAP_TILE_ROW_WIDTH;
	}
	playerWorldTileStart = get_map_tile_base_position(); // Clean up after yourself darnit!!
	
	
	SWITCH_ROM_MBC1(BANK_SPRITE_DATA);
	tempPointer = currentMapSprites[playerWorldPos];
	temp1 = 0x00; // Generic data
	temp2 = 0U; // Position
	while(temp2 != MAX_SPRITES) {
		temp1 = tempPointer++[0];
		if (temp1 == 255U)
			break;
		
		// Temp1 is our position.. convert to x/y
		sprites[temp2].x = ((temp1 % 10U) << 4U) + 12U; // Add 4 to place at center of tile, add 8 to deal with offset by 1.
		sprites[temp2].y = ((temp1 / 10U) << 4U) + 20U; // Add 16 so the first tile = 16, then add 4 to center within tile.
		sprites[temp2].size = 8U;

		sprites[temp2].type = tempPointer++[0];
				
		// Apply it to some real-world sprites too!
		// Leaving room for 4 sprite... sprites, but for now, we'll just use the first.
		set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2U), ENEMY_SPRITE_START + (sprites[temp2].type << 2U));
		move_sprite(WORLD_SPRITE_START + (temp2 << 2U), sprites[temp2].x, sprites[temp2].y);
		temp2++;
	}
	
	while (temp2 != MAX_SPRITES) {
		// Fill in the rest -- both in actual sprites and in our structs.
		for (i = 0U; i < 4U; i++)
			move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
		
		sprites[temp2].type = SPRITE_TYPE_NONE;
		sprites[temp2].x = sprites[temp2].y = SPRITE_OFFSCREEN;
		sprites[temp2].size = 0U;
		temp2++;
	}
}

void init_screen() {
	
	disable_interrupts();
	DISPLAY_OFF;
	
	SWITCH_ROM_MBC1(BANK_GRAPHICS);
	set_bkg_data(0U, 128U, base_tiles);
	set_win_data(0U, 128U, base_tiles);
	set_sprite_data(0U, 64U, base_sprites);
	scroll_bkg(0U, 0U);
	SPRITES_8x8;
	
	// Main char is first 4 sprites. (Though sometimse 1 will be used...)
	for (i = 0U; i < 4U; i++) 
		set_sprite_tile(i, i);
	
	load_map();

	SHOW_BKG;
	SHOW_SPRITES;

	move_win(0, 128);
	SHOW_WIN;
	
	DISPLAY_ON;
	enable_interrupts();
}

// Get the position of the top left corner of a room on the map.
// Shamelessly stolen from Isle Quest GB.
INT16 get_map_tile_base_position() {
	return ((playerWorldPos / 10U) * (MAP_TILE_ROW_WIDTH*MAP_TILE_ROW_HEIGHT)) + ((playerWorldPos % 10U) * MAP_TILES_ACROSS);
}

// Returns collision, and ALSO SETS temp3 to whatever was collided with.
UINT16 test_collision(UBYTE x, UBYTE y) {
	// This offsets us by one tile to get us in line with 0-7= tile 0, 8-f = tile 1, etc...
	x -= 8;
	temp16 = playerWorldTileStart + (MAP_TILE_ROW_WIDTH * (((UINT16)y>>4U) - 1U)) + (((UINT16)x)>>4U);
	temp3 = currentMap[temp16];
	
	if (temp3 == WALKABLE_LOG_TILE) {
		return !isMiniMode; // Tiny you can walk. Big you can't. GET STUCKED!!
	}
	if (temp3 > FIRST_WATER_TILE && temp3 < FIRST_LOG_TILE) {
		return isMiniMode;
	}
	if (temp3 > FIRST_SOLID_TILE - 1U) {
		return 1;
	}
	return 0;
}

void move_sprites() {
	temp1 = cycleCounter % MAX_SPRITES;
	if (sprites[temp1].type == SPRITE_TYPE_NONE)
		return;
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	directionalize_sprites();
	
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);
	if (sprites[temp1].direction == SPRITE_DIRECTION_STOP)
		return;
	temp4 = sprites[temp1].x;
	temp5 = sprites[temp1].y;

	switch (sprites[temp1].direction) {
		case SPRITE_DIRECTION_LEFT: 
			temp4 -= SPIDER_SPEED;
			break;
		case SPRITE_DIRECTION_RIGHT:
			temp4 += SPIDER_SPEED;
			break;
		case SPRITE_DIRECTION_UP:
			temp5 -= SPIDER_SPEED;
			break;
		case SPRITE_DIRECTION_DOWN:
			temp5 += SPIDER_SPEED;
			break;
	}
	
	// Now, we test collision with our temp4 and temp5
	
	sprites[temp1].x = temp4;
	sprites[temp1].y = temp5;
	
	set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U), ENEMY_SPRITE_START + (sprites[temp1].type << 2U) + ((sys_time & SPRITE_ANIM_INTERVAL) >> SPRITE_ANIM_SHIFT));
	move_sprite(WORLD_SPRITE_START + (temp1 << 2U), temp4, temp5);
	// TODO: Other 4 for larger sprites.
}

void main_game_loop() {
	
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	
	oldBtns = btns;
	btns = joypad();
	handle_input();
	
	move_sprites();
	
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);
	temp1 = playerX + playerXVel;
	temp2 = playerY + playerYVel;
	temp3 = 0U;
	if (playerXVel != 0) {
		if (temp1+spriteSize >= SCREEN_WIDTH) {
			playerX = 8U + PLAYER_MOVE_DISTANCE;
			playerWorldPos++;
			load_map();
			return;
		} else if (temp1 <= 4U) {
			playerX = SCREEN_WIDTH - spriteSize - PLAYER_MOVE_DISTANCE;
			playerWorldPos--;
			load_map();
			return;
		} else {
			if (playerXVel == PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1+spriteSize, temp2) || test_collision(temp1+spriteSize, temp2+spriteSize)) {
					temp1 = playerX;
				}
			} else {
				if (test_collision(temp1-1U, temp2) || test_collision(temp1-1U, temp2+spriteSize)) {
					temp1 = playerX;
				}
			}
		}
	}
	
	if (playerYVel != 0) {
		if (temp2+spriteSize >= SCREEN_HEIGHT) {
			playerY = spriteSize + PLAYER_MOVE_DISTANCE;
			playerWorldPos += 10U;
			load_map();
			return;
		} else if (temp2 <= 4U) {
			playerY = (SCREEN_HEIGHT - STATUS_BAR_HEIGHT) - PLAYER_MOVE_DISTANCE;
			playerWorldPos -= 10U;
			load_map();
			return;
		} else {
			if (playerYVel == PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1, temp2+spriteSize) || test_collision(temp1+spriteSize, temp2+spriteSize)) {
					temp2 = playerY;
				}
			} else {
				if (test_collision(temp1, temp2) || test_collision(temp1+spriteSize, temp2)) {
					temp2 = playerY;
				}
			}
		}
	}
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	do_player_movey_stuff();
	
	// Limit us to not-batnose-crazy speeds
	wait_vbl_done();
}

void main(void) {
	startOver:
	init_vars();
	
	SWITCH_ROM_MBC1(BANK_TITLE);
	show_title();
	initrand(sys_time);
	
	init_screen();
	
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	update_health();
	
	while(1) {
		switch (gameState) {
			case GAME_STATE_RUNNING:
				main_game_loop();
				break;
			case GAME_STATE_PAUSED:
				SWITCH_ROM_MBC1(BANK_HELPER_1);
				pause_loop();
				break;
			case GAME_STATE_GAME_OVER:
				SWITCH_ROM_MBC1(BANK_TITLE);
				show_game_over();
				goto startOver; // FREEDOM!!!!!!!! Start over.
		}
		cycleCounter++;
	}
}