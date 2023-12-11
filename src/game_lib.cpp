#define STEAMWORKS
#include <shim4/shim4.h>

#include "booboo/booboo.h"
#include "booboo/internal.h"

using namespace booboo;

extern bool quit;

struct MML_Info {
	int mml_id;
	std::map<int, audio::MML *> mmls;
};

struct Sample_Info {
	int sample_id;
	std::map<int, audio::Sample *> samples;
};

struct Image_Info {
	int image_id;
	std::map<int, gfx::Image *> images;
};

struct Font_Info {
	int font_id;
	std::map<int, gfx::TTF *> fonts;
};

struct Tilemap_Info {
	int tilemap_id;
	std::map<int, gfx::Tilemap *> tilemaps;
};

struct Sprite_Info {
	int sprite_id;
	std::map<int, gfx::Sprite *> sprites;
};

struct Config_Value
{
	Variable::Variable_Type type;

	double n;
	std::string s;
};

struct CFG_Info {
	int cfg_id;
	std::map<int, std::map<std::string, Config_Value> > cfgs;
};

struct Shader_Info {
	int shader_id;
	std::map<int, gfx::Shader *> shaders;
};

struct JSON_Info {
	int json_id;
	std::map<int, util::JSON *> jsons;
};

static std::string remove_quotes(std::string s)
{
	int start = 0;
	int count = s.length();

	if (s[0] == '"') {
		start++;
		count--;
	}

	if (s[s.length()-1] == '"') {
		count--;
	}

	return s.substr(start, count);
}

