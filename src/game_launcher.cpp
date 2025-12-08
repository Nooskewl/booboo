#include <unistd.h>
#include <climits>
#include <sys/stat.h>

#ifdef _WIN32
#include <shlwapi.h>
#endif

#include <shim5/shim5.h>
#include <shim5/internal/gfx.h>

#include <libutil/libutil.h>
using namespace noo;

#include "booboo/booboo.h"
#include "booboo/standard_lib.h"
#include "booboo/game_lib.h"
#include "booboo/internal.h"
using namespace booboo;

const int MAX_OPS_PER_SECOND = 10000000;

Program *prg;

int orig_argc;
char **orig_argv;

std::string extra_args;
std::string extra_args_orig;

bool toggle_fullscreen;
bool invert_mouse_wheel;

static util::Point<int> mouse_pos;
static bool mouse_b1;
static bool mouse_b2;
static bool mouse_b3;
static int mouse_wheel_y;
static int mouse_dx;
static int mouse_dy;
static bool delta_got;

static int exit_key = TGUIK_F12;

extern int num_ops;
Uint64 op_time = 0;
double ops_sec = 0;
bool limits = true;
bool show_ops;

static bool mousefunc_set_relative(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool onoff = as_number(prg, v[0]);

	SDL_SetWindowRelativeMouseMode(gfx::internal::gfx_context.window, (bool)onoff);

	return true;
}

static Variable exprfunc_mouse_get_delta(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	Variable v2;
	v1.type = Variable::NUMBER;
	v2.type = Variable::NUMBER;

	if (delta_got == false) {
		mouse_dx = 0;
		mouse_dy = 0;
		delta_got = true;
	}

	v1.n = mouse_dx;
	v2.n = mouse_dy;

	mouse_dx = 0;
	mouse_dy = 0;

	Variable vec;
	vec.type = Variable::VECTOR;
	vec.v.push_back(v1);
	vec.v.push_back(v2);

	return vec;
}

static Variable exprfunc_mouse_get_position(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	Variable v2;
	v1.type = Variable::NUMBER;
	v2.type = Variable::NUMBER;
	
	v1.n = mouse_pos.x;
	v2.n = mouse_pos.y;

	Variable vec;
	vec.type = Variable::VECTOR;
	vec.v.push_back(v1);
	vec.v.push_back(v2);

	return vec;
}

static Variable exprfunc_mouse_get_buttons(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	Variable v2;
	Variable v3;
	Variable v4;
	v1.type = Variable::NUMBER;
	v2.type = Variable::NUMBER;
	v3.type = Variable::NUMBER;
	v4.type = Variable::NUMBER;
	
	v1.n = mouse_b1;
	v2.n = mouse_b2;
	v3.n = mouse_b3;
	v4.n = mouse_wheel_y;

	Variable vec;
	vec.type = Variable::VECTOR;
	vec.v.push_back(v1);
	vec.v.push_back(v2);
	vec.v.push_back(v3);
	vec.v.push_back(v4);

	return vec;
}

std::list<int> keys_pressed;

static Variable exprfunc_key_get(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;
	bool pressed = false;
	int checking = as_number(prg, v[0]);

	for (std::list<int>::iterator it = keys_pressed.begin(); it != keys_pressed.end(); it++) {
		int &k = *it;
		if (k == checking) {
			pressed = true;
			break;
		}
	}

	v1.n = pressed;

	return v1;
}

bool start()
{
	if (util::bool_arg(true, shim::argc, shim::argv, "limits") == false) {
		limits = false;
	}

	mouse_pos.x = 0;
	mouse_pos.y = 0;
	mouse_b1 = false;
	mouse_b2 = false;
	mouse_b3 = false;
	mouse_wheel_y = 0;
	mouse_dx = 0;
	mouse_dy = 0;
	delta_got = false;

	toggle_fullscreen = false;
	invert_mouse_wheel = false;

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
		int count;
		SDL_DisplayID *disp = SDL_GetDisplays(&count);
		if (shim::adapter >= count) {
			shim::adapter = 0;
		}
	}

	gfx::set_minimum_window_size(util::Size<int>(640, 360));
	util::Size<int> desktop_resolution = gfx::get_desktop_resolution();
	//gfx::set_maximum_window_size(desktop_resolution);

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

	if (shim::start_all(640, 360, false, win_w, win_h) == false) {
		gui::fatalerror("ERROR", "Initialization failed", gui::OK, true);
		return false;
	}

#ifdef _WIN32
	gfx::enable_press_and_hold(false);
