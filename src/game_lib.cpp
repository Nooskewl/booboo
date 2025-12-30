#include <shim5/shim5.h>
#include <shim5/shaders/glsl/default_vertex.h>
#include <shim5/shaders/glsl/default_textured_fragment.h>

#include <libutil/libutil.h>
using namespace noo;

#include "booboo/booboo.h"
#include "booboo/internal.h"
using namespace booboo;

#include "booboo/game_lib.h"

static GUI_Transition_Type transition_in_type = TRANSITION_ENLARGE;
static GUI_Transition_Type transition_out_type = TRANSITION_SHRINK;
static bool custom_projection_set = false;
static glm::mat4 custom_mv;
static glm::mat4 custom_proj;
static int my_scissor[4];
static bool custom_scissor_set;
static int my_viewport[4];
static bool custom_viewport_set;

extern bool quit;
extern Program *prg;

#define INFO_EXISTS(m, i) if (m.find(i) == m.end()) { \
	throw Error(std::string(__FUNCTION__) + ": " + "Invalid handle at " + get_error_info(prg)); \
}

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
	util::Size<int> size;
	bool created;
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

struct JSON_Info {
	unsigned int json_id;
	std::map<int, util::JSON *> jsons;
};

struct Vertex_Buffer {
	float *v;
	int num_triangles;
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
	Variable *data;
};

struct Widget_Info
{
	int widget_id;
	std::map<int, Widget *> widgets;
};

static MML_Info *mml_info(Program *prg)
{
	MML_Info *info = (MML_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.mml");
	if (info == nullptr) {
		info = new MML_Info;
		info->mml_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.mml", info);
	}
	return info;
}

static Sample_Info *sample_info(Program *prg)
{
	Sample_Info *info = (Sample_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.sample");
	if (info == nullptr) {
		info = new Sample_Info;
		info->sample_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.sample", info);
	}
	return info;
}

static Image_Info *image_info(Program *prg)
{
	Image_Info *info = (Image_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.image");
	if (info == nullptr) {
		info = new Image_Info;
		info->image_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.image", info);
	}
	return info;
}

static Font_Info *font_info(Program *prg)
{
	Font_Info *info = (Font_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.font");
	if (info == nullptr) {
		info = new Font_Info;
		info->font_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.font", info);
	}
	return info;
}

static Tilemap_Info *tilemap_info(Program *prg)
{
	Tilemap_Info *info = (Tilemap_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.tilemap");
	if (info == nullptr) {
		info = new Tilemap_Info;
		info->tilemap_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.tilemap", info);
	}
	return info;
}

static Sprite_Info *sprite_info(Program *prg)
{
	Sprite_Info *info = (Sprite_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.sprite");
	if (info == nullptr) {
		info = new Sprite_Info;
		info->sprite_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.sprite", info);
	}
	return info;
}

static Shader_Info *shader_info(Program *prg)
{
	Shader_Info *info = (Shader_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.shader");
	if (info == nullptr) {
		info = new Shader_Info;
		info->shader_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.shader", info);
	}
	return info;
}

static JSON_Info *json_info(Program *prg)
{
	JSON_Info *info = (JSON_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.json");
	if (info == nullptr) {
		info = new JSON_Info;
		info->json_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.json", info);
	}
	return info;
}

static Vertex_Buffer_Info *vertex_buffer_info(Program *prg)
{
	Vertex_Buffer_Info *info = (Vertex_Buffer_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.vertex_buffer");
	if (info == nullptr) {
		info = new Vertex_Buffer_Info;
		info->vertex_buffer_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.vertex_buffer", info);
	}
	return info;
}

static Model_Info *model_info(Program *prg)
{
	Model_Info *info = (Model_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.model");
	if (info == nullptr) {
		info = new Model_Info;
		info->model_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.model", info);
	}
	return info;
}

static Billboard_Info *billboard_info(Program *prg)
{
	Billboard_Info *info = (Billboard_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.billboard");
	if (info == nullptr) {
		info = new Billboard_Info;
		info->billboard_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.billboard", info);
	}
	return info;
}

static Widget_Info *widget_info(Program *prg)
{
	Widget_Info *info = (Widget_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.widget");
	if (info == nullptr) {
		info = new Widget_Info;
		info->widget_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.widget", info);
	}
	return info;
}

static bool miscfunc_inspect(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	char buf[1000];

	if (v[0].type == Token::NUMBER) {
		snprintf(buf, 1000, "%g", v[0].n);
	}
	else if (v[0].type == Token::SYMBOL) {
		Variable &var = get_variable(prg, v[0].i);
		if (IS_NUMBER(var)) {
			snprintf(buf, 1000, "%g", var.n);
		}
		else if (IS_STRING(var)) {
			snprintf(buf, 1000, "%s", var.s.c_str());
		}
		else if (IS_VECTOR(var)) {
			snprintf(buf, 1000, "-vector-");
		}
		else if (IS_MAP(var)) {
			snprintf(buf, 1000, "-map-");
		}
		else if (IS_FUNCTION(var)) {
			snprintf(buf, 1000, "-function-");
		}
		else if (IS_LABEL(var)) {
			snprintf(buf, 1000, "-label-");
		}
		else if (IS_POINTER(var)) {
			snprintf(buf, 1000, "-pointer-");
		}
	}
	else {
		strcpy(buf, "Unknown");
	}

	gui::popup("INSPECTOR", buf, gui::OK);

	return true;
}

static bool miscfunc_delay(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int millis = (int)as_number(prg, v[0]);
	SDL_Delay(millis);
	return true;
}

static Variable exprfunc_misc_get_ticks(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = SDL_GetTicks();

	return v1;
}

static Variable exprfunc_misc_get_args(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::VECTOR;

	for (int i = 0; i < shim::argc; i++) {
		Variable var;
		var.type = Variable::STRING;
		var.s = shim::argv[i];
		v1.v.push_back(var);
	}

	return v1;
}

static Variable exprfunc_misc_get_logic_rate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = shim::logic_rate;

	return v1;
}

static bool miscfunc_set_logic_rate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int rate = as_number(prg, v[0]);

	if (rate < 1 || rate > 1000) {
		throw Error(std::string(__FUNCTION__) + ": " + "Logic rate must be between 1 and 1000 at " + get_error_info(prg));
	}

	shim::logic_rate = rate;

	return true;
}

static Variable exprfunc_misc_file_list(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::VECTOR;

	std::vector<std::string> l = shim::cpa->get_all_filenames();

	for (size_t i = 0; i < l.size(); i++) {
		Variable var;
		var.type = Variable::STRING;
		var.s = l[i];
		v1.v.push_back(var);
	}

	return v1;
}

static bool gfxfunc_set_scissor(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int x = as_number(prg, v[0]);
	int y = as_number(prg, v[1]);
	int w = as_number(prg, v[2]);
	int h = as_number(prg, v[3]);

	gfx::set_scissor(x, y, w, h);

	custom_scissor_set = true;
	my_scissor[0] = x;
	my_scissor[1] = y;
	my_scissor[2] = w;
	my_scissor[3] = h;

	return true;
}

static bool gfxfunc_unset_scissor(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::unset_scissor();
	
	custom_scissor_set = false;

	return true;
}

static bool gfxfunc_set_viewport(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int x = as_number(prg, v[0]);
	int y = as_number(prg, v[1]);
	int w = as_number(prg, v[2]);
	int h = as_number(prg, v[3]);

	gfx::set_viewport(x, y, w, h);

	custom_viewport_set = true;
	my_viewport[0] = x;
	my_viewport[1] = y;
	my_viewport[2] = w;
	my_viewport[3] = h;

	return true;
}

static bool gfxfunc_unset_viewport(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::unset_viewport();

	custom_viewport_set = false;

	return true;
}

static bool gfxfunc_clear(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = 255;

	gfx::clear(c);

	return true;
}

static bool gfxfunc_flip(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::flip();

	return true;
}

static bool gfxfunc_resize(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)
	
	int w = as_number(prg, v[0]);
	int h = as_number(prg, v[1]);

	double aspect = (double)w/h;
	gfx::set_min_aspect_ratio(aspect-0.001f);
	gfx::set_max_aspect_ratio(aspect+0.001f);
	gfx::set_scaled_size({w, h});

	return true;
}

static Variable exprfunc_gfx_get_screen_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable vec;
	vec.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;
	var.n = shim::real_screen_size.w;
	vec.v.push_back(var);
	var.n = shim::real_screen_size.h;
	vec.v.push_back(var);

	return vec;
}

static Variable exprfunc_gfx_get_buffer_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable vec;
	vec.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;
	var.n = shim::screen_size.w;
	vec.v.push_back(var);
	var.n = shim::screen_size.h;
	vec.v.push_back(var);

	return vec;
}

static Variable exprfunc_gfx_get_screen_offset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable vec;
	vec.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;
	var.n = shim::screen_offset.x;
	vec.v.push_back(var);
	var.n = shim::screen_offset.y;
	vec.v.push_back(var);

	return vec;
}

static Variable exprfunc_gfx_get_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = shim::scale;

	return v1;
}

static bool gfxfunc_set_target(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	gfx::set_target_image(img);

	return true;
}

static bool gfxfunc_set_target_backbuffer(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::set_target_backbuffer();

	return true;
}

static bool gfxfunc_screen_shake(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	double amount = as_number(prg, v[0]);
	Uint32 duration = as_number(prg, v[1]);

	gfx::screen_shake(amount, duration);

	return true;
}

static bool gfxfunc_add_notification(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string s = as_string(prg, v[0]);

	gfx::add_notification(s);

	return true;
}

static Variable exprfunc_gfx_is_fullscreen(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = gfx::is_fullscreen();

	return v1;
}

static void gen_f11()
{
	TGUI_Event event;
	event.type = TGUI_KEY_DOWN;
	event.keyboard.is_repeat = false;
	event.keyboard.code = TGUIK_F11;
	event.keyboard.simulated = true;
	shim::push_event(event);
}

static bool gfxfunc_toggle_fullscreen(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gen_f11();

	return true;
}

static Variable exprfunc_gfx_get_refresh_rate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = shim::refresh_rate;

	return v1;
}

static bool gfxfunc_set_blend_mode(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	gfx::Blend_Mode src = (gfx::Blend_Mode)as_number(prg, v[0]);
	gfx::Blend_Mode dest = (gfx::Blend_Mode)as_number(prg, v[1]);

	gfx::set_blend_mode(src, dest);

	return true;
}

static bool gfxfunc_clear_depth_buffer(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	float val = as_number(prg, v[0]);

	gfx::clear_depth_buffer(val);

	return true;
}

static bool gfxfunc_clear_stencil_buffer(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int val = (int)as_number(prg, v[0]);

	gfx::clear_stencil_buffer(val);

	return true;
}

static bool gfxfunc_enable_depth_test(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool onoff = (bool)as_number(prg, v[0]);

	gfx::enable_depth_test(onoff);

	return true;
}

static bool gfxfunc_enable_depth_write(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool onoff = (bool)as_number(prg, v[0]);

	gfx::enable_depth_write(onoff);

	return true;
}

static bool gfxfunc_set_depth_mode(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int val = (int)as_number(prg, v[0]);

	gfx::set_depth_mode((gfx::Compare_Func)val);

	return true;
}

static bool gfxfunc_enable_stencil(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool onoff = (bool)as_number(prg, v[0]);

	gfx::enable_stencil(onoff);

	return true;
}

static bool gfxfunc_enable_two_sided_stencil(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool onoff = (bool)as_number(prg, v[0]);

	gfx::enable_two_sided_stencil(onoff);

	return true;
}

static bool gfxfunc_set_stencil_mode(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	int func = (int)as_number(prg, v[0]);
	int fail = (int)as_number(prg, v[1]);
	int zfail = (int)as_number(prg, v[2]);
	int pass = (int)as_number(prg, v[3]);
	int reference = (int)as_number(prg, v[4]);
	int mask = (int)as_number(prg, v[5]);

	gfx::set_stencil_mode((gfx::Compare_Func)func, (gfx::Stencil_Op)fail, (gfx::Stencil_Op)zfail, (gfx::Stencil_Op)pass, reference, mask);

	return true;
}

static bool gfxfunc_set_stencil_mode_backfaces(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	int func = (int)as_number(prg, v[0]);
	int fail = (int)as_number(prg, v[1]);
	int zfail = (int)as_number(prg, v[2]);
	int pass = (int)as_number(prg, v[3]);
	int reference = (int)as_number(prg, v[4]);
	int mask = (int)as_number(prg, v[5]);

	gfx::set_stencil_mode_backfaces((gfx::Compare_Func)func, (gfx::Stencil_Op)fail, (gfx::Stencil_Op)zfail, (gfx::Stencil_Op)pass, reference, mask);

	return true;
}

static bool gfxfunc_set_cull_mode(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int val = (int)as_number(prg, v[0]);

	gfx::set_cull_mode((gfx::Faces)val);

	return true;
}

static bool gfxfunc_enable_colour_write(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool val = (bool)as_number(prg, v[0]);

	gfx::enable_colour_write(val);

	return true;
}

static bool gfxfunc_set_projection(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &mv = as_variable(prg, v[0]);
	Variable &proj = as_variable(prg, v[1]);

	CHECK_VECTOR(mv)
	CHECK_VECTOR(proj)

	glm::mat4 glm_mv = to_glm_mat4(mv);
	glm::mat4 glm_proj = to_glm_mat4(proj);

	gfx::set_matrices(glm_mv, glm_proj);
	gfx::update_projection();

	custom_projection_set = true;
	custom_mv = glm_mv;
	custom_proj = glm_proj;

	return true;
}

static bool gfxfunc_set_default_projection(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	if (is_3d) {
		set_3d();
	}
	else {
		set_2d();
	}

	custom_projection_set = false;

	return true;
}

static bool gfxfunc_resize_vertex_cache(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int increase = as_number(prg, v[0]);

	gfx::Vertex_Cache::instance()->maybe_resize_cache(increase);

	return true;
}

static bool primfunc_start_primitives(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::draw_primitives_start();

	return true;
}

static bool primfunc_end_primitives(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::draw_primitives_end();

	return true;
}

static bool primfunc_line(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(8)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p1, p2;

	p1.x = as_number(prg, v[4]);
	p1.y = as_number(prg, v[5]);
	p2.x = as_number(prg, v[6]);
	p2.y = as_number(prg, v[7]);

	float thick;
	if (v.size() > 8) {
		thick = as_number(prg, v[8]);
	}
	else {
		thick = 1.0f;
	}

	gfx::draw_line(c, p1, p2, thick);

	return true;
}

static bool primfunc_triangle(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(10)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);
	double x1 = as_number(prg, v[4]);
	double y1 = as_number(prg, v[5]);
	double x2 = as_number(prg, v[6]);
	double y2 = as_number(prg, v[7]);
	double x3 = as_number(prg, v[8]);
	double y3 = as_number(prg, v[9]);

	double thick;
	if (v.size() > 10) {
		thick = as_number(prg, v[10]);
	}
	else {
		thick = 1.0f;
	}

	gfx::draw_triangle(c, util::Point<float>(x1, y1), util::Point<float>(x2, y2), util::Point<float>(x3, y3), thick);

	return true;
}

static bool primfunc_filled_triangle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(18)

	SDL_Color c[3];
	c[0].r = as_number(prg, v[0]);
	c[0].g = as_number(prg, v[1]);
	c[0].b = as_number(prg, v[2]);
	c[0].a = as_number(prg, v[3]);
	c[1].r = as_number(prg, v[4]);
	c[1].g = as_number(prg, v[5]);
	c[1].b = as_number(prg, v[6]);
	c[1].a = as_number(prg, v[7]);
	c[2].r = as_number(prg, v[8]);
	c[2].g = as_number(prg, v[9]);
	c[2].b = as_number(prg, v[10]);
	c[2].a = as_number(prg, v[11]);

	util::Point<float> p1, p2, p3;

	p1.x = as_number(prg, v[12]);
	p1.y = as_number(prg, v[13]);
	p2.x = as_number(prg, v[14]);
	p2.y = as_number(prg, v[15]);
	p3.x = as_number(prg, v[16]);
	p3.y = as_number(prg, v[17]);

	gfx::draw_filled_triangle(c, p1, p2, p3);

	return true;
}

static bool primfunc_rectangle(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(8)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;
	util::Size<float> sz;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);
	sz.w = as_number(prg, v[6]);
	sz.h = as_number(prg, v[7]);

	float thick;
	if (v.size() > 8) {
		thick = as_number(prg, v[8]);
	}
	else {
		thick = 1.0f;
	}

	gfx::draw_rectangle(c, p, sz, thick);

	return true;
}

static bool primfunc_filled_rectangle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(20)

	SDL_Color c[4];
	c[0].r = as_number(prg, v[0]);
	c[0].g = as_number(prg, v[1]);
	c[0].b = as_number(prg, v[2]);
	c[0].a = as_number(prg, v[3]);
	c[1].r = as_number(prg, v[4]);
	c[1].g = as_number(prg, v[5]);
	c[1].b = as_number(prg, v[6]);
	c[1].a = as_number(prg, v[7]);
	c[2].r = as_number(prg, v[8]);
	c[2].g = as_number(prg, v[9]);
	c[2].b = as_number(prg, v[10]);
	c[2].a = as_number(prg, v[11]);
	c[3].r = as_number(prg, v[12]);
	c[3].g = as_number(prg, v[13]);
	c[3].b = as_number(prg, v[14]);
	c[3].a = as_number(prg, v[15]);

	util::Point<float> p;

	p.x = as_number(prg, v[16]);
	p.y = as_number(prg, v[17]);

	util::Size<float> sz;

	sz.w = as_number(prg, v[18]);
	sz.h = as_number(prg, v[19]);

	gfx::draw_filled_rectangle(c, p, sz);

	return true;
}

