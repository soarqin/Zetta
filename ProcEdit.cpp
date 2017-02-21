#include "pch.h"

#include "ProcEdit.h"

#include "Zetta!.h"

#include <stdio.h>
#include <TlHelp32.h>
#include <Psapi.h>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "psapi.lib")

extern "C" {

static uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t crc32(uint32_t crc, const void *buf, size_t size)
{
	const uint8_t *p;
	p = (const uint8_t*)buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}

uint32_t crc_file(const TCHAR *filename) {
	FILE *f = _tfopen(filename, _T("rb"));
	uint32_t c = 0u;
	if (f == NULL) return c;
	while (!feof(f)) {
		char buf[65536];
		int n = fread(buf, 1, 65536, f);
		if (n > 0)
			c = crc32(c, buf, n);
	}
	fclose(f);
	return c;
}

}

ProcEdit gProcEdit;

ProcEdit::~ProcEdit() {
    Cleanup();
}

void ProcEdit::Find(const std::string& className) {
    classname_.clear();
    version_.clear();
    hProc_ = NULL;
    baseAddr_ = NULL;
    memAddr_ = NULL;
    virtAddr_ = NULL;
    baseSize_ = 0;
    crc_ = 0;
    HWND hwnd = FindWindowA(className.c_str(), NULL);
    if (hwnd == NULL) return;
    DWORD procId = 0;
    GetWindowThreadProcessId(hwnd, &procId);
    hProc_ = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

    if (hProc_ == NULL) return;

    TCHAR fname[256];
    GetModuleFileNameEx(hProc_, NULL, fname, 256);
    TCHAR ver[256];
    GetFileVersion(fname, ver);
	crc_ = crc_file(fname);
    version_.assign(ver, ver + lstrlen(ver));
    classname_ = className;
}

int ProcEdit::Check() {
    if (hProc_ == NULL) return 0;
    DWORD exitCode = 0;
    if (GetExitCodeProcess(hProc_, &exitCode) && exitCode == STILL_ACTIVE) return 1;
    Cleanup();
    return -1;
}

