#ifndef BOOBOO_CORE_LIB_H
#define BOOBOO_CORE_LIB_H

#include "booboo.h"

void BOOBOO_EXPORT start_lib_standard();
void BOOBOO_EXPORT end_lib_standard();
void BOOBOO_EXPORT standard_lib_destroy_program(booboo::Program *prg);

struct File_Info {
	int file_id;
	std::map<int, SDL_IOStream *> files;
};

struct Config_Value
{
	booboo::Variable::Variable_Type type;

	double n;
	std::string s;
};

struct CFG_Info {
	unsigned int cfg_id;
	std::map<int, std::map<std::string, Config_Value> > cfgs;
};

struct JSON_Info {
	unsigned int json_id;
	std::map<int, util::JSON *> jsons;
};

struct CPA {
	util::CPA *cpa;
};

struct CPA_Info
{
	int cpa_id;
	std::map<int, CPA *> cpas;
};

File_Info BOOBOO_EXPORT *file_info(booboo::Program *prg);
CFG_Info BOOBOO_EXPORT *cfg_info(booboo::Program *prg);
JSON_Info BOOBOO_EXPORT *json_info(booboo::Program *prg);
CPA_Info BOOBOO_EXPORT *cpa_info(booboo::Program *prg);

#endif // BOOBOO_CORE_LIB_H
