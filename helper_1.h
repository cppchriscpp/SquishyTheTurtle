#ifndef HELPER_1_H
#define HELPER_1_H
#include <gb/gb.h>
	void handle_input();
	void update_health();
	void pause_loop();
	void do_player_movey_stuff();
	void directionalize_sprites();
	void test_sprite_collision();
	void clear_extra_sprites();
	void init_vars();
	void finish_init_screen();
	void update_egg();
	void test_for_egg();
	void move_sprites_for_load();
	UBYTE get_collision_with_temp3();
	void animate_exit();
	void write_map_to_memory();
	void move_enemy_sprite();
	void make_player_hurt_noise();
	void make_clear_level_noise();
	void make_egg_noise();
	void make_shrink_sound();
	void make_grow_sound();
	void make_pause_sound();
	void make_unpause_sound();
#endif