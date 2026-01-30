#ifndef BOOBOO_GAME_LIB_H
#define BOOBOO_GAME_LIB_H

#include "booboo.h"

enum GUI_Transition_Type {
	TRANSITION_NONE = 0,
	TRANSITION_ENLARGE,
	TRANSITION_SHRINK,
	TRANSITION_SLIDE,
	TRANSITION_SLIDE_VERTICAL,
	TRANSITION_FADE
};

void BOOBOO_EXPORT start_lib_game();
void BOOBOO_EXPORT end_lib_game();
void BOOBOO_EXPORT game_lib_destroy_program(booboo::Program *prg);

extern bool is_3d;
void set_2d();
void set_3d();

void BOOBOO_EXPORT register_game_callbacks();
void BOOBOO_EXPORT unregister_game_callbacks();

#endif // BOOBOO_GAME_LIB_H
