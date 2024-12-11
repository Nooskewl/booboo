#ifndef BOOBOO_GAME_LIB_H
#define BOOBOO_GAME_LIB_H

#include "booboo.h"

enum GUI_Transition_Type {
	TRANSITION_ENLARGE,
	TRANSITION_SHRINK,
	TRANSITION_APPEAR,
	TRANSITION_SLIDE,
	TRANSITION_SLIDE_VERTICAL
};

void start_lib_game();
void end_lib_game();
void game_lib_destroy_program(booboo::Program *prg);

extern bool is_3d;
void set_2d();
void set_3d();

#endif // BOOBOO_GAME_LIB_H
