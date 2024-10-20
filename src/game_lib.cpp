#include <shim5/shim5.h>
#include <shim5/shaders/glsl/default_vertex.h>

#include <libutil/libutil.h>
using namespace noo;

#include "booboo/booboo.h"
#include "booboo/internal.h"
using namespace booboo;

extern bool quit;

struct MML_Info {
	unsigned int mml_id;
	std::map<int, audio::MML *> mmls;
};

struct Sample_Info {
	unsigned int sample_id;
	std::map<int, audio::Sample *> samples;
};

struct Image_Info {
	unsigned int image_id;
	std::map<int, gfx::Image *> images;
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

struct Config_Value
{
	Variable::Variable_Type type;

	double n;
	std::string s;
};

struct CFG_Info {
	unsigned int cfg_id;
	std::map<int, std::map<std::string, Config_Value> > cfgs;
};

struct Shader_Info {
	unsigned int shader_id;
	std::map<int, gfx::Shader *> shaders;
};

struct JSON_Info {
	unsigned int json_id;
	std::map<int, util::JSON *> jsons;
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
	MML_Info *info = (MML_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.mml");
	if (info == nullptr) {
		info = new MML_Info;
		info->mml_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.mml", info);
	}
	return info;
}

static Sample_Info *sample_info(Program *prg)
{
	Sample_Info *info = (Sample_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.sample");
	if (info == nullptr) {
		info = new Sample_Info;
		info->sample_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.sample", info);
	}
	return info;
}

static Image_Info *image_info(Program *prg)
{
	Image_Info *info = (Image_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.image");
	if (info == nullptr) {
		info = new Image_Info;
		info->image_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.image", info);
	}
	return info;
}

static Font_Info *font_info(Program *prg)
{
	Font_Info *info = (Font_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.font");
	if (info == nullptr) {
		info = new Font_Info;
		info->font_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.font", info);
	}
	return info;
}

static Tilemap_Info *tilemap_info(Program *prg)
{
	Tilemap_Info *info = (Tilemap_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.tilemap");
	if (info == nullptr) {
		info = new Tilemap_Info;
		info->tilemap_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.tilemap", info);
	}
	return info;
}

static Sprite_Info *sprite_info(Program *prg)
{
	Sprite_Info *info = (Sprite_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.sprite");
	if (info == nullptr) {
		info = new Sprite_Info;
		info->sprite_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.sprite", info);
	}
	return info;
}

static CFG_Info *cfg_info(Program *prg)
{
	CFG_Info *info = (CFG_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.cfg");
	if (info == nullptr) {
		info = new CFG_Info;
		info->cfg_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.cfg", info);
	}
	return info;
}

static Shader_Info *shader_info(Program *prg)
{
	Shader_Info *info = (Shader_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.shader");
	if (info == nullptr) {
		info = new Shader_Info;
		info->shader_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.shader", info);
	}
	return info;
}

static JSON_Info *json_info(Program *prg)
{
	JSON_Info *info = (JSON_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.json");
	if (info == nullptr) {
		info = new JSON_Info;
		info->json_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.json", info);
	}
	return info;
}

static Model_Info *model_info(Program *prg)
{
	Model_Info *info = (Model_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.model");
	if (info == nullptr) {
		info = new Model_Info;
		info->model_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.model", info);
	}
	return info;
}

static Billboard_Info *billboard_info(Program *prg)
{
	Billboard_Info *info = (Billboard_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.billboard");
	if (info == nullptr) {
		info = new Billboard_Info;
		info->billboard_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.billboard", info);
	}
	return info;
}

static Widget_Info *widget_info(Program *prg)
{
	Widget_Info *info = (Widget_Info *)booboo::get_black_box(prg, "com.illnorth.booboo.widget");
	if (info == nullptr) {
		info = new Widget_Info;
		info->widget_id = 0;
		booboo::set_black_box(prg, "com.illnorth.booboo.widget", info);
	}
	return info;
}

static std::string save_dir()
{
	std::string path;

#ifdef ANDROID
	path = util::get_standard_path(util::SAVED_GAMES, true);
#elif defined _WIN32
	path = util::get_standard_path(util::SAVED_GAMES, true);
	path += "/" + shim::game_name;
	util::mkdir(path);
#else
	path = util::get_appdata_dir();
#endif

	return path;
}

static std::string cfg_path(std::string cfg_name)
{
	std::string path = save_dir() + "/" + cfg_name + ".txt";
	return path;
}

static std::map<std::string, Config_Value> load_cfg(Program *prg, std::string cfg_name)
{
	std::map<std::string, Config_Value> v;

	std::string text;

	try {
		text = util::load_text_from_filesystem(cfg_path(cfg_name));
	}
	catch (util::Error &e) {
		return v;
	}

	util::Tokenizer t(text, '\n');

	std::string line;

	while ((line = t.next()) != "") {
		util::Tokenizer t2(line, '=');
		std::string name = t2.next();
		std::string value = t2.next();
		util::trim(value);

		if (name == "") {
			continue;
		}

		Config_Value val;
		
		if (value.length() > 0 && value[0] == '"') {
			val.type = Variable::STRING;
			val.s = util::remove_quotes(value);
		}
		else {
			val.type = Variable::NUMBER;
			val.n = atof(value.c_str());
		}

		v[name] = val;
	}

	return v;
}

static bool save_cfg(Program *prg, int id, std::string cfg_name)
{
	FILE *f = fopen(cfg_path(cfg_name).c_str(), "w");
	if (f == nullptr) {
		return false;
	}

	std::map<std::string, Config_Value>::iterator it;

	CFG_Info *info = cfg_info(prg);

	for (it = info->cfgs[id].begin(); it != info->cfgs[id].end(); it++) {
		std::string name = (*it).first;
		Config_Value &v = (*it).second;
		if (IS_NUMBER(v)) {
			fprintf(f, "%s=%g\n", name.c_str(), v.n);
		}
		else {
			fprintf(f, "%s=\"%s\"\n", name.c_str(), v.s.c_str());
		}
	}

	fclose(f);

	return true;
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

	int millis = (int)as_number_inline(prg, v[0]);
	SDL_Delay(millis);
	return true;
}

static bool miscfunc_get_ticks(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_NUMBER(v1)	

	v1.n = SDL_GetTicks();

	return true;
}

static bool miscfunc_rand(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);

	int min_incl = as_number_inline(prg, v[1]);
	int max_incl = as_number_inline(prg, v[2]);

	CHECK_NUMBER(v1)

	v1.n = util::rand(min_incl, max_incl);

	return true;
}

static bool miscfunc_args(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_VECTOR(v1)	

	v1.v.clear();

	for (int i = 0; i < shim::argc; i++) {
		Variable var;
		var.type = Variable::STRING;
		var.name = "-booboo-";
		var.s = shim::argv[i];
		v1.v.push_back(var);
	}

	return true;
}

static bool miscfunc_get_logic_rate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_NUMBER(v1)

	v1.n = shim::logic_rate;

	return true;
}

static bool miscfunc_set_logic_rate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int rate = as_number_inline(prg, v[0]);

	if (rate < 1 || rate > 1000) {
		throw Error(std::string(__FUNCTION__) + ": " + "Logic rate must be between 1 and 1000 at " + get_error_info(prg));
	}

	shim::logic_rate = rate;

	return true;
}

static bool miscfunc_file_list(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_VECTOR(v1)	

	v1.v.clear();

	std::vector<std::string> l = shim::cpa->get_all_filenames();

	for (size_t i = 0; i < l.size(); i++) {
		Variable var;
		var.type = Variable::STRING;
		var.name = "-booboo-";
		var.s = l[i];
		v1.v.push_back(var);
	}

	return true;
}

static bool gfxfunc_set_scissor(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int x = as_number_inline(prg, v[0]);
	int y = as_number_inline(prg, v[1]);
	int w = as_number_inline(prg, v[2]);
	int h = as_number_inline(prg, v[3]);

	gfx::set_scissor(x, y, w, h);

	return true;
}

static bool gfxfunc_clear(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
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
	
	int w = as_number_inline(prg, v[0]);
	int h = as_number_inline(prg, v[1]);

	double aspect = (double)w/h;
	gfx::set_min_aspect_ratio(aspect-0.001f);
	gfx::set_max_aspect_ratio(aspect+0.001f);
	gfx::set_scaled_size({w, h});

	return true;
}

static bool gfxfunc_get_screen_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);

	CHECK_NUMBER(v1)
	CHECK_NUMBER(v2)	

	v1.n = shim::real_screen_size.w;
	v2.n = shim::real_screen_size.h;

	return true;
}

static bool gfxfunc_get_buffer_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);

	CHECK_NUMBER(v1)
	CHECK_NUMBER(v2)	

	v1.n = shim::screen_size.w;
	v2.n = shim::screen_size.h;

	return true;
}

static bool gfxfunc_get_screen_offset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);

	CHECK_NUMBER(v1)
	CHECK_NUMBER(v2)	

	v1.n = shim::screen_offset.x;
	v2.n = shim::screen_offset.y;

	return true;
}

static bool gfxfunc_get_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_NUMBER(v1)	

	v1.n = shim::scale;

	return true;
}

static bool gfxfunc_set_target(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);

	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(id) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Image at " + get_error_info(prg));
	}
#endif

	gfx::Image *img = info->images[id];

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

	double amount = as_number_inline(prg, v[0]);
	Uint32 duration = as_number_inline(prg, v[1]);

	gfx::screen_shake(amount, duration);

	return true;
}

static bool gfxfunc_add_notification(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string s = as_string_inline(prg, v[0]);

	gfx::add_notification(s);

	return true;
}

static bool gfxfunc_is_fullscreen(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_NUMBER(v1)

	v1.n = gfx::is_fullscreen();

	return true;
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

static bool gfxfunc_get_refresh_rate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_NUMBER(v1)

	v1.n = shim::refresh_rate;

	return true;
}

static bool gfxfunc_unset_scissor(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::unset_scissor();

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
	COUNT_ARGS(9)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
	c.a = as_number_inline(prg, v[3]);

	util::Point<float> p1, p2;

	p1.x = as_number_inline(prg, v[4]);
	p1.y = as_number_inline(prg, v[5]);
	p2.x = as_number_inline(prg, v[6]);
	p2.y = as_number_inline(prg, v[7]);

	float thick = as_number_inline(prg, v[8]);

	gfx::draw_line(c, p1, p2, thick);

	return true;
}

