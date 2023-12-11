//#define LUA_BENCH
//#define LUA_BENCH2
//#define LUA_BENCH3
//#define CPP_BENCH
//#define CPP_BENCH2

#if defined LUA_BENCH || defined LUA_BENCH2 || defined LUA_BENCH3
#define LUA_BENCH_ANY
#endif

#include <shim4/shim4.h>

#include "booboo/booboo.h"
#include "booboo/game_lib.h"

using namespace booboo;

#if defined __linux__ || defined __LP64__
#include <unistd.h>
#endif

#include <climits>

#include <sys/stat.h>

#ifdef CPP_BENCH
gfx::Image *grass;
gfx::Image *robot;
#endif

Program *prg;

int orig_argc;
char **orig_argv;

std::string extra_args;
std::string extra_args_orig;

#if defined LUA_BENCH_ANY
extern "C" {
#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>
#include <lua5.4/lualib.h>
}

lua_State *lua_state;
std::vector<gfx::Image *> lua_images;

void dump_lua_stack(lua_State *l)
{
        int i;
        int top = lua_gettop(l);
	char buf[1000];

        snprintf(buf, 1000, "--- stack ---\n");
	printf("%s", buf);
        snprintf(buf, 1000, "top=%u   ...   ", top);
	printf("%s", buf);

        for (i = 1; i <= top; i++) {  /* repeat for each level */
                int t = lua_type(l, i);
                switch (t) {

                case LUA_TSTRING:  /* strings */
                        snprintf(buf, 1000, "`%s'", lua_tostring(l, i));
			printf("%s", buf);
                        break;

                case LUA_TBOOLEAN:  /* booleans */
                        snprintf(buf, 1000, lua_toboolean(l, i) ? "true" : "false");
			printf("%s", buf);
                        break;

                case LUA_TNUMBER:  /* numbers */
                        snprintf(buf, 1000, "%g", lua_tonumber(l, i));
			printf("%s", buf);
                        break;

                case LUA_TTABLE:   /* table */
                        snprintf(buf, 1000, "table");
			printf("%s", buf);
                        break;

                default:  /* other values */
                        snprintf(buf, 1000, "%s", lua_typename(l, t));
			printf("%s", buf);
                        break;

                }
                snprintf(buf, 1000, "  ");  /* put a separator */
        }
        snprintf(buf, 1000, "\n");  /* end the listing */
	printf("%s", buf);

        snprintf(buf, 1000, "-------------\n");
	printf("%s", buf);
}


/*
 * Call a Lua function, leaving the results on the stack.
 */
void call_lua(lua_State* lua_state, const char *func, const char *sig, ...)
{
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	lua_getglobal(lua_state, func);  /* get function */

	if (!lua_isfunction(lua_state, -1)) {
		lua_pop(lua_state, 1);
		return;
	}

	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {
			case 'd':  /* double argument */
				lua_pushnumber(lua_state, va_arg(vl, double));
				break;
			case 'b':  /* boolean (int) argument */
				lua_pushboolean(lua_state, va_arg(vl, int));
				break;
			case 'i':  /* int argument */
				lua_pushnumber(lua_state, va_arg(vl, int));
				break;

			case 's':  /* string argument */
				lua_pushstring(lua_state, va_arg(vl, char *));
				break;
			case 'u':  /* userdata argument */
				lua_pushlightuserdata(lua_state, va_arg(vl, void *));
				break;
			case '>':
				goto endwhile;
			default:
				break;
		}
		narg++;
		luaL_checkstack(lua_state, 1, "too many arguments");
	}
endwhile:

	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if (lua_pcall(lua_state, narg, nres, 0) != 0) {
		dump_lua_stack(lua_state);
	}

	va_end(vl);
}

