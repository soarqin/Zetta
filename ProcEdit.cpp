#include "pch.h"

#include "ProcEdit.h"

#include "Zetta!.h"

#include <stdio.h>
#include <TlHelp32.h>
#include <Psapi.h>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "psapi.lib")

ProcEdit gProcEdit;

ProcEdit::~ProcEdit() {
    Cleanup();
}

void ProcEdit::Find(const std::string& className) {
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
    version_.assign(ver, ver + lstrlen(ver));
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
    LPVOID buff = malloc(0x200000);
    SIZE_T sread = 0;
    ReadProcessMemory(hProc_, memAddr_, buff, 0x200000, &sread);
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
