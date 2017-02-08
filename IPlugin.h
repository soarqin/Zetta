#pragma once

#include <string>

namespace WTL {
class CAppModule;
}

class IProcEdit {
public:
    virtual ~IProcEdit() {}

    virtual void Read(bool mem, uintptr_t addr, void* buf, size_t count) = 0;
    virtual void Write(bool mem, uintptr_t addr, const void* buf, size_t count) = 0;
    virtual void ReadRaw(uintptr_t addr, void* buf, size_t count) = 0;
    virtual void WriteRaw(uintptr_t addr, const void* buf, size_t count) = 0;
    virtual const std::string& GetVersion() = 0;
	virtual uint32_t GetCRC() = 0;
};

class IPlugin {
public:
    virtual ~IPlugin() {}

    virtual void Init(WTL::CAppModule*) = 0;
    virtual LPCWSTR GetName() = 0;
    virtual bool Enable(IProcEdit*, void*) = 0;
    virtual void Disable() = 0;
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Tick() = 0;
};

typedef IPlugin*(WINAPI* PLUGINLOADPROC)();