extern "C" {

static int c_start_primitives(lua_State *stack)
{
	gfx::draw_primitives_start();
	return 0;
}

static int c_end_primitives(lua_State *stack)
{
	gfx::draw_primitives_end();
	return 0;
}

static int c_filled_circle(lua_State *stack)
{
	int r = lua_tonumber(stack, 1);
	int g = lua_tonumber(stack, 2);
	int b = lua_tonumber(stack, 3);
	int a = lua_tonumber(stack, 4);
	float x = lua_tonumber(stack, 5);
	float y = lua_tonumber(stack, 6);
	float radius = lua_tonumber(stack, 7);
	int sections = lua_tonumber(stack, 8);

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	util::Point<float> p(x, y);

	gfx::draw_filled_circle(c, p, radius, sections);

	return 0;
}

static int c_filled_rectangle(lua_State *stack)
{
	int r = lua_tonumber(stack, 1);
	int g = lua_tonumber(stack, 2);
	int b = lua_tonumber(stack, 3);
	int a = lua_tonumber(stack, 4);
	int r2 = lua_tonumber(stack, 5);
	int g2 = lua_tonumber(stack, 6);
	int b2 = lua_tonumber(stack, 7);
	int a2 = lua_tonumber(stack, 8);
	int r3 = lua_tonumber(stack, 9);
	int g3 = lua_tonumber(stack, 10);
	int b3 = lua_tonumber(stack, 11);
	int a3 = lua_tonumber(stack, 12);
	int r4 = lua_tonumber(stack, 13);
	int g4 = lua_tonumber(stack, 14);
	int b4 = lua_tonumber(stack, 15);
	int a4 = lua_tonumber(stack, 16);
	float x = lua_tonumber(stack, 17);
	float y = lua_tonumber(stack, 18);
	float w = lua_tonumber(stack, 19);
	float h = lua_tonumber(stack, 20);

	SDL_Color c[4];
	c[0].r = r;
	c[0].g = g;
	c[0].b = b;
	c[0].a = a;
	c[1].r = r2;
	c[1].g = g2;
	c[1].b = b2;
	c[1].a = a2;
	c[2].r = r3;
	c[2].g = g3;
	c[2].b = b3;
	c[2].a = a3;
	c[3].r = r4;
	c[3].g = g4;
	c[3].b = b4;
	c[3].a = a4;

	gfx::draw_filled_rectangle(c, util::Point<float>(x, y), util::Size<float>(w, h));

	return 0;
}

static int c_clear(lua_State *stack)
{
	int r = lua_tonumber(stack, 1);
	int g = lua_tonumber(stack, 2);
	int b = lua_tonumber(stack, 3);

	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = 255;

	gfx::clear(c);

	return 0;
}

static int c_load_image(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);

	int n = lua_images.size();

	lua_images.push_back(new gfx::Image(name));

	lua_pushnumber(stack, n);

	return 1;
}

static int c_image_size(lua_State *stack)
{
	int n = lua_tonumber(stack, 1);

	lua_pushnumber(stack, lua_images[n]->size.w);
	lua_pushnumber(stack, lua_images[n]->size.h);

	return 2;
}

static int c_image_draw(lua_State *stack)
{
	int n = lua_tonumber(stack, 1);
	int tint_r = lua_tonumber(stack, 2);
	int tint_g = lua_tonumber(stack, 3);
	int tint_b = lua_tonumber(stack, 4);
	int tint_a = lua_tonumber(stack, 5);
	int x = lua_tonumber(stack, 6);
	int y = lua_tonumber(stack, 7);
	int flip_h = lua_tonumber(stack, 8);
	int flip_v = lua_tonumber(stack, 9);

	SDL_Colour c;
	c.r = tint_r;
	c.g = tint_g;
	c.b = tint_b;
	c.a = tint_a;

	int flip = 0;
	if (flip_h) {
		flip |= gfx::Image::FLIP_H;
	}
	if (flip_v) {
		flip |= gfx::Image::FLIP_V;
	}

	lua_images[n]->draw_tinted(c, util::Point<float>(x, y), flip);

	return 0;
}

static int c_image_start(lua_State *stack)
{
	int n = lua_tonumber(stack, 1);

	lua_images[n]->start_batch();

	return 0;
}

static int c_image_end(lua_State *stack)
{
	int n = lua_tonumber(stack, 1);

	lua_images[n]->end_batch();

	return 0;
}