static MML_Info *mml_info(Program *prg)
{
	MML_Info *info = (MML_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.mml");
	if (info == nullptr) {
		info = new MML_Info;
		info->mml_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.mml", info);
	}
	return info;
}

static Sample_Info *sample_info(Program *prg)
{
	Sample_Info *info = (Sample_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.sample");
	if (info == nullptr) {
		info = new Sample_Info;
		info->sample_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.sample", info);
	}
	return info;
}

static Image_Info *image_info(Program *prg)
{
	Image_Info *info = (Image_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.image");
	if (info == nullptr) {
		info = new Image_Info;
		info->image_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.image", info);
	}
	return info;
}

static Font_Info *font_info(Program *prg)
{
	Font_Info *info = (Font_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.font");
	if (info == nullptr) {
		info = new Font_Info;
		info->font_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.font", info);
	}
	return info;
}

static Tilemap_Info *tilemap_info(Program *prg)
{
	Tilemap_Info *info = (Tilemap_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.tilemap");
	if (info == nullptr) {
		info = new Tilemap_Info;
		info->tilemap_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.tilemap", info);
	}
	return info;
}

static Sprite_Info *sprite_info(Program *prg)
{
	Sprite_Info *info = (Sprite_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.sprite");
	if (info == nullptr) {
		info = new Sprite_Info;
		info->sprite_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.sprite", info);
	}
	return info;
}

static CFG_Info *cfg_info(Program *prg)
{
	CFG_Info *info = (CFG_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.cfg");
	if (info == nullptr) {
		info = new CFG_Info;
		info->cfg_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.cfg", info);
	}
	return info;
}

static Shader_Info *shader_info(Program *prg)
{
	Shader_Info *info = (Shader_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.shader");
	if (info == nullptr) {
		info = new Shader_Info;
		info->shader_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.shader", info);
	}
	return info;
}

static JSON_Info *json_info(Program *prg)
{
	JSON_Info *info = (JSON_Info *)booboo::get_black_box(prg, "com.b1stable.booboo.json");
	if (info == nullptr) {
		info = new JSON_Info;
		info->json_id = 0;
		booboo::set_black_box(prg, "com.b1stable.booboo.json", info);
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
			val.s = remove_quotes(value);
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
		if (v.type == Variable::NUMBER) {
			fprintf(f, "%s=%g\n", name.c_str(), v.n);
		}
		else {
			fprintf(f, "%s=\"%s\"\n", name.c_str(), v.s.c_str());
		}
	}

	fclose(f);

	return true;
}

static bool miscfunc_inspect(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	char buf[1000];

	if (v[0].type == Token::NUMBER) {
		snprintf(buf, 1000, "%g", v[0].n);
	}
	else if (v[0].type == Token::SYMBOL) {
		Variable &var = get_variable(prg, v[0].i);
		if (var.type == Variable::NUMBER) {
			snprintf(buf, 1000, "%g", var.n);
		}
		else if (var.type == Variable::STRING) {
			snprintf(buf, 1000, "%s", var.s.c_str());
		}
		else if (var.type == Variable::VECTOR) {
			snprintf(buf, 1000, "-vector-");
		}
		else if (var.type == Variable::MAP) {
			snprintf(buf, 1000, "-map-");
		}
		else if (var.type == Variable::POINTER) {
			snprintf(buf, 1000, "-pointer-");
		}
		else if (var.type == Variable::FUNCTION) {
			snprintf(buf, 1000, "-function-");
		}
		else if (var.type == Variable::LABEL) {
			snprintf(buf, 1000, "-label-");
		}
	}
	else {
		strcpy(buf, "Unknown");
	}

	gui::popup("INSPECTOR", buf, gui::OK);

	return true;
}

static bool miscfunc_delay(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int millis = (int)as_number_inline(prg, v[0]);
	SDL_Delay(millis);
	return true;
}

static bool miscfunc_get_ticks(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);
	
	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = SDL_GetTicks();

	return true;
}

static bool miscfunc_rand(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);

	int min_incl = as_number_inline(prg, v[1]);
	int max_incl = as_number_inline(prg, v[2]);

	if (v1.type == Variable::NUMBER) {
		v1.n = util::rand(min_incl, max_incl);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

static bool miscfunc_args(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);
	
	if (v1.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.v.clear();

	for (int i = 0; i < shim::argc; i++) {
		Variable var;
		var.type = Variable::STRING;
		var.name = "-constant-";
		var.s = shim::argv[i];
		v1.v.push_back(var);
	}

	return true;
}

static bool miscfunc_get_logic_rate(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = shim::logic_rate;

	return true;
}

static bool miscfunc_set_logic_rate(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int rate = as_number_inline(prg, v[0]);

	if (rate < 1 || rate > 1000) {
		throw Error(std::string(__FUNCTION__) + ": " + "Logic rate must be between 1 and 1000 at " + get_error_info(prg));
	}

	shim::logic_rate = rate;

	return true;
}

static bool gfxfunc_set_scissor(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int x = as_number_inline(prg, v[0]);
	int y = as_number_inline(prg, v[1]);
	int w = as_number_inline(prg, v[2]);
	int h = as_number_inline(prg, v[3]);

	gfx::set_scissor(x, y, w, h);

	return true;
}

static bool gfxfunc_clear(Program *prg, std::vector<Token> &v)
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

static bool gfxfunc_flip(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::flip();

	return true;
}

static bool gfxfunc_resize(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)
	
	int w = as_number_inline(prg, v[0]);
	int h = as_number_inline(prg, v[1]);

	float aspect = (float)w/h;
	gfx::set_min_aspect_ratio(aspect-0.001f);
	gfx::set_max_aspect_ratio(aspect+0.001f);
	gfx::restart(w, h, false, shim::real_screen_size.w, shim::real_screen_size.h);

	return true;
}

static bool gfxfunc_get_screen_size(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);
	
	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = shim::real_screen_size.w;
	v2.n = shim::real_screen_size.h;

	return true;
}

static bool gfxfunc_get_buffer_size(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);
	
	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = shim::screen_size.w;
	v2.n = shim::screen_size.h;

	return true;
}

static bool gfxfunc_get_screen_offset(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);
	
	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = shim::screen_offset.x;
	v2.n = shim::screen_offset.y;

	return true;
}

static bool gfxfunc_get_scale(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);
	
	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = shim::scale;

	return true;
}

static bool gfxfunc_set_target(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double id = as_number_inline(prg, v[0]);

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

static bool gfxfunc_set_target_backbuffer(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::set_target_backbuffer();

	return true;
}

static bool gfxfunc_screen_shake(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	float amount = as_number_inline(prg, v[0]);
	Uint32 duration = as_number_inline(prg, v[1]);

	gfx::screen_shake(amount, duration);

	return true;
}

static bool gfxfunc_add_notification(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string s = as_string_inline(prg, v[0]);

	gfx::add_notification(s);

	return true;
}

static bool gfxfunc_is_fullscreen(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

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

static bool gfxfunc_toggle_fullscreen(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gen_f11();

	return true;
}

static bool gfxfunc_get_refresh_rate(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = shim::refresh_rate;

	return true;
}

static bool gfxfunc_unset_scissor(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::unset_scissor();

	return true;
}

static bool primfunc_start_primitives(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::draw_primitives_start();

	return true;
}

static bool primfunc_end_primitives(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	gfx::draw_primitives_end();

	return true;
}

static bool primfunc_line(Program *prg, std::vector<Token> &v)
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

static bool primfunc_filled_triangle(Program *prg, std::vector<Token> &v)
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

static bool primfunc_rectangle(Program *prg, std::vector<Token> &v)
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

static bool primfunc_filled_rectangle(Program *prg, std::vector<Token> &v)
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

static bool primfunc_ellipse(Program *prg, std::vector<Token> &v)
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

	float _rx = as_number_inline(prg, v[6]);
	float _ry = as_number_inline(prg, v[7]);
	float thick = as_number_inline(prg, v[8]);
	float _sections = as_number_inline(prg, v[9]);

	gfx::draw_ellipse(c, p, _rx, _ry, thick, _sections);

	return true;
}

static bool primfunc_filled_ellipse(Program *prg, std::vector<Token> &v)
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

	float _rx = as_number_inline(prg, v[6]);
	float _ry = as_number_inline(prg, v[7]);
	float _sections = as_number_inline(prg, v[8]);

	gfx::draw_filled_ellipse(c, p, _rx, _ry, _sections);

	return true;
}

static bool primfunc_circle(Program *prg, std::vector<Token> &v)
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

static bool primfunc_filled_circle(Program *prg, std::vector<Token> &v)
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
	float _r = as_number_inline(prg, v[6]);
	int _sections = as_number_inline(prg, v[7]);

	gfx::draw_filled_circle(c, p, _r, _sections);

	return true;
}

static bool mmlfunc_create(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string str = as_string_inline(prg, v[1]);
	
	MML_Info *info = mml_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->mml_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	Uint8 *bytes = (Uint8 *)str.c_str();
	SDL_RWops *file = SDL_RWFromMem(bytes, str.length());
	audio::MML *mml = new audio::MML(file); // this closes the file

	info->mmls[info->mml_id++] = mml;

	return true;
}

static bool mmlfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	MML_Info *info = mml_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->mml_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	audio::MML *mml = new audio::MML(name);

	info->mmls[info->mml_id++] = mml;

	return true;
}

static bool mmlfunc_play(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	float volume = as_number_inline(prg, v[1]);
	bool loop = as_number_inline(prg, v[2]);

	MML_Info *info = mml_info(prg);

#ifdef DEBUG
	if (info->mmls.find(id) == info->mmls.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid MML at " + get_error_info(prg));
	}
#endif

	audio::MML *mml = info->mmls[id];

	mml->play(volume, loop);

	return true;
}

static bool mmlfunc_stop(Program *prg, std::vector<Token> &v)
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

static bool samplefunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	Sample_Info *info = sample_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->sample_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	audio::Sample *sample = new audio::Sample(name);

	info->samples[info->sample_id++] = sample;

	return true;
}

