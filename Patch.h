#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <vector>

#ifdef _WIN64
const size_t SKIP_LEN = 14;
#else
const size_t SKIP_LEN = 5;
#endif

enum PatchType {
    PT_U8,
    PT_U16,
    PT_U32,
    PT_U64,
    PT_I8,
    PT_I16,
    PT_I32,
    PT_I64,
	PT_HARDPATCH,
    PT_BYTES,
};

enum PatchPos {
    PP_MEM,
    PP_CODE,
};

struct PatchBytes {
    size_t skip;
    size_t poff;
    std::vector<uint8_t> search;
    std::vector<uint8_t> searchMask;
    std::vector<uint8_t> patch;
    std::vector<uint8_t> patchMask;
	std::vector<uint8_t> post;
	std::vector<uint8_t> postMask;
};

struct PatchAddr {
    std::string name;
	int group;
    PatchType type;
    PatchPos pos;
    uintptr_t offset;
    std::vector<PatchBytes> bytes;
    uint32_t index;
};

struct PatchBlock {
    bool newColumn = false;
    std::string name;
    std::vector<PatchAddr> patches;
};

struct PatchSpec {
    std::wstring name;
    uintptr_t memAddr;
    std::vector<PatchBlock> blocks;
};

struct Patch {
    void Load(const char* filename);
    std::map<std::string, std::map<std::string, PatchSpec>> versions;
};

extern Patch gPatch;