static int c_rand(lua_State *stack)
{
	int min_incl = lua_tonumber(stack, 1);
	int max_incl = lua_tonumber(stack, 2);

	lua_pushnumber(stack, util::rand(min_incl, max_incl));

	return 1;
}

}

void init_lua()
{
	lua_state = luaL_newstate();

	luaL_openlibs(lua_state);

	#define REGISTER_FUNCTION(name) \
		lua_pushcfunction(lua_state, c_ ## name); \
		lua_setglobal(lua_state, #name);

	REGISTER_FUNCTION(clear);
	REGISTER_FUNCTION(start_primitives);
	REGISTER_FUNCTION(end_primitives);
	REGISTER_FUNCTION(filled_circle);
	REGISTER_FUNCTION(filled_rectangle);
	REGISTER_FUNCTION(load_image);
	REGISTER_FUNCTION(image_size);
	REGISTER_FUNCTION(image_draw);
	REGISTER_FUNCTION(image_start);
	REGISTER_FUNCTION(image_end);
	REGISTER_FUNCTION(rand);

	#undef REGISTER_FUNCION

#ifdef LUA_BENCH
	std::string program = util::load_text_from_filesystem("robots.lua");
#elif defined LUA_BENCH2
	std::string program = util::load_text_from_filesystem("sine.lua");
#else
	std::string program = util::load_text_from_filesystem("plasma.lua");
#endif
	luaL_loadstring(lua_state, program.c_str());
	if (lua_pcall(lua_state, 0, 0, 0) != 0) {
		dump_lua_stack(lua_state);
	}
}
#endif

static util::Point<int> mouse_pos;
static bool mouse_b1;
static bool mouse_b2;
static bool mouse_b3;

static bool mousefunc_get_position(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);
	Variable &v2 = as_variable(prg, v[1]);
	
	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = mouse_pos.x;
	v2.n = mouse_pos.y;

	return true;
}

static bool mousefunc_get_buttons(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable(prg, v[0]);
	Variable &v2 = as_variable(prg, v[1]);
	Variable &v3 = as_variable(prg, v[2]);
	
	if (v1.type != Variable::NUMBER || v2.type != Variable::NUMBER || v3.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.n = mouse_b1;
	v2.n = mouse_b2;
	v3.n = mouse_b3;

	return true;
}

std::list<int> keys_pressed;

static bool keyfunc_get(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);
	
	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	bool pressed = false;
	int checking = as_number(prg, v[1]);

	for (std::list<int>::iterator it = keys_pressed.begin(); it != keys_pressed.end(); it++) {
		int &k = *it;
		if (k == checking) {
			pressed = true;
			break;
		}
	}

	v1.n = pressed;

	return true;
}

bool start()
{
	mouse_pos.x = 0;
	mouse_pos.y = 0;
	mouse_b1 = false;
	mouse_b2 = false;
	mouse_b3 = false;

	// This is basically 16:9 only, with a tiny bit of leeway
	gfx::set_min_aspect_ratio(1.776f);
	gfx::set_max_aspect_ratio(1.778f);

	if (util::bool_arg(false, shim::argc, shim::argv, "logging")) {
		shim::logging = true;
	}

	if (util::bool_arg(false, shim::argc, shim::argv, "dump-images")) {
		gfx::Image::keep_data = true;
		gfx::Image::save_palettes = util::bool_arg(false, shim::argc, shim::argv, "save-palettes");
		gfx::Image::save_rle = true;
		gfx::Image::save_rgba = false;
		gfx::Image::premultiply_alpha = false;
	}

	/* get_desktop_resolution uses shim::adapter, but it's not normally set until shim::start is called, so set it here since
	 * it's used below.
	 */
	int adapter_i = util::check_args(shim::argc, shim::argv, "+adapter");
	if (adapter_i >= 0) {
		shim::adapter = atoi(shim::argv[adapter_i+1]);
		if (shim::adapter >= SDL_GetNumVideoDisplays()-1) {
			shim::adapter = 0;
		}
	}

	gfx::set_minimum_window_size(util::Size<int>(640, 360));
	util::Size<int> desktop_resolution = gfx::get_desktop_resolution();
	gfx::set_maximum_window_size(desktop_resolution);

#if !defined IOS && !defined ANDROID
	const int min_supp_w = 1280;
	const int min_supp_h = 720;

	if (desktop_resolution.w < min_supp_w || desktop_resolution.h < min_supp_h) {
		gui::popup("Unsupported System", "The minimum resolution supported by this game is 1280x720, which this system does not meet. Exiting.", gui::OK);
		exit(1);
	}
#endif

	int win_w = 1280;
	int win_h = 720;

	if (desktop_resolution.w >= 1920*2 && desktop_resolution.h >= 1080*2) {
		win_w = 2560;
		win_h = 1440;
	}
	else if (desktop_resolution.w >= 2560 && desktop_resolution.h >= 1440) {
		win_w = 1920;
		win_h = 1080;
	}

	//if (shim::start_all(0, 0, false, desktop_resolution.w, desktop_resolution.h) == false) {
	if (shim::start_all(640, 360, false, win_w, win_h) == false) {
		gui::fatalerror("ERROR", "Initialization failed", gui::OK, true);
		return false;
	}

#ifdef _WIN32
	gfx::enable_press_and_hold(false);
#endif

	/*
	if (shim::font == 0) {
		gui::fatalerror("Fatal Error", "Font not found! Aborting.", gui::OK, false);
		return false;
	}
	*/

	if (util::bool_arg(false, shim::argc, shim::argv, "dump-images")) {
		std::vector<std::string> filenames = shim::cpa->get_all_filenames();
		for (size_t i = 0; i < filenames.size(); i++) {
			std::string filename =  filenames[i];
			if (filename.find(".tga") != std::string::npos) {
				gfx::Image *image = new gfx::Image(filename, true);
				std::string path = "out/" + filename;
				std::string dir;
				size_t i;
				while ((i = path.find("/")) != std::string::npos) {
					dir += path.substr(0, i);
					util::mkdir(dir.c_str());
					path = path.substr(i+1);
					dir += "/";
				}
				image->save("out/" + filename);
				delete image;
			}
		}
		exit(0);
	}

	TGUI::set_focus_sloppiness(0);

	//gfx::register_lost_device_callbacks(lost_device, found_device);
	//shim::joystick_disconnect_callback = joystick_disconnected;

	return true;
}

void handle_event(TGUI_Event *event)
{
	if (event->type == TGUI_UNKNOWN) {
		return;
	}
	else if (event->type == TGUI_QUIT) {
		quit = true;
	}
	else if (event->type == TGUI_MOUSE_AXIS) {
		if (event->mouse.normalised) {
			mouse_pos.x = event->mouse.x * shim::screen_size.w;
			mouse_pos.y = event->mouse.y * shim::screen_size.h;
		}
		else {
			mouse_pos.x = event->mouse.x;
			mouse_pos.y = event->mouse.y;
		}
	}
	else if (event->type == TGUI_MOUSE_DOWN && !event->mouse.is_repeat) {
		switch (event->mouse.button) {
			case 1:
				mouse_b1 = true;
				break;
			case 2:
				mouse_b2 = true;
				break;
			case 3:
				mouse_b3 = true;
				break;
		}
	}
	else if (event->type == TGUI_MOUSE_UP) {
		switch (event->mouse.button) {
			case 1:
				mouse_b1 = false;
				break;
			case 2:
				mouse_b2 = false;
				break;
			case 3:
				mouse_b3 = false;
				break;
		}
	}
	else if (event->type == TGUI_KEY_DOWN && !event->keyboard.is_repeat) {
		bool exists = false;
		for (std::list<int>::iterator it = keys_pressed.begin(); it != keys_pressed.end(); it++) {
			int &k = *it;
			if (k == event->keyboard.code) {
				exists = true;
				break;
			}
		}
		if (exists == false) {
			keys_pressed.push_back(event->keyboard.code);
		}
	}
	else if (event->type == TGUI_KEY_UP) {
		std::list<int>::iterator it;
		for (it = keys_pressed.begin(); it != keys_pressed.end();) {
			int &k = *it;
			if (k == event->keyboard.code) {
				it = keys_pressed.erase(it);
			}
			else {
				it++;
			}
		}
	}
}

void draw_all()
{
	gfx::clear(shim::black);

	gfx::set_cull_mode(gfx::NO_FACE);
	
#if defined LUA_BENCH_ANY
	call_lua(lua_state, "draw", "");
#elif defined CPP_BENCH2
	SDL_Colour c;
	c.r = 0;
	c.g = 255;
	c.b = 0;
	c.a = 25;
	for (int x = 0; x < 640; x+=2) {
		float y = sin(x/640.0f*M_PI*2)*90;
		gfx::draw_filled_circle(c, util::Point<float>(x, y+180), 8);
	}
	Uint32 now = SDL_GetTicks();
	now /= 16;
	now *= 5;
	now %= 640;
	float y = sin(now/640.0f*M_PI*2)*90;
	c.r = 128;
	c.b = 128;
	gfx::draw_filled_circle(c, util::Point<float>(now, y+180), 32);
#elif defined CPP_BENCH
	int w = 640 / grass->size.w;
	int h = 360 / grass->size.h;
	grass->start_batch();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			grass->draw(util::Point<float>(x*grass->size.w, y*grass->size.h));
		}
	}
	grass->end_batch();
	for (int i = 0; i < 17; i++) {
		int x = util::rand(0, w-1);
		int y = util::rand(0, h-1);
		robot->draw(util::Point<float>(x*robot->size.w, y*robot->size.h));
	}
