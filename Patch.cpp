#include "pch.h"

#include "Patch.h"

#include <rapidjson/filereadstream.h>
#include <rapidjson/Document.h>

uint64_t ParseUInt64(const char* str) {
    if (str[0] == '0') {
        if (str[1] == 'x' || str[1] == 'X')
            return strtoull(str + 2, nullptr, 16);
        return strtoull(str + 1, nullptr, 8);
    }
    return strtoull(str, nullptr, 10);
}

void ParseBytes(const char* str, std::vector<uint8_t>& result, std::vector<uint8_t>& mask) {
    const char* v = str;
    while (*v == ' ') ++v;
    while (*v != 0) {
        char* end;
        if (v[0] == 'X') {
            result.push_back(0);
            mask.push_back(1+(uint8_t)strtoul(v + 1, &end, 16));
        } else {
            result.push_back((uint8_t)strtoul(v, &end, 16));
            mask.push_back(0);
        }
        v = end;
        while (*v == ' ') ++v;
    }
}

void Patch::Load(const char* filename) {
    using namespace rapidjson;
    FILE* fp = fopen(filename, "rb");
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document d;
    d.ParseStream(is);
    for (auto d1 = d.MemberBegin(); d1 != d.MemberEnd(); ++d1) {
        std::string className = d1->name.GetString();
        auto& ver = versions[className];
        auto vnode = d1->value.GetObject();
        uint32_t index = 0;
        size_t sz = 0, hardoff = 0xFFFFFFFF;
        for (auto m1 = vnode.MemberBegin(); m1 != vnode.MemberEnd(); ++m1) {
            auto& spec = ver[m1->name.GetString()];
            const char* nm = m1->value["Name"].GetString();
            wchar_t wnm[256];
            MultiByteToWideChar(CP_UTF8, 0, nm, -1, wnm, 256);
            spec.name = wnm;
            spec.memAddr = (uintptr_t)ParseUInt64(m1->value["MemAddr"].GetString());
            auto mnode = m1->value["Offset"].GetObject();
            for (auto m2 = mnode.MemberBegin(); m2 != mnode.MemberEnd(); ++m2) {
                spec.blocks.resize(spec.blocks.size() + 1);
                auto& block = spec.blocks.back();
                block.name = m2->name.GetString();
                if (block.name[0] == '+') {
                    block.newColumn = true;
                    block.name.erase(0, 1);
                }
                for (auto m3 = m2->value.Begin(); m3 != m2->value.End(); ++m3) {
                    block.patches.resize(block.patches.size() + 1);
                    auto& paddr = block.patches.back();
                    paddr.index = index++;
                    paddr.name = (*m3)["name"].GetString();
                    std::string tstr = (*m3)["type"].GetString();
					if (tstr == "u8")
						paddr.type = PT_U8;
					else if (tstr == "u16")
						paddr.type = PT_U16;
					else if (tstr == "u32")
						paddr.type = PT_U32;
					else if (tstr == "u64")
						paddr.type = PT_U64;
					else if (tstr == "i8")
						paddr.type = PT_U8;
					else if (tstr == "i16")
						paddr.type = PT_U16;
					else if (tstr == "i32")
						paddr.type = PT_U32;
					else if (tstr == "i64")
						paddr.type = PT_U64;
					else if (tstr == "hard")
						paddr.type = PT_HARDPATCH;
					else
                        paddr.type = PT_BYTES;
                    tstr = (*m3)["block"].GetString();
                    if (tstr == "mem")
                        paddr.pos = PP_MEM;
                    else
                        paddr.pos = PP_CODE;
					if (paddr.type == PT_HARDPATCH) {
						auto oobj = (*m3)["offset"].GetArray();
						for (auto m4 = oobj.Begin(); m4 != oobj.End(); ++m4) {
							PatchBytes pb = { (size_t)(*m4)["skip"].GetInt(), hardoff-- };
							ParseBytes((*m4)["patch"].GetString(), pb.patch, pb.patchMask);
							ParseBytes((*m4)["search"].GetString(), pb.search, pb.searchMask);
							paddr.bytes.push_back(pb);
						}
					} else if (paddr.type == PT_BYTES) {
                        auto oobj = (*m3)["offset"].GetArray();
                        for (auto m4 = oobj.Begin(); m4 != oobj.End(); ++m4) {
                            PatchBytes pb = { (size_t)(*m4)["skip"].GetInt(), sz };
                            ParseBytes((*m4)["patch"].GetString(), pb.patch, pb.patchMask);
                            ParseBytes((*m4)["search"].GetString(), pb.search, pb.searchMask);
                            paddr.bytes.push_back(pb);
                            sz = (sz + pb.patch.size() + 5 + 0x0FUL) & ~0x0FUL;
                        }
                    }
                    else
                        paddr.offset = (uintptr_t)ParseUInt64((*m3)["offset"].GetString());
                }
            }
        }
    }
    fclose(fp);
}

Patch gPatch;
