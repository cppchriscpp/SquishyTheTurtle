#ifndef SPRITE_H
#define SPRITE_H

#define WORLD_SPRITE_START 16U
#define ENEMY_SPRITE_START 12U

#define SPRITE_ANIM_INTERVAL 0x08U
#define SPRITE_ANIM_SHIFT 3U
#define MAX_SPRITES 6U

#define SPIDER_SPEED 2U

#define SPIDER_SPRITE 12U

enum SPRITE_TYPE {
	SPRITE_TYPE_SPIDER = 0U,
	SPRITE_TYPE_NONE = 250U
};

enum SPRITE_DIRECTION {
	SPRITE_DIRECTION_STOP = 0U,
	SPRITE_DIRECTION_LEFT = 1U,
	SPRITE_DIRECTION_RIGHT = 2U,
	SPRITE_DIRECTION_UP = 3U,
	SPRITE_DIRECTION_DOWN = 4U
};

struct SPRITE {
	UBYTE x;
	UBYTE y;
	UBYTE size;
	UBYTE type;
	enum SPRITE_DIRECTION direction;
};

#endif