#else
	std::vector<Token> tmp;
	call_void_function(prg, "draw", tmp);
#endif

	gfx::draw_guis();
	gfx::draw_notifications();

	gfx::flip();
}

static void loop()
{
	// These keep the logic running at 60Hz and drawing at refresh rate is possible
	// NOTE: screen refresh rate has to be 60Hz or higher for this to work.
	Uint32 start = SDL_GetTicks();
	int logic_frames = 0;
	int drawing_frames = 0;
	bool can_draw = true;
	bool can_logic = true;
	std::string old_music_name = "";
#if defined IOS || defined ANDROID
	float old_volume = 1.0f;
#endif
	int curr_logic_rate = shim::logic_rate;

	while (quit == false) {
		// EVENTS
		while (true) {
			SDL_Event sdl_event;
			TGUI_Event *e = nullptr;

			bool all_done = false;

			if (!SDL_PollEvent(&sdl_event)) {
				e = shim::pop_pushed_event();
				if (e == nullptr) {
					all_done = true;
				}
			}

			if (all_done) {
				break;
			}

			if (e == nullptr) {
				if (sdl_event.type == SDL_QUIT) {
					if (can_logic == false) {
						shim::handle_event(&sdl_event);
						quit = true;
						break;
					}
				}
				else if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_F12) {
					//load_translation();
				}
			}

			TGUI_Event *event;

			if (e) {
				event = e;
			}
			else {
				if (sdl_event.type == SDL_QUIT) {
					static TGUI_Event quit_event;
					quit_event.type = TGUI_QUIT;
					event = &quit_event;
				}
				else {
					event = shim::handle_event(&sdl_event);
				}
			}

			handle_event(event);
		}

		// Logic rate can change in devsettings
		if (shim::logic_rate != curr_logic_rate) {
			curr_logic_rate = shim::logic_rate;
			logic_frames = 0;
			drawing_frames = 0;
			start = SDL_GetTicks();
		}

		// TIMING
		Uint32 now = SDL_GetTicks();
		int diff = now - start;
		bool skip_drawing = false;
		int logic_reps = diff / 16;

		if (logic_reps > 0) {
			start += 16 * logic_reps;
		}

		for (int logic = 0; logic < logic_reps; logic++) {
			can_draw = shim::update();

			// Generate a timer tick event (TGUI_TICK)
			SDL_Event sdl_event;
			sdl_event.type = shim::timer_event_id;
			TGUI_Event *event = shim::handle_event(&sdl_event);
			handle_event(event);

#if defined LUA_BENCH_ANY
			call_lua(lua_state, "run", "");
#else
			std::vector<Token> tmp;
			call_void_function(prg, "run", tmp);
#endif

			if (reset_game_name != "") {
				quit = true;
			}

			logic_frames++;
		}

		// LOGIC
		if (can_logic) {
			for (int logic = 0; logic < logic_reps; logic++) {
				gfx::update_animations();
				// logic
			}
		}

		// DRAWING
		if (skip_drawing == false && can_draw) {
			draw_all();
		}

		if (quit) {
			break;
		}

		drawing_frames++;
	}
}