static bool primfunc_ellipse(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(8)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);

	double rx = as_number(prg, v[6]);
	double ry = as_number(prg, v[7]);

	double thick;
	double sections;

	if (v.size() > 8) {
		thick = as_number(prg, v[8]);
	}
	else {
		thick = 1.0f;
	}

	if (v.size() > 9) {
		sections = as_number(prg, v[9]);
	}
	else {
		sections = -1;
	}

	gfx::draw_ellipse(c, p, rx, ry, thick, sections);

	return true;
}

static bool primfunc_filled_ellipse(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(8)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);

	double rx = as_number(prg, v[6]);
	double ry = as_number(prg, v[7]);
	double sections;

 	if (v.size() > 8) {
		sections = as_number(prg, v[8]);
	}
	else {
		sections = -1;
	}	

	gfx::draw_filled_ellipse(c, p, rx, ry, sections);

	return true;
}

static bool primfunc_circle(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(7)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);
	float r = as_number(prg, v[6]);

	double thick;
	double sections;

	if (v.size() > 7) {
		thick = as_number(prg, v[7]);
	}
	else {
		thick = 1.0f;
	}

	if (v.size() > 8) {
		sections = as_number(prg, v[8]);
	}
	else {
		sections = -1;
	}

	gfx::draw_circle(c, p, r, thick, sections);

	return true;
}

static bool primfunc_filled_circle(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(7)

	SDL_Color c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);
	double r = as_number(prg, v[6]);

	int sections;

	if (v.size() > 7) {
		sections = as_number(prg, v[7]);
	}
	else {
		sections = -1;
	}

	gfx::draw_filled_circle(c, p, r, sections);

	return true;
}

static Variable exprfunc_mml_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
       	v1.type = Variable::NUMBER;

	std::string str = as_string(prg, v[0]);
	
	MML_Info *info = mml_info(prg);

	v1.n = info->mml_id;

	Uint8 *bytes = (Uint8 *)str.c_str();
	SDL_IOStream *file = SDL_IOFromMem(bytes, str.length());

	try {
		audio::MML *mml = new audio::MML(file); // this closes the file
		info->mmls[info->mml_id++] = mml;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool mmlfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	MML_Info *info = mml_info(prg);
	INFO_EXISTS(info->mmls, id)
	delete info->mmls[id];
	info->mmls.erase(info->mmls.find(id));

	return true;
}

static Variable exprfunc_mml_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;
       	v1.type = Variable::NUMBER;

	std::string name = as_string(prg, v[0]);

	MML_Info *info = mml_info(prg);

	v1.n = info->mml_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		audio::MML *mml = new audio::MML(name, load_from_filesystem);
		info->mmls[info->mml_id++] = mml;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool mmlfunc_play(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int id = as_number(prg, v[0]);

	double volume;
	bool loop;

	if (v.size() > 1) {
		volume = as_number(prg, v[1]);
	}
	else {
		volume = 1.0;
	}

	if (v.size() > 2) {
		loop = as_number(prg, v[2]);
	}
	else {
		loop = false;
	}

	MML_Info *info = mml_info(prg);

	INFO_EXISTS(info->mmls, id)

	audio::MML *mml = info->mmls[id];

	mml->play(shim::music_volume*volume, loop);

	return true;
}

static bool mmlfunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	MML_Info *info = mml_info(prg);

	INFO_EXISTS(info->mmls, id)

	audio::MML *mml = info->mmls[id];

	mml->stop();

	return true;
}

static Variable exprfunc_sample_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	std::string name = as_string(prg, v[0]);

	Sample_Info *info = sample_info(prg);

	v1.n = info->sample_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		audio::Sample *sample = new audio::Sample(name, load_from_filesystem);
		info->samples[info->sample_id++] = sample;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool samplefunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Sample_Info *info = sample_info(prg);
	INFO_EXISTS(info->samples, id)
	delete info->samples[id];
	info->samples.erase(info->samples.find(id));

	return true;
}

static bool samplefunc_play(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int id = as_number(prg, v[0]);

	double volume;
	bool loop;

	if (v.size() > 1) {
		volume = as_number(prg, v[1]);
	}
	else {
		volume = 1.0;
	}

	if (v.size() > 2) {
		loop = as_number(prg, v[2]);
	}
	else {
		loop = false;
	}

	Sample_Info *info = sample_info(prg);

	INFO_EXISTS(info->samples, id)

	audio::Sample *sample = info->samples[id];

	sample->play(shim::music_volume * volume, loop);

	return true;
}

static bool samplefunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Sample_Info *info = sample_info(prg);

	INFO_EXISTS(info->samples, id)

	audio::Sample *sample = info->samples[id];

	sample->stop();

	return true;
}

static Variable exprfunc_image_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
	v1.type = Variable::NUMBER;

	int w = as_number(prg, v[0]);
	int h = as_number(prg, v[1]);

	Image_Info *info = image_info(prg);

	v1.n = info->image_id;

	try {
		gfx::Image *img = new gfx::Image(util::Size<int>(w, h));

		Image *i = new Image;
		i->image = img;
		i->created = true;
		i->size = util::Size<int>(w, h);

		info->images[info->image_id++] = i;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static Variable exprfunc_image_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	std::string name = as_string(prg, v[0]);

	Image_Info *info = image_info(prg);

	v1.n = info->image_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		gfx::Image *img = new gfx::Image(name, false, load_from_filesystem);

		Image *i = new Image;
		i->image = img;
		i->created = false;

		info->images[info->image_id++] = i;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool imagefunc_save(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string filename = as_string(prg, v[1]);

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	unsigned char *buf = gfx::Image::read_texture(img);

	if (filename.find(".tga") != std::string::npos) {
		gfx::Image::save_tga(filename, buf, img->size);
	}
	else {
		gfx::Image::save_png(filename, buf, img->size);
	}

	delete[] buf;

	return true;
}

static bool imagefunc_screenshot(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string filename = as_string(prg, v[0]);

	util::Size<int> size;

	bool include_black_bars = true;
	if (v.size() > 1) {
		include_black_bars = as_number(prg, v[1]);
	}

	unsigned char *buf = gfx::Image::read_backbuffer(include_black_bars, &size.w, &size.h);

	if (buf != nullptr) {
		if (filename.find(".tga") != std::string::npos) {
			gfx::Image::save_tga(filename, buf, size);
		}
		else {
			gfx::Image::save_png(filename, buf, size);
		}

		delete[] buf;
	}
	else {
		// Error!
	}

	return true;
}

static bool imagefunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Image_Info *info = image_info(prg);
	INFO_EXISTS(info->images, id)
	delete info->images[id]->image;
	info->images.erase(info->images.find(id));

	return true;
}

static bool imagefunc_draw(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(7)

	int id = as_number(prg, v[0]);
	double r = as_number(prg, v[1]);
	double g = as_number(prg, v[2]);
	double b = as_number(prg, v[3]);
	double a = as_number(prg, v[4]);
	double x = as_number(prg, v[5]);
	double y = as_number(prg, v[6]);

	double flip_h;
	double flip_v;

	if (v.size() > 7) {
		flip_h = as_number(prg, v[7]);
	}
	else {
		flip_h = false;
	}

	if (v.size() > 8) {
		flip_v = as_number(prg, v[8]);
	}
	else {
		flip_v = false;
	}

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	int flags = 0;
	if (flip_h != 0.0) {
		flags |= gfx::Image::FLIP_H;
	}
	if (flip_v != 0.0) {
		flags |= gfx::Image::FLIP_V;
	}

	img->draw_tinted(c, util::Point<float>(x, y), flags);

	return true;
}

static bool imagefunc_stretch_region(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(13)

	int id = as_number(prg, v[0]);
	double r = as_number(prg, v[1]);
	double g = as_number(prg, v[2]);
	double b = as_number(prg, v[3]);
	double a = as_number(prg, v[4]);
	double sx = as_number(prg, v[5]);
	double sy = as_number(prg, v[6]);
	double sw = as_number(prg, v[7]);
	double sh = as_number(prg, v[8]);
	double dx = as_number(prg, v[9]);
	double dy = as_number(prg, v[10]);
	double dw = as_number(prg, v[11]);
	double dh = as_number(prg, v[12]);

	double flip_h;
	double flip_v;

	if (v.size() > 13) {
		flip_h = as_number(prg, v[13]);
	}
	else {
		flip_h = false;
	}

	if (v.size() > 14) {
		flip_v = as_number(prg, v[14]);
	}
	else {
		flip_v = false;
	}
	
	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	int flags = 0;
	if (flip_h != 0.0) {
		flags |= gfx::Image::FLIP_H;
	}
	if (flip_v != 0.0) {
		flags |= gfx::Image::FLIP_V;
	}

	img->stretch_region_tinted(c, util::Point<float>(sx, sy), util::Size<float>(sw, sh), util::Point<float>(dx, dy), util::Size<float>(dw, dh), flags);

	return true;
}

static bool imagefunc_draw_rotated_scaled(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(12)

	int id = as_number(prg, v[0]);
	double r = as_number(prg, v[1]);
	double g = as_number(prg, v[2]);
	double b = as_number(prg, v[3]);
	double a = as_number(prg, v[4]);
	double cx = as_number(prg, v[5]);
	double cy = as_number(prg, v[6]);
	double x = as_number(prg, v[7]);
	double y = as_number(prg, v[8]);
	double angle = as_number(prg, v[9]);
	double scale_x = as_number(prg, v[10]);
	double scale_y = as_number(prg, v[11]);

	double flip_h;
	double flip_v;

	if (v.size() > 12) {
		flip_h = as_number(prg, v[12]);
	}
	else {
		flip_h = false;
	}

	if (v.size() > 13) {
		flip_v = as_number(prg, v[13]);
	}
	else {
		flip_v = false;
	}

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	int flags = 0;
	if (flip_h != 0.0) {
		flags |= gfx::Image::FLIP_H;
	}
	if (flip_v != 0.0) {
		flags |= gfx::Image::FLIP_V;
	}

	img->draw_tinted_rotated_scaledxy(c, util::Point<float>(cx, cy), util::Point<float>(x, y), angle, scale_x, scale_y, flags);

	return true;
}

