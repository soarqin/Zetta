#pragma once

#include "IPlugin.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <map>

class ProcEdit: public IProcEdit {
public:
    virtual ~ProcEdit() override;
    void Find(const std::string& className);
    int Check();
    void UpdateAddr(uintptr_t offset);
    void DumpMemory();
    inline bool Found() {
        return hProc_ != NULL;
    }
    template<typename T>
    inline T Read(bool mem, uintptr_t addr) {
        T result = T();
        Read(mem, addr, &result, sizeof(T));
        return result;
    }
    template<typename T>
    inline void Write(bool mem, uintptr_t addr, T value) {
        Write(mem, addr, &value, sizeof(T));
    }
    virtual void Read(bool mem, uintptr_t addr, void* buf, size_t count) override {
        SIZE_T nread = 0;
        ReadProcessMemory(hProc_, (mem ? memAddr_ : baseAddr_) + addr, buf, count, &nread);
    }
    virtual void Write(bool mem, uintptr_t addr, const void* buf, size_t count) override {
        SIZE_T nwrite = 0;
        WriteProcessMemory(hProc_, (mem ? memAddr_ : baseAddr_) + addr, buf, count, &nwrite);
    }
    virtual void ReadRaw(uintptr_t addr, void* buf, size_t count) override {
        SIZE_T nread = 0;
        ReadProcessMemory(hProc_, (void*)addr, buf, count, &nread);
    }
    virtual void WriteRaw(uintptr_t addr, const void* buf, size_t count) override {
        SIZE_T nwrite = 0;
        WriteProcessMemory(hProc_, (void*)addr, buf, count, &nwrite);
    }
    virtual const std::string& ClassName() override {
        return classname_;
    }
    virtual const std::string& GetVersion() override {
        return version_;
    }
	virtual uint32_t GetCRC() override {
		return crc_;
	}
    void MakePatch(const std::vector<uint8_t>& search, const std::vector<uint8_t>& searchMask, const std::vector<uint8_t>& patch, const std::vector<uint8_t>& patchMask, size_t skip, size_t poff);
    void CancelPatch(size_t poff);

private:
    void Cleanup();

private:
    HANDLE hProc_ = NULL;
    LPBYTE baseAddr_ = NULL, memAddr_ = NULL;
    LPBYTE virtAddr_ = NULL;
    SIZE_T baseSize_ = 0;
    std::string classname_;
    std::string version_;
	uint32_t crc_;
    std::map<size_t, std::pair<BYTE*, std::vector<uint8_t>>> patched_;
};

extern ProcEdit gProcEdit;
