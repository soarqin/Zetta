#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <map>

class ProcEdit {
public:
    ~ProcEdit();
    void Find(const std::string& className);
    int Check();
    void UpdateAddr(uintptr_t offset);
    void DumpMemory();
    inline bool Found() {
        return hProc_ != NULL;
    }
    template<typename T>
    inline T Read(bool mem, uintptr_t addr) {
        SIZE_T nread = 0;
        T result = T();
        ReadProcessMemory(hProc_, (mem ? memAddr_ : baseAddr_) + addr, &result, sizeof(T), &nread);
        return result;
    }
    template<typename T>
    inline void Write(bool mem, uintptr_t addr, T value) {
        SIZE_T nwrite = 0;
        WriteProcessMemory(hProc_, (mem ? memAddr_ : baseAddr_) + addr, &value, sizeof(T), &nwrite);
    }
    inline void Write(bool mem, uintptr_t addr, void* buf, size_t count) {
        SIZE_T nwrite = 0;
        WriteProcessMemory(hProc_, (mem ? memAddr_ : baseAddr_) + addr, buf, count, &nwrite);
    }
    inline std::string GetVersion() {
        return version_;
    }
    void MakePatch(const std::vector<uint8_t>& search, const std::vector<uint8_t>& searchMask, const std::vector<uint8_t>& patch, const std::vector<uint8_t>& patchMask, size_t skip, size_t poff);
    void CancelPatch(size_t poff);

private:
    void Cleanup();

private:
    HANDLE hProc_ = NULL;
    LPBYTE baseAddr_ = NULL, memAddr_ = NULL;
    LPBYTE virtAddr_ = NULL;
    SIZE_T baseSize_ = 0, memSize_ = 0;
    std::string version_;
    std::map<size_t, std::pair<BYTE*, std::vector<uint8_t>>> patched_;
};

extern ProcEdit gProcEdit;