bool go()
{
	loop();

	return true;
}

void end()
{
	// If Alt-F4 is pressed the title gui can remain in shim::guis. Leaving it to shim to destruct won't work, because ~Title_GUI accesses Globals which is destroyed below
	for (std::vector<gui::GUI *>::iterator it = shim::guis.begin(); it != shim::guis.end();) {
		gui::GUI *gui = *it;
		delete gui;
		it = shim::guis.erase(it);
	}

	shim::end_all();
}

static int set_orig_args(bool forced, bool count_only)
{
	int count = 0;

	for (int i = 0; i < orig_argc; i++) {
		int skip = 0;
		if (forced &&
			(std::string(orig_argv[i]) == "+windowed" ||
			std::string(orig_argv[i]) == "+fullscreen")) {
			skip = 1;
		}
		else if (forced &&
			(std::string(orig_argv[i]) == "+width" ||
			std::string(orig_argv[i]) == "+height" ||
			std::string(orig_argv[i]) == "+scale")) {
			skip = 2;
		}

		if (skip > 0) {
			i += skip-1;
			continue;
		}

		if (count_only == false) {
			shim::argv[count] = new char[strlen(orig_argv[i])+1];
			strcpy(shim::argv[count], orig_argv[i]);
		}
		
		count++;
	}

	return count;
}