static bool primfunc_triangle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(11)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
	c.a = as_number_inline(prg, v[3]);
	double x1 = as_number_inline(prg, v[4]);
	double y1 = as_number_inline(prg, v[5]);
	double x2 = as_number_inline(prg, v[6]);
	double y2 = as_number_inline(prg, v[7]);
	double x3 = as_number_inline(prg, v[8]);
	double y3 = as_number_inline(prg, v[9]);
	double thick = as_number_inline(prg, v[10]);

	gfx::draw_triangle(c, util::Point<float>(x1, y1), util::Point<float>(x2, y2), util::Point<float>(x3, y3), thick);

	return true;
}

static bool primfunc_filled_triangle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(18)

	SDL_Colour c[3];
	c[0].r = as_number_inline(prg, v[0]);
	c[0].g = as_number_inline(prg, v[1]);
	c[0].b = as_number_inline(prg, v[2]);
	c[0].a = as_number_inline(prg, v[3]);
	c[1].r = as_number_inline(prg, v[4]);
	c[1].g = as_number_inline(prg, v[5]);
	c[1].b = as_number_inline(prg, v[6]);
	c[1].a = as_number_inline(prg, v[7]);
	c[2].r = as_number_inline(prg, v[8]);
	c[2].g = as_number_inline(prg, v[9]);
	c[2].b = as_number_inline(prg, v[10]);
	c[2].a = as_number_inline(prg, v[11]);

	util::Point<float> p1, p2, p3;

	p1.x = as_number_inline(prg, v[12]);
	p1.y = as_number_inline(prg, v[13]);
	p2.x = as_number_inline(prg, v[14]);
	p2.y = as_number_inline(prg, v[15]);
	p3.x = as_number_inline(prg, v[16]);
	p3.y = as_number_inline(prg, v[17]);

	gfx::draw_filled_triangle(c, p1, p2, p3);

	return true;
}

static bool primfunc_rectangle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
	c.a = as_number_inline(prg, v[3]);

	util::Point<float> p;
	util::Size<float> sz;

	p.x = as_number_inline(prg, v[4]);
	p.y = as_number_inline(prg, v[5]);
	sz.w = as_number_inline(prg, v[6]);
	sz.h = as_number_inline(prg, v[7]);

	float thick = as_number_inline(prg, v[8]);

	gfx::draw_rectangle(c, p, sz, thick);

	return true;
}

static bool primfunc_filled_rectangle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(20)

	SDL_Colour c[4];
	c[0].r = as_number_inline(prg, v[0]);
	c[0].g = as_number_inline(prg, v[1]);
	c[0].b = as_number_inline(prg, v[2]);
	c[0].a = as_number_inline(prg, v[3]);
	c[1].r = as_number_inline(prg, v[4]);
	c[1].g = as_number_inline(prg, v[5]);
	c[1].b = as_number_inline(prg, v[6]);
	c[1].a = as_number_inline(prg, v[7]);
	c[2].r = as_number_inline(prg, v[8]);
	c[2].g = as_number_inline(prg, v[9]);
	c[2].b = as_number_inline(prg, v[10]);
	c[2].a = as_number_inline(prg, v[11]);
	c[3].r = as_number_inline(prg, v[12]);
	c[3].g = as_number_inline(prg, v[13]);
	c[3].b = as_number_inline(prg, v[14]);
	c[3].a = as_number_inline(prg, v[15]);

	util::Point<float> p;

	p.x = as_number_inline(prg, v[16]);
	p.y = as_number_inline(prg, v[17]);

	util::Size<float> sz;

	sz.w = as_number_inline(prg, v[18]);
	sz.h = as_number_inline(prg, v[19]);

	gfx::draw_filled_rectangle(c, p, sz);

	return true;
}

static bool primfunc_ellipse(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(10)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
	c.a = as_number_inline(prg, v[3]);

	util::Point<float> p;

	p.x = as_number_inline(prg, v[4]);
	p.y = as_number_inline(prg, v[5]);

	double _rx = as_number_inline(prg, v[6]);
	double _ry = as_number_inline(prg, v[7]);
	double thick = as_number_inline(prg, v[8]);
	double _sections = as_number_inline(prg, v[9]);

	gfx::draw_ellipse(c, p, _rx, _ry, thick, _sections);

	return true;
}

static bool primfunc_filled_ellipse(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
	c.a = as_number_inline(prg, v[3]);

	util::Point<float> p;

	p.x = as_number_inline(prg, v[4]);
	p.y = as_number_inline(prg, v[5]);

	double _rx = as_number_inline(prg, v[6]);
	double _ry = as_number_inline(prg, v[7]);
	double _sections = as_number_inline(prg, v[8]);

	gfx::draw_filled_ellipse(c, p, _rx, _ry, _sections);

	return true;
}

static bool primfunc_circle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
	c.a = as_number_inline(prg, v[3]);

	util::Point<float> p;

	p.x = as_number_inline(prg, v[4]);
	p.y = as_number_inline(prg, v[5]);
	float _r = as_number_inline(prg, v[6]);
	float thick = as_number_inline(prg, v[7]);
	int _sections = as_number_inline(prg, v[8]);

	gfx::draw_circle(c, p, _r, thick, _sections);

	return true;
}

static bool primfunc_filled_circle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(8)

	SDL_Colour c;
	c.r = as_number_inline(prg, v[0]);
	c.g = as_number_inline(prg, v[1]);
	c.b = as_number_inline(prg, v[2]);
	c.a = as_number_inline(prg, v[3]);

	util::Point<float> p;

	p.x = as_number_inline(prg, v[4]);
	p.y = as_number_inline(prg, v[5]);
	double _r = as_number_inline(prg, v[6]);
	int _sections = as_number_inline(prg, v[7]);

	gfx::draw_filled_circle(c, p, _r, _sections);

	return true;
}

static bool mmlfunc_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string str = as_string_inline(prg, v[1]);
	
	MML_Info *info = mml_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->mml_id;

	Uint8 *bytes = (Uint8 *)str.c_str();
	SDL_RWops *file = SDL_RWFromMem(bytes, str.length());
	audio::MML *mml = new audio::MML(file); // this closes the file

	info->mmls[info->mml_id++] = mml;

	return true;
}

static bool mmlfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	MML_Info *info = mml_info(prg);
	delete info->mmls[id];
	info->mmls.erase(id);

	return true;
}

static bool mmlfunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	MML_Info *info = mml_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->mml_id;

	audio::MML *mml = new audio::MML(name);

	info->mmls[info->mml_id++] = mml;

	return true;
}

static bool mmlfunc_play(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	double volume = as_number_inline(prg, v[1]);
	bool loop = as_number_inline(prg, v[2]);

	MML_Info *info = mml_info(prg);

#ifdef DEBUG
	if (info->mmls.find(id) == info->mmls.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid MML at " + get_error_info(prg));
	}
#endif

	audio::MML *mml = info->mmls[id];

	mml->play(shim::music_volume*volume, loop);

	return true;
}

static bool mmlfunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);

	MML_Info *info = mml_info(prg);

#ifdef DEBUG
	if (info->mmls.find(id) == info->mmls.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid MML at " + get_error_info(prg));
	}
#endif

	audio::MML *mml = info->mmls[id];

	mml->stop();

	return true;
}

static bool samplefunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	Sample_Info *info = sample_info(prg);

	CHECK_NUMBER(v1)
		
	v1.n = info->sample_id;

	audio::Sample *sample = new audio::Sample(name);

	info->samples[info->sample_id++] = sample;

	return true;
}

static bool samplefunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Sample_Info *info = sample_info(prg);
	delete info->samples[id];
	info->samples.erase(id);

	return true;
}

static bool samplefunc_play(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	double volume = as_number_inline(prg, v[1]);
	bool loop = as_number_inline(prg, v[2]);

	Sample_Info *info = sample_info(prg);

#ifdef DEBUG
	if (info->samples.find(id) == info->samples.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Sample at " + get_error_info(prg));
	}
#endif

	audio::Sample *sample = info->samples[id];

	sample->play(shim::music_volume * volume, loop);

	return true;
}

static bool samplefunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);

	Sample_Info *info = sample_info(prg);

#ifdef DEBUG
	if (info->samples.find(id) == info->samples.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Sample at " + get_error_info(prg));
	}
#endif

	audio::Sample *sample = info->samples[id];

	sample->stop();

	return true;
}

static bool imagefunc_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);

	int w = as_number_inline(prg, v[1]);
	int h = as_number_inline(prg, v[2]);

	Image_Info *info = image_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->image_id;

	gfx::Image *img = new gfx::Image(util::Size<int>(w, h));

	info->images[info->image_id++] = img;

	return true;
}

static bool imagefunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	Image_Info *info = image_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->image_id;

	gfx::Image *img = new gfx::Image(name);

	info->images[info->image_id++] = img;

	return true;
}

static bool imagefunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Image_Info *info = image_info(prg);
	delete info->images[id];
	info->images.erase(id);

	return true;
}

static bool imagefunc_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	int id = as_number_inline(prg, v[0]);
	double r = as_number_inline(prg, v[1]);
	double g = as_number_inline(prg, v[2]);
	double b = as_number_inline(prg, v[3]);
	double a = as_number_inline(prg, v[4]);
	double x = as_number_inline(prg, v[5]);
	double y = as_number_inline(prg, v[6]);
	double flip_h = as_number_inline(prg, v[7]);
	double flip_v = as_number_inline(prg, v[8]);

	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(id) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Image at " + get_error_info(prg));
	}
#endif

	gfx::Image *img = info->images[id];

	SDL_Colour c;
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
	COUNT_ARGS(15)

	int id = as_number_inline(prg, v[0]);
	double r = as_number_inline(prg, v[1]);
	double g = as_number_inline(prg, v[2]);
	double b = as_number_inline(prg, v[3]);
	double a = as_number_inline(prg, v[4]);
	double sx = as_number_inline(prg, v[5]);
	double sy = as_number_inline(prg, v[6]);
	double sw = as_number_inline(prg, v[7]);
	double sh = as_number_inline(prg, v[8]);
	double dx = as_number_inline(prg, v[9]);
	double dy = as_number_inline(prg, v[10]);
	double dw = as_number_inline(prg, v[11]);
	double dh = as_number_inline(prg, v[12]);
	double flip_h = as_number_inline(prg, v[13]);
	double flip_v = as_number_inline(prg, v[14]);
	
	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(id) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Image at " + get_error_info(prg));
	}