static bool imagefunc_start(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double img = as_number(prg, v[0]);

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, img)

	gfx::Image *image = info->images[img]->image;

	image->start_batch();

	return true;
}

static bool imagefunc_end(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double img = as_number(prg, v[0]);

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, img)

	gfx::Image *image = info->images[img]->image;

	image->end_batch();

	return true;
}

static Variable exprfunc_image_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable vec;
	vec.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;
	
	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	var.n = img->size.w;
	vec.v.push_back(var);
	var.n = img->size.h;
	vec.v.push_back(var);

	return vec;
}

static bool imagefunc_draw_9patch(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	int id = as_number(prg, v[0]);
	int r = as_number(prg, v[1]);
	int g = as_number(prg, v[2]);
	int b = as_number(prg, v[3]);
	int a = as_number(prg, v[4]);
	double x = as_number(prg, v[5]);
	double y = as_number(prg, v[6]);
	int w = as_number(prg, v[7]);
	int h = as_number(prg, v[8]);

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	gfx::draw_9patch_tinted(c, img, util::Point<float>(x, y), util::Size<int>(w, h));


	return true;
}

static Variable exprfunc_image_read_texture(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable v1;
	v1.type = Variable::VECTOR;

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	unsigned char *pixels = gfx::Image::read_texture(img);

	for (int y = 0; y < img->size.h; y++) {
		Variable var;
		var.type = Variable::VECTOR;
		for (int x = 0; x < img->size.w; x++) {
			Variable var2;
			var2.type = Variable::VECTOR;
			Variable var3;
			var3.type = Variable::NUMBER;
			var3.n = pixels[(img->size.h-y-1)*img->size.w*4+x*4+0];
			var2.v.push_back(var3);
			var3.n = pixels[(img->size.h-y-1)*img->size.w*4+x*4+1];
			var2.v.push_back(var3);
			var3.n = pixels[(img->size.h-y-1)*img->size.w*4+x*4+2];
			var2.v.push_back(var3);
			var3.n = pixels[(img->size.h-y-1)*img->size.w*4+x*4+3];
			var2.v.push_back(var3);
			var.v.push_back(var2);
		}
		v1.v.push_back(var);
	}

	delete[] pixels;

	return v1;
}

static Variable exprfunc_image_to_texture(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	Variable &v2 = as_variable(prg, v[0]);

	Image_Info *info = image_info(prg);

	v1.n = info->image_id;

	int w = v2.v.size();
	int h = v2.v[0].v.size();

	unsigned char *pixels = new unsigned char[w*h*4];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int yy = h-y-1;
			pixels[yy*w*4+x*4+0] = v2.v[y].v[x].v[0].n;
			pixels[yy*w*4+x*4+1] = v2.v[y].v[x].v[1].n;
			pixels[yy*w*4+x*4+2] = v2.v[y].v[x].v[2].n;
			pixels[yy*w*4+x*4+3] = v2.v[y].v[x].v[3].n;
		}
	}

	gfx::Image *img = new gfx::Image(util::Size<int>(w, h), pixels);

	delete[] pixels;

	Image *i = new Image;
	i->image = img;
	i->created = true;
	i->size = util::Size<int>(w, h);

	info->images[info->image_id++] = i;

	return v1;
}

static bool imagefunc_update(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	Variable &v1 = as_variable(prg, v[1]);

	Image_Info *info = image_info(prg);

	INFO_EXISTS(info->images, id)

	gfx::Image *img = info->images[id]->image;

	int w = v1.v.size();
	int h = v1.v[0].v.size();

	unsigned char *pixels = new unsigned char[w*h*4];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			pixels[(h-y-1)*w*4+x*4+0] = v1.v[y].v[x].v[0].n;
			pixels[(h-y-1)*w*4+x*4+1] = v1.v[y].v[x].v[1].n;
			pixels[(h-y-1)*w*4+x*4+2] = v1.v[y].v[x].v[2].n;
			pixels[(h-y-1)*w*4+x*4+3] = v1.v[y].v[x].v[3].n;
		}
	}

	img->update(pixels);

	delete[] pixels;

	return true;
}

static Variable exprfunc_font_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(3)

	std::string name = as_string(prg, v[0]);
	int size = as_number(prg, v[1]);
	bool smooth = as_number(prg, v[2]);

	Variable v1;
	v1.type = Variable::NUMBER;

	Font_Info *info = font_info(prg);

	v1.n = info->font_id;

	bool load_from_filesystem = false;
	if (v.size() > 3) {
		load_from_filesystem = as_number(prg, v[3]);
	}

	try {
		gfx::TTF *font = new gfx::TTF(name, size, 1024, load_from_filesystem);
		font->set_smooth(smooth);
		info->fonts[info->font_id++] = font;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool fontfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Font_Info *info = font_info(prg);
	INFO_EXISTS(info->fonts, id)
	delete info->fonts[id];
	info->fonts.erase(info->fonts.find(id));

	return true;
}

static bool fontfunc_draw(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(8)

	int id = as_number(prg, v[0]);
	double r = as_number(prg, v[1]);
	double g = as_number(prg, v[2]);
	double b = as_number(prg, v[3]);
	double a = as_number(prg, v[4]);
	std::string text = as_string(prg, v[5]);
	double x = as_number(prg, v[6]);
	double y = as_number(prg, v[7]);
	bool rtl;

	if (v.size() > 8) {
		rtl = as_number(prg, v[8]);
	}
	else {
		rtl = false;
	}

	Font_Info *info = font_info(prg);

	INFO_EXISTS(info->fonts, id)

	gfx::TTF *font = info->fonts[id];

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	font->draw(c, text, util::Point<float>(x, y), true, false, true, rtl);

	return true;
}

static Variable exprfunc_font_width(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string text = as_string(prg, v[1]);
	
	Font_Info *info = font_info(prg);

	INFO_EXISTS(info->fonts, id)

	gfx::TTF *font = info->fonts[id];

	int w = font->get_text_width(text);

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = w;

	return v1;
}

static Variable exprfunc_font_height(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	
	Font_Info *info = font_info(prg);

	INFO_EXISTS(info->fonts, id)

	gfx::TTF *font = info->fonts[id];

	int h = font->get_height();

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = h;

	return v1;
}

static bool fontfunc_add_extra_glyph(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	int glyph_id = as_number(prg, v[1]);
	int image_id = as_number(prg, v[2]);
	
	Font_Info *info = font_info(prg);
	INFO_EXISTS(info->fonts, id)
	gfx::TTF *font = info->fonts[id];

	Image_Info *iinfo = image_info(prg);
	INFO_EXISTS(iinfo->images, image_id)
	gfx::Image *image = iinfo->images[image_id]->image;

	font->add_extra_glyph(glyph_id, image);

	return true;
}

static bool tilemapfunc_set_tile_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	shim::tile_size = as_number(prg, v[0]);

	return true;
}

static Variable exprfunc_tilemap_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	std::string name = as_string(prg, v[0]);
	
	Tilemap_Info *info = tilemap_info(prg);

	v1.n = info->tilemap_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		gfx::Tilemap *tilemap = new gfx::Tilemap(name, load_from_filesystem);
		info->tilemaps[info->tilemap_id++] = tilemap;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool tilemapfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Tilemap_Info *info = tilemap_info(prg);
	INFO_EXISTS(info->tilemaps, id)
	delete info->tilemaps[id];
	info->tilemaps.erase(info->tilemaps.find(id));

	return true;
}

static bool tilemapfunc_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int id = as_number(prg, v[0]);
	int start_layer = as_number(prg, v[1]);
	int end_layer = as_number(prg, v[2]);
	double x = as_number(prg, v[3]);
	double y = as_number(prg, v[4]);
	
	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];
	
	tilemap->draw(start_layer, end_layer, util::Point<float>(x, y));

	return true;
}

static Variable exprfunc_tilemap_num_layers(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable v1;
	v1.type = Variable::NUMBER;
	
	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	v1.n = tilemap->get_num_layers();

	return v1;
}

static Variable exprfunc_tilemap_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	util::Size<int> sz = tilemap->get_size();

	Variable vec;
	vec.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;
	var.n = sz.w;
	vec.v.push_back(var);
	var.n = sz.h;
	vec.v.push_back(var);

	return vec;
}

static Variable exprfunc_tilemap_is_solid(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	int x = as_number(prg, v[1]);
	int y = as_number(prg, v[2]);
	
	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = tilemap->is_solid(-1, util::Point<int>(x, y));

	return v1;
}

static Variable exprfunc_tilemap_get_groups(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable vec;
	vec.type = Variable::VECTOR;
	
	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	std::vector<gfx::Tilemap::Group> &groups = tilemap->get_groups();

	for (size_t i = 0; i < groups.size(); i++) {
		gfx::Tilemap::Group &g = groups[i];
		Variable v;
		v.type = Variable::VECTOR;
		Variable type;
		type.type = Variable::NUMBER;
		type.n = g.type;
		v.v.push_back(type);
		Variable x;
		x.type = Variable::NUMBER;
		x.n = g.x;
		v.v.push_back(x);
		Variable y;
		y.type = Variable::NUMBER;
		y.n = g.y;
		v.v.push_back(y);
		Variable w;
		w.type = Variable::NUMBER;
		w.n = g.w;
		v.v.push_back(w);
		Variable h;
		h.type = Variable::NUMBER;
		h.n = g.h;
		v.v.push_back(h);
		vec.v.push_back(v);
	}

	return vec;
}

static bool tilemapfunc_set_animated_tiles(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int id = as_number(prg, v[0]);
	int delay = (int)as_number(prg, v[1]);
	int w = (int)as_number(prg, v[2]);
	int h = (int)as_number(prg, v[3]);
	Variable v1 = as_variable_resolve(prg, v[4]);
	
	CHECK_VECTOR(v1)

	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];
	
	gfx::Tilemap::Animation_Data anim;
	anim.topleft.x = v1.v[0].v[0].n;
	anim.topleft.y = v1.v[0].v[1].n;
	anim.delay = delay;
	anim.size.w = w;
	anim.size.h = h;

	for (size_t i = 1; i < v1.v.size(); i++) {
		util::Point<int> p;
		p.x = v1.v[i].v[0].n;
		p.y = v1.v[i].v[1].n;
		anim.frames.push_back(p);
	}

	tilemap->add_animation_data(anim);

	return true;
}

static Variable exprfunc_tilemap_find_path(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	int id = as_number(prg, v[0]);
	Variable entity_solids = as_variable_resolve(prg, v[1]);
	int start_x = (int)as_number(prg, v[2]);
	int start_y = (int)as_number(prg, v[3]);
	int end_x = (int)as_number(prg, v[4]);
	int end_y = (int)as_number(prg, v[5]);

	CHECK_VECTOR(entity_solids)	

	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	std::vector< util::Rectangle<int> > entity_rects;
	for (size_t i = 0; i < entity_solids.v.size(); i++) {
		int x = entity_solids.v[i].v[0].n;
		int y = entity_solids.v[i].v[1].n;
		util::Rectangle<int> r;
		r.pos.x = x;
		r.pos.y = y;
		r.size.w = 1;
		r.size.h = 1;
		entity_rects.push_back(r);
	}

	util::A_Star *a_star = new util::A_Star(tilemap, entity_rects);	
	
	std::list<util::A_Star::Node> path = a_star->find_path(util::Point<int>(start_x, start_y), util::Point<int>(end_x, end_y));

	Variable v1;
	v1.type = Variable::VECTOR;

	for (std::list<util::A_Star::Node>::iterator it = path.begin(); it != path.end(); it++) {
		util::A_Star::Node &n = *it;
		Variable vec;
		vec.type = Variable::VECTOR;
		Variable x;
		x.type = Variable::NUMBER;
		x.n = n.position.x;
		vec.v.push_back(x);
		Variable y;
		y.type = Variable::NUMBER;
		y.n = n.position.y;
		vec.v.push_back(y);
		v1.v.push_back(vec);
	}

	delete a_star;

	return v1;
}

static bool tilemapfunc_set_solid(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int id = as_number(prg, v[0]);
	int x = as_number(prg, v[1]);
	int y = as_number(prg, v[2]);
	bool solid = as_number(prg, v[3]);
	
	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	tilemap->set_solid(-1, util::Point<int>(x, y), util::Size<int>(1, 1), solid);

	return true;
}

static bool tilemapfunc_set_tile(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(7)

	int id = as_number(prg, v[0]);
	int layer = as_number(prg, v[1]);
	int x = as_number(prg, v[2]);
	int y = as_number(prg, v[3]);
	int tile_x = as_number(prg, v[4]);
	int tile_y = as_number(prg, v[5]);
	bool solid = as_number(prg, v[6]);
	
	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	tilemap->set_tile(layer, util::Point<int>(x, y), util::Point<int>(tile_x, tile_y), solid);

	return true;
}