void ProcEdit::UpdateAddr(uintptr_t offset) {
    if (hProc_ == NULL) return;
    baseAddr_ = NULL;
    memAddr_ = NULL;
    baseSize_ = 0;
    HANDLE snaphot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hProc_));
    if (snaphot_handle != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 mod_entry;
        mod_entry.dwSize = sizeof(mod_entry);
        if (Module32First(snaphot_handle, &mod_entry)) {
            baseAddr_ = mod_entry.modBaseAddr;
        }
        CloseHandle(snaphot_handle);
    }

    LPBYTE addr = baseAddr_;
    memAddr_ = baseAddr_ + offset;
    MEMORY_BASIC_INFORMATION info;
    while (VirtualQueryEx(hProc_, addr, &info, sizeof(info)) != 0) {
        addr = (BYTE*)info.BaseAddress + info.RegionSize;
        if (info.State != MEM_COMMIT) continue;
        if ((LPBYTE)info.BaseAddress <= baseAddr_ + 0x10000 && addr > baseAddr_ + 0x10000 && info.Protect == 0x20 && info.Type == 0x1000000) {
            baseSize_ = addr - baseAddr_;
            break;
        }
    }
    virtAddr_ = (LPBYTE)VirtualAllocEx(hProc_, 0, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

void ProcEdit::DumpMemory() {
    FILE* f = fopen("dump.bin", "wb");
    LPVOID buff = malloc(0x2000000);
    SIZE_T sread = 0;
    ReadProcessMemory(hProc_, baseAddr_, buff, 0x2000000, &sread);
    fwrite(buff, 1, sread, f);
    fclose(f);
    free(buff);

    f = fopen("dump.txt", "wt");
    fprintf(f, "Base Addr: %08X\n", (uintptr_t)baseAddr_);
    LPBYTE addr = baseAddr_;

    fprintf(f, "ADDRESS       SIZE   PROTECT      TYPE\n");
    MEMORY_BASIC_INFORMATION info;
    while (VirtualQueryEx(hProc_, addr, &info, sizeof(info)) != 0) {
        addr = (BYTE*)info.BaseAddress + info.RegionSize;
        if (info.State != MEM_COMMIT) continue;
        fprintf(f, "%08X  %8X  %8X  %8X\n", (uint32_t)(uintptr_t)info.BaseAddress, info.RegionSize, info.Protect, info.Type);
    }
    fclose(f);
}

inline bool MatchMem(size_t sz, const uint8_t* mem, const uint8_t* search, const uint8_t* mask) {
    while(sz > 0) {
        if (*mask == 0) {
            if (*mem != *search) return false;
        }
        ++mem;
        ++search;
        ++mask;
        --sz;
    }
    return true;
}

void ProcEdit::MakePatch(const std::vector<uint8_t>& search, const std::vector<uint8_t>& searchMask, const std::vector<uint8_t>& patch, const std::vector<uint8_t>& patchMask, size_t skip, size_t poff) {
    if (skip < 5) return;
    BYTE* data = (BYTE*)malloc(baseSize_);
    SIZE_T nread;
    if (!ReadProcessMemory(hProc_, baseAddr_, data, baseSize_, &nread)) {
        free(data);
        return;
    }
    auto ssz = search.size();
    SIZE_T nmax = baseSize_ - ssz;
    auto vaddr = virtAddr_ + poff;
    for (SIZE_T i = 0; i < nmax; ++i) {
        if (MatchMem(ssz, data + i, &search[0], &searchMask[0])) {
            std::vector<uint8_t> orig(data + i, data + i + 5);
            patched_[poff] = std::make_pair(baseAddr_ + i, orig);
            SIZE_T nwrite;
            uint8_t odata[5] = {0xE9, 0, 0, 0, 0};
            uint32_t offset = vaddr - (baseAddr_ + i + 5);
            memcpy(odata + 1, &offset, 4);
            WriteProcessMemory(hProc_, baseAddr_ + i, odata, 5, &nwrite);
            std::vector<uint8_t> vdata;
            if (skip > 5) {
                vdata.resize(skip - 5);
                memset(&vdata[0], 0x90, skip - 5);
                WriteProcessMemory(hProc_, baseAddr_ + i + 5, &vdata[0], skip - 5, &nwrite);
            }
            vdata.resize(patch.size() + 5);
            memcpy(&vdata[0], &patch[0], patch.size());
            for (size_t j = 0; j < patchMask.size(); ++j) {
                if (patchMask[j] > 0) {
                    for (size_t k = 0; k < searchMask.size(); ++k) {
                        if (searchMask[k] == patchMask[j])
                            vdata[j] = data[i + k];
                    }
                }
            }
            vdata[patch.size()] = 0xE9;
            offset = baseAddr_ + i + skip - (vaddr + patch.size() + 5);
            memcpy(&vdata[patch.size() + 1], &offset, 4);
            WriteProcessMemory(hProc_, vaddr, &vdata[0], patch.size() + 5, &nwrite);
            break;
        }
    }
    free(data);
}

void ProcEdit::CancelPatch(size_t poff) {
    auto ite = patched_.find(poff);
    if (ite == patched_.end()) return;
    SIZE_T nwrite;
    WriteProcessMemory(hProc_, ite->second.first, &ite->second.second[0], ite->second.second.size(), &nwrite);
}

void ProcEdit::Cleanup() {
    for (auto& p : patched_) {
        SIZE_T nwrite;
        WriteProcessMemory(hProc_, p.second.first, &p.second.second[0], p.second.second.size(), &nwrite);
    }
    patched_.clear();
    baseAddr_ = NULL;
    memAddr_ = NULL;
    if (virtAddr_ != NULL) {
        VirtualFreeEx(hProc_, virtAddr_, 0, MEM_RELEASE);
        virtAddr_ = NULL;
    }
    baseSize_ = 0;
    version_.clear();
    if (hProc_ != NULL) {
        CloseHandle(hProc_);
        hProc_ = NULL;
    }
}
