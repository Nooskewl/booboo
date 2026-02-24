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

/* These are for accessing assets in the black box */

struct MML_Info {
	unsigned int mml_id;
	std::map<int, audio::MML *> mmls;
};

struct Sample_Info {
	unsigned int sample_id;
	std::map<int, audio::Sample *> samples;
};

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
	bool is_clone;
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

MML_Info BOOBOO_EXPORT *mml_info(booboo::Program *prg);
Sample_Info BOOBOO_EXPORT *sample_info(booboo::Program *prg);
Image_Info BOOBOO_EXPORT *image_info(booboo::Program *prg);
Font_Info BOOBOO_EXPORT *font_info(booboo::Program *prg);
Tilemap_Info BOOBOO_EXPORT *tilemap_info(booboo::Program *prg);
Sprite_Info BOOBOO_EXPORT *sprite_info(booboo::Program *prg);
Shader_Info BOOBOO_EXPORT *shader_info(booboo::Program *prg);
Vertex_Buffer_Info BOOBOO_EXPORT *vertex_buffer_info(booboo::Program *prg);
Model_Info BOOBOO_EXPORT *model_info(booboo::Program *prg);
Billboard_Info BOOBOO_EXPORT *billboard_info(booboo::Program *prg);
Widget_Info BOOBOO_EXPORT *widget_info(booboo::Program *prg);

#endif // BOOBOO_GAME_LIB_H