#endif

	gfx::Image *img = info->images[id];

	SDL_Colour c;
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
	COUNT_ARGS(14)

	int id = as_number_inline(prg, v[0]);
	double r = as_number_inline(prg, v[1]);
	double g = as_number_inline(prg, v[2]);
	double b = as_number_inline(prg, v[3]);
	double a = as_number_inline(prg, v[4]);
	double cx = as_number_inline(prg, v[5]);
	double cy = as_number_inline(prg, v[6]);
	double x = as_number_inline(prg, v[7]);
	double y = as_number_inline(prg, v[8]);
	double angle = as_number_inline(prg, v[9]);
	double scale_x = as_number_inline(prg, v[10]);
	double scale_y = as_number_inline(prg, v[11]);
	double flip_h = as_number_inline(prg, v[12]);
	double flip_v = as_number_inline(prg, v[13]);

	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(id) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Image at " + get_error_info(prg));
	}
#endif

	gfx::Image *img = info->images[id];

	SDL_Colour c;
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

	double img = as_number_inline(prg, v[0]);

	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(img) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Unknown image at " + get_error_info(prg));
	}
#endif

	gfx::Image *image = info->images[img];

	image->start_batch();

	return true;
}

static bool imagefunc_end(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double img = as_number_inline(prg, v[0]);

	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(img) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Unknown image at " + get_error_info(prg));
	}
#endif

	gfx::Image *image = info->images[img];

	image->end_batch();

	return true;
}

static bool imagefunc_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);

	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	
	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(id) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid image at " + get_error_info(prg));
	}
#endif

	gfx::Image *img = info->images[id];

	CHECK_NUMBER(v1)
	CHECK_NUMBER(v2)
	
	v1.n = img->size.w;
	v2.n = img->size.h;

	return true;
}

static bool imagefunc_draw_9patch(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	int id = as_number_inline(prg, v[0]);
	int r = as_number_inline(prg, v[1]);
	int g = as_number_inline(prg, v[2]);
	int b = as_number_inline(prg, v[3]);
	int a = as_number_inline(prg, v[4]);
	double x = as_number_inline(prg, v[5]);
	double y = as_number_inline(prg, v[6]);
	int w = as_number_inline(prg, v[7]);
	int h = as_number_inline(prg, v[8]);

	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(id) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid image at " + get_error_info(prg));
	}
#endif

	gfx::Image *img = info->images[id];

	SDL_Colour c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	gfx::draw_9patch_tinted(c, img, util::Point<float>(x, y), util::Size<int>(w, h));


	return true;
}

static bool fontfunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	std::string name = as_string_inline(prg, v[1]);
	int size = as_number_inline(prg, v[2]);
	bool smooth = as_number_inline(prg, v[3]);

	Variable &v1 = as_variable_inline(prg, v[0]);

	Font_Info *info = font_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->font_id;

	gfx::TTF *font = new gfx::TTF(name, size, 1024);
	font->set_smooth(smooth);

	info->fonts[info->font_id++] = font;

	return true;
}

static bool fontfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Font_Info *info = font_info(prg);
	delete info->fonts[id];
	info->fonts.erase(id);

	return true;
}

static bool fontfunc_draw(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(8)

	int id = as_number_inline(prg, v[0]);
	double r = as_number_inline(prg, v[1]);
	double g = as_number_inline(prg, v[2]);
	double b = as_number_inline(prg, v[3]);
	double a = as_number_inline(prg, v[4]);
	std::string text = as_string_inline(prg, v[5]);
	double x = as_number_inline(prg, v[6]);
	double y = as_number_inline(prg, v[7]);
	bool rtl;

	if (v.size() > 8) {
		rtl = as_number_inline(prg, v[8]);
	}
	else {
		rtl = false;
	}

	Font_Info *info = font_info(prg);

#ifdef DEBUG
	if (info->fonts.find(id) == info->fonts.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Font at " + get_error_info(prg));
	}
#endif

	gfx::TTF *font = info->fonts[id];

	SDL_Colour c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	font->draw(c, text, util::Point<float>(x, y), true, false, true, rtl);

	return true;
}

static bool fontfunc_width(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string text = as_string_inline(prg, v[2]);
	
	Font_Info *info = font_info(prg);

	gfx::TTF *font = info->fonts[id];

	int w = font->get_text_width(text);

	CHECK_NUMBER(v1)
	
	v1.n = w;

	return true;
}

static bool fontfunc_height(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Font_Info *info = font_info(prg);

	gfx::TTF *font = info->fonts[id];

	int h = font->get_height();

	CHECK_NUMBER(v1)

	v1.n = h;

	return true;
}

static bool fontfunc_add_extra_glyph(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	int glyph_id = as_number_inline(prg, v[1]);
	int image_id = as_number_inline(prg, v[2]);
	
	Font_Info *info = font_info(prg);
	gfx::TTF *font = info->fonts[id];

	Image_Info *iinfo = image_info(prg);
	gfx::Image *image = iinfo->images[image_id];

	font->add_extra_glyph(glyph_id, image);

	return true;
}

static bool tilemapfunc_set_tile_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	shim::tile_size = as_number_inline(prg, v[0]);

	return true;
}

static bool tilemapfunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	
	Tilemap_Info *info = tilemap_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->tilemap_id;

	gfx::Tilemap *tilemap = new gfx::Tilemap(name);

	info->tilemaps[info->tilemap_id++] = tilemap;

	return true;
}

static bool tilemapfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Tilemap_Info *info = tilemap_info(prg);
	delete info->tilemaps[id];
	info->tilemaps.erase(id);

	return true;
}

static bool tilemapfunc_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int id = as_number_inline(prg, v[0]);
	int start_layer = as_number_inline(prg, v[1]);
	int end_layer = as_number_inline(prg, v[2]);
	double x = as_number_inline(prg, v[3]);
	double y = as_number_inline(prg, v[4]);
	
	Tilemap_Info *info = tilemap_info(prg);

	gfx::Tilemap *tilemap = info->tilemaps[id];
	
	tilemap->draw(start_layer, end_layer, util::Point<float>(x, y));

	return true;
}

static bool tilemapfunc_num_layers(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Tilemap_Info *info = tilemap_info(prg);

	CHECK_NUMBER(v1)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	v1.n = tilemap->get_num_layers();

	return true;
}

static bool tilemapfunc_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	
	Tilemap_Info *info = tilemap_info(prg);

	CHECK_NUMBER(v1)
	CHECK_NUMBER(v2)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	util::Size<int> sz = tilemap->get_size();

	v1.n = sz.w;
	v2.n = sz.h;

	return true;
}

static bool tilemapfunc_is_solid(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	int x = as_number_inline(prg, v[2]);
	int y = as_number_inline(prg, v[3]);
	
	Tilemap_Info *info = tilemap_info(prg);

	CHECK_NUMBER(v1)

	gfx::Tilemap *tilemap = info->tilemaps[id];

	v1.n = tilemap->is_solid(-1, util::Point<int>(x, y));

	return true;
}

static bool tilemapfunc_get_groups(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &vec = as_variable_inline(prg, v[1]);

	CHECK_VECTOR(vec)
	
	Tilemap_Info *info = tilemap_info(prg);

	gfx::Tilemap *tilemap = info->tilemaps[id];

	std::vector<gfx::Tilemap::Group> &groups = tilemap->get_groups();

	for (size_t i = 0; i < groups.size(); i++) {
		gfx::Tilemap::Group &g = groups[i];
		Variable v;
		v.type = Variable::VECTOR;
		v.name = "-booboo-";
		Variable type;
		type.type = Variable::NUMBER;
		type.name = "-booboo-";
		type.n = g.type;
		v.v.push_back(type);
		Variable x;
		x.type = Variable::NUMBER;
		x.name = "-booboo-";
		x.n = g.x;
		v.v.push_back(x);
		Variable y;
		y.type = Variable::NUMBER;
		y.name = "-booboo-";
		y.n = g.y;
		v.v.push_back(y);
		Variable w;
		w.type = Variable::NUMBER;
		w.name = "-booboo-";
		w.n = g.w;
		v.v.push_back(w);
		Variable h;
		h.type = Variable::NUMBER;
		h.name = "-booboo-";
		h.n = g.h;
		v.v.push_back(h);
		vec.v.push_back(v);
	}

	return true;
}

