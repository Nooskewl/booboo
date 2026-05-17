#ifndef BOOBOO_CORE_LIB_H
#define BOOBOO_CORE_LIB_H

#include "booboo.h"

void BOOBOO_EXPORT start_lib_standard();
void BOOBOO_EXPORT end_lib_standard();
void BOOBOO_EXPORT standard_lib_destroy_program(booboo::Program *prg);

/* These are for accessing assets in the black box */

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

struct MML_Info {
	unsigned int mml_id;
	std::map<int, audio::MML *> mmls;
};

struct MML_Instance {
	audio::MML *mml;
	int instance;
};

struct MML_Instance_Info {
	unsigned int instance_id;
	std::map<int, MML_Instance *> instances;
};

struct Sample_Info {
	unsigned int sample_id;
	std::map<int, audio::Sample *> samples;
};

struct Sample_Instance_Info {
	unsigned int instance_id;
	std::map<int, audio::Sample_Instance *> instances;
};

File_Info BOOBOO_EXPORT *file_info(booboo::Program *prg);
CFG_Info BOOBOO_EXPORT *cfg_info(booboo::Program *prg);
JSON_Info BOOBOO_EXPORT *json_info(booboo::Program *prg);
CPA_Info BOOBOO_EXPORT *cpa_info(booboo::Program *prg);
MML_Info BOOBOO_EXPORT *mml_info(booboo::Program *prg);
MML_Instance_Info BOOBOO_EXPORT *mml_instance_info(booboo::Program *prg);
Sample_Info BOOBOO_EXPORT *sample_info(booboo::Program *prg);
Sample_Instance_Info BOOBOO_EXPORT *sample_instance_info(booboo::Program *prg);

#endif // BOOBOO_CORE_LIB_H