void set_shim_args(bool initial, bool force_windowed, bool force_fullscreen)
{
	if (initial) {
		for (int i = 0; i < orig_argc; i++) {
			if (std::string(orig_argv[i]) == "+windowed" || std::string(orig_argv[i]) == "+fullscreen" || std::string(orig_argv[i]) == "+width" || std::string(orig_argv[i]) == "+height" || std::string(orig_argv[i]) == "+adapter") {
				force_windowed = false;
				force_fullscreen = false;
				break;
			}
		}
	}

	bool force = force_windowed || force_fullscreen;
	
	int count = set_orig_args(force, true);

	if (force) {
		count++;
	}

	std::vector<std::string> v;
	if (extra_args != "") {
		util::Tokenizer t(extra_args, ',');
		std::string tok;
		while ((tok = t.next()) != "") {
			v.push_back(tok);
		}
		count += v.size();
	}
	extra_args = ""; // Do this?

	shim::argc = count;
	shim::argv = new char *[count];

	int i = set_orig_args(force, false);

	if (force_windowed) {
		shim::argv[i] = new char[strlen("+windowed")+1];
		strcpy(shim::argv[i], "+windowed");
		i++;
	}
	else if (force_fullscreen) {
		shim::argv[i] = new char[strlen("+fullscreen")+1];
		strcpy(shim::argv[i], "+fullscreen");
		i++;
	}

	for (auto s : v) {
		shim::argv[i] = new char[s.length()+1];
		strcpy(shim::argv[i], s.c_str());
		i++;
	}
}