static bool tilemapfunc_set_animated_tiles(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int id = as_number_inline(prg, v[0]);
	int delay = (int)as_number_inline(prg, v[1]);
	int w = (int)as_number_inline(prg, v[2]);
	int h = (int)as_number_inline(prg, v[3]);
	Variable v1 = as_variable_resolve_inline(prg, v[4]);
	
	Tilemap_Info *info = tilemap_info(prg);

	CHECK_VECTOR(v1)

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

static bool tilemapfunc_find_path(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(7)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable entity_solids = as_variable_resolve_inline(prg, v[2]);
	int start_x = (int)as_number_inline(prg, v[3]);
	int start_y = (int)as_number_inline(prg, v[4]);
	int end_x = (int)as_number_inline(prg, v[5]);
	int end_y = (int)as_number_inline(prg, v[6]);

	CHECK_VECTOR(v1)
	CHECK_VECTOR(entity_solids)	

	Tilemap_Info *info = tilemap_info(prg);

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

	v1.v.clear();

	for (std::list<util::A_Star::Node>::iterator it = path.begin(); it != path.end(); it++) {
		util::A_Star::Node &n = *it;
		Variable vec;
		vec.type = Variable::VECTOR;
		vec.name = "-booboo-";
		Variable x;
		x.type = Variable::NUMBER;
		x.name = "-booboo-";
		x.n = n.position.x;
		vec.v.push_back(x);
		Variable y;
		y.type = Variable::NUMBER;
		y.name = "-booboo-";
		y.n = n.position.y;
		vec.v.push_back(y);
		v1.v.push_back(vec);
	}

	delete a_star;

	return true;
}

static bool tilemapfunc_set_solid(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int id = as_number_inline(prg, v[0]);
	int x = as_number_inline(prg, v[1]);
	int y = as_number_inline(prg, v[2]);
	bool solid = as_number_inline(prg, v[3]);
	
	Tilemap_Info *info = tilemap_info(prg);

	gfx::Tilemap *tilemap = info->tilemaps[id];

	tilemap->set_solid(-1, util::Point<int>(x, y), util::Size<int>(1, 1), solid);

	return true;
}

static bool tilemapfunc_set_tile(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(7)

	int id = as_number_inline(prg, v[0]);
	int layer = as_number_inline(prg, v[1]);
	int x = as_number_inline(prg, v[2]);
	int y = as_number_inline(prg, v[3]);
	int tile_x = as_number_inline(prg, v[4]);
	int tile_y = as_number_inline(prg, v[5]);
	bool solid = as_number_inline(prg, v[6]);
	
	Tilemap_Info *info = tilemap_info(prg);

	gfx::Tilemap *tilemap = info->tilemaps[id];

	tilemap->set_tile(layer, util::Point<int>(x, y), util::Point<int>(tile_x, tile_y), solid);

	return true;
}

static bool tilemapfunc_get_tile(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(7)

	int id = as_number_inline(prg, v[0]);
	Variable &vx = as_variable_inline(prg, v[1]);
	Variable &vy = as_variable_inline(prg, v[2]);
	Variable &vs = as_variable_inline(prg, v[3]);
	int layer = as_number_inline(prg, v[4]);
	int x = as_number_inline(prg, v[5]);
	int y = as_number_inline(prg, v[6]);

	CHECK_NUMBER(vx)
	CHECK_NUMBER(vy)
	CHECK_NUMBER(vs)
	
	Tilemap_Info *info = tilemap_info(prg);

	gfx::Tilemap *tilemap = info->tilemaps[id];

	util::Point<int> tile_xy;
	bool solid;

	tilemap->get_tile(layer, util::Point<int>(x, y), tile_xy, solid);

	vx.n = tile_xy.x;
	vy.n = tile_xy.y;
	vs.n = solid;

	return true;
}

static bool spritefunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->sprite_id;

	gfx::Sprite *sprite = new gfx::Sprite(name, name);

	info->sprites[info->sprite_id++] = sprite;

	return true;
}

static bool spritefunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Sprite_Info *info = sprite_info(prg);
	delete info->sprites[id];
	info->sprites.erase(id);

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
	v.push_back(t);
	call_void_function(d->prg, d->function, v, 0);
	delete d;
}

static bool spritefunc_set_animation_lazy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::string anim = as_string_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	sprite->set_animation_lazy(anim);

	return true;
}

static bool spritefunc_set_animation(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::string anim = as_string_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	if (v.size() > 2) {
		Sprite_Callback_Data *d = new Sprite_Callback_Data;
		d->prg = prg;
		d->function = as_function_inline(prg, v[2]);
		d->id = id;
		sprite->set_animation(anim, sprite_callback, d);
	}
	else {
		sprite->set_animation(anim);
	}

	return true;
}

static bool spritefunc_get_animation(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_STRING(v1)

	gfx::Sprite *sprite = info->sprites[id];

	v1.s = sprite->get_animation();

	return true;
}

static bool spritefunc_get_previous_animation(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_STRING(v1)

	gfx::Sprite *sprite = info->sprites[id];

	v1.s = sprite->get_previous_animation();

	return true;
}

static bool spritefunc_current_frame(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_current_frame();

	return true;
}

static bool spritefunc_num_frames(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_num_frames();

	return true;
}

static bool spritefunc_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_length();

	return true;
}

static bool spritefunc_current_frame_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)
	CHECK_NUMBER(v2)

	gfx::Sprite *sprite = info->sprites[id];

	gfx::Image *img = sprite->get_current_image();

	v1.n = img->size.w;
	v2.n = img->size.h;

	return true;
}

static bool spritefunc_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	int id = as_number_inline(prg, v[0]);
	int r = as_number_inline(prg, v[1]);
	int g = as_number_inline(prg, v[2]);
	int b = as_number_inline(prg, v[3]);
	int a = as_number_inline(prg, v[4]);
	double dx = as_number_inline(prg, v[5]);
	double dy = as_number_inline(prg, v[6]);
	int flip_h = as_number_inline(prg, v[7]);
	int flip_v = as_number_inline(prg, v[8]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	gfx::Image *img = sprite->get_current_image();

	int flags = 0;
	if (flip_h != 0.0) {
		flags |= gfx::Image::FLIP_H;
	}
	if (flip_v != 0.0) {
		flags |= gfx::Image::FLIP_V;
	}

	SDL_Colour tint;
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

	int id = as_number_inline(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	sprite->start();

	return true;
}

static bool spritefunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number_inline(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	sprite->stop();

	return true;
}

static bool spritefunc_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number_inline(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	sprite->reset();

	return true;
}

static bool spritefunc_bounds(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	Variable &v3 = as_variable_inline(prg, v[3]);
	Variable &v4 = as_variable_inline(prg, v[4]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)
	CHECK_NUMBER(v2)
	CHECK_NUMBER(v3)
	CHECK_NUMBER(v4)

	gfx::Sprite *sprite = info->sprites[id];

	util::Point<int> topleft;
	util::Point<int> bottomright;

	sprite->get_bounds(topleft, bottomright);

	v1.n = topleft.x;
	v2.n = topleft.y;
	v3.n = bottomright.x;
	v4.n = bottomright.y;

	return true;
}

static bool spritefunc_elapsed(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_elapsed();

	return true;
}

static bool spritefunc_frame_times(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &vec = as_variable_inline(prg, v[1]);

	CHECK_VECTOR(vec)	

	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	std::vector<Uint32> times = sprite->get_frame_times();

	for (size_t i = 0; i < times.size(); i++) {
		Variable v;
		v.type = Variable::NUMBER;
		v.name = "-booboo-";
		v.n = times[i];
		vec.v.push_back(v);
	}

	return true;
}

static bool spritefunc_is_started(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	CHECK_NUMBER(v1)

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->is_started();

	return true;
}

static void set_string_or_number(Program *prg, int index, double value)
{
	Variable &v1 = booboo::get_variable(prg, index);

	CHECK_NUMBER(v1)
       
	v1.n = value;
}

static bool joyfunc_poll(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(21)

	double num = as_number_inline(prg, v[0]);
	int x1 = v[1].i;
	int y1 = v[2].i;
	int x2 = v[3].i;
	int y2 = v[4].i;
	int x3 = v[5].i;
	int y3 = v[6].i;
	int l = v[7].i;
	int r = v[8].i;
	int u = v[9].i;
	int d = v[10].i;
	int a = v[11].i;
	int b = v[12].i;
	int x = v[13].i;
	int y = v[14].i;
	int lb = v[15].i;
	int rb = v[16].i;
	int ls = v[17].i;
	int rs = v[18].i;
	int back = v[19].i;
	int start = v[20].i;

	SDL_JoystickID id = input::get_controller_id(num);
	SDL_GameController *gc = input::get_sdl_gamecontroller(id);
	bool connected = gc != nullptr;

	if (connected == false) {
		set_string_or_number(prg, x1, 0);
		set_string_or_number(prg, y1, 0);
		set_string_or_number(prg, x2, 0);
		set_string_or_number(prg, y2, 0);
		set_string_or_number(prg, a, 0);
		set_string_or_number(prg, l, 0);
		set_string_or_number(prg, r, 0);
		set_string_or_number(prg, u, 0);
		set_string_or_number(prg, d, 0);
		set_string_or_number(prg, b, 0);
		set_string_or_number(prg, x, 0);
		set_string_or_number(prg, y, 0);
		set_string_or_number(prg, lb, 0);
		set_string_or_number(prg, rb, 0);
		set_string_or_number(prg, back, 0);
		set_string_or_number(prg, start, 0);
		set_string_or_number(prg, ls, 0);
		set_string_or_number(prg, rs, 0);
		set_string_or_number(prg, x3, 0);
		set_string_or_number(prg, y3, 0);
	}
	else {

		Sint16 si_x1 = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTX);
		Sint16 si_y1 = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY);
		Sint16 si_x2 = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTX);
		Sint16 si_y2 = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTY);
		Sint16 si_x3 = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
		Sint16 si_y3 = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

		double x1f;
		double y1f;
		double x2f;
		double y2f;
		double x3f;
		double y3f;

		if (si_x1 < 0) {
			x1f = si_x1 / 32768.0;
		}
		else {
			x1f = si_x1 / 32767.0;
		}

		if (si_y1 < 0) {
			y1f = si_y1 / 32768.0;
		}
		else {
			y1f = si_y1 / 32767.0;
		}

		if (si_x2 < 0) {
			x2f = si_x2 / 32768.0;
		}
		else {
			x2f = si_x2 / 32767.0;
		}

		if (si_y2 < 0) {
			y2f = si_y2 / 32768.0;
		}
		else {
			y2f = si_y2 / 32767.0;
		}

		if (si_x3 < 0) {
			x3f = si_x3 / 32768.0;
		}
		else {
			x3f = si_x3 / 32767.0;
		}

		if (si_y3 < 0) {
			y3f = si_y3 / 32768.0;
		}
		else {
			y3f = si_y3 / 32767.0;
		}

		set_string_or_number(prg, x1, x1f);
		set_string_or_number(prg, y1, y1f);

		set_string_or_number(prg, x2, x2f);
		set_string_or_number(prg, y2, y2f);

		set_string_or_number(prg, x3, x3f);
		set_string_or_number(prg, y3, y3f);

		double ab = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_A);
		double bb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_B);
		double xb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_X);
		double yb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_Y);
		double lbb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_LB);
		double rbb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_RB);
		double backb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_BACK);
		double startb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_START);
		double lsb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_LS);
		double rsb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_RS);
		double _lb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_L);
		double _rb = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_R);
		double ub = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_U);
		double db = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)TGUI_B_D);

		set_string_or_number(prg, l, _lb);
		set_string_or_number(prg, r, _rb);
		set_string_or_number(prg, u, ub);
		set_string_or_number(prg, d, db);
		set_string_or_number(prg, a, ab);
		set_string_or_number(prg, b, bb);
		set_string_or_number(prg, x, xb);
		set_string_or_number(prg, y, yb);
		set_string_or_number(prg, lb, lbb);
		set_string_or_number(prg, rb, rbb);
		set_string_or_number(prg, ls, lsb);
		set_string_or_number(prg, rs, rsb);
		set_string_or_number(prg, back, backb);
		set_string_or_number(prg, start, startb);
	}

	return true;
}