static Variable exprfunc_tilemap_get_tile(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int id = as_number(prg, v[0]);
	int layer = as_number(prg, v[1]);
	int x = as_number(prg, v[2]);
	int y = as_number(prg, v[3]);

	Variable vx;
	vx.type = Variable::NUMBER;
	Variable vy;
	vy.type = Variable::NUMBER;
	Variable vs;
	vs.type = Variable::NUMBER;

	Tilemap_Info *info = tilemap_info(prg);

	INFO_EXISTS(info->tilemaps, id)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	util::Point<int> tile_xy;
	bool solid;

	tilemap->get_tile(layer, util::Point<int>(x, y), tile_xy, solid);

	vx.n = tile_xy.x;
	vy.n = tile_xy.y;
	vs.n = solid;

	Variable vec;
	vec.type = Variable::VECTOR;
	vec.v.push_back(vx);
	vec.v.push_back(vy);
	vec.v.push_back(vs);

	return vec;
}

static Variable exprfunc_sprite_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	std::string name = as_string(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	v1.n = info->sprite_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		gfx::Sprite *sprite = new gfx::Sprite(name, name, load_from_filesystem);
		info->sprites[info->sprite_id++] = sprite;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool spritefunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Sprite_Info *info = sprite_info(prg);
	INFO_EXISTS(info->sprites, id)
	delete info->sprites[id];
	info->sprites.erase(info->sprites.find(id));

	return true;
}

static bool spritefunc_set_animation_lazy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string anim = as_string(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);
	
	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	sprite->set_animation_lazy(anim);

	return true;
}

struct Sprite_Callback_Data
{
	Program *prg;
	int function;
	int id;
};

static void sprite_callback(void *data)
{
	Sprite_Callback_Data *d = static_cast<Sprite_Callback_Data *>(data);
	std::vector<Token> v;
	Token t;
	t.type = Token::NUMBER;
	t.n = d->id;
	t.dereference = false;
	v.push_back(t);
	call_void_function(d->prg, d->function, v, 0);
	delete d;
}

static bool spritefunc_set_animation(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string anim = as_string(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);
	
	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	if (v.size() > 2) {
		Sprite_Callback_Data *d = new Sprite_Callback_Data;
		d->prg = prg;
		d->function = as_function(prg, v[2]);
		d->id = id;
		sprite->set_animation(anim, sprite_callback, d);
	}
	else {
		sprite->set_animation(anim);
	}

	return true;
}

static Variable exprfunc_sprite_get_animation(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable v1;
	v1.type = Variable::STRING;

	Sprite_Info *info = sprite_info(prg);
	
	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	v1.s = sprite->get_animation();

	return v1;
}

static Variable exprfunc_sprite_get_previous_animation(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable v1;
	v1.type = Variable::STRING;

	Sprite_Info *info = sprite_info(prg);
	
	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	v1.s = sprite->get_previous_animation();

	return v1;
}

static Variable exprfunc_sprite_current_frame(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable v1;
	v1.type = Variable::NUMBER;

	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_current_frame();

	return v1;
}

static Variable exprfunc_sprite_num_frames(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable v1;
	v1.type = Variable::NUMBER;

	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_num_frames();

	return v1;
}

static Variable exprfunc_sprite_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable v1;
	v1.type = Variable::NUMBER;

	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_length();

	return v1;
}

static Variable exprfunc_sprite_current_frame_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	gfx::Image *img = sprite->get_current_image();

	Variable vec;
	vec.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;
	var.n = img->size.w;
	vec.v.push_back(var);
	var.n = img->size.h;
	vec.v.push_back(var);

	return vec;
}

static bool spritefunc_draw(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(7)

	int id = as_number(prg, v[0]);
	int r = as_number(prg, v[1]);
	int g = as_number(prg, v[2]);
	int b = as_number(prg, v[3]);
	int a = as_number(prg, v[4]);
	double dx = as_number(prg, v[5]);
	double dy = as_number(prg, v[6]);

	int flip_h;
	int flip_v;

	if (v.size() > 7) {
		flip_h = as_number(prg, v[7]);
	}
	else {
		flip_h = false;
	}

	if (v.size() > 8) {
		flip_v = as_number(prg, v[8]);
	}
	else {
		flip_v = false;
	}
	
	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	gfx::Image *img = sprite->get_current_image();

	int flags = 0;
	if (flip_h != 0.0) {
		flags |= gfx::Image::FLIP_H;
	}
	if (flip_v != 0.0) {
		flags |= gfx::Image::FLIP_V;
	}

	SDL_Color tint;
	tint.r = r;
	tint.g = g;
	tint.b = b;
	tint.a = a;

	img->draw_tinted(tint, util::Point<float>(dx, dy), flags);

	return true;
}

static bool spritefunc_start(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	sprite->start();

	return true;
}

static bool spritefunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	sprite->stop();

	return true;
}

static bool spritefunc_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	sprite->reset();

	return true;
}

static Variable exprfunc_sprite_bounds(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	util::Point<int> topleft;
	util::Point<int> bottomright;

	sprite->get_bounds(topleft, bottomright);

	Variable vec;
	vec.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;
	var.n = topleft.x;
	vec.v.push_back(var);
	var.n = topleft.y;
	vec.v.push_back(var);
	var.n = bottomright.x;
	vec.v.push_back(var);
	var.n = bottomright.y;
	vec.v.push_back(var);

	return vec;
}

static Variable exprfunc_sprite_elapsed(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable v1;
	v1.type = Variable::NUMBER;
	
	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_elapsed();

	return v1;
}

static Variable exprfunc_sprite_frame_times(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	std::vector<Uint32> times = sprite->get_frame_times();

	Variable vec;
	vec.type = Variable::VECTOR;

	for (size_t i = 0; i < times.size(); i++) {
		Variable v;
		v.type = Variable::NUMBER;
		v.n = times[i];
		vec.v.push_back(v);
	}

	return vec;
}

static Variable exprfunc_sprite_is_started(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable v1;
	v1.type = Variable::NUMBER;

	Sprite_Info *info = sprite_info(prg);

	INFO_EXISTS(info->sprites, id)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->is_started();

	return v1;
}

static Variable exprfunc_joy_count(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = input::get_num_joysticks();

	return v1;
}

static bool joyfunc_rumble(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int ms = as_number(prg, v[0]);
	int num = -1;

	if (v.size() > 1) {
		num = as_number(prg, v[1]);
	}

	input::rumble(ms, num);

	return true;
}

static Variable exprfunc_joy_get_button(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int index = as_number(prg, v[0]);
	int n = as_number(prg, v[1]);

	Variable v1;
	v1.type = Variable::NUMBER;

	SDL_JoystickID id = input::get_controller_id(index);
	SDL_Gamepad *gc = input::get_sdl_gamepad(id);
	v1.n = SDL_GetGamepadButton(gc, (SDL_GamepadButton)n);

	return v1;
}

static Variable exprfunc_joy_get_axis(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int index = as_number(prg, v[0]);
	int n = as_number(prg, v[1]);

	Variable v1;
	v1.type = Variable::NUMBER;

	SDL_JoystickID id = input::get_controller_id(index);
	SDL_Gamepad *gc = input::get_sdl_gamepad(id);
	v1.n = SDL_GetGamepadAxis(gc, (SDL_GamepadAxis)n);

	if (v1.n < 0) {
		v1.n /= 32768.0f;
	}
	else {
		v1.n /= 32767;
	}

	return v1;
}

static Variable exprfunc_shader_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	std::string vname = as_string(prg, v[0]);
	std::string fname = as_string(prg, v[1]);
	
	Shader_Info *info = shader_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = info->shader_id;

	gfx::Shader *shader = nullptr;

	bool load_from_filesystem = false;
	if (v.size() > 2) {
		load_from_filesystem = as_number(prg, v[2]);
	}

	std::string vs, fs;
	std::string (*load_text)(std::string filename);
	if (load_from_filesystem) {
		load_text = util::load_text_from_filesystem;
	}
	else {
		load_text = util::load_text;
	}
	if (vname == "") {
		vs = DEFAULT_GLSL_VERTEX_SHADER;
	}
	else {
		vs = load_text("gfx/shaders/glsl/" + vname + ".txt");
	}
	if (fname == "") {
		fs = DEFAULT_GLSL_TEXTURED_FRAGMENT_SHADER;
	}
	else {
		fs = load_text("gfx/shaders/glsl/" + fname + ".txt");
	}

	try {
		gfx::Shader::OpenGL_Shader *vert = gfx::Shader::load_opengl_vertex_shader(vs, gfx::Shader::HIGH);
		gfx::Shader::OpenGL_Shader *frag = gfx::Shader::load_opengl_fragment_shader(fs, gfx::Shader::HIGH);

		shader = new gfx::Shader(vert, frag);

		info->shaders[info->shader_id++] = shader;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool shaderfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	delete info->shaders[id];
	info->shaders.erase(info->shaders.find(id));

	return true;
}

static bool shaderfunc_use(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	shim::current_shader = shader;
	shim::current_shader->use();
	gfx::update_projection();

	return true;
}

static bool shaderfunc_use_default(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	shim::current_shader = shim::default_shader;
	shim::current_shader->use();
	gfx::update_projection();

	return true;
}

static bool shaderfunc_set_bool(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	bool b = as_number(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	shader->set_bool(name, b);

	return true;
}

static bool shaderfunc_set_int(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	int i = as_number(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	shader->set_int(name, i);

	return true;
}

static bool shaderfunc_set_float(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	double f = as_number(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	shader->set_float(name, f);

	return true;
}

static bool shaderfunc_set_texture(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	double t = as_number(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	Image_Info *info2 = image_info(prg);
	INFO_EXISTS(info2->images, t)
	gfx::Image *img = info2->images[t]->image;

	shader->set_texture(name, img);

	return true;
}

static bool shaderfunc_set_float_vector(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	Variable &vec = as_variable(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	std::vector<float> floats;
	for (size_t i = 0; i < vec.v.size(); i++) {
		floats.push_back(vec.v[i].n);
	}

	shader->set_float_vector(name, floats.size(), &floats[0], 1);

	return true;
}

static bool shaderfunc_set_matrix(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	Variable &vec = as_variable(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	glm::mat4 mat;

	for (size_t i = 0; i < vec.v.size(); i++) {
		for (size_t j = 0; j < vec.v[i].v.size(); j++) {
			mat[i][j] = vec.v[i].v[j].n;
		}
	}

	shader->set_matrix(name, mat);

	return true;
}

static bool shaderfunc_set_matrix_array(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	Variable &vec = as_variable(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	glm::mat4 mat[vec.v.size()];

	for (size_t i = 0; i < vec.v.size(); i++) {
		for (size_t j = 0; j < vec.v[i].v.size(); j++) {
			for (size_t k = 0; k < vec.v[i].v[j].v.size(); k++) {
				mat[i][j][k] = vec.v[i].v[j].v[k].n;
			}
		}
	}

	shader->set_matrix_array(name, vec.v.size(), mat);

	return true;
}

static bool shaderfunc_set_colour(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	int r = as_number(prg, v[2]);
	int g = as_number(prg, v[3]);
	int b = as_number(prg, v[4]);
	int a = as_number(prg, v[5]);
	
	Shader_Info *info = shader_info(prg);
	INFO_EXISTS(info->shaders, id)
	gfx::Shader *shader = info->shaders[id];

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	shader->set_colour(name, c);

	return true;
}

static Variable exprfunc_json_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string name = as_string(prg, v[0]);

	JSON_Info *info = json_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = info->json_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		util::JSON *json = new util::JSON(name, load_from_filesystem);

		info->jsons[info->json_id++] = json;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool jsonfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	delete info->jsons[id];
	info->jsons.erase(info->jsons.find(id));

	return true;
}

static Variable exprfunc_json_get_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	Variable v1;
	v1.type = Variable::STRING;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.s = n->as_string();

	return v1;
}

static Variable exprfunc_json_get_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	Variable v1;
	v1.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.n = n->as_double();

	return v1;
}

static Variable exprfunc_json_get_bool(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	Variable v1;
	v1.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.n = n->as_bool();

	return v1;
}

static bool jsonfunc_set_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	std::string val = as_string(prg, v[2]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	util::JSON::Node *n = json->get_root();
	n->add_nested_string(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_set_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	double val = as_number(prg, v[2]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	util::JSON::Node *n = json->get_root();
	n->add_nested_double(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_set_bool(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	bool val = (bool)as_number(prg, v[2]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	util::JSON::Node *n = json->get_root();
	n->add_nested_bool(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_remove(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	json->remove(name);

	return true;
}

static Variable exprfunc_json_save(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string fn = as_string(prg, v[1]);
	
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	util::JSON *json = info->jsons[id];

	Variable var;
	var.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root();
	std::string s = n->to_json();

	FILE *f = fopen(fn.c_str(), "w");
	if (f == nullptr) {
		var.n = 0;
	}
	else {
		fprintf(f, "%s", s.c_str());
		fclose(f);
		var.n = 1;
	}

	return var;
}

bool is_3d = false;

void set_3d()
{
	double aspect = shim::screen_size.w / (double)shim::screen_size.h;
	glm::mat4 _proj = glm::perspective(float(M_PI/4.0f), (float)aspect, 0.1f, 10000.0f);

	glm::mat4 _mv;

	gfx::set_matrices(_mv, _proj);
	gfx::update_projection();

	is_3d = true;
	
	custom_projection_set = false;
}

void set_2d()
{
	gfx::set_default_projection(shim::screen_size, shim::screen_offset, shim::scale);
	gfx::update_projection();

	gfx::apply_screen_shake();

	is_3d = false;

	custom_projection_set = false;
}

static Variable exprfunc_model_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string name = as_string(prg, v[0]);

	Model_Info *info = model_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = info->model_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		gfx::Model *model = new gfx::Model(name, load_from_filesystem);

		Model *m = new Model;
		m->mat = glm::mat4();
		m->model = model;
		m->is_clone = false;

		info->models[info->model_id++] = m;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool modelfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, id)
	if (info->models[id]->is_clone == false) {
		delete info->models[id]->model;
	}
	delete info->models[id];
	info->models.erase(info->models.find(id));

	return true;
}

static bool modelfunc_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int model_id = as_number(prg, v[0]);
	int r = as_number(prg, v[1]);
	int g = as_number(prg, v[2]);
	int b = as_number(prg, v[3]);
	int a = as_number(prg, v[4]);

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	glm::mat4 save_mv, save_p;
	gfx::get_matrices(save_mv, save_p);

	glm::mat4 t = save_mv;
	//huh
	t = t * model->mat;
	//t = model->mat * t;

	gfx::set_matrices(t, save_p);
	gfx::update_projection();

	gfx::enable_depth_write(true);
	gfx::enable_depth_test(true);

	model->model->draw_tinted_textured(c);

	gfx::enable_depth_test(false);
	gfx::enable_depth_write(false);

	gfx::set_matrices(save_mv, save_p);
	gfx::update_projection();

	return true;
}

static bool modelfunc_identity(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int model_id = as_number(prg, v[0]);

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];

	model->mat = glm::mat4();

	return true;
}

static bool modelfunc_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int model_id = as_number(prg, v[0]);
	double sx = as_number(prg, v[1]);
	double sy = as_number(prg, v[2]);
	double sz = as_number(prg, v[3]);

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];

	model->mat = glm::scale(model->mat, glm::vec3(sx, sy, sz));

	return true;
}

static bool modelfunc_rotate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int model_id = as_number(prg, v[0]);
	double angle = as_number(prg, v[1]);
	double ax = as_number(prg, v[2]);
	double ay = as_number(prg, v[3]);
	double az = as_number(prg, v[4]);

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];

	model->mat = glm::rotate(model->mat, (float)angle, glm::vec3(ax, ay, az));

	return true;
}

static bool modelfunc_translate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int model_id = as_number(prg, v[0]);
	double x = as_number(prg, v[1]);
	double y = as_number(prg, v[2]);
	double z = as_number(prg, v[3]);

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];

	model->mat = glm::translate(model->mat, glm::vec3(x, y, z));

	return true;
}

static Variable exprfunc_model_get_position(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int model_id = as_number(prg, v[0]);

	Variable x;
	x.type = Variable::NUMBER;
	Variable y;
	y.type = Variable::NUMBER;
	Variable z;
	z.type = Variable::NUMBER;

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];

	glm::vec3 pos = glm::vec3(model->mat[3]);
	x.n = pos.x;
	y.n = pos.y;
	z.n = pos.z;

	Variable vec;
	vec.type = Variable::VECTOR;
	vec.v.push_back(x);
	vec.v.push_back(y);
	vec.v.push_back(z);

	return vec;
}