int main(int argc, char **argv)
{
	std::string fn;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '+' && argv[i][0] != '-') {
			fn = argv[i];
			break;
		}
	}

	if (fn != "") {
		struct stat s;
		if (stat(fn.c_str(), &s) == 0 && (s.st_mode & S_IFMT) == S_IFDIR) {
			chdir(fn.c_str());
			fn = "";
		}
		else {
			int pos = fn.length()-1;
#ifdef _WIN32
			while (pos > 0 && (fn[pos] != '/' && fn[pos] != '\\')) {
#else
			while (pos > 0 && fn[pos] != '/') {
#endif
				pos--;
			}
#ifdef _WIN32
			if (fn[pos] == '/' || fn[pos] == '\\') {
#else
			if (fn[pos] == '/') {
#endif
				chdir(fn.substr(0, pos).c_str());
				fn = fn.substr(pos+1);
			}
		}
	}

	try {

#ifdef _WIN32
	SDL_RegisterApp("BooBoo", 0, 0);
#endif

	orig_argc = argc;
	orig_argv = argv;

	// this must be before static_start which inits SDL
#ifdef _WIN32
	bool directsound = util::bool_arg(true, argc, argv, "directsound");
	bool wasapi = util::bool_arg(false, argc, argv, "wasapi");
	bool winmm = util::bool_arg(false, argc, argv, "winmm");

	if (directsound) {
		_putenv_s("SDL_AUDIODRIVER", "directsound");
	}
	else if (wasapi) {
		_putenv_s("SDL_AUDIODRIVER", "wasapi");
	}
	else if (winmm) {
		_putenv_s("SDL_AUDIODRIVER", "winmm");
	}
#endif

	shim::window_title = "BooBoo";
	shim::organisation_name = "b1stable";
	shim::game_name = "BooBoo";
	//
	shim::logging = true;
	gfx::Image::ignore_palette = true;

	// Need to be set temporarily to original values so +error-level works. They get changed below
	shim::argc = orig_argc;
	shim::argv = orig_argv;

	//set_shim_args(true, false, true);
	set_shim_args(true, true, false);
	
	shim::use_cwd = true;

	shim::static_start_all();
	
	shim::use_cwd = true;

	util::srand((uint32_t)time(NULL));

	//shim::create_depth_buffer = true;
	shim::font_size = 24;

	shim::convert_directions_to_focus_events = false;

	::start();

	if (shim::font != nullptr) {
		shim::devsettings_num_rows = (shim::screen_size.h-16)/shim::font->get_height();
		shim::devsettings_max_width = shim::screen_size.w-10;
	}

	booboo::start();
	start_lib_game();
	add_instruction("mouse_get_position", mousefunc_get_position);
	add_instruction("mouse_get_buttons", mousefunc_get_buttons);
	add_instruction("key_get", keyfunc_get);

again:
	quit = false;
	bool was_reset = false;

	if (reset_game_name != "") {
		fn = reset_game_name;
		reset_game_name = "";
		was_reset = true;
	}

	std::string code;

	if (was_reset) {
		try {
			code = util::load_text("scripts/" + fn);
		}
		catch (util::Error &e) {
			gui::fatalerror("ERROR", "Program is missing or corrupt!", gui::OK, true);
		}

		main_program_name = fn;
	}
	else {
		if (fn != "") {
			try {
				code = util::load_text("scripts/" + fn);
			}
			catch (util::Error &e) {
				gui::fatalerror("ERROR", "Program is missing or corrupt!", gui::OK, true);
			}

			main_program_name = fn;
		}
		else {
			try {
				code = util::load_text("scripts/main.boo");
			}
			catch (util::Error &e) {
				gui::fatalerror("ERROR", "Program is missing or corrupt!", gui::OK, true);
			}
			
			main_program_name = "main.boo";
		}
	}

	prg = create_program(code);

	while (interpret(prg, INT_MAX)) {
	}

#if defined LUA_BENCH_ANY
	init_lua();
#elif defined CPP_BENCH
	grass = new gfx::Image("misc/grass.tga");
	robot = new gfx::Image("misc/robot.tga");
#endif

	if (reset_game_name == "") {
		go();
	}

	std::vector<Token> tmp;
	call_void_function(prg, "end", tmp);

	game_lib_destroy_program(prg);
	destroy_program(prg);

	if (reset_game_name != "") {
		fn = "";
		goto again;
	}

	end_lib_game();
	booboo::end();

	::end();

	}
	catch (util::Error &e) {
		gui::fatalerror("ERROR", e.error_message.c_str(), gui::OK, true);
	}
	catch (Error &e) {
		gui::fatalerror("ERROR", e.error_message.c_str(), gui::OK, true);
	}

	return return_code;
}