static bool joyfunc_count(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	CHECK_NUMBER(v1)
	
	v1.n = input::get_num_joysticks();

	return true;
}

static bool joyfunc_rumble(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int ms = as_number_inline(prg, v[0]);
	int num = -1;

	if (v.size() > 1) {
		num = as_number_inline(prg, v[1]);
	}

	input::rumble(ms, num);

	return true;
}

static bool cfgfunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string cfg_name = as_string_inline(prg, v[1]);

	CFG_Info *info = cfg_info(prg);

	CHECK_NUMBER(v1)
	
	std::map<std::string, Config_Value> val = load_cfg(prg, cfg_name);
	int id = info->cfg_id++;
	v1.n = id;
	info->cfgs[id] = val;

	return true;
}

static bool cfgfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	CFG_Info *info = cfg_info(prg);
	info->cfgs.erase(id);

	return true;
}

static bool cfgfunc_save(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string cfg_name = as_string_inline(prg, v[2]);

	bool success = save_cfg(prg, id, cfg_name);

	CHECK_NUMBER(v1)	
	
	v1.n = success;

	return true;
}

static bool cfgfunc_get_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

	CHECK_NUMBER(v2)

#ifdef DEBUG
	if (info->cfgs.find(id) == info->cfgs.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid CFG Value at " + get_error_info(prg));
	}
#endif
	       	
	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		return true;
	}

	v2.n = info->cfgs[id][name].n;

	return true;
}

static bool cfgfunc_get_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

	CHECK_STRING(v2)

#ifdef DEBUG
	if (info->cfgs.find(id) == info->cfgs.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid CFG Value at " + get_error_info(prg));
	}
#endif

	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		return true;
	}

	v2.s = info->cfgs[id][name].s;

	return true;
}

static bool cfgfunc_set_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	double val = as_number_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

#ifdef DEBUG
	if (info->cfgs.find(id) == info->cfgs.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid CFG Value at " + get_error_info(prg));
	}
#endif

	Config_Value value;
	value.type = Variable::NUMBER;
	value.n = val;

	info->cfgs[id][name] = value;

	return true;
}

static bool cfgfunc_set_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	std::string val = as_string_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

#ifdef DEBUG
	if (info->cfgs.find(id) == info->cfgs.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid CFG Value at " + get_error_info(prg));
	}
#endif

	Config_Value value;
	value.type = Variable::STRING;
	value.s = val;

	info->cfgs[id][name] = value;

	return true;
}

static bool cfgfunc_exists(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

#ifdef DEBUG
	if (info->cfgs.find(id) == info->cfgs.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid CFG Value at " + get_error_info(prg));
	}
#endif

	bool found = info->cfgs[id].find(name) != info->cfgs[id].end();

	CHECK_NUMBER(v1)
	
	v1.n = found;

	return true;
}

static bool cfgfunc_erase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	CFG_Info *info = cfg_info(prg);

#ifdef DEBUG
	if (info->cfgs.find(id) == info->cfgs.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid CFG Value at " + get_error_info(prg));
	}
#endif

	std::map<std::string, Config_Value>::iterator it;
	if ((it = info->cfgs[id].find(name)) == info->cfgs[id].end()) {
		return true;
	}
	info->cfgs[id].erase(it);

	return true;
}

static bool shaderfunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);

	std::string fname = as_string_inline(prg, v[1]);

	gfx::Shader::Precision vp = gfx::Shader::MEDIUM;
	gfx::Shader::Precision fp = (gfx::Shader::Precision)as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->shader_id;

	gfx::Shader *shader = nullptr;

	if (shim::opengl) {
		std::string vs = DEFAULT_GLSL_VERTEX_SHADER;
		std::string fs = util::load_text("gfx/shaders/glsl/" + fname + ".txt");

		gfx::Shader::OpenGL_Shader *vert = gfx::Shader::load_opengl_vertex_shader(vs, vp);
		gfx::Shader::OpenGL_Shader *frag = gfx::Shader::load_opengl_fragment_shader(fs, fp);

		shader = new gfx::Shader(vert, frag);
	}
#ifdef _WIN32
	else {
		gfx::Shader::D3D_Vertex_Shader *vert = gfx::Shader::load_d3d_vertex_shader("default_vertex");
		gfx::Shader::D3D_Fragment_Shader *frag = gfx::Shader::load_d3d_fragment_shader(fname);

		shader = new gfx::Shader(vert, frag);
	}
#endif

	info->shaders[info->shader_id++] = shader;

	return true;
}

static bool shaderfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Shader_Info *info = shader_info(prg);
	delete info->shaders[id];
	info->shaders.erase(id);

	return true;
}

static bool shaderfunc_use(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	
	Shader_Info *info = shader_info(prg);
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

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	bool b = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	shader->set_bool(name, b);

	return true;
}

static bool shaderfunc_set_int(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	int i = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	shader->set_int(name, i);

	return true;
}

static bool shaderfunc_set_float(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	double f = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	shader->set_float(name, f);

	return true;
}

static bool shaderfunc_set_texture(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	double t = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	Image_Info *info2 = image_info(prg);
	gfx::Image *img = info2->images[t];

	shader->set_texture(name, img);

	return true;
}

static bool shaderfunc_set_colour(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	int id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	int r = as_number_inline(prg, v[2]);
	int g = as_number_inline(prg, v[3]);
	int b = as_number_inline(prg, v[4]);
	int a = as_number_inline(prg, v[5]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	SDL_Colour c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	shader->set_colour(name, c);

	return true;
}

static bool jsonfunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	std::string name = as_string_inline(prg, v[1]);

	JSON_Info *info = json_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->json_id;

	util::JSON *json = new util::JSON(name);

	info->jsons[info->json_id++] = json;

	return true;
}

static bool jsonfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	JSON_Info *info = json_info(prg);
	delete info->jsons[id];
	info->jsons.erase(id);

	return true;
}

static bool jsonfunc_get_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);
	
	JSON_Info *info = json_info(prg);
	util::JSON *json = info->jsons[id];

	CHECK_STRING(v1)
	
	util::JSON::Node *n = json->get_root()->find(name);
	v1.s = n->as_string();

	return true;
}

static bool jsonfunc_get_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);
	
	JSON_Info *info = json_info(prg);
	util::JSON *json = info->jsons[id];

	CHECK_NUMBER(v1)
	
	util::JSON::Node *n = json->get_root()->find(name);
	v1.n = n->as_double();

	return true;
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
}

void set_2d()
{
	gfx::set_default_projection(shim::screen_size, shim::screen_offset, shim::scale);
	gfx::update_projection();

	gfx::apply_screen_shake();

	is_3d = false;
}

static bool modelfunc_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	Model_Info *info = model_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->model_id;

	gfx::Model *model = new gfx::Model(name);

	Model *m = new Model;
	m->mat = glm::mat4();
	m->model = model;
	m->is_clone = false;

	info->models[info->model_id++] = m;

	return true;
}

static bool modelfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Model_Info *info = model_info(prg);
	if (info->models[id]->is_clone == false) {
		delete info->models[id]->model;
	}
	delete info->models[id];
	info->models.erase(id);

	return true;
}

static bool modelfunc_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int model_id = as_number_inline(prg, v[0]);
	int r = as_number_inline(prg, v[1]);
	int g = as_number_inline(prg, v[2]);
	int b = as_number_inline(prg, v[3]);
	int a = as_number_inline(prg, v[4]);

	Model_Info *info = model_info(prg);
	Model *model = info->models[model_id];

	SDL_Colour c;
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
/*
	if (shim::opengl) {
		glDisable_ptr(GL_SCISSOR_TEST);
	}
#ifdef _WIN32
	else {
		shim::d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
#endif
*/

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

	int model_id = as_number_inline(prg, v[0]);

	Model_Info *info = model_info(prg);

	Model *model = info->models[model_id];

	model->mat = glm::mat4();

	return true;
}

static bool modelfunc_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int model_id = as_number_inline(prg, v[0]);
	double sx = as_number_inline(prg, v[1]);
	double sy = as_number_inline(prg, v[2]);
	double sz = as_number_inline(prg, v[3]);

	Model_Info *info = model_info(prg);

	Model *model = info->models[model_id];

	model->mat = glm::scale(model->mat, glm::vec3(sx, sy, sz));

	return true;
}

static bool modelfunc_rotate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int model_id = as_number_inline(prg, v[0]);
	double angle = as_number_inline(prg, v[1]);
	double ax = as_number_inline(prg, v[2]);
	double ay = as_number_inline(prg, v[3]);
	double az = as_number_inline(prg, v[4]);

	Model_Info *info = model_info(prg);

	Model *model = info->models[model_id];

	model->mat = glm::rotate(model->mat, (float)angle, glm::vec3(ax, ay, az));

	return true;
}

static bool modelfunc_translate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int model_id = as_number_inline(prg, v[0]);
	double x = as_number_inline(prg, v[1]);
	double y = as_number_inline(prg, v[2]);
	double z = as_number_inline(prg, v[3]);

	Model_Info *info = model_info(prg);

	Model *model = info->models[model_id];

	model->mat = glm::translate(model->mat, glm::vec3(x, y, z));

	return true;
}

