#include <shim4/shim4.h>

#include "booboo/booboo.h"

using namespace booboo;

extern bool quit;

struct MML_Info {
	int mml_id;
	std::map<int, audio::MML *> mmls;
};

struct Image_Info {
	int image_id;
	std::map<int, gfx::Image *> images;
};

struct Font_Info {
	int font_id;
	std::map<int, gfx::TTF *> fonts;
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
	MML_Info *info = (MML_Info *)booboo::get_black_box(prg, "mml");
	if (info == nullptr) {
		info = new MML_Info;
		info->mml_id = 0;
		booboo::set_black_box(prg, "mml", info);
	}
	return info;
}

static Image_Info *image_info(Program *prg)
{
	Image_Info *info = (Image_Info *)booboo::get_black_box(prg, "image");
	if (info == nullptr) {
		info = new Image_Info;
		info->image_id = 0;
		booboo::set_black_box(prg, "image", info);
	}
	return info;
}

static Font_Info *font_info(Program *prg)
{
	Font_Info *info = (Font_Info *)booboo::get_black_box(prg, "font");
	if (info == nullptr) {
		info = new Font_Info;
		info->font_id = 0;
		booboo::set_black_box(prg, "font", info);
	}
	return info;
}

static CFG_Info *cfg_info(Program *prg)
{
	CFG_Info *info = (CFG_Info *)booboo::get_black_box(prg, "cfg");
	if (info == nullptr) {
		info = new CFG_Info;
		info->cfg_id = 0;
		booboo::set_black_box(prg, "cfg", info);
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
		else if (var.type == Variable::POINTER) {
			snprintf(buf, 1000, "-pointer-");
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

	int millis = (int)as_number(prg, v[0]);
	SDL_Delay(millis);
	return true;
}

static bool miscfunc_get_ticks(Program *prg, std::vector<Token> &v)
{
	Variable &v1 = as_variable(prg, v[0]);
	
	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = SDL_GetTicks();

	return true;
}

static bool miscfunc_rand(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable(prg, v[0]);

	int min_incl = as_number(prg, v[1]);
	int max_incl = as_number(prg, v[2]);

	if (v1.type == Variable::NUMBER) {
		v1.n = util::rand(min_incl, max_incl);
	}
	else if (v1.type == Variable::STRING) {
		v1.s = util::itos(util::rand(min_incl, max_incl));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

static bool gfxfunc_clear(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	SDL_Colour c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
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
	
	int w = as_number(prg, v[0]);
	int h = as_number(prg, v[1]);

	float aspect = (float)w/h;
	gfx::set_min_aspect_ratio(aspect-0.001f);
	gfx::set_max_aspect_ratio(aspect+0.001f);
	gfx::restart(w, h, false, shim::real_screen_size.w, shim::real_screen_size.h);

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
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p1, p2;

	p1.x = as_number(prg, v[4]);
	p1.y = as_number(prg, v[5]);
	p2.x = as_number(prg, v[6]);
	p2.y = as_number(prg, v[7]);

	float thick = as_number(prg, v[8]);

	gfx::draw_line(c, p1, p2, thick);

	return true;
}

static bool primfunc_filled_triangle(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(18)

	SDL_Colour c[3];
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

static bool primfunc_rectangle(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(9)

	SDL_Colour c;
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

	float thick = as_number(prg, v[8]);

	gfx::draw_rectangle(c, p, sz, thick);

	return true;
}

static bool primfunc_filled_rectangle(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(20)

	SDL_Colour c[4];
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

static bool primfunc_ellipse(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(10)

	SDL_Colour c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);

	float _rx = as_number(prg, v[6]);
	float _ry = as_number(prg, v[7]);
	float thick = as_number(prg, v[8]);
	float _sections = as_number(prg, v[9]);

	gfx::draw_ellipse(c, p, _rx, _ry, thick, _sections);

	return true;
}

static bool primfunc_filled_ellipse(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(9)

	SDL_Colour c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);

	float _rx = as_number(prg, v[6]);
	float _ry = as_number(prg, v[7]);
	float _sections = as_number(prg, v[8]);

	gfx::draw_filled_ellipse(c, p, _rx, _ry, _sections);

	return true;
}

static bool primfunc_circle(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(9)

	SDL_Colour c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);
	float _r = as_number(prg, v[6]);
	float thick = as_number(prg, v[7]);
	int _sections = as_number(prg, v[8]);

	gfx::draw_circle(c, p, _r, thick, _sections);

	return true;
}

static bool primfunc_filled_circle(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(8)

	SDL_Colour c;
	c.r = as_number(prg, v[0]);
	c.g = as_number(prg, v[1]);
	c.b = as_number(prg, v[2]);
	c.a = as_number(prg, v[3]);

	util::Point<float> p;

	p.x = as_number(prg, v[4]);
	p.y = as_number(prg, v[5]);
	float _r = as_number(prg, v[6]);
	int _sections = as_number(prg, v[7]);

	gfx::draw_filled_circle(c, p, _r, _sections);

	return true;
}

static bool mmlfunc_create(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);
	std::string str = as_string(prg, v[1]);
	
	MML_Info *info = mml_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->mml_id;
	}
	else if (v1.type == Variable::STRING) {
		v1.s = util::itos(info->mml_id);
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

	Variable &v1 = as_variable(prg, v[0]);
	std::string name = as_string(prg, v[1]);

	MML_Info *info = mml_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->mml_id;
	}
	else if (v1.type == Variable::STRING) {
		v1.s = util::itos(info->mml_id);
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

	int id = as_number(prg, v[0]);
	float volume = as_number(prg, v[1]);
	bool loop = as_number(prg, v[2]);

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

	int id = as_number(prg, v[0]);

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

static bool imagefunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);
	std::string name = as_string(prg, v[1]);

	Image_Info *info = image_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->image_id;
	}
	else if (v1.type == Variable::STRING) {
		v1.s = util::itos(info->image_id);
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

	double id = as_number(prg, v[0]);
	double r = as_number(prg, v[1]);
	double g = as_number(prg, v[2]);
	double b = as_number(prg, v[3]);
	double a = as_number(prg, v[4]);
	double x = as_number(prg, v[5]);
	double y = as_number(prg, v[6]);
	double flip_h = as_number(prg, v[7]);
	double flip_v = as_number(prg, v[8]);

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

	double id = as_number(prg, v[0]);
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
	double flip_h = as_number(prg, v[13]);
	double flip_v = as_number(prg, v[14]);
	
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

	double id = as_number(prg, v[0]);
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
	double flip_h = as_number(prg, v[12]);
	double flip_v = as_number(prg, v[13]);

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

	double img = as_number(prg, v[0]);

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

	double img = as_number(prg, v[0]);

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

	double id = as_number(prg, v[0]);

	Variable &v1 = as_variable(prg, v[1]);
	Variable &v2 = as_variable(prg, v[2]);
	
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

static bool fontfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(4)

	std::string name = as_string(prg, v[1]);
	int size = as_number(prg, v[2]);
	bool smooth = as_number(prg, v[3]);

	Variable &v1 = as_variable(prg, v[0]);

	Font_Info *info = font_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->font_id;
	}
	else if (v1.type == Variable::STRING) {
		v1.s = util::itos(info->font_id);
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

	double id = as_number(prg, v[0]);
	double r = as_number(prg, v[1]);
	double g = as_number(prg, v[2]);
	double b = as_number(prg, v[3]);
	double a = as_number(prg, v[4]);
	std::string text = as_string(prg, v[5]);
	double x = as_number(prg, v[6]);
	double y = as_number(prg, v[7]);

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

	double id = as_number(prg, v[0]);
	Variable &v1 = as_variable(prg, v[1]);
	std::string text = as_string(prg, v[2]);
	
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

	double id = as_number(prg, v[0]);
	Variable &v1 = as_variable(prg, v[1]);
	
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

	double num = as_number(prg, v[0]);
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

	int num_keys;
	const Uint8 *keys = SDL_GetKeyboardState(&num_keys);
	bool kreturn = keys[SDL_SCANCODE_RETURN];
	bool kw = keys[SDL_SCANCODE_W];
	bool ka = keys[SDL_SCANCODE_A];
	bool ks = keys[SDL_SCANCODE_S];
	bool kd = keys[SDL_SCANCODE_D];
	bool kleft = keys[SDL_SCANCODE_LEFT];
	bool kright = keys[SDL_SCANCODE_RIGHT];
	bool kup = keys[SDL_SCANCODE_UP];
	bool kdown = keys[SDL_SCANCODE_DOWN];
	bool kesc = keys[SDL_SCANCODE_ESCAPE];
	bool kspace = keys[SDL_SCANCODE_SPACE];
	float kx1;
	float ky1;
	float kx2;
	float ky2;
	kx1 = ka ? -1 : (kd ? 1 : 0);
	ky1 = kw ? -1 : (ks ? 1 : 0);
	kx2 = kleft ? -1 : (kright ? 1 : 0);
	ky2 = kup ? -1 : (kdown ? 1 : 0);

	SDL_JoystickID id = input::get_controller_id(num);
	SDL_GameController *gc = input::get_sdl_gamecontroller(id);
	bool connected = gc != nullptr;

	if (connected == false) {
		set_string_or_number(prg, x1, kx1);
		set_string_or_number(prg, y1, ky1);
		set_string_or_number(prg, x2, kx2);
		set_string_or_number(prg, y2, ky2);
		set_string_or_number(prg, a, kspace);
		set_string_or_number(prg, l, kleft);
		set_string_or_number(prg, r, kright);
		set_string_or_number(prg, u, kup);
		set_string_or_number(prg, d, kdown);
		set_string_or_number(prg, b, kreturn);
		set_string_or_number(prg, x, 0);
		set_string_or_number(prg, y, 0);
		set_string_or_number(prg, lb, 0);
		set_string_or_number(prg, rb, 0);
		set_string_or_number(prg, back, kesc);
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

		if (x1f == 0 && y1f == 0) {
			set_string_or_number(prg, x1, kx1);
			set_string_or_number(prg, y1, ky1);
		}
		else {
			set_string_or_number(prg, x1, x1f);
			set_string_or_number(prg, y1, y1f);
		}

		if (x2f == 0 && y2f == 0) {
			set_string_or_number(prg, x2, kx2);
			set_string_or_number(prg, y2, ky2);
		}
		else {
			set_string_or_number(prg, x2, x2f);
			set_string_or_number(prg, y2, y2f);
		}

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

		set_string_or_number(prg, l, _lb || kleft);
		set_string_or_number(prg, r, _rb || kright);
		set_string_or_number(prg, u, ub || kup);
		set_string_or_number(prg, d, db || kdown);
		set_string_or_number(prg, a, ab || kspace);
		set_string_or_number(prg, b, bb || kreturn);
		set_string_or_number(prg, x, xb);
		set_string_or_number(prg, y, yb);
		set_string_or_number(prg, lb, lbb);
		set_string_or_number(prg, rb, rbb);
		set_string_or_number(prg, ls, lsb);
		set_string_or_number(prg, rs, rsb);
		set_string_or_number(prg, back, backb || kesc);
		set_string_or_number(prg, start, startb);
	}

	return true;
}

static bool joyfunc_count(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = input::get_num_joysticks();
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

static bool cfgfunc_load(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);
	std::string cfg_name = as_string(prg, v[1]);

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

	int id = as_number(prg, v[0]);
	Variable &v1 = as_variable(prg, v[1]);
	std::string cfg_name = as_string(prg, v[2]);

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

	Variable &v1 = as_variable(prg, v[0]);
	Variable &v2 = as_variable(prg, v[1]);
	std::string name = as_string(prg, v[2]);

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

	Variable &v1 = as_variable(prg, v[0]);
	Variable &v2 = as_variable(prg, v[1]);
	std::string name = as_string(prg, v[2]);

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

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	double val = as_number(prg, v[2]);

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

	Variable &v1 = as_variable(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	std::string val = as_string(prg, v[2]);

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

	int id = as_number(prg, v[0]);
	Variable &v1 = as_variable(prg, v[1]);
	std::string name = as_string(prg, v[2]);

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

void start_lib_game()
{
	add_syntax("inspect", miscfunc_inspect);
	add_syntax("delay", miscfunc_delay);
	add_syntax("get_ticks", miscfunc_get_ticks);
	add_syntax("rand", miscfunc_rand);
	add_syntax("clear", gfxfunc_clear);
	add_syntax("flip", gfxfunc_flip);
	add_syntax("resize", gfxfunc_resize);
	add_syntax("start_primitives", primfunc_start_primitives);
	add_syntax("end_primitives", primfunc_end_primitives);
	add_syntax("line", primfunc_line);
	add_syntax("filled_triangle", primfunc_filled_triangle);
	add_syntax("rectangle", primfunc_rectangle);
	add_syntax("filled_rectangle", primfunc_filled_rectangle);
	add_syntax("ellipse", primfunc_ellipse);
	add_syntax("filled_ellipse", primfunc_filled_ellipse);
	add_syntax("circle", primfunc_circle);
	add_syntax("filled_circle", primfunc_filled_circle);
	add_syntax("image_load", imagefunc_load);
	add_syntax("image_draw", imagefunc_draw);
	add_syntax("image_stretch_region", imagefunc_stretch_region);
	add_syntax("image_draw_rotated_scaled", imagefunc_draw_rotated_scaled);
	add_syntax("image_start", imagefunc_start);
	add_syntax("image_end", imagefunc_end);
	add_syntax("image_size", imagefunc_size);
	add_syntax("font_load", fontfunc_load);
	add_syntax("font_draw", fontfunc_draw);
	add_syntax("font_width", fontfunc_width);
	add_syntax("font_height", fontfunc_height);
	add_syntax("mml_create", mmlfunc_create);
	add_syntax("mml_load", mmlfunc_load);
	add_syntax("mml_play", mmlfunc_play);
	add_syntax("mml_stop", mmlfunc_stop);
	add_syntax("joystick_poll", joyfunc_poll);
	add_syntax("joystick_count", joyfunc_count);
	add_syntax("cfg_load", cfgfunc_load);
	add_syntax("cfg_save", cfgfunc_save);
	add_syntax("cfg_get_number", cfgfunc_get_number);
	add_syntax("cfg_get_string", cfgfunc_get_string);
	add_syntax("cfg_set_number", cfgfunc_set_number);
	add_syntax("cfg_set_string", cfgfunc_set_string);
	add_syntax("cfg_exists", cfgfunc_exists);
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
	Image_Info *image_i = image_info(prg);
	for (size_t i = 0; i < image_i->images.size(); i++) {
		delete image_i->images[i];
	}
	Font_Info *font_i = font_info(prg);
	for (size_t i = 0; i < font_i->fonts.size(); i++) {
		delete font_i->fonts[i];
	}
	CFG_Info *cfg_i = cfg_info(prg);

	delete mml_i;
	delete image_i;
	delete font_i;
	delete cfg_i;

	booboo::set_black_box(prg, "mml", nullptr);
	booboo::set_black_box(prg, "image", nullptr);
	booboo::set_black_box(prg, "font", nullptr);
	booboo::set_black_box(prg, "cfg", nullptr);
}