#endif

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
	
	util::JSON::Node *root = shim::shim_json->get_root();
	exit_key = root->get_nested_int("booboo>input>exit_key", &exit_key, TGUIK_F12);
	show_ops = util::bool_arg(false, shim::argc, shim::argv, "ops");
	show_ops = root->get_nested_bool("booboo>game>show_ops", &show_ops, show_ops);

	return true;
}

void handle_event(TGUI_Event *event)
{
	if (quit == true) {
		return;
	}

	if (event->type == TGUI_UNKNOWN) {
		return;
	}
	else if (event->type == TGUI_QUIT || (event->type == TGUI_KEY_DOWN && event->keyboard.code == exit_key)) {
		quit = true;
		return;
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
		mouse_dx += event->mouse.dx;
		mouse_dy += event->mouse.dy;
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
	else if (event->type == TGUI_MOUSE_WHEEL) {
		if (event->mouse.y < 0) {
			mouse_wheel_y = -1;
		}
		else if (event->mouse.y > 0) {
			mouse_wheel_y = 1;
		}
		if (invert_mouse_wheel) {
			mouse_wheel_y *= -1;
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
	else if (event->type == TGUI_JOY_DOWN) {
		if (event->joystick.button == TGUI_B_GUIDE) {
			quit = true;
			return;
		}
	}

	std::vector<Token> args;
	Token t;
	t.type = Token::NUMBER;
	t.n = (int)event->type;
	t.dereference = false;
	args.push_back(t);

	for (int i = 0; i < 4; i++) {
		args.push_back(t);
	}

	bool go = true;

	if (event->type == TGUI_KEY_DOWN || event->type == TGUI_KEY_UP) {
		args[1].n = event->keyboard.code;
		args[2].n = event->keyboard.is_repeat;
	}
	else if (event->type == TGUI_JOY_DOWN || event->type == TGUI_JOY_UP) {
		args[1].n = (Uint32)event->joystick.id;
		args[2].n = event->joystick.button;
		args[3].n = event->joystick.is_repeat;
	}
	else if (event->type == TGUI_JOY_AXIS) {
		args[1].n = (Uint32)event->joystick.id;
		args[2].n = event->joystick.axis;
		args[3].n = event->joystick.value;
	}
	else if (event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_UP) {
		args[1].n = event->mouse.button;
		args[2].n = event->mouse.is_repeat;
		args[3].n = event->mouse.x;
		args[4].n = event->mouse.y;
	}
	else if (event->type == TGUI_MOUSE_AXIS) {
		args[1].n = event->mouse.x;
		args[2].n = event->mouse.y;
		args[3].n = event->mouse.dx;
		args[4].n = event->mouse.dy;
	}
	else if (event->type == TGUI_MOUSE_WHEEL) {
		args[1].n = event->mouse.x;
		args[2].n = event->mouse.y;
	}
	else {
		go = false;
	}
	
	if (go) {
		call_void_function_obfuscated(prg, "event", args);
	}
}

void draw_all()
{
	gfx::clear_buffers();

	gfx::set_cull_mode(gfx::NO_FACE);
	
	std::vector<Token> tmp;
	call_void_function_obfuscated(prg, "draw", tmp);

	glm::mat4 _mv, _proj;
	gfx::get_matrices(_mv, _proj);

	gfx::set_default_projection(shim::screen_size, shim::screen_offset, shim::scale);
	gfx::update_projection();

	gfx::draw_guis();
	gfx::draw_notifications();

	if (shim::font && show_ops) {
		Uint64 now = SDL_GetTicks();
		Uint64 diff = now - op_time;
		if (diff >= 5000) {
			ops_sec = num_ops / (diff / 1000.0);
			num_ops = 0;
			op_time = now;
		}
		if (limits && ops_sec >= MAX_OPS_PER_SECOND) {
			throw Error("Over 10,000,000 ops per second - bailing...");
		}
		std::string d = util::string_printf("%.2f", ops_sec);
		int w = shim::font->get_text_width(d);
		for (float y = 0; y < 4; y++) {
			for (float x = 0; x < 4; x++) {
				shim::font->draw(shim::white, d, {shim::screen_size.w-w-x, y});
			}
		}
		for (float y = 1; y < 3; y++) {
			for (float x = 0; x < 4; x++) {
				shim::font->draw(shim::black, d, {shim::screen_size.w-w-x, y});
			}
		}
	}

	gfx::set_matrices(_mv, _proj);
	gfx::update_projection();

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
				if (sdl_event.type == SDL_EVENT_QUIT) {
					if (can_logic == false) {
						shim::handle_event(&sdl_event);
						quit = true;
						break;
					}
				}
			}

			TGUI_Event *event;

			if (e) {
				event = e;
			}
			else {
				if (sdl_event.type == SDL_EVENT_QUIT) {
					static TGUI_Event quit_event;
					quit_event.type = TGUI_QUIT;
					event = &quit_event;
				}
				else {
					event = shim::handle_event(&sdl_event);
				}
			}

			handle_event(event);
			if (quit) {
				break;
			}
		}

		if (quit) {
			break;
		}

		// Logic rate can change in devsettings
		if (shim::logic_rate != curr_logic_rate) {
			curr_logic_rate = shim::logic_rate;
			logic_frames = 0;
			drawing_frames = 0;
			start = SDL_GetTicks();
		}

		// TIMING
		float ms_per_logic_frame = 1000 / shim::logic_rate;
		Uint32 now = SDL_GetTicks();
		int diff = now - start;
		bool skip_drawing = false;
		int logic_reps = diff / ms_per_logic_frame;

		if (logic_reps > 0) {
			start += ms_per_logic_frame * logic_reps;
		}

		for (int logic = 0; logic < logic_reps; logic++) {
			can_draw = shim::update();

			// Generate a timer tick event (TGUI_TICK)
			SDL_Event sdl_event;
			sdl_event.type = shim::timer_event_id;
			TGUI_Event *event = shim::handle_event(&sdl_event);
			handle_event(event);
			if (quit) {
				break;
			}

			std::vector<Token> tmp;
			call_void_function_obfuscated(prg, "run", tmp);

			mouse_wheel_y = 0;

			if (reset_game_name != "") {
				quit = true;
			}

			logic_frames++;
		}

		if (quit) {
			break;
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

static void gen_f11()
{
	TGUI_Event event;
	event.type = TGUI_KEY_DOWN;
	event.keyboard.is_repeat = false;
	event.keyboard.code = TGUIK_F11;
	event.keyboard.simulated = true;
	shim::push_event(event);
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

int main(int argc, char **argv)
{
	booboo::load_text = util::load_text;

	char cwd_buf[1000];
	std::string start_cwd = getcwd(cwd_buf, 1000);

	std::string fn;
	std::string set_dir;

	if (argc > 1 && argv[1][0] != '+' && argv[1][0] != '-') {
		fn = argv[1];
	}

	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "+set-dir" && i < (argc-1)) {
			set_dir = argv[i+1];
		}
	}

	if (fn != "") {
#ifdef _WIN32
		if (PathIsDirectory(fn.c_str())) {
#else
		struct stat s;
		if (stat(fn.c_str(), &s) && S_ISDIR(s.st_mode)) {
#endif
			chdir(fn.c_str());
			fn = "";
		}
		else {
			int pos = fn.length()-1;
	
			if (pos >= 0 && fn[pos] != '/' && fn[pos] != '\\') {
				fn += "/";
				pos++;
			}

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

#ifdef __linux__
	if (std::string(getenv("XDG_SESSION_TYPE")) == "wayland") {
		putenv("SDL_VIDEODRIVER=wayland");
	}
#endif

	shim::window_title = "BooBoo";
	shim::organisation_name = "Nooskewl";
	shim::game_name = "BooBoo";
	//
	shim::logging = true;
	gfx::Image::ignore_palette = true;

	// Need to be set temporarily to original values so +error-level works. They get changed below
	shim::argc = orig_argc;
	shim::argv = orig_argv;

	set_shim_args(true, true, false);
	
	shim::use_cwd = true;

	shim::static_start_all();
	
	shim::use_cwd = true;
	shim::font_size = 16;
	shim::convert_directions_to_focus_events = false;
	shim::create_depth_buffer = true;

	::start();

	if (shim::font != nullptr) {
		shim::devsettings_num_rows = (shim::screen_size.h-16)/shim::font->get_height();
		shim::devsettings_max_width = shim::screen_size.w-10;
	}

	booboo::start();
	start_lib_standard();
	start_lib_game();

	add_instruction("mouse_set_relative", mousefunc_set_relative);
	add_expression_handler("mouse_get_delta", exprfunc_mouse_get_delta);
	add_expression_handler("mouse_get_position", exprfunc_mouse_get_position);
	add_expression_handler("mouse_get_buttons", exprfunc_mouse_get_buttons);
	add_expression_handler("key_get", exprfunc_key_get);

	try {
		std::string path = save_dir();
		std::string cfg_text = util::load_text_from_filesystem(path + "/com.nooskewl.booboo.launcher.txt");
		util::Tokenizer t(cfg_text, '\n');
		std::string line;
		while ((line = t.next()) != "") {
			util::Tokenizer t2(line, '=');
			std::string key = t2.next();
			std::string value = t2.next();
			if (key == "fullscreen") {
				toggle_fullscreen = atoi(value.c_str());
			}
			else if (key == "invert_mouse_wheel") {
				invert_mouse_wheel = atoi(value.c_str());
			}
			else if (key == "volume") {
				shim::music_volume = atoi(value.c_str())/255.0f;
			}
		}
	}
	catch (util::Error &e) {
	}

	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "+volume" && i < (argc-1)) {
			shim::music_volume = atoi(argv[i+1])/255.0f;
		}
	}

	if (toggle_fullscreen) {
		gen_f11();
	}

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

	bool ob = false;
	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "+obfuscate") {
			ob = true;
			break;
		}
	}

	if (ob) {
		booboo::obfuscate(prg);
	}
	else {
		register_game_callbacks();

		while (interpret(prg)) {
		}

		if (reset_game_name == "") {
			go();
		}

		std::vector<Token> tmp;
		call_void_function_obfuscated(prg, "end", tmp);

		unregister_game_callbacks();
	}

	shim::current_shader = shim::default_shader;
	shim::current_shader->use();
	gfx::update_projection();

	std::string out_path = save_dir() + "/com.nooskewl.booboo.launcher.txt";
	FILE *f = fopen(out_path.c_str(), "w");
	fprintf(f, "fullscreen=%d\n", gfx::is_fullscreen_window());
	fprintf(f, "invert_mouse_wheel=%d\n", invert_mouse_wheel);
	fprintf(f, "volume=%d\n", int(shim::music_volume*255));
	fclose(f);

	Program *tmp_prg = prg;
	prg = nullptr;

	SDL_Delay(100);

	game_lib_destroy_program(tmp_prg);
	standard_lib_destroy_program(tmp_prg);
	destroy_program(tmp_prg);

	if (reset_game_name != "") {
		fn = "";
		goto again;
	}

	end_lib_game();
	end_lib_standard();
	booboo::end();

	::end();

	}
	catch (util::Error &e) {
		gui::fatalerror("ERROR", e.error_message.c_str(), gui::OK, true);
	}
	catch (Error &e) {
		gui::fatalerror("ERROR", e.error_message.c_str(), gui::OK, true);
	}

	chdir(start_cwd.c_str());

	std::string path = save_dir();
	std::string text;
	bool relaunch = false;
	std::string dir;
	try {
		std::string cfg_path = path + "/" + "com.nooskewl.launcher.reload.txt";
		text = util::load_text_from_filesystem(cfg_path);

		util::Tokenizer t(text, '=');
		std::string key = t.next();
		dir = t.next();
		dir = util::trim(dir);
		dir = util::remove_quotes(dir);
		if (key == "launch") {
			relaunch = true;
			remove(cfg_path.c_str());
		}
	}
	catch (util::Error &e) {
	}

	bool beepboop = false;

	for (int i = 0; i < orig_argc; i++) {
		if (std::string(orig_argv[i]) == "+beepboop") {
			beepboop = true;
			break;
		}
	}

	if (beepboop) {
#ifdef __linux__
		pid_t pid = fork();
		if (pid == 0) {
			char * const args[] = {
				orig_argv[0],
				"+dir",
				(char *)set_dir.c_str(),
				nullptr
			};
			execv(argv[0], args);
			exit(0);
		}
#else
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		char cmd[1000];
		snprintf(cmd, 1000, "\"%s\" +dir \"%s\"", orig_argv[0], set_dir.c_str());
		if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		}
#endif
	}
	else if (relaunch) {
#ifdef __linux__
		pid_t pid = fork();
		if (pid == 0) {
			char * const args[] = {
				orig_argv[0],
				(char *)dir.c_str(),
				"+beepboop",
				"+set-dir",
				(char *)dir.c_str(),
				nullptr
			};
			execv(argv[0], args);
			exit(0);
		}
#else
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		char cmd[1000];
		snprintf(cmd, 1000, "\"%s\" \"%s\" +beepboop +set-dir \"%s\"", orig_argv[0], dir.c_str(), dir.c_str());
		if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		}
#endif
	}

	return return_code;
}