static bool modelfunc_get_position(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int model_id = as_number_inline(prg, v[0]);

	Variable &x = as_variable_inline(prg, v[1]);
	Variable &y = as_variable_inline(prg, v[2]);
	Variable &z = as_variable_inline(prg, v[3]);

	CHECK_NUMBER(x)
	CHECK_NUMBER(y)
	CHECK_NUMBER(z)

	Model_Info *info = model_info(prg);

	Model *model = info->models[model_id];

	glm::vec3 pos = glm::vec3(model->mat[3]);
	x.n = pos.x;
	y.n = pos.y;
	z.n = pos.z;

	return true;
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

	double sx = as_number_inline(prg, v[0]);
	double sy = as_number_inline(prg, v[1]);
	double sz = as_number_inline(prg, v[2]);

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

	double angle = as_number_inline(prg, v[0]);
	double ax = as_number_inline(prg, v[1]);
	double ay = as_number_inline(prg, v[2]);
	double az = as_number_inline(prg, v[3]);

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

	double x = as_number_inline(prg, v[0]);
	double y = as_number_inline(prg, v[1]);
	double z = as_number_inline(prg, v[2]);

	glm::mat4 mv, proj;
	gfx::get_matrices(mv, proj);
	mv = glm::translate(mv, glm::vec3(x, y, z));
	gfx::set_matrices(mv, proj);
	gfx::update_projection();

	return true;
}

static void found_device_callback()
{
	if (is_3d) {
		set_3d();
	}
	else {
		set_2d();
	}
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
	v.push_back(t);
	call_void_function(d->prg, d->function, v, 0);
	delete d;
}

static bool modelfunc_set_animation(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::string anim = as_string_inline(prg, v[1]);
	
	Model_Info *info = model_info(prg);

	Model *model = info->models[id];

	if (v.size() > 2) {
		Model_Callback_Data *d = new Model_Callback_Data;
		d->prg = prg;
		d->function = as_function_inline(prg, v[2]);
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

	int id = as_number_inline(prg, v[0]);
	
	Model_Info *info = model_info(prg);

	Model *model = info->models[id];

	model->model->stop();

	return true;
}

static bool modelfunc_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	
	Model_Info *info = model_info(prg);

	Model *model = info->models[id];

	model->model->reset();

	return true;
}

static bool modelfunc_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int id = as_number_inline(prg, v[0]);

	Variable &out_x = as_variable_inline(prg, v[1]);
	Variable &out_y = as_variable_inline(prg, v[2]);
	Variable &out_z = as_variable_inline(prg, v[3]);

	CHECK_NUMBER(out_x)
	CHECK_NUMBER(out_y)
	CHECK_NUMBER(out_z)
	
	Model_Info *info = model_info(prg);

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

	return true;
}

static bool modelfunc_draw_3d(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	Variable verts = as_variable_resolve_inline(prg, v[0]);
	Variable faces = as_variable_resolve_inline(prg, v[1]);
	Variable colours = as_variable_resolve_inline(prg, v[2]);
	int num_triangles = as_number_inline(prg, v[3]);

	static float *vert_vec = nullptr;
	static int vec_sz = 0;

	if (vert_vec == nullptr) {
		vert_vec = (float *)malloc(12*3*num_triangles*sizeof(float));
		vec_sz = num_triangles;
	}
	else if (num_triangles > vec_sz) {
		vert_vec = (float *)realloc(vert_vec, 12*3*num_triangles*sizeof(float));
		vec_sz = num_triangles;
	}

	int count = 0;
	int ccount = 0;

	for (int i = 0; i < num_triangles; i++) {
		for (int j = 0; j < 3; j++) {
			int index = faces.v[i*3+j].n;
			// xyz
			vert_vec[count++] = verts.v[index*3+0].n;
			vert_vec[count++] = verts.v[index*3+1].n;
			vert_vec[count++] = verts.v[index*3+2].n;
			// normals
			vert_vec[count++] = 0.0f;
			vert_vec[count++] = 0.0f;
			vert_vec[count++] = 0.0f;
			// texcoord
			vert_vec[count++] = 0.0f;
			vert_vec[count++] = 0.0f;
			// colour
			if (colours.v.size() > 0) {
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
			}
			else {
				for (int k = 0; k < 4; k++) {
					vert_vec[count++] = 1.0f;
				}
			}
		}
	}
	
	gfx::enable_depth_write(true);
	gfx::enable_depth_test(true);
/*
	if (shim::opengl) {
		glDisable_ptr(GL_SCISSOR_TEST);
	}
#ifdef _WIN32
	else {
		shim::d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
#endif
*/

	gfx::Vertex_Cache::instance()->start();
	gfx::Vertex_Cache::instance()->cache_3d_immediate(vert_vec, num_triangles);
	gfx::Vertex_Cache::instance()->end();

	gfx::enable_depth_test(false);
	gfx::enable_depth_write(false);

	return true;
}

static bool modelfunc_draw_3d_textured(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	int tex = as_number_inline(prg, v[0]);
	Variable verts = as_variable_resolve_inline(prg, v[1]);
	Variable faces = as_variable_resolve_inline(prg, v[2]);
	Variable colours = as_variable_resolve_inline(prg, v[3]);
	Variable texcoords = as_variable_resolve_inline(prg, v[4]);
	int num_triangles = as_number_inline(prg, v[5]);

	float vert_vec[12*3*num_triangles];

	int count = 0;
	int ccount = 0;
	int tcount = 0;

	for (int i = 0; i < num_triangles; i++) {
		for (int j = 0; j < 3; j++) {
			int index = faces.v[i*3+j].n;
			// xyz
			vert_vec[count++] = verts.v[index*3+0].n;
			vert_vec[count++] = verts.v[index*3+1].n;
			vert_vec[count++] = verts.v[index*3+2].n;
			// normals
			vert_vec[count++] = 0.0f;
			vert_vec[count++] = 0.0f;
			vert_vec[count++] = 0.0f;
			// texcoord
			vert_vec[count++] = texcoords.v[tcount++].n;
			vert_vec[count++] = texcoords.v[tcount++].n;
			// colour
			if (colours.v.size() > 0) {
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
				vert_vec[count++] = colours.v[ccount++].n / 255.0f;
			}
			else {
				for (int k = 0; k < 4; k++) {
					vert_vec[count++] = 1.0f;
				}
			}
		}
	}
	
	gfx::enable_depth_write(true);
	gfx::enable_depth_test(true);
/*
	if (shim::opengl) {
		glDisable_ptr(GL_SCISSOR_TEST);
	}
#ifdef _WIN32
	else {
		shim::d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
#endif
*/

	Image_Info *iinfo = image_info(prg);
	gfx::Image *image = iinfo->images[tex];
	if (shim::opengl) {
		GLuint texture = image->get_opengl_texture();
		glBindTexture_ptr(GL_TEXTURE_2D, texture);
		PRINT_GL_ERROR("glActiveTexture\n");
		glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		PRINT_GL_ERROR("glTexParameteri\n");
		glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		PRINT_GL_ERROR("glTexParameteri\n");
	}
#ifdef _WIN32
	else {
		if (shim::d3d_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP) != D3D_OK) {
			util::infomsg("SetSamplerState failed.\n");
		}
		if (shim::d3d_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP) != D3D_OK) {
			util::infomsg("SetSamplerState failed.\n");
		}
	}
#endif
	
	gfx::Vertex_Cache::instance()->start(image);
	gfx::Vertex_Cache::instance()->cache_3d_immediate(vert_vec, num_triangles);
	gfx::Vertex_Cache::instance()->end();

	gfx::enable_depth_test(false);
	gfx::enable_depth_write(false);

	if (shim::opengl) {
		glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		PRINT_GL_ERROR("glTexParameteri\n");
		glTexParameteri_ptr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		PRINT_GL_ERROR("glTexParameteri\n");
	}
#ifdef _WIN32
	else {
		if (shim::d3d_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP) != D3D_OK) {
			util::infomsg("SetSamplerState failed.\n");
		}
		if (shim::d3d_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP) != D3D_OK) {
			util::infomsg("SetSamplerState failed.\n");
		}
	}
#endif

	return true;
}

static bool modelfunc_clone(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &result = as_variable_inline(prg, v[1]);

	CHECK_NUMBER(result)
	
	Model_Info *info = model_info(prg);
	Model *model = new Model;
	Model *orig = info->models[id];
	model->model = orig->model;
	model->mat = orig->mat;
	model->is_clone = true;
	result.n = info->model_id;
	info->models[info->model_id++] = model;

	return true;
}

static bool modelfunc_billboard_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(7)

	Variable &result = as_variable_inline(prg, v[0]);
	int image_id = as_number_inline(prg, v[1]);
	double x = as_number_inline(prg, v[2]);
	double y = as_number_inline(prg, v[3]);
	double z = as_number_inline(prg, v[4]);
	double w = as_number_inline(prg, v[5]);
	double h = as_number_inline(prg, v[6]);

	CHECK_NUMBER(result)
	
	Image_Info *info = image_info(prg);
	gfx::Image *image = info->images[image_id];
	
	Billboard_Info *info2 = billboard_info(prg);
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
	result.n = info2->billboard_id;
	info2->billboards[info2->billboard_id++] = billboard;
	return true;
}

static bool modelfunc_billboard_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	Billboard_Info *info = billboard_info(prg);
	delete info->billboards[id];
	info->billboards.erase(id);

	return true;
}

static bool modelfunc_billboard_from_sprite(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(8)

	Variable &result = as_variable_inline(prg, v[0]);
	int sprite_id = as_number_inline(prg, v[1]);
	double x = as_number_inline(prg, v[2]);
	double y = as_number_inline(prg, v[3]);
	double z = as_number_inline(prg, v[4]);
	double w = as_number_inline(prg, v[5]);
	double h = as_number_inline(prg, v[6]);
	double unit = as_number_inline(prg, v[7]);

	CHECK_NUMBER(result)
	
	Sprite_Info *info = sprite_info(prg);
	gfx::Sprite *sprite = info->sprites[sprite_id];
	
	Billboard_Info *info2 = billboard_info(prg);
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
	result.n = info2->billboard_id;
	info2->billboards[info2->billboard_id++] = billboard;
	return true;
}

static bool modelfunc_billboard_draw(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int billboard_id = as_number_inline(prg, v[0]);
	int r = as_number_inline(prg, v[1]);
	int g = as_number_inline(prg, v[2]);
	int b = as_number_inline(prg, v[3]);
	int a = as_number_inline(prg, v[4]);

	Billboard_Info *info = billboard_info(prg);
	Billboard *billboard = info->billboards[billboard_id];

	const float verts[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	const int faces[] = {
		0, 1, 2, 1, 2, 3
	};
	float texcoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
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

static bool modelfunc_billboard_translate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int billboard_id = as_number_inline(prg, v[0]);
	double x = as_number_inline(prg, v[1]);
	double y = as_number_inline(prg, v[2]);
	double z = as_number_inline(prg, v[3]);

	Billboard_Info *info = billboard_info(prg);
	Billboard *billboard = info->billboards[billboard_id];

	billboard->tx += x;
	billboard->ty += y;
	billboard->tz += z;

	return true;
}

static bool modelfunc_billboard_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int billboard_id = as_number_inline(prg, v[0]);
	double sx = as_number_inline(prg, v[1]);
	double sy = as_number_inline(prg, v[2]);

	Billboard_Info *info = billboard_info(prg);
	Billboard *billboard = info->billboards[billboard_id];

	billboard->sx *= sx;
	billboard->sy *= sy;

	return true;
}