static bool modelfunc_identity_3d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	glm::mat4 mv, proj;
	gfx::get_matrices(mv, proj);
	mv = glm::mat4();
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	return true;
}

static bool modelfunc_scale_3d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double sx = as_number(prg, v[0]);
	double sy = as_number(prg, v[1]);
	double sz = as_number(prg, v[2]);

	glm::mat4 mv, proj;
	gfx::get_matrices(mv, proj);
	mv = glm::scale(mv, glm::vec3(sx, sy, sz));
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	return true;
}

static bool modelfunc_rotate_3d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	double angle = as_number(prg, v[0]);
	double ax = as_number(prg, v[1]);
	double ay = as_number(prg, v[2]);
	double az = as_number(prg, v[3]);

	glm::mat4 mv, proj;
	gfx::get_matrices(mv, proj);
	mv = glm::rotate(mv, (float)angle, glm::vec3(ax, ay, az));
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	return true;
}

static bool modelfunc_translate_3d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double x = as_number(prg, v[0]);
	double y = as_number(prg, v[1]);
	double z = as_number(prg, v[2]);

	glm::mat4 mv, proj;
	gfx::get_matrices(mv, proj);
	mv = glm::translate(mv, glm::vec3(x, y, z));
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	return true;
}

static bool modelfunc_set_2d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	set_2d();

	return true;
}

static bool modelfunc_set_3d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	set_3d();

	return true;
}

struct Model_Callback_Data
{
	Program *prg;
	int function;
	int id;
};

static void model_callback(void *data)
{
	Model_Callback_Data *d = static_cast<Model_Callback_Data *>(data);
	std::vector<Token> v;
	Token t;
	t.type = Token::NUMBER;
	t.n = d->id;
	t.dereference = false;
	v.push_back(t);
	call_void_function(d->prg, d->function, v, 0);
	delete d;
}

static bool modelfunc_set_animation(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string anim = as_string(prg, v[1]);
	
	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, id)
	Model *model = info->models[id];

	if (v.size() > 2) {
		Model_Callback_Data *d = new Model_Callback_Data;
		d->prg = prg;
		d->function = as_function(prg, v[2]);
		d->id = id;
		model->model->set_animation(anim, model_callback, d);
	}
	else {
		model->model->set_animation(anim);
	}

	model->model->start();

	return true;
}

static bool modelfunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	
	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, id)
	Model *model = info->models[id];

	model->model->stop();

	return true;
}

static bool modelfunc_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	
	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, id)
	Model *model = info->models[id];

	model->model->reset();

	return true;
}

static Variable exprfunc_model_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable out_x;
	out_x.type = Variable::NUMBER;
	Variable out_y;
	out_y.type = Variable::NUMBER;
	Variable out_z;
	out_z.type = Variable::NUMBER;

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, id)
	Model *model = info->models[id];

	gfx::Model::Node *n = model->model->find("Model");
	if (n == nullptr) {
		std::vector<gfx::Model::Node *> nodes = model->model->get_nodes();
		if (nodes.size() > 0) {
			n = nodes[0];
		}
	}

	double szx = 0.0f;
	double szy = 0.0f;
	double szz = 0.0f;

	if (n != nullptr) {
		szx = n->max_x - n->min_x;
		szy = n->max_y - n->min_y;
		szz = n->max_z - n->min_z;
	}

	out_x.n = szx;
	out_y.n = szy;
	out_z.n = szz;

	Variable vec;
	vec.type = Variable::VECTOR;
	vec.v.push_back(out_x);
	vec.v.push_back(out_y);
	vec.v.push_back(out_z);

	return vec;
}

static Variable exprfunc_model_create_vertex_buffer(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	Variable *verts = as_variable(prg, v[0]).p;
	Variable *faces = as_variable(prg, v[1]).p;
	Variable *colours = as_variable(prg, v[2]).p;
	Variable *normals = as_variable(prg, v[3]).p;
	Variable *texcoords = as_variable(prg, v[4]).p;
	int num_triangles = as_number(prg, v[5]);

	Vertex_Buffer_Info *info = vertex_buffer_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = info->vertex_buffer_id;

	Vertex_Buffer *vb = new Vertex_Buffer;
	vb->v = (float *)malloc(12*3*num_triangles*sizeof(float));
	vb->num_triangles = num_triangles;

	info->vertex_buffers[info->vertex_buffer_id++] = vb;

	int count = 0;
	int ccount = 0;
	int ncount = 0;
	int tcount = 0;

	for (int i = 0; i < num_triangles; i++) {
		for (int j = 0; j < 3; j++) {
			int index = faces->v[i*3+j].n;
			// xyz
			vb->v[count++] = verts->v[index*3+0].n;
			vb->v[count++] = verts->v[index*3+1].n;
			vb->v[count++] = verts->v[index*3+2].n;
			// normals
			if (normals->v.size() > 0) {
				vb->v[count++] = normals->v[ncount++].n;
				vb->v[count++] = normals->v[ncount++].n;
				vb->v[count++] = normals->v[ncount++].n;
			}
			else {
				vb->v[count++] = 0.0f;
				vb->v[count++] = 0.0f;
				vb->v[count++] = 0.0f;
			}
			// texcoord
			if (texcoords->v.size() > 0) {
				vb->v[count++] = texcoords->v[tcount++].n;
				vb->v[count++] = texcoords->v[tcount++].n;
			}
			else {
				vb->v[count++] = 0.0f;
				vb->v[count++] = 0.0f;
			}
			// colour
			if (colours->v.size() > 0) {
				vb->v[count++] = colours->v[ccount++].n / 255.0f;
				vb->v[count++] = colours->v[ccount++].n / 255.0f;
				vb->v[count++] = colours->v[ccount++].n / 255.0f;
				vb->v[count++] = colours->v[ccount++].n / 255.0f;
			}
			else {
				for (int k = 0; k < 4; k++) {
					vb->v[count++] = 1.0f;
				}
			}
		}
	}

	return v1;
}

static bool modelfunc_destroy_vertex_buffer(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Vertex_Buffer_Info *info = vertex_buffer_info(prg);
	INFO_EXISTS(info->vertex_buffers, id)
	free(info->vertex_buffers[id]->v);
	delete info->vertex_buffers[id];
	info->vertex_buffers.erase(info->vertex_buffers.find(id));

	return true;
}

static bool modelfunc_draw_3d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Vertex_Buffer_Info *info = vertex_buffer_info(prg);
	INFO_EXISTS(info->vertex_buffers, id)
	Vertex_Buffer *vb = info->vertex_buffers[id];

	gfx::enable_depth_write(true);
	gfx::enable_depth_test(true);

	gfx::Vertex_Cache::instance()->start();
	gfx::Vertex_Cache::instance()->cache_3d_immediate(vb->v, vb->num_triangles);
	gfx::Vertex_Cache::instance()->end();

	gfx::enable_depth_test(false);
	gfx::enable_depth_write(false);

	return true;
}

static bool modelfunc_draw_3d_textured(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	int tex = as_number(prg, v[1]);

	Vertex_Buffer_Info *info = vertex_buffer_info(prg);
	Vertex_Buffer *vb = info->vertex_buffers[id];

	gfx::enable_depth_write(true);
	gfx::enable_depth_test(true);

	Image_Info *iinfo = image_info(prg);
	INFO_EXISTS(iinfo->images, tex)
	gfx::Image *image = iinfo->images[tex]->image;
	GLuint texture = image->get_opengl_texture();
	glBindTexture_ptr(GL_TEXTURE_2D, texture);
	PRINT_GL_ERROR("glActiveTexture\n");
	glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	PRINT_GL_ERROR("glTexParameteri\n");
	glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	PRINT_GL_ERROR("glTexParameteri\n");
	
	gfx::Vertex_Cache::instance()->start(image);
	gfx::Vertex_Cache::instance()->cache_3d_immediate(vb->v, vb->num_triangles);
	gfx::Vertex_Cache::instance()->end();

	gfx::enable_depth_test(false);
	gfx::enable_depth_write(false);

	glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	PRINT_GL_ERROR("glTexParameteri\n");
	glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	PRINT_GL_ERROR("glTexParameteri\n");

	return true;
}

static Variable exprfunc_model_clone(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Variable result;
	result.type = Variable::NUMBER;

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, id)
	Model *model = new Model;
	Model *orig = info->models[id];
	model->model = orig->model;
	model->mat = orig->mat;
	model->is_clone = true;
	result.n = info->model_id;
	info->models[info->model_id++] = model;

	return result;
}

static Variable exprfunc_billboard_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	Variable result;
	result.type = Variable::NUMBER;

	int image_id = as_number(prg, v[0]);
	double x = as_number(prg, v[1]);
	double y = as_number(prg, v[2]);
	double z = as_number(prg, v[3]);
	double w = as_number(prg, v[4]);
	double h = as_number(prg, v[5]);

	Image_Info *info = image_info(prg);
	INFO_EXISTS(info->images, image_id)
	gfx::Image *image = info->images[image_id]->image;
		
	Billboard_Info *info2 = billboard_info(prg);
	result.n = info2->billboard_id;

	try {	
		Billboard *billboard = new Billboard;
		billboard->image = image;
		billboard->sprite = nullptr;
		billboard->x = x;
		billboard->y = y;
		billboard->z = z;
		billboard->w = w;
		billboard->h = h;
		billboard->tx = 0;
		billboard->ty = 0;
		billboard->tz = 0;
		billboard->sx = 1;
		billboard->sy = 1;
		billboard->unit = 0;
		info2->billboards[info2->billboard_id++] = billboard;
	}
	catch (util::Error &e) {
		result.n = -1;
	}

	return result;
}

static bool billboardfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	Billboard_Info *info = billboard_info(prg);
	INFO_EXISTS(info->billboards, id)
	delete info->billboards[id];
	info->billboards.erase(info->billboards.find(id));

	return true;
}

static Variable exprfunc_billboard_from_sprite(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(7)

	Variable result;
	result.type = Variable::NUMBER;

	int sprite_id = as_number(prg, v[0]);
	double x = as_number(prg, v[1]);
	double y = as_number(prg, v[2]);
	double z = as_number(prg, v[3]);
	double w = as_number(prg, v[4]);
	double h = as_number(prg, v[5]);
	double unit = as_number(prg, v[6]);

	Sprite_Info *info = sprite_info(prg);
	INFO_EXISTS(info->sprites, sprite_id)
	gfx::Sprite *sprite = info->sprites[sprite_id];
	
	Billboard_Info *info2 = billboard_info(prg);
	result.n = info2->billboard_id;

	try {
		Billboard *billboard = new Billboard;
		billboard->image = nullptr;
		billboard->sprite = sprite;
		billboard->x = x;
		billboard->y = y;
		billboard->z = z;
		billboard->w = w;
		billboard->h = h;
		billboard->tx = 0;
		billboard->ty = 0;
		billboard->tz = 0;
		billboard->sx = 1;
		billboard->sy = 1;
		billboard->unit = unit;
		info2->billboards[info2->billboard_id++] = billboard;
	}
	catch (util::Error &e) {
		result.n = -1;
	}

	return result;
}

