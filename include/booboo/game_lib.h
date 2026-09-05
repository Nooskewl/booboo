#ifndef BOOBOO_GAME_LIB_H
#define BOOBOO_GAME_LIB_H

#include "booboo.h"

#ifdef _WIN32
#ifdef BOOBOO_STATIC
#define BOOBOO_GAME_EXPORT
#else
#ifdef BOOBOO_GAME_BUILD
#define BOOBOO_GAME_EXPORT __declspec(dllexport)
#else
#define BOOBOO_GAME_EXPORT __declspec(dllimport)
#endif
#endif
#else
#ifdef BOOBOO_LIB_BUILD
#define BOOBOO_GAME_EXPORT __attribute__((visibility("default")))
#else
#define BOOBOO_GAME_EXPORT
#endif
#endif

enum GUI_Transition_Type {
	TRANSITION_NONE = 0,
	TRANSITION_ENLARGE,
	TRANSITION_SHRINK,
	TRANSITION_SLIDE,
	TRANSITION_SLIDE_VERTICAL
};

void BOOBOO_GAME_EXPORT start_lib_game();
void BOOBOO_GAME_EXPORT end_lib_game();
void BOOBOO_GAME_EXPORT game_lib_destroy_program(booboo::Program *prg);

extern bool is_3d;
void set_2d();
void set_3d();

void BOOBOO_GAME_EXPORT register_game_callbacks();
void BOOBOO_GAME_EXPORT unregister_game_callbacks();

/* These are for accessing assets in the black box */

struct Image {
	gfx::Image *image;
};

struct Image_Info {
	unsigned int image_id;
	std::map<int, Image *> images;
};

struct Font_Info {
	unsigned int font_id;
	std::map<int, gfx::TTF *> fonts;
};

struct Tilemap_Info {
	unsigned int tilemap_id;
	std::map<int, gfx::Tilemap *> tilemaps;
};

struct Sprite_Info {
	unsigned int sprite_id;
	std::map<int, gfx::Sprite *> sprites;
};

struct Shader_Info {
	unsigned int shader_id;
	std::map<int, gfx::Shader *> shaders;
};

struct Vertex_Buffer {
	float *v;
	int num_triangles;
	bool has_vbo;
	GLuint vbo;
};

struct Vertex_Buffer_Info {
	unsigned int vertex_buffer_id;
	std::map<int, Vertex_Buffer *> vertex_buffers;
};

struct Model {
	glm::mat4 mat;
	gfx::Model *model;
};

struct Model_Info {
	unsigned int model_id;
	std::map<int, Model *> models;
};

struct Billboard {
	double x;
	double y;
	double z;
	double w;
	double h;
	double tx;
	double ty;
	double tz;
	double sx;
	double sy;
	double unit;
	gfx::Image *image;
	gfx::Sprite *sprite;
};

struct Billboard_Info {
	unsigned int billboard_id;
	std::map<int, Billboard *> billboards;
};

class BooBoo_Widget;

struct Widget {
	BooBoo_Widget *widget;
	booboo::Variable *data;
};

struct Widget_Info
{
	int widget_id;
	std::map<int, Widget *> widgets;
};

Image_Info BOOBOO_GAME_EXPORT *image_info(booboo::Program *prg);
Font_Info BOOBOO_GAME_EXPORT *font_info(booboo::Program *prg);
Tilemap_Info BOOBOO_GAME_EXPORT *tilemap_info(booboo::Program *prg);
Sprite_Info BOOBOO_GAME_EXPORT *sprite_info(booboo::Program *prg);
Shader_Info BOOBOO_GAME_EXPORT *shader_info(booboo::Program *prg);
Vertex_Buffer_Info BOOBOO_GAME_EXPORT *vertex_buffer_info(booboo::Program *prg);
Model_Info BOOBOO_GAME_EXPORT *model_info(booboo::Program *prg);
Billboard_Info BOOBOO_GAME_EXPORT *billboard_info(booboo::Program *prg);
Widget_Info BOOBOO_GAME_EXPORT *widget_info(booboo::Program *prg);

#endif // BOOBOO_GAME_LIB_H