static bool cdfunc_model_point(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	Variable &result = as_variable_inline(prg, v[0]);
	int model_id = as_number_inline(prg, v[1]);
	double x = as_number_inline(prg, v[2]);
	double y = as_number_inline(prg, v[3]);
	double z = as_number_inline(prg, v[4]);

	CHECK_NUMBER(result)
	
	Model_Info *info = model_info(prg);
	Model *model = info->models[model_id];
			
	result.n = cd::model_point(model->model, model->mat, glm::vec3(x, y, z));

	return true;
}

static bool cdfunc_model_line_segment(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(11)

	Variable &result = as_variable_inline(prg, v[0]);
	int model_id = as_number_inline(prg, v[1]);
	double x = as_number_inline(prg, v[2]);
	double y = as_number_inline(prg, v[3]);
	double z = as_number_inline(prg, v[4]);
	double x2 = as_number_inline(prg, v[5]);
	double y2 = as_number_inline(prg, v[6]);
	double z2 = as_number_inline(prg, v[7]);
	Variable &out_x = as_variable_inline(prg, v[8]);
	Variable &out_y = as_variable_inline(prg, v[9]);
	Variable &out_z = as_variable_inline(prg, v[10]);

	CHECK_NUMBER(result)
	CHECK_NUMBER(out_x)
	CHECK_NUMBER(out_y)
	CHECK_NUMBER(out_z)
	
	Model_Info *info = model_info(prg);
	Model *model = info->models[model_id];
			
	glm::vec3 out;

	result.n = cd::model_line_segment(model->model, model->mat, glm::vec3(x, y, z), glm::vec3(x2, y2, z2), out);

	out_x.n = out.x;
	out_y.n = out.y;
	out_z.n = out.z;

	return true;
}

static bool cdfunc_sphere_sphere(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(9)

	Variable &result = as_variable_inline(prg, v[0]);
	double x = as_number_inline(prg, v[1]);
	double y = as_number_inline(prg, v[2]);
	double z = as_number_inline(prg, v[3]);
	double r = as_number_inline(prg, v[4]);
	double x2 = as_number_inline(prg, v[5]);
	double y2 = as_number_inline(prg, v[6]);
	double z2 = as_number_inline(prg, v[7]);
	double r2 = as_number_inline(prg, v[8]);

	CHECK_NUMBER(result)

	x -= x2;
	y -= y2;
	z -= z2;
	double len = sqrt(x*x + y*y + z*z);
	result.n = len < (r+r2);

	return true;
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
	BooBoo_Widget *widget = info->widgets[id]->widget;

	std::vector<Token> tmp;
	Token t;

	t.type = Token::NUMBER;
	t.n = id;
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
	BooBoo_Widget *widget = info->widgets[id]->widget;

	std::vector<Token> tmp;
	Token t;

	t.type = Token::NUMBER;
	t.n = id;
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
		tmp[2].n = event->joystick.button;
		tmp[3].n = event->joystick.is_repeat;
	}
	else if (event->type == TGUI_JOY_AXIS) {
		tmp[2].n = event->joystick.axis;
		tmp[3].n = event->joystick.value;
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

static bool widgetfunc_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	Variable &v1 = as_variable_inline(prg, v[0]);
	double w = as_number_inline(prg, v[1]);
	double h = as_number_inline(prg, v[2]);
	Variable &data = as_variable_inline(prg, v[3]);
	
	Widget_Info *info = widget_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->widget_id;

	Widget *widget = new Widget;
	widget->data = &data;

	if (w < 1.0 && h < 1.0) {
		widget->widget = new BooBoo_Widget(prg, info->widget_id, (float)w, (float)h);
	}
	else if (w < 1.0) {
		widget->widget = new BooBoo_Widget(prg, info->widget_id, (float)w, (int)h);
	}
	else if (h < 1.0) {
		widget->widget = new BooBoo_Widget(prg, info->widget_id, (int)w, (float)h);
	}
	else {
		widget->widget = new BooBoo_Widget(prg, info->widget_id, (int)w, (int)h);
	}

	widget->widget->set_accepts_focus(true);

	info->widgets[info->widget_id++] = widget;

	return true;
}

static bool widgetfunc_create_fit(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	Variable &v1 = as_variable_inline(prg, v[0]);
	int fit = as_number_inline(prg, v[1]);
	double other = as_number_inline(prg, v[2]);
	Variable &data = as_variable_inline(prg, v[3]);
	
	Widget_Info *info = widget_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->widget_id;

	Widget *widget = new Widget;
	widget->data = &data;

	if (other < 1.0) {
		widget->widget = new BooBoo_Widget(prg, info->widget_id, (BooBoo_Widget::Fit)fit, (float)other);
	}
	else {
		widget->widget = new BooBoo_Widget(prg, info->widget_id, fit, (int)other);
	}

	widget->widget->set_accepts_focus(true);

	info->widgets[info->widget_id++] = widget;

	return true;
}

static bool widgetfunc_create_fit_both(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &data = as_variable_inline(prg, v[1]);
	
	Widget_Info *info = widget_info(prg);

	CHECK_NUMBER(v1)
	
	v1.n = info->widget_id;

	Widget *widget = new Widget;
	widget->data = &data;

	widget->widget = new BooBoo_Widget(prg, info->widget_id);

	widget->widget->set_accepts_focus(true);

	info->widgets[info->widget_id++] = widget;

	return true;
}

static bool widgetfunc_set_parent(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int child = as_number_inline(prg, v[0]);
	unsigned int parent = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[child]->widget->set_parent(info->widgets[parent]->widget);

	return true;
}

static bool widgetfunc_set_left_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	unsigned int left = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_left_widget(info->widgets[left]->widget);

	return true;
}

static bool widgetfunc_set_right_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	unsigned int right = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_right_widget(info->widgets[right]->widget);

	return true;
}

static bool widgetfunc_set_up_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	unsigned int up = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_up_widget(info->widgets[up]->widget);

	return true;
}

static bool widgetfunc_set_down_widget(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	unsigned int down = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_down_widget(info->widgets[down]->widget);

	return true;
}

static bool widgetfunc_set_float_left(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_float_left(val);

	return true;
}

static bool widgetfunc_set_float_right(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_float_right(val);

	return true;
}

static bool widgetfunc_set_float_bottom(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_float_bottom(val);

	return true;
}

static bool widgetfunc_set_centre_x(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_centre_x(val);

	return true;
}

static bool widgetfunc_set_centre_y(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_centre_y(val);

	return true;
}

static bool widgetfunc_set_clear_float_x(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_clear_float_x(val);

	return true;
}

static bool widgetfunc_set_clear_float_y(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_clear_float_y(val);

	return true;
}

static bool widgetfunc_set_break_line(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_break_line(val);

	return true;
}

static bool widgetfunc_set_accepts_focus(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	bool val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	info->widgets[widget]->widget->set_accepts_focus(val);

	return true;
}