static bool billboardfunc_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int billboard_id = as_number(prg, v[0]);
	int r = as_number(prg, v[1]);
	int g = as_number(prg, v[2]);
	int b = as_number(prg, v[3]);
	int a = as_number(prg, v[4]);

	Billboard_Info *info = billboard_info(prg);
	INFO_EXISTS(info->billboards, billboard_id)
	Billboard *billboard = info->billboards[billboard_id];

	const float verts[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	const int faces[] = {
		0, 2, 1, 1, 2, 3
	};
	float texcoords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};

	gfx::Image *img = billboard->image;
	if (img == nullptr) {
		img = billboard->sprite->get_current_image();
		billboard->w = img->size.w / (double)billboard->unit;
		billboard->h = img->size.h / (double)billboard->unit;
		gfx::Image *root = img->get_root();
		double tx1 = (double)img->get_offset().x / root->size.w;
		double tx2 = tx1 + 1.0f / billboard->sprite->get_num_frames();
		for (int i = 0; i < 6; i++) {
			if (texcoords[i*2+0] == 0.0f) {
				texcoords[i*2+0] = tx1;
			}
			else {
				texcoords[i*2+0] = tx2;
			}
		}
	}

	glm::mat4 mv, proj;
	gfx::get_matrices(mv, proj);

	glm::vec3 camera_right(mv[0][0], mv[1][0], mv[2][0]);
	glm::vec3 camera_up(mv[0][1], mv[1][1], mv[2][1]);

	float vec[12*2*3];

	int count = 0;
	int tc = 0;

	for (size_t i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			double x = verts[faces[i*3+j]*3+0];
			double y = verts[faces[i*3+j]*3+1];
			glm::vec3 pos = glm::vec3(billboard->x+billboard->tx, billboard->y+billboard->ty, billboard->z+billboard->tz);
			glm::vec3 pt = pos + camera_right * x * billboard->w * billboard->sx + camera_up * y * billboard->h * billboard->sy;
			vec[count++] = pt.x;
			vec[count++] = pt.y;
			vec[count++] = pt.z;
			// normals
			vec[count++] = 0.0f;
			vec[count++] = 0.0f;
			vec[count++] = 0.0f;
			// texcoord
			vec[count++] = texcoords[tc++];
			vec[count++] = texcoords[tc++];
			vec[count++] = r / 255.0f;
			vec[count++] = g / 255.0f;
			vec[count++] = b / 255.0f;
			vec[count++] = a / 255.0f;
		}
	}
	
	glm::mat4 t = mv;
	//t = glm::translate(t, glm::vec3(billboard->x, billboard->y, billboard->z));
	
	gfx::set_matrices(t, proj);
	gfx::update_projection();
	
	gfx::enable_depth_write(true);
	gfx::enable_depth_test(true);

	gfx::Vertex_Cache::instance()->start(img);
	gfx::Vertex_Cache::instance()->cache_3d_immediate(vec, 2);
	gfx::Vertex_Cache::instance()->end();

	gfx::enable_depth_test(false);
	gfx::enable_depth_write(false);
	
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	return true;
}

static bool billboardfunc_translate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int billboard_id = as_number(prg, v[0]);
	double x = as_number(prg, v[1]);
	double y = as_number(prg, v[2]);
	double z = as_number(prg, v[3]);

	Billboard_Info *info = billboard_info(prg);
	INFO_EXISTS(info->billboards, billboard_id)
	Billboard *billboard = info->billboards[billboard_id];

	billboard->tx += x;
	billboard->ty += y;
	billboard->tz += z;

	return true;
}

static bool billboardfunc_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int billboard_id = as_number(prg, v[0]);
	double sx = as_number(prg, v[1]);
	double sy = as_number(prg, v[2]);

	Billboard_Info *info = billboard_info(prg);
	INFO_EXISTS(info->billboards, billboard_id)
	Billboard *billboard = info->billboards[billboard_id];

	billboard->sx *= sx;
	billboard->sy *= sy;

	return true;
}

static Variable exprfunc_cd_model_point(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	Variable result;
	result.type = Variable::NUMBER;

	int model_id = as_number(prg, v[0]);
	double x = as_number(prg, v[1]);
	double y = as_number(prg, v[2]);
	double z = as_number(prg, v[3]);

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];
			
	result.n = cd::model_point(model->model, model->mat, glm::vec3(x, y, z));

	return result;
}

static Variable exprfunc_cd_model_line_segment(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(7)

	Variable result;
	result.type = Variable::VECTOR;
	Variable var;
	var.type = Variable::NUMBER;

	int model_id = as_number(prg, v[0]);
	double x = as_number(prg, v[1]);
	double y = as_number(prg, v[2]);
	double z = as_number(prg, v[3]);
	double x2 = as_number(prg, v[4]);
	double y2 = as_number(prg, v[5]);
	double z2 = as_number(prg, v[6]);

	Model_Info *info = model_info(prg);
	INFO_EXISTS(info->models, model_id)
	Model *model = info->models[model_id];
			
	glm::vec3 out;

	var.n = cd::model_line_segment(model->model, model->mat, glm::vec3(x, y, z), glm::vec3(x2, y2, z2), out);
	result.v.push_back(var);
	var.n = out.x;
	result.v.push_back(var);
	var.n = out.y;
	result.v.push_back(var);
	var.n = out.z;
	result.v.push_back(var);

	return result;
}

static Variable exprfunc_cd_sphere_sphere(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(8)

	Variable result;
	result.type = Variable::NUMBER;

	double x = as_number(prg, v[0]);
	double y = as_number(prg, v[1]);
	double z = as_number(prg, v[2]);
	double r = as_number(prg, v[3]);
	double x2 = as_number(prg, v[4]);
	double y2 = as_number(prg, v[5]);
	double z2 = as_number(prg, v[6]);
	double r2 = as_number(prg, v[7]);

	x -= x2;
	y -= y2;
	z -= z2;
	double len = sqrt(x*x + y*y + z*z);
	result.n = len < (r+r2);

	return result;
}

class BooBoo_Widget : public TGUI_Widget {
public:
	BooBoo_Widget(Program *prg, int id, int w, int h);
	BooBoo_Widget(Program *prg, int id, float percent_w, float percent_h);
	BooBoo_Widget(Program *prg, int id, int w, float percent_h);
	BooBoo_Widget(Program *prg, int id, float percent_w, int h);
	BooBoo_Widget(Program *prg, int id, TGUI_Widget::Fit fit, int other);
	BooBoo_Widget(Program *prg, int id, TGUI_Widget::Fit fit, float percent_other);
	BooBoo_Widget(Program *prg, int id); // Fit both
	virtual ~BooBoo_Widget();

	virtual void draw();
	virtual void handle_event(TGUI_Event *event);

protected:
	void start(Program *prg, int id);
	bool is_focussed();

	Program *prg;
	int id;
};

bool BooBoo_Widget::is_focussed()
{
	if (shim::guis.size() == 0) {
		return false;
	}
	if (gui != shim::guis.back()->gui) {
		return false;
	}
	return gui->get_focus() == this;
}

void BooBoo_Widget::start(Program *prg, int id)
{
	this->prg = prg;
	this->id = id;
}

BooBoo_Widget::BooBoo_Widget(Program *prg, int id, int w, int h) :
	TGUI_Widget(w, h)
{
	start(prg, id);
}

BooBoo_Widget::BooBoo_Widget(Program *prg, int id, float percent_w, float percent_h) :
	TGUI_Widget(percent_w, percent_h)
{
	start(prg, id);
}

BooBoo_Widget::BooBoo_Widget(Program *prg, int id, int w, float percent_h) :
	TGUI_Widget(w, percent_h)
{
	start(prg, id);
}

BooBoo_Widget::BooBoo_Widget(Program *prg, int id, float percent_w, int h) :
	TGUI_Widget(percent_w, h)
{
	start(prg, id);
}

BooBoo_Widget::BooBoo_Widget(Program *prg, int id, TGUI_Widget::Fit fit, int other) :
	TGUI_Widget(fit, other)
{
	start(prg, id);
}

BooBoo_Widget::BooBoo_Widget(Program *prg, int id, TGUI_Widget::Fit fit, float percent_other) :
	TGUI_Widget(fit, percent_other)
{
	start(prg, id);
}

BooBoo_Widget::BooBoo_Widget(Program *prg, int id) :
	TGUI_Widget()
{
	start(prg, id);
}

BooBoo_Widget::~BooBoo_Widget()
{
}

void BooBoo_Widget::draw()
{
	TGUI_Widget::draw();

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, id)
	BooBoo_Widget *widget = info->widgets[id]->widget;

	std::vector<Token> tmp;
	Token t;

	t.type = Token::NUMBER;
	t.n = id;
	t.dereference = false;
	tmp.push_back(t);

	t.type = Token::NUMBER;
	t.n = widget->get_x();
	tmp.push_back(t);
	t.n = widget->get_y();
	tmp.push_back(t);
	t.n = widget->get_width();
	tmp.push_back(t);
	t.n = widget->get_height();
	tmp.push_back(t);
	
	t.n = widget->is_focussed();
	tmp.push_back(t);

	t.type = Token::SYMBOL;
	t.i = prg->variables_map[info->widgets[id]->data->name];
	tmp.push_back(t);

	call_void_function(prg, "gui_draw", tmp);
}

void BooBoo_Widget::handle_event(TGUI_Event *event)
{
	TGUI_Widget::handle_event(event);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, id)
	BooBoo_Widget *widget = info->widgets[id]->widget;

	std::vector<Token> tmp;
	Token t;

	t.type = Token::NUMBER;
	t.n = id;
	t.dereference = false;
	tmp.push_back(t);

	t.type = Token::NUMBER;
	t.n = (int)event->type;
	tmp.push_back(t);

	for (int i = 0; i < 4; i++) {
		tmp.push_back(t);
	}

	t.type = Token::NUMBER;
	t.n = widget->get_x();
	tmp.push_back(t);
	t.n = widget->get_y();
	tmp.push_back(t);
	t.n = widget->get_width();
	tmp.push_back(t);
	t.n = widget->get_height();
	tmp.push_back(t);

	t.n = widget->is_focussed();
	tmp.push_back(t);

	t.type = Token::SYMBOL;
	t.i = prg->variables_map[info->widgets[id]->data->name];
	tmp.push_back(t);

	bool go = true;

	if (event->type == TGUI_KEY_DOWN || event->type == TGUI_KEY_UP) {
		tmp[2].n = event->keyboard.code;
		tmp[3].n = event->keyboard.is_repeat;
	}
	else if (event->type == TGUI_JOY_DOWN || event->type == TGUI_JOY_UP) {
		tmp[2].n = (Uint32)event->joystick.id;
		tmp[3].n = event->joystick.button;
		tmp[4].n = event->joystick.is_repeat;
	}
	else if (event->type == TGUI_JOY_AXIS) {
		tmp[2].n = (Uint32)event->joystick.id;
		tmp[3].n = event->joystick.axis;
		tmp[4].n = event->joystick.value;
	}
	else if (event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_UP) {
		tmp[2].n = event->mouse.button;
		tmp[3].n = event->mouse.is_repeat;
		tmp[4].n = event->mouse.x;
		tmp[5].n = event->mouse.y;
	}
	else if (event->type == TGUI_MOUSE_AXIS) {
		tmp[2].n = event->mouse.x;
		tmp[3].n = event->mouse.y;
		tmp[4].n = event->mouse.dx;
		tmp[5].n = event->mouse.dy;
	}
	else if (event->type == TGUI_MOUSE_WHEEL) {
		tmp[2].n = event->mouse.x;
		tmp[3].n = event->mouse.y;
	}
	else {
		go = false;
	}
	
	if (go) {
		call_void_function(prg, "gui_event", tmp);
	}
}

static Variable exprfunc_widget_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable v1;
	v1.type = Variable::NUMBER;

	double w = as_number(prg, v[0]);
	double h = as_number(prg, v[1]);
	Variable &data = as_variable(prg, v[2]);
	
	Widget_Info *info = widget_info(prg);

	v1.n = info->widget_id;

	try {
		Widget *widget = new Widget;
		widget->data = &data;

		if (w <= 1.0 && h <= 1.0) {
			widget->widget = new BooBoo_Widget(prg, info->widget_id, (float)w, (float)h);
		}
		else if (w <= 1.0) {
			widget->widget = new BooBoo_Widget(prg, info->widget_id, (float)w, (int)h);
		}
		else if (h <= 1.0) {
			widget->widget = new BooBoo_Widget(prg, info->widget_id, (int)w, (float)h);
		}
		else {
			widget->widget = new BooBoo_Widget(prg, info->widget_id, (int)w, (int)h);
		}

		widget->widget->set_accepts_focus(true);

		info->widgets[info->widget_id++] = widget;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool widgetfunc_set_parent(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int child = as_number(prg, v[0]);
	unsigned int parent = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	
	INFO_EXISTS(info->widgets, child)
	INFO_EXISTS(info->widgets, parent)

	info->widgets[child]->widget->set_parent(info->widgets[parent]->widget);

	return true;
}

static bool widgetfunc_set_left_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	unsigned int left = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	INFO_EXISTS(info->widgets, widget)
	INFO_EXISTS(info->widgets, left)

	info->widgets[widget]->widget->set_left_widget(info->widgets[left]->widget);

	return true;
}

static bool widgetfunc_set_right_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	unsigned int right = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	
	INFO_EXISTS(info->widgets, widget)
	INFO_EXISTS(info->widgets, right)

	info->widgets[widget]->widget->set_right_widget(info->widgets[right]->widget);

	return true;
}