static bool samplefunc_play(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	float volume = as_number_inline(prg, v[1]);
	bool loop = as_number_inline(prg, v[2]);

	Sample_Info *info = sample_info(prg);

#ifdef DEBUG
	if (info->samples.find(id) == info->samples.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid Sample at " + get_error_info(prg));
	}
#endif

	audio::Sample *sample = info->samples[id];

	sample->play(volume, loop);

	return true;
}

static bool samplefunc_stop(Program *prg, std::vector<Token> &v)
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

static bool imagefunc_create(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);

	int w = as_number_inline(prg, v[1]);
	int h = as_number_inline(prg, v[2]);

	Image_Info *info = image_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->image_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Image *img = new gfx::Image(util::Size<int>(w, h));

	info->images[info->image_id++] = img;

	return true;
}

static bool imagefunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	Image_Info *info = image_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->image_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Image *img = new gfx::Image(name);

	info->images[info->image_id++] = img;

	return true;
}

static bool imagefunc_draw(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(9)

	double id = as_number_inline(prg, v[0]);
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

static bool imagefunc_stretch_region(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(15)

	double id = as_number_inline(prg, v[0]);
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

static bool imagefunc_draw_rotated_scaled(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(14)

	double id = as_number_inline(prg, v[0]);
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

static bool imagefunc_start(Program *prg, std::vector<Token> &v)
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

static bool imagefunc_end(Program *prg, std::vector<Token> &v)
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

static bool imagefunc_size(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);

	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	
	Image_Info *info = image_info(prg);

#ifdef DEBUG
	if (info->images.find(id) == info->images.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid image at " + get_error_info(prg));
	}
#endif

	gfx::Image *img = info->images[id];

	if (v1.type == Variable::NUMBER) {
		v1.n = img->size.w;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	if (v2.type == Variable::NUMBER) {
		v2.n = img->size.h;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool imagefunc_draw_9patch(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(9)

	double id = as_number_inline(prg, v[0]);
	int r = as_number_inline(prg, v[1]);
	int g = as_number_inline(prg, v[2]);
	int b = as_number_inline(prg, v[3]);
	int a = as_number_inline(prg, v[4]);
	float x = as_number_inline(prg, v[5]);
	float y = as_number_inline(prg, v[6]);
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

static bool fontfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(4)

	std::string name = as_string_inline(prg, v[1]);
	int size = as_number_inline(prg, v[2]);
	bool smooth = as_number_inline(prg, v[3]);

	Variable &v1 = as_variable_inline(prg, v[0]);

	Font_Info *info = font_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->font_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::TTF *font = new gfx::TTF(name, size, 1024);
	font->set_smooth(smooth);

	info->fonts[info->font_id++] = font;

	return true;
}

static bool fontfunc_draw(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(8)

	double id = as_number_inline(prg, v[0]);
	double r = as_number_inline(prg, v[1]);
	double g = as_number_inline(prg, v[2]);
	double b = as_number_inline(prg, v[3]);
	double a = as_number_inline(prg, v[4]);
	std::string text = as_string_inline(prg, v[5]);
	double x = as_number_inline(prg, v[6]);
	double y = as_number_inline(prg, v[7]);

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

	font->draw(c, text, util::Point<float>(x, y));

	return true;
}

static bool fontfunc_width(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string text = as_string_inline(prg, v[2]);
	
	Font_Info *info = font_info(prg);

	gfx::TTF *font = info->fonts[id];

	int w = font->get_text_width(text);

	if (v1.type == Variable::NUMBER) {
		v1.n = w;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool fontfunc_height(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	double id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Font_Info *info = font_info(prg);

	gfx::TTF *font = info->fonts[id];

	int h = font->get_height();

	if (v1.type == Variable::NUMBER) {
		v1.n = h;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool tilemapfunc_set_tile_size(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	shim::tile_size = as_number_inline(prg, v[0]);

	return true;
}

static bool tilemapfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	
	Tilemap_Info *info = tilemap_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->tilemap_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Tilemap *tilemap = new gfx::Tilemap(name);

	info->tilemaps[info->tilemap_id++] = tilemap;

	return true;
}

static bool tilemapfunc_draw(Program *prg, std::vector<Token> &v)
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

static bool tilemapfunc_num_layers(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Tilemap_Info *info = tilemap_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Tilemap *tilemap = info->tilemaps[id];

	v1.n = tilemap->get_num_layers();

	return true;
}

static bool tilemapfunc_size(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	
	Tilemap_Info *info = tilemap_info(prg);

	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Tilemap *tilemap = info->tilemaps[id];

	util::Size<int> sz = tilemap->get_size();

	v1.n = sz.w;
	v2.n = sz.h;

	return true;
}

static bool tilemapfunc_is_solid(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	int x = as_number_inline(prg, v[2]);
	int y = as_number_inline(prg, v[3]);
	
	Tilemap_Info *info = tilemap_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Tilemap *tilemap = info->tilemaps[id];

	v1.n = tilemap->is_solid(-1, util::Point<int>(x, y));

	return true;
}

static bool tilemapfunc_get_groups(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::vector<Variable> &vec = as_vector_inline(prg, v[1]);
	
	Tilemap_Info *info = tilemap_info(prg);

	gfx::Tilemap *tilemap = info->tilemaps[id];

	std::vector<gfx::Tilemap::Group> &groups = tilemap->get_groups();

	for (size_t i = 0; i < groups.size(); i++) {
		gfx::Tilemap::Group &g = groups[i];
		Variable v;
		v.type = Variable::VECTOR;
		v.name = "-constant-";
		Variable type;
		type.type = Variable::NUMBER;
		type.name = "-constant-";
		type.n = g.type;
		v.v.push_back(type);
		Variable x;
		x.type = Variable::NUMBER;
		x.name = "-constant-";
		x.n = g.x;
		v.v.push_back(x);
		Variable y;
		y.type = Variable::NUMBER;
		y.name = "-constant-";
		y.n = g.y;
		v.v.push_back(y);
		Variable w;
		w.type = Variable::NUMBER;
		w.name = "-constant-";
		w.n = g.w;
		v.v.push_back(w);
		Variable h;
		h.type = Variable::NUMBER;
		h.name = "-constant-";
		h.n = g.h;
		v.v.push_back(h);
		vec.push_back(v);
	}

	return true;
}

static bool tilemapfunc_set_animated_tiles(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int id = as_number_inline(prg, v[0]);
	int delay = (int)as_number_inline(prg, v[1]);
	int w = (int)as_number_inline(prg, v[2]);
	int h = (int)as_number_inline(prg, v[3]);
	Variable &v1 = as_variable_inline(prg, v[4]);
	
	Tilemap_Info *info = tilemap_info(prg);

	if (v1.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

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

static bool tilemapfunc_find_path(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(7)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::vector<Variable> &entity_solids = as_vector_inline(prg, v[2]);
	int start_x = (int)as_number_inline(prg, v[3]);
	int start_y = (int)as_number_inline(prg, v[4]);
	int end_x = (int)as_number_inline(prg, v[5]);
	int end_y = (int)as_number_inline(prg, v[6]);
	
	if (v1.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	Tilemap_Info *info = tilemap_info(prg);

	gfx::Tilemap *tilemap = info->tilemaps[id];

	std::vector< util::Rectangle<int> > entity_rects;
	for (size_t i = 0; i < entity_solids.size(); i++) {
		int x = entity_solids[i].v[0].n;
		int y = entity_solids[i].v[1].n;
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
		vec.name = "-constant-";
		Variable x;
		x.type = Variable::NUMBER;
		x.name = "-constant-";
		x.n = n.position.x;
		vec.v.push_back(x);
		Variable y;
		y.type = Variable::NUMBER;
		y.name = "-constant-";
		y.n = n.position.y;
		vec.v.push_back(y);
		v1.v.push_back(vec);
	}

	delete a_star;

	return true;
}

static bool tilemapfunc_set_solid(Program *prg, std::vector<Token> &v)
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

static bool tilemapfunc_set_tile(Program *prg, std::vector<Token> &v)
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

static bool tilemapfunc_get_tile(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(7)

	int id = as_number_inline(prg, v[0]);
	Variable &vx = as_variable_inline(prg, v[1]);
	Variable &vy = as_variable_inline(prg, v[2]);
	Variable &vs = as_variable_inline(prg, v[3]);
	int layer = as_number_inline(prg, v[4]);
	int x = as_number_inline(prg, v[5]);
	int y = as_number_inline(prg, v[6]);

	if (vx.type != Variable::NUMBER || vy.type != Variable::NUMBER || vs.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	
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

static bool spritefunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->sprite_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = new gfx::Sprite(name, name);

	info->sprites[info->sprite_id++] = sprite;

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

static bool spritefunc_set_animation(Program *prg, std::vector<Token> &v)
{
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

static bool spritefunc_get_animation(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	v1.s = sprite->get_animation();

	return true;
}

static bool spritefunc_get_previous_animation(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	v1.s = sprite->get_previous_animation();

	return true;
}

static bool spritefunc_current_frame(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_current_frame();

	return true;
}

static bool spritefunc_num_frames(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_num_frames();

	return true;
}

static bool spritefunc_length(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_length();

	return true;
}

static bool spritefunc_current_frame_size(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	gfx::Image *img = sprite->get_current_image();

	v1.n = img->size.w;
	v2.n = img->size.h;

	return true;
}

static bool spritefunc_draw(Program *prg, std::vector<Token> &v)
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

static bool spritefunc_start(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number_inline(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	sprite->start();

	return true;
}

static bool spritefunc_stop(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number_inline(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	sprite->stop();

	return true;
}

static bool spritefunc_reset(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int id = as_number_inline(prg, v[0]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	sprite->reset();

	return true;
}

static bool spritefunc_bounds(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(5)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	Variable &v2 = as_variable_inline(prg, v[2]);
	Variable &v3 = as_variable_inline(prg, v[3]);
	Variable &v4 = as_variable_inline(prg, v[4]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER || v3.type != Variable::NUMBER || v4.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

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

static bool spritefunc_elapsed(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->get_elapsed();

	return true;
}

static bool spritefunc_frame_times(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::vector<Variable> &vec = as_vector_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	gfx::Sprite *sprite = info->sprites[id];

	std::vector<Uint32> times = sprite->get_frame_times();

	for (size_t i = 0; i < times.size(); i++) {
		Variable v;
		v.type = Variable::NUMBER;
		v.name = "-constant-";
		v.n = times[i];
		vec.push_back(v);
	}

	return true;
}

static bool spritefunc_is_started(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	
	Sprite_Info *info = sprite_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	gfx::Sprite *sprite = info->sprites[id];

	v1.n = sprite->is_started();

	return true;
}

static void set_string_or_number(Program *prg, int index, double value)
{
       Variable &v1 = booboo::get_variable(prg, index);

       if (v1.type == Variable::NUMBER) {
               v1.n = value;
       }
       else {
               throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
       }
}

static bool joyfunc_poll(Program *prg, std::vector<Token> &v)
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

static bool joyfunc_count(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = input::get_num_joysticks();
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

static bool joyfunc_rumble(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int ms = as_number_inline(prg, v[0]);

	input::rumble(ms);

	return true;
}

static bool cfgfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string cfg_name = as_string_inline(prg, v[1]);

	CFG_Info *info = cfg_info(prg);

	if (v1.type == Variable::NUMBER) {
		std::map<std::string, Config_Value> v = load_cfg(prg, cfg_name);
		int id = info->cfg_id++;
		v1.n = id;
		info->cfgs[id] = v;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool cfgfunc_save(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string cfg_name = as_string_inline(prg, v[2]);

	bool success = save_cfg(prg, id, cfg_name);
	
	if (v1.type == Variable::NUMBER) {
		v1.n = success;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool cfgfunc_get_number(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	if (v2.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	int id = v1.n;
	
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

static bool cfgfunc_get_string(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Variable &v2 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	if (v2.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	int id = v1.n;
	
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

static bool cfgfunc_set_number(Program *prg, std::vector<Token> &v)
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

static bool cfgfunc_set_string(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	std::string val = as_string_inline(prg, v[2]);

	CFG_Info *info = cfg_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	int id = v1.n;
	
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

static bool cfgfunc_exists(Program *prg, std::vector<Token> &v)
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

	if (v1.type == Variable::NUMBER) {
		v1.n = found;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool cfgfunc_erase(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);

	CFG_Info *info = cfg_info(prg);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	int id = v1.n;
	
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

static bool shaderfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);

	std::string vname = "glsl/default_vertex.txt";
	std::string fname = as_string_inline(prg, v[1]);

	gfx::Shader::Precision vp = gfx::Shader::HIGH;
	gfx::Shader::Precision fp = (gfx::Shader::Precision)as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->shader_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	std::string vs = util::load_text("gfx/shaders/" + vname);
	std::string fs = util::load_text("gfx/shaders/" + fname);

	gfx::Shader::OpenGL_Shader *vert = gfx::Shader::load_opengl_vertex_shader(vs, vp);
	gfx::Shader::OpenGL_Shader *frag = gfx::Shader::load_opengl_fragment_shader(fs, fp);

	gfx::Shader *shader = new gfx::Shader(vert, frag);

	info->shaders[info->shader_id++] = shader;

	return true;
}

static bool shaderfunc_use(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double id = as_number_inline(prg, v[0]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	shader->use();
	gfx::update_projection();

	return true;
}

static bool shaderfunc_use_default(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(0)

	shim::default_shader->use();
	gfx::update_projection();

	return true;
}

static bool shaderfunc_set_bool(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	bool b = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	shader->set_bool(name, b);

	return true;
}

static bool shaderfunc_set_int(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	int i = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	shader->set_int(name, i);

	return true;
}

static bool shaderfunc_set_float(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	double f = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	shader->set_float(name, f);

	return true;
}

static bool shaderfunc_set_texture(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);
	std::string name = as_string_inline(prg, v[1]);
	double t = as_number_inline(prg, v[2]);
	
	Shader_Info *info = shader_info(prg);
	gfx::Shader *shader = info->shaders[id];

	Image_Info *info2 = image_info(prg);
	gfx::Image *img = info2->images[t];

	shader->set_texture(name, img);

	return true;
}

static bool shaderfunc_set_colour(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(6)

	double id = as_number_inline(prg, v[0]);
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

static bool jsonfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	std::string name = as_string_inline(prg, v[1]);

	JSON_Info *info = json_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->json_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	util::JSON *json = new util::JSON(name);

	info->jsons[info->json_id++] = json;

	return true;
}

static bool jsonfunc_get_string(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);
	
	JSON_Info *info = json_info(prg);
	util::JSON *json = info->jsons[id];

	if (v1.type == Variable::STRING) {
		util::JSON::Node *n = json->get_root()->find(name);
		v1.s = n->as_string();
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool jsonfunc_get_number(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double id = as_number_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);
	std::string name = as_string_inline(prg, v[2]);
	
	JSON_Info *info = json_info(prg);
	util::JSON *json = info->jsons[id];

	if (v1.type == Variable::NUMBER) {
		util::JSON::Node *n = json->get_root()->find(name);
		v1.n = n->as_double();
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

void start_lib_game()
{
	add_instruction("inspect", miscfunc_inspect);
	add_instruction("delay", miscfunc_delay);
	add_instruction("get_ticks", miscfunc_get_ticks);
	add_instruction("rand", miscfunc_rand);
	add_instruction("args", miscfunc_args);
	add_instruction("get_logic_rate", miscfunc_get_logic_rate);
	add_instruction("set_logic_rate", miscfunc_set_logic_rate);
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
	add_instruction("filled_triangle", primfunc_filled_triangle);
	add_instruction("rectangle", primfunc_rectangle);
	add_instruction("filled_rectangle", primfunc_filled_rectangle);
	add_instruction("ellipse", primfunc_ellipse);
	add_instruction("filled_ellipse", primfunc_filled_ellipse);
	add_instruction("circle", primfunc_circle);
	add_instruction("filled_circle", primfunc_filled_circle);
	add_instruction("image_create", imagefunc_create);
	add_instruction("image_load", imagefunc_load);
	add_instruction("image_draw", imagefunc_draw);
	add_instruction("image_stretch_region", imagefunc_stretch_region);
	add_instruction("image_draw_rotated_scaled", imagefunc_draw_rotated_scaled);
	add_instruction("image_start", imagefunc_start);
	add_instruction("image_end", imagefunc_end);
	add_instruction("image_size", imagefunc_size);
	add_instruction("image_draw_9patch", imagefunc_draw_9patch);
	add_instruction("font_load", fontfunc_load);
	add_instruction("font_draw", fontfunc_draw);
	add_instruction("font_width", fontfunc_width);
	add_instruction("font_height", fontfunc_height);
	add_instruction("set_tile_size", tilemapfunc_set_tile_size);
	add_instruction("tilemap_load", tilemapfunc_load);
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
	add_instruction("mml_play", mmlfunc_play);
	add_instruction("mml_stop", mmlfunc_stop);
	add_instruction("sample_load", samplefunc_load);
	add_instruction("sample_play", samplefunc_play);
	add_instruction("sample_stop", samplefunc_stop);
	add_instruction("joystick_poll", joyfunc_poll);
	add_instruction("joystick_count", joyfunc_count);
	add_instruction("rumble", joyfunc_rumble);
	add_instruction("cfg_load", cfgfunc_load);
	add_instruction("cfg_save", cfgfunc_save);
	add_instruction("cfg_get_number", cfgfunc_get_number);
	add_instruction("cfg_get_string", cfgfunc_get_string);
	add_instruction("cfg_set_number", cfgfunc_set_number);
	add_instruction("cfg_set_string", cfgfunc_set_string);
	add_instruction("cfg_exists", cfgfunc_exists);
	add_instruction("cfg_erase", cfgfunc_erase);
	add_instruction("shader_load", shaderfunc_load);
	add_instruction("shader_use", shaderfunc_use);
	add_instruction("shader_use_default", shaderfunc_use_default);
	add_instruction("shader_set_bool", shaderfunc_set_bool);
	add_instruction("shader_set_int", shaderfunc_set_int);
	add_instruction("shader_set_float", shaderfunc_set_float);
	add_instruction("shader_set_colour", shaderfunc_set_colour);
	add_instruction("shader_set_texture", shaderfunc_set_texture);
	add_instruction("json_load", jsonfunc_load);
	add_instruction("json_get_string", jsonfunc_get_string);
	add_instruction("json_get_number", jsonfunc_get_number);
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
	CFG_Info *cfg_i = cfg_info(prg);

	delete mml_i;
	delete sample_i;
	delete image_i;
	delete font_i;
	delete tilemap_i;
	delete cfg_i;
	delete shader_i;

	booboo::set_black_box(prg, "com.b1stable.booboo.mml", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.sample", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.image", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.font", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.tilemap", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.sprite", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.cfg", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.shader", nullptr);
	booboo::set_black_box(prg, "com.b1stable.booboo.json", nullptr);
}