static bool widgetfunc_set_padding(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	unsigned int widget = as_number_inline(prg, v[0]);
	double val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	if (val < 1.0) {
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

	unsigned int widget = as_number_inline(prg, v[0]);
	double val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	if (val < 1.0) {
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

	unsigned int widget = as_number_inline(prg, v[0]);
	double val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	if (val < 1.0) {
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

	unsigned int widget = as_number_inline(prg, v[0]);
	double val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	if (val < 1.0) {
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

	unsigned int widget = as_number_inline(prg, v[0]);
	double val = as_number_inline(prg, v[1]);

	Widget_Info *info = widget_info(prg);

	if (val < 1.0) {
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
};

BooBoo_GUI::BooBoo_GUI(BooBoo_Widget *root)
{
	root->set_centre_x(true);
	root->set_centre_y(true);

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	root->set_parent(modal_main_widget);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

BooBoo_GUI::~BooBoo_GUI()
{
}

static bool widgetfunc_gui_start(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	unsigned int id = as_number_inline(prg, v[0]);

	Widget_Info *info = widget_info(prg);

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

	Widget_Info *info = widget_info(prg);

	unsigned int id = as_number_inline(prg, v[0]);

	if (shim::guis.size() > 0) {
		shim::guis[shim::guis.size()-1]->gui->set_focus(info->widgets[id]->widget);
	}

	return true;
}

void start_lib_game()
{
	gfx::register_lost_device_callbacks(nullptr, found_device_callback);

	add_instruction("inspect", miscfunc_inspect);
	add_instruction("delay", miscfunc_delay);
	add_instruction("get_ticks", miscfunc_get_ticks);
	add_instruction("rand", miscfunc_rand);
	add_instruction("args", miscfunc_args);
	add_instruction("get_logic_rate", miscfunc_get_logic_rate);
	add_instruction("set_logic_rate", miscfunc_set_logic_rate);
	add_instruction("file_list", miscfunc_file_list);
	add_instruction("clear", gfxfunc_clear);
	add_instruction("flip", gfxfunc_flip);
	add_instruction("resize", gfxfunc_resize);
	add_instruction("get_screen_size", gfxfunc_get_screen_size);
	add_instruction("get_buffer_size", gfxfunc_get_buffer_size);
	add_instruction("get_screen_offset", gfxfunc_get_screen_offset);
	add_instruction("get_scale", gfxfunc_get_scale);
	add_instruction("set_target", gfxfunc_set_target);
	add_instruction("set_target_backbuffer", gfxfunc_set_target_backbuffer);
	add_instruction("screen_shake", gfxfunc_screen_shake);
	add_instruction("add_notification", gfxfunc_add_notification);
	add_instruction("is_fullscreen", gfxfunc_is_fullscreen);
	add_instruction("toggle_fullscreen", gfxfunc_toggle_fullscreen);
	add_instruction("get_refresh_rate", gfxfunc_get_refresh_rate);
	add_instruction("set_scissor", gfxfunc_set_scissor);
	add_instruction("unset_scissor", gfxfunc_unset_scissor);
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
	add_instruction("image_create", imagefunc_create);
	add_instruction("image_load", imagefunc_load);
	add_instruction("image_destroy", imagefunc_destroy);
	add_instruction("image_draw", imagefunc_draw);
	add_instruction("image_stretch_region", imagefunc_stretch_region);
	add_instruction("image_draw_rotated_scaled", imagefunc_draw_rotated_scaled);
	add_instruction("image_start", imagefunc_start);
	add_instruction("image_end", imagefunc_end);
	add_instruction("image_size", imagefunc_size);
	add_instruction("image_draw_9patch", imagefunc_draw_9patch);
	add_instruction("font_load", fontfunc_load);
	add_instruction("font_destroy", fontfunc_destroy);
	add_instruction("font_draw", fontfunc_draw);
	add_instruction("font_width", fontfunc_width);
	add_instruction("font_height", fontfunc_height);
	add_instruction("font_add_extra_glyph", fontfunc_add_extra_glyph);
	add_instruction("set_tile_size", tilemapfunc_set_tile_size);
	add_instruction("tilemap_load", tilemapfunc_load);
	add_instruction("tilemap_destroy", tilemapfunc_destroy);
	add_instruction("tilemap_draw", tilemapfunc_draw);
	add_instruction("tilemap_num_layers", tilemapfunc_num_layers);
	add_instruction("tilemap_size", tilemapfunc_size);
	add_instruction("tilemap_is_solid", tilemapfunc_is_solid);
	add_instruction("tilemap_get_groups", tilemapfunc_get_groups);
	add_instruction("tilemap_set_animated_tiles", tilemapfunc_set_animated_tiles);
	add_instruction("tilemap_find_path", tilemapfunc_find_path);
	add_instruction("tilemap_set_solid", tilemapfunc_set_solid);
	add_instruction("tilemap_set_tile", tilemapfunc_set_tile);
	add_instruction("tilemap_get_tile", tilemapfunc_get_tile);
	add_instruction("sprite_load", spritefunc_load);
	add_instruction("sprite_destroy", spritefunc_destroy);
	add_instruction("sprite_set_animation_lazy", spritefunc_set_animation_lazy);
	add_instruction("sprite_set_animation", spritefunc_set_animation);
	add_instruction("sprite_get_animation", spritefunc_get_animation);
	add_instruction("sprite_get_previous_animation", spritefunc_get_previous_animation);
	add_instruction("sprite_length", spritefunc_length);
	add_instruction("sprite_current_frame", spritefunc_current_frame);
	add_instruction("sprite_num_frames", spritefunc_num_frames);
	add_instruction("sprite_current_frame_size", spritefunc_current_frame_size);
	add_instruction("sprite_draw", spritefunc_draw);
	add_instruction("sprite_start", spritefunc_start);
	add_instruction("sprite_stop", spritefunc_stop);
	add_instruction("sprite_reset", spritefunc_reset);
	add_instruction("sprite_bounds", spritefunc_bounds);
	add_instruction("sprite_elapsed", spritefunc_elapsed);
	add_instruction("sprite_frame_times", spritefunc_frame_times);
	add_instruction("sprite_is_started", spritefunc_is_started);
	add_instruction("mml_create", mmlfunc_create);
	add_instruction("mml_load", mmlfunc_load);
	add_instruction("mml_destroy", mmlfunc_destroy);
	add_instruction("mml_play", mmlfunc_play);
	add_instruction("mml_stop", mmlfunc_stop);
	add_instruction("sample_load", samplefunc_load);
	add_instruction("sample_destroy", samplefunc_destroy);
	add_instruction("sample_play", samplefunc_play);
	add_instruction("sample_stop", samplefunc_stop);
	add_instruction("joystick_poll", joyfunc_poll);
	add_instruction("joystick_count", joyfunc_count);
	add_instruction("rumble", joyfunc_rumble);
	add_instruction("cfg_load", cfgfunc_load);
	add_instruction("cfg_destroy", cfgfunc_destroy);
	add_instruction("cfg_save", cfgfunc_save);
	add_instruction("cfg_get_number", cfgfunc_get_number);
	add_instruction("cfg_get_string", cfgfunc_get_string);
	add_instruction("cfg_set_number", cfgfunc_set_number);
	add_instruction("cfg_set_string", cfgfunc_set_string);
	add_instruction("cfg_exists", cfgfunc_exists);
	add_instruction("cfg_erase", cfgfunc_erase);
	add_instruction("shader_load", shaderfunc_load);
	add_instruction("shader_destroy", shaderfunc_destroy);
	add_instruction("shader_use", shaderfunc_use);
	add_instruction("shader_use_default", shaderfunc_use_default);
	add_instruction("shader_set_bool", shaderfunc_set_bool);
	add_instruction("shader_set_int", shaderfunc_set_int);
	add_instruction("shader_set_float", shaderfunc_set_float);
	add_instruction("shader_set_colour", shaderfunc_set_colour);
	add_instruction("shader_set_texture", shaderfunc_set_texture);
	add_instruction("json_load", jsonfunc_load);
	add_instruction("json_destroy", jsonfunc_destroy);
	add_instruction("json_get_string", jsonfunc_get_string);
	add_instruction("json_get_number", jsonfunc_get_number);
	add_instruction("model_load", modelfunc_load);
	add_instruction("model_destroy", modelfunc_destroy);
	add_instruction("model_draw", modelfunc_draw);
	add_instruction("set_2d", modelfunc_set_2d);
	add_instruction("set_3d", modelfunc_set_3d);
	add_instruction("model_identity", modelfunc_identity);
	add_instruction("model_scale", modelfunc_scale);
	add_instruction("model_rotate", modelfunc_rotate);
	add_instruction("model_translate", modelfunc_translate);
	add_instruction("model_get_position", modelfunc_get_position);
	add_instruction("identity_3d", modelfunc_identity_3d);
	add_instruction("scale_3d", modelfunc_scale_3d);
	add_instruction("rotate_3d", modelfunc_rotate_3d);
	add_instruction("translate_3d", modelfunc_translate_3d);
	add_instruction("model_set_animation", modelfunc_set_animation);
	add_instruction("model_stop", modelfunc_stop);
	add_instruction("model_reset", modelfunc_reset);
	add_instruction("model_size", modelfunc_size);
	add_instruction("draw_3d", modelfunc_draw_3d);
	add_instruction("draw_3d_textured", modelfunc_draw_3d_textured);
	add_instruction("model_clone", modelfunc_clone);
	add_instruction("billboard_create", modelfunc_billboard_create);
	add_instruction("billboard_from_sprite", modelfunc_billboard_from_sprite);
	add_instruction("billboard_destroy", modelfunc_billboard_destroy);
	add_instruction("billboard_draw", modelfunc_billboard_draw);
	add_instruction("billboard_translate", modelfunc_billboard_translate);
	add_instruction("billboard_scale", modelfunc_billboard_scale);
	add_instruction("cd_model_point", cdfunc_model_point);
	add_instruction("cd_model_line_segment", cdfunc_model_line_segment);
	add_instruction("cd_sphere_sphere", cdfunc_sphere_sphere);
	add_instruction("widget_create", widgetfunc_create);
	add_instruction("widget_create_fit", widgetfunc_create_fit);
	add_instruction("widget_create_fit_both", widgetfunc_create_fit_both);
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
}

void end_lib_game()
{
}

void game_lib_destroy_program(Program *prg)
{
	MML_Info *mml_i = mml_info(prg);
	for (size_t i = 0; i < mml_i->mmls.size(); i++) {
		delete mml_i->mmls[i];
	}
	Sample_Info *sample_i = sample_info(prg);
	for (size_t i = 0; i < sample_i->samples.size(); i++) {
		delete sample_i->samples[i];
	}
	Image_Info *image_i = image_info(prg);
	for (size_t i = 0; i < image_i->images.size(); i++) {
		delete image_i->images[i];
	}
	Font_Info *font_i = font_info(prg);
	for (size_t i = 0; i < font_i->fonts.size(); i++) {
		delete font_i->fonts[i];
	}
	Tilemap_Info *tilemap_i = tilemap_info(prg);
	for (size_t i = 0; i < tilemap_i->tilemaps.size(); i++) {
		delete tilemap_i->tilemaps[i];
	}
	Sprite_Info *sprite_i = sprite_info(prg);
	for (size_t i = 0; i < sprite_i->sprites.size(); i++) {
		delete sprite_i->sprites[i];
	}
	Shader_Info *shader_i = shader_info(prg);
	for (size_t i = 0; i < shader_i->shaders.size(); i++) {
		delete shader_i->shaders[i];
	}
	JSON_Info *json_i = json_info(prg);
	for (size_t i = 0; i < json_i->jsons.size(); i++) {
		delete json_i->jsons[i];
	}
	Model_Info *model_i = model_info(prg);
	for (size_t i = 0; i < model_i->models.size(); i++) {
		if (model_i->models[i]->is_clone == false) {
			delete model_i->models[i]->model;
		}
		delete model_i->models[i];
	}
	Billboard_Info *billboard_i = billboard_info(prg);
	for (size_t i = 0; i < billboard_i->billboards.size(); i++) {
		delete billboard_i->billboards[i];
	}
	Widget_Info *widget_i = widget_info(prg);
	for (size_t i = 0; i < widget_i->widgets.size(); i++) {
		delete widget_i->widgets[i];
	}
	CFG_Info *cfg_i = cfg_info(prg);

	delete mml_i;
	delete sample_i;
	delete image_i;
	delete font_i;
	delete tilemap_i;
	delete shader_i;
	delete json_i;
	delete model_i;
	delete billboard_i;
	delete widget_i;
	delete cfg_i;

	booboo::set_black_box(prg, "com.illnorth.booboo.mml", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.sample", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.image", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.font", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.tilemap", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.sprite", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.cfg", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.shader", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.json", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.model", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.billboard", nullptr);
	booboo::set_black_box(prg, "com.illnorth.booboo.widget", nullptr);
}