static bool widgetfunc_set_up_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	unsigned int up = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	
	INFO_EXISTS(info->widgets, widget)
	INFO_EXISTS(info->widgets, up)

	info->widgets[widget]->widget->set_up_widget(info->widgets[up]->widget);

	return true;
}

static bool widgetfunc_set_down_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	unsigned int down = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	
	INFO_EXISTS(info->widgets, widget)
	INFO_EXISTS(info->widgets, down)

	info->widgets[widget]->widget->set_down_widget(info->widgets[down]->widget);

	return true;
}

static bool widgetfunc_set_float_left(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_float_left(val);

	return true;
}

static bool widgetfunc_set_float_right(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_float_right(val);

	return true;
}

static bool widgetfunc_set_float_bottom(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_float_bottom(val);

	return true;
}

static bool widgetfunc_set_centre_x(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_centre_x(val);

	return true;
}

static bool widgetfunc_set_centre_y(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_centre_y(val);

	return true;
}

static bool widgetfunc_set_clear_float_x(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_clear_float_x(val);

	return true;
}

static bool widgetfunc_set_clear_float_y(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_clear_float_y(val);

	return true;
}

static bool widgetfunc_set_break_line(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_break_line(val);

	return true;
}

static bool widgetfunc_set_accepts_focus(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	bool val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	info->widgets[widget]->widget->set_accepts_focus(val);

	return true;
}

static bool widgetfunc_set_padding(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	double val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	if (val <= 1.0) {
		info->widgets[widget]->widget->set_padding((float)val);
	}
	else {
		info->widgets[widget]->widget->set_padding((int)val);
	}

	return true;
}

static bool widgetfunc_set_padding_left(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	double val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	if (val <= 1.0) {
		info->widgets[widget]->widget->set_padding_left((float)val);
	}
	else {
		info->widgets[widget]->widget->set_padding_left((int)val);
	}

	return true;
}

static bool widgetfunc_set_padding_right(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	double val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	if (val <= 1.0) {
		info->widgets[widget]->widget->set_padding_right((float)val);
	}
	else {
		info->widgets[widget]->widget->set_padding_right((int)val);
	}

	return true;
}

static bool widgetfunc_set_padding_top(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	double val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	if (val <= 1.0) {
		info->widgets[widget]->widget->set_padding_top((float)val);
	}
	else {
		info->widgets[widget]->widget->set_padding_top((int)val);
	}

	return true;
}

static bool widgetfunc_set_padding_bottom(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number(prg, v[0]);
	double val = as_number(prg, v[1]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, widget)

	if (val <= 1.0) {
		info->widgets[widget]->widget->set_padding_bottom((float)val);
	}
	else {
		info->widgets[widget]->widget->set_padding_bottom((int)val);
	}

	return true;
}

class BooBoo_GUI : public gui::GUI
{
public:
	BooBoo_GUI(BooBoo_Widget *root);
	virtual ~BooBoo_GUI();

	virtual void update();

private:
	bool done_transition_in;
};

BooBoo_GUI::BooBoo_GUI(BooBoo_Widget *root) :
	done_transition_in(false)
{
	transition = transition_in_type != TRANSITION_NONE;

	switch (transition_in_type) {
		case TRANSITION_ENLARGE:
			transition_is_enlarge = true;
			break;
		case TRANSITION_SHRINK:
			transition_is_shrink = true;
			break;
		case TRANSITION_SLIDE:
			transition_is_slide = true;
			break;
		case TRANSITION_SLIDE_VERTICAL:
			transition_is_slide_vertical = true;
			break;
		case TRANSITION_NONE:
		case TRANSITION_FADE:
			break;
	}

	root->set_centre_x(true);
	root->set_centre_y(true);

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	root->set_parent(modal_main_widget);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

BooBoo_GUI::~BooBoo_GUI()
{
}

void BooBoo_GUI::update()
{
	if (transitioning_in == false && done_transition_in == false) {
		switch (transition_in_type) {
			case TRANSITION_ENLARGE:
				transition_is_enlarge = false;
				break;
			case TRANSITION_SHRINK:
				transition_is_shrink = false;
				break;
			case TRANSITION_SLIDE:
				transition_is_slide = false;
				break;
			case TRANSITION_SLIDE_VERTICAL:
				transition_is_slide_vertical = false;
				break;
			case TRANSITION_NONE:
			case TRANSITION_FADE:
				break;
		}
		switch (transition_out_type) {
			case TRANSITION_ENLARGE:
				transition_is_enlarge = true;
				break;
			case TRANSITION_SHRINK:
				transition_is_shrink = true;
				break;
			case TRANSITION_SLIDE:
				transition_is_slide = true;
				break;
			case TRANSITION_SLIDE_VERTICAL:
				transition_is_slide_vertical = true;
				break;
			case TRANSITION_NONE:
			case TRANSITION_FADE:
				break;
		}
		transition = transition_out_type != TRANSITION_NONE;
		done_transition_in = true;
	}

	gui::GUI::update();
}

static bool widgetfunc_gui_start(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	unsigned int id = as_number(prg, v[0]);

	Widget_Info *info = widget_info(prg);
	INFO_EXISTS(info->widgets, id)

	BooBoo_GUI *g = new BooBoo_GUI(info->widgets[id]->widget);

	shim::guis.push_back(g);
	
	shim::convert_directions_to_focus_events = true;

	return true;
}

static bool widgetfunc_gui_exit(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	if (shim::guis.size() > 0) {
		shim::guis[shim::guis.size()-1]->exit();
		if (shim::guis.size() == 0) {
			shim::convert_directions_to_focus_events = false;
		}
	}

	return true;
}

static bool widgetfunc_gui_set_focus(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	unsigned int id = as_number(prg, v[0]);

	Widget_Info *info = widget_info(prg);

	INFO_EXISTS(info->widgets, id)

	if (shim::guis.size() > 0) {
		shim::guis[shim::guis.size()-1]->gui->set_focus(info->widgets[id]->widget);
	}

	return true;
}

static bool widgetfunc_gui_set_transition_types(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int in = as_number(prg, v[0]);
	int out = as_number(prg, v[1]);

	transition_in_type = (GUI_Transition_Type)in;
	transition_out_type = (GUI_Transition_Type)out;

	return true;
}

static void black_bars_callback(gfx::Black_Bar_Type type, int x, int y, int w, int h)
{
	if (prg == nullptr) {
		return;
	}

	SDL_Color c[4];

	// draw a default
	switch (type) {
		case gfx::BAR_TOP:
			c[0].r = 255;
			c[0].g = 0;
			c[0].b = 216;
			c[0].a = 255;
			c[1].r = 255;
			c[1].g = 0;
			c[1].b = 216;
			c[1].a = 255;
			c[2].r = 0;
			c[2].g = 0;
			c[2].b = 0;
			c[2].a = 255;
			c[3].r = 0;
			c[3].g = 0;
			c[3].b = 0;
			c[3].a = 255;
			break;
		case gfx::BAR_BOTTOM:
			c[0].r = 0;
			c[0].g = 0;
			c[0].b = 0;
			c[0].a = 255;
			c[1].r = 0;
			c[1].g = 0;
			c[1].b = 0;
			c[1].a = 255;
			c[2].r = 255;
			c[2].g = 0;
			c[2].b = 216;
			c[2].a = 255;
			c[3].r = 255;
			c[3].g = 0;
			c[3].b = 216;
			c[3].a = 255;
			break;
		case gfx::BAR_LEFT:
			c[0].r = 255;
			c[0].g = 0;
			c[0].b = 216;
			c[0].a = 255;
			c[1].r = 0;
			c[1].g = 0;
			c[1].b = 0;
			c[1].a = 255;
			c[2].r = 0;
			c[2].g = 0;
			c[2].b = 0;
			c[2].a = 255;
			c[3].r = 255;
			c[3].g = 0;
			c[3].b =  216;
			c[3].a = 255;
			break;
		case gfx::BAR_RIGHT:
			c[0].r = 0;
			c[0].g = 0;
			c[0].b = 0;
			c[0].a = 255;
			c[1].r = 255;
			c[1].g = 0;
			c[1].b = 216;
			c[1].a = 255;
			c[2].r = 255;
			c[2].g = 0;
			c[2].b = 216;
			c[2].a = 255;
			c[3].r = 0;
			c[3].g = 0;
			c[3].b = 0;
			c[3].a = 255;
			break;
	}

	gfx::draw_filled_rectangle(c, util::Point<int>(x, y), util::Size<int>(w, h));

	// now user can draw theirs
	std::vector<Token> v;
	Token t;
	t.type = Token::NUMBER;
	t.dereference = false;
	t.n = (int)type;
	v.push_back(t);
	t.n = x;
	v.push_back(t);
	t.n = y;
	v.push_back(t);
	t.n = w;
	v.push_back(t);
	t.n = h;
	v.push_back(t);
	call_void_function(prg, "draw_black_bar", v, 0);
}

static void lost_device_callback()
{
}

static void found_device_callback()
{
	if (custom_projection_set) {
		gfx::set_matrices(custom_mv, custom_proj);
		gfx::update_projection();
	}
	else {
		if (is_3d) {
			set_3d();
		}
		else {
			set_2d();
		}
	}
	if (custom_scissor_set) {
		gfx::set_scissor(my_scissor[0], my_scissor[1], my_scissor[2], my_scissor[3]);
	}
	if (custom_viewport_set) {
		gfx::set_viewport(my_viewport[0], my_viewport[1], my_viewport[2], my_viewport[3]);
	}
}

void register_game_callbacks()
{
	gfx::register_lost_device_callbacks(lost_device_callback, found_device_callback);
	gfx::register_black_bars_callback(black_bars_callback);
}
 
void unregister_game_callbacks()
{
	gfx::register_lost_device_callbacks(nullptr, nullptr);
	gfx::register_black_bars_callback(nullptr);
}

void start_lib_game()
{
	add_instruction("inspect", miscfunc_inspect);
	add_instruction("delay", miscfunc_delay);
	add_expression_handler("get_ticks", exprfunc_misc_get_ticks);
	add_expression_handler("get_args", exprfunc_misc_get_args);
	add_expression_handler("get_logic_rate", exprfunc_misc_get_logic_rate);
	add_instruction("set_logic_rate", miscfunc_set_logic_rate);
	add_expression_handler("file_list", exprfunc_misc_file_list);
	add_instruction("clear", gfxfunc_clear);
	add_instruction("flip", gfxfunc_flip);
	add_instruction("resize", gfxfunc_resize);
	add_expression_handler("get_screen_size", exprfunc_gfx_get_screen_size);
	add_expression_handler("get_buffer_size", exprfunc_gfx_get_buffer_size);
	add_expression_handler("get_screen_offset", exprfunc_gfx_get_screen_offset);
	add_expression_handler("get_scale", exprfunc_gfx_get_scale);
	add_instruction("set_target", gfxfunc_set_target);
	add_instruction("set_target_backbuffer", gfxfunc_set_target_backbuffer);
	add_instruction("screen_shake", gfxfunc_screen_shake);
	add_instruction("add_notification", gfxfunc_add_notification);
	add_expression_handler("is_fullscreen", exprfunc_gfx_is_fullscreen);
	add_instruction("toggle_fullscreen", gfxfunc_toggle_fullscreen);
	add_expression_handler("get_refresh_rate", exprfunc_gfx_get_refresh_rate);
	add_instruction("set_scissor", gfxfunc_set_scissor);
	add_instruction("unset_scissor", gfxfunc_unset_scissor);
	add_instruction("set_viewport", gfxfunc_set_viewport);
	add_instruction("unset_viewport", gfxfunc_unset_viewport);
	add_instruction("set_blend_mode", gfxfunc_set_blend_mode);
	add_instruction("clear_depth_buffer", gfxfunc_clear_depth_buffer);
	add_instruction("clear_stencil_buffer", gfxfunc_clear_stencil_buffer);
	add_instruction("enable_depth_test", gfxfunc_enable_depth_test);
	add_instruction("enable_depth_write", gfxfunc_enable_depth_write);
	add_instruction("set_depth_mode", gfxfunc_set_depth_mode);
	add_instruction("enable_stencil", gfxfunc_enable_stencil);
	add_instruction("enable_two_sided_stencil", gfxfunc_enable_two_sided_stencil);
	add_instruction("set_stencil_mode", gfxfunc_set_stencil_mode);
	add_instruction("set_stencil_mode_backfaces", gfxfunc_set_stencil_mode_backfaces);
	add_instruction("set_cull_mode", gfxfunc_set_cull_mode);
	add_instruction("enable_colour_write", gfxfunc_enable_colour_write);
	add_instruction("set_projection", gfxfunc_set_projection);
	add_instruction("set_default_projection", gfxfunc_set_default_projection);
	add_instruction("resize_vertex_cache", gfxfunc_resize_vertex_cache);
	add_instruction("start_primitives", primfunc_start_primitives);
	add_instruction("end_primitives", primfunc_end_primitives);
	add_instruction("line", primfunc_line);
	add_instruction("triangle", primfunc_triangle);
	add_instruction("filled_triangle", primfunc_filled_triangle);
	add_instruction("rectangle", primfunc_rectangle);
	add_instruction("filled_rectangle", primfunc_filled_rectangle);
	add_instruction("ellipse", primfunc_ellipse);
	add_instruction("filled_ellipse", primfunc_filled_ellipse);
	add_instruction("circle", primfunc_circle);
	add_instruction("filled_circle", primfunc_filled_circle);
	add_expression_handler("image_create", exprfunc_image_create);
	add_expression_handler("image_load", exprfunc_image_load);
	add_instruction("image_save", imagefunc_save);
	add_instruction("screenshot", imagefunc_screenshot);
	add_instruction("image_destroy", imagefunc_destroy);
	add_instruction("image_draw", imagefunc_draw);
	add_instruction("image_stretch_region", imagefunc_stretch_region);
	add_instruction("image_draw_rotated_scaled", imagefunc_draw_rotated_scaled);
	add_instruction("image_start", imagefunc_start);
	add_instruction("image_end", imagefunc_end);
	add_expression_handler("image_size", exprfunc_image_size);
	add_instruction("image_draw_9patch", imagefunc_draw_9patch);
	add_expression_handler("image_read_texture", exprfunc_image_read_texture);
	add_expression_handler("image_to_texture", exprfunc_image_to_texture);
	add_instruction("image_update", imagefunc_update);
	add_expression_handler("font_load", exprfunc_font_load);
	add_instruction("font_destroy", fontfunc_destroy);
	add_instruction("font_draw", fontfunc_draw);
	add_expression_handler("font_width", exprfunc_font_width);
	add_expression_handler("font_height", exprfunc_font_height);
	add_instruction("font_add_extra_glyph", fontfunc_add_extra_glyph);
	add_instruction("set_tile_size", tilemapfunc_set_tile_size);
	add_expression_handler("tilemap_load", exprfunc_tilemap_load);
	add_instruction("tilemap_destroy", tilemapfunc_destroy);
	add_instruction("tilemap_draw", tilemapfunc_draw);
	add_expression_handler("tilemap_num_layers", exprfunc_tilemap_num_layers);
	add_expression_handler("tilemap_size", exprfunc_tilemap_size);
	add_expression_handler("tilemap_is_solid", exprfunc_tilemap_is_solid);
	add_expression_handler("tilemap_get_groups", exprfunc_tilemap_get_groups);
	add_instruction("tilemap_set_animated_tiles", tilemapfunc_set_animated_tiles);
	add_expression_handler("tilemap_find_path", exprfunc_tilemap_find_path);
	add_instruction("tilemap_set_solid", tilemapfunc_set_solid);
	add_instruction("tilemap_set_tile", tilemapfunc_set_tile);
	add_expression_handler("tilemap_get_tile", exprfunc_tilemap_get_tile);
	add_expression_handler("sprite_load", exprfunc_sprite_load);
	add_instruction("sprite_destroy", spritefunc_destroy);
	add_instruction("sprite_set_animation_lazy", spritefunc_set_animation_lazy);
	add_instruction("sprite_set_animation", spritefunc_set_animation);
	add_expression_handler("sprite_get_animation", exprfunc_sprite_get_animation);
	add_expression_handler("sprite_get_previous_animation", exprfunc_sprite_get_previous_animation);
	add_expression_handler("sprite_length", exprfunc_sprite_length);
	add_expression_handler("sprite_current_frame", exprfunc_sprite_current_frame);
	add_expression_handler("sprite_num_frames", exprfunc_sprite_num_frames);
	add_expression_handler("sprite_current_frame_size", exprfunc_sprite_current_frame_size);
	add_instruction("sprite_draw", spritefunc_draw);
	add_instruction("sprite_start", spritefunc_start);
	add_instruction("sprite_stop", spritefunc_stop);
	add_instruction("sprite_reset", spritefunc_reset);
	add_expression_handler("sprite_bounds", exprfunc_sprite_bounds);
	add_expression_handler("sprite_elapsed", exprfunc_sprite_elapsed);
	add_expression_handler("sprite_frame_times", exprfunc_sprite_frame_times);
	add_expression_handler("sprite_is_started", exprfunc_sprite_is_started);
	add_expression_handler("mml_create", exprfunc_mml_create);
	add_expression_handler("mml_load", exprfunc_mml_load);
	add_instruction("mml_destroy", mmlfunc_destroy);
	add_instruction("mml_play", mmlfunc_play);
	add_instruction("mml_stop", mmlfunc_stop);
	add_expression_handler("sample_load", exprfunc_sample_load);
	add_instruction("sample_destroy", samplefunc_destroy);
	add_instruction("sample_play", samplefunc_play);
	add_instruction("sample_stop", samplefunc_stop);
	add_expression_handler("joy_count", exprfunc_joy_count);
	add_instruction("rumble", joyfunc_rumble);
	add_expression_handler("joy_get_button", exprfunc_joy_get_button);
	add_expression_handler("joy_get_axis", exprfunc_joy_get_axis);
	add_expression_handler("shader_load", exprfunc_shader_load);
	add_instruction("shader_destroy", shaderfunc_destroy);
	add_instruction("shader_use", shaderfunc_use);
	add_instruction("shader_use_default", shaderfunc_use_default);
	add_instruction("shader_set_bool", shaderfunc_set_bool);
	add_instruction("shader_set_int", shaderfunc_set_int);
	add_instruction("shader_set_float", shaderfunc_set_float);
	add_instruction("shader_set_colour", shaderfunc_set_colour);
	add_instruction("shader_set_texture", shaderfunc_set_texture);
	add_instruction("shader_set_float_vector", shaderfunc_set_float_vector);
	add_instruction("shader_set_matrix", shaderfunc_set_matrix);
	add_instruction("shader_set_matrix_array", shaderfunc_set_matrix_array);
	add_expression_handler("json_load", exprfunc_json_load);
	add_instruction("json_destroy", jsonfunc_destroy);
	add_expression_handler("json_get_string", exprfunc_json_get_string);
	add_expression_handler("json_get_number", exprfunc_json_get_number);
	add_expression_handler("json_get_bool", exprfunc_json_get_bool);
	add_instruction("json_set_string", jsonfunc_set_string);
	add_instruction("json_set_number", jsonfunc_set_number);
	add_instruction("json_set_bool", jsonfunc_set_bool);
	add_instruction("json_remove", jsonfunc_remove);
	add_expression_handler("json_save", exprfunc_json_save);
	add_expression_handler("model_load", exprfunc_model_load);
	add_instruction("model_destroy", modelfunc_destroy);
	add_instruction("model_draw", modelfunc_draw);
	add_instruction("set_2d", modelfunc_set_2d);
	add_instruction("set_3d", modelfunc_set_3d);
	add_instruction("model_identity", modelfunc_identity);
	add_instruction("model_scale", modelfunc_scale);
	add_instruction("model_rotate", modelfunc_rotate);
	add_instruction("model_translate", modelfunc_translate);
	add_expression_handler("model_get_position", exprfunc_model_get_position);
	add_instruction("identity_3d", modelfunc_identity_3d);
	add_instruction("scale_3d", modelfunc_scale_3d);
	add_instruction("rotate_3d", modelfunc_rotate_3d);
	add_instruction("translate_3d", modelfunc_translate_3d);
	add_instruction("model_set_animation", modelfunc_set_animation);
	add_instruction("model_stop", modelfunc_stop);
	add_instruction("model_reset", modelfunc_reset);
	add_expression_handler("model_size", exprfunc_model_size);
	add_expression_handler("create_vertex_buffer", exprfunc_model_create_vertex_buffer);
	add_instruction("destroy_vertex_buffer", modelfunc_destroy_vertex_buffer);
	add_instruction("draw_3d", modelfunc_draw_3d);
	add_instruction("draw_3d_textured", modelfunc_draw_3d_textured);
	add_expression_handler("model_clone", exprfunc_model_clone);
	add_expression_handler("billboard_create", exprfunc_billboard_create);
	add_expression_handler("billboard_from_sprite", exprfunc_billboard_from_sprite);
	add_instruction("billboard_destroy", billboardfunc_destroy);
	add_instruction("billboard_draw", billboardfunc_draw);
	add_instruction("billboard_translate", billboardfunc_translate);
	add_instruction("billboard_scale", billboardfunc_scale);
	add_expression_handler("cd_model_point", exprfunc_cd_model_point);
	add_expression_handler("cd_model_line_segment", exprfunc_cd_model_line_segment);
	add_expression_handler("cd_sphere_sphere", exprfunc_cd_sphere_sphere);
	add_expression_handler("widget_create", exprfunc_widget_create);
	add_instruction("widget_set_parent", widgetfunc_set_parent);
	add_instruction("widget_set_float_left", widgetfunc_set_float_left);
	add_instruction("widget_set_float_right", widgetfunc_set_float_right);
	add_instruction("widget_set_float_bottom", widgetfunc_set_float_bottom);
	add_instruction("widget_set_centre_x", widgetfunc_set_centre_x);
	add_instruction("widget_set_centre_y", widgetfunc_set_centre_y);
	add_instruction("widget_set_clear_float_x", widgetfunc_set_clear_float_x);
	add_instruction("widget_set_clear_float_y", widgetfunc_set_clear_float_y);
	add_instruction("widget_set_break_line", widgetfunc_set_break_line);
	add_instruction("widget_set_accepts_focus", widgetfunc_set_accepts_focus);
	add_instruction("widget_set_padding", widgetfunc_set_padding);
	add_instruction("widget_set_padding_left", widgetfunc_set_padding_left);
	add_instruction("widget_set_padding_right", widgetfunc_set_padding_right);
	add_instruction("widget_set_padding_top", widgetfunc_set_padding_top);
	add_instruction("widget_set_padding_bottom", widgetfunc_set_padding_bottom);
	add_instruction("widget_set_left_widget", widgetfunc_set_left_widget);
	add_instruction("widget_set_right_widget", widgetfunc_set_right_widget);
	add_instruction("widget_set_up_widget", widgetfunc_set_up_widget);
	add_instruction("widget_set_down_widget", widgetfunc_set_down_widget);
	add_instruction("gui_start", widgetfunc_gui_start);
	add_instruction("gui_exit", widgetfunc_gui_exit);
	add_instruction("gui_set_focus", widgetfunc_gui_set_focus);
	add_instruction("gui_set_transition_types", widgetfunc_gui_set_transition_types);

	add_special_function("end");
	add_special_function("run");
	add_special_function("draw");
	add_special_function("event");
	add_special_function("gui_event");
	add_special_function("gui_draw");
	add_special_function("draw_black_bar");
}

void end_lib_game()
{
}

void game_lib_destroy_program(Program *prg)
{
	MML_Info *mml_i = mml_info(prg);
	for (std::map<int, audio::MML *>::iterator i = mml_i->mmls.begin(); i != mml_i->mmls.end(); i++) {
		delete mml_i->mmls[(*i).first];
	}
	Sample_Info *sample_i = sample_info(prg);
	for (std::map<int, audio::Sample *>::iterator i = sample_i->samples.begin(); i != sample_i->samples.end(); i++) {
		delete sample_i->samples[(*i).first];
	}
	Image_Info *image_i = image_info(prg);
	for (std::map<int, Image *>::iterator i = image_i->images.begin(); i != image_i->images.end(); i++) {
		delete image_i->images[(*i).first]->image;
		delete image_i->images[(*i).first];
	}
	Font_Info *font_i = font_info(prg);
	for (std::map<int, gfx::TTF *>::iterator i = font_i->fonts.begin(); i != font_i->fonts.end(); i++) {
		delete font_i->fonts[(*i).first];
	}
	Tilemap_Info *tilemap_i = tilemap_info(prg);
	for (std::map<int, gfx::Tilemap *>::iterator i = tilemap_i->tilemaps.begin(); i != tilemap_i->tilemaps.end(); i++) {
		delete tilemap_i->tilemaps[(*i).first];
	}
	Sprite_Info *sprite_i = sprite_info(prg);
	for (std::map<int, gfx::Sprite *>::iterator i = sprite_i->sprites.begin(); i != sprite_i->sprites.end(); i++) {
		delete sprite_i->sprites[(*i).first];
	}
	Shader_Info *shader_i = shader_info(prg);
	for (std::map<int, gfx::Shader *>::iterator i = shader_i->shaders.begin(); i != shader_i->shaders.end(); i++) {
		delete shader_i->shaders[(*i).first];
	}
	JSON_Info *json_i = json_info(prg);
	for (std::map<int, util::JSON *>::iterator i = json_i->jsons.begin(); i != json_i->jsons.end(); i++) {
		delete json_i->jsons[(*i).first];
	}
	Vertex_Buffer_Info *vertex_buffer_i = vertex_buffer_info(prg);
	for (std::map<int, Vertex_Buffer *>::iterator i = vertex_buffer_i->vertex_buffers.begin(); i != vertex_buffer_i->vertex_buffers.end(); i++) {
		delete vertex_buffer_i->vertex_buffers[(*i).first]->v;
		delete vertex_buffer_i->vertex_buffers[(*i).first];
	}
	Model_Info *model_i = model_info(prg);
	for (std::map<int, Model *>::iterator i = model_i->models.begin(); i != model_i->models.end(); i++) {
		if (model_i->models[(*i).first]->is_clone == false) {
			delete model_i->models[(*i).first]->model;
		}
		delete model_i->models[(*i).first];
	}
	Billboard_Info *billboard_i = billboard_info(prg);
	for (std::map<int, Billboard *>::iterator i = billboard_i->billboards.begin(); i != billboard_i->billboards.end(); i++) {
		delete billboard_i->billboards[(*i).first];
	}
	Widget_Info *widget_i = widget_info(prg);
	for (std::map<int, Widget *>::iterator i = widget_i->widgets.begin(); i != widget_i->widgets.end(); i++) {
		delete widget_i->widgets[(*i).first];
	}

	delete mml_i;
	delete sample_i;
	delete image_i;
	delete font_i;
	delete tilemap_i;
	delete shader_i;
	delete json_i;
	delete vertex_buffer_i;
	delete model_i;
	delete billboard_i;
	delete widget_i;

	booboo::set_black_box(prg, "com.nooskewl.booboo.mml", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.sample", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.image", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.font", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.tilemap", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.sprite", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.shader", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.json", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.vertex_buffer", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.model", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.billboard", nullptr);
	booboo::set_black_box(prg, "com.nooskewl.booboo.widget", nullptr);
}
