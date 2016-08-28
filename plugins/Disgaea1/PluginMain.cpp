#include "pch.cpp"

#include "IPlugin.h"

class PluginDisgaea1: public IPlugin {
public:
    virtual void Init(WTL::CAppModule*) override {

    }
    virtual LPCWSTR GetName() override {
        return L"Ãÿ ‚";
    }
    virtual void Enable(IProcEdit*, void*) override {

    }
    virtual void Disable() override {

    }
    virtual void Show() override {

    }
    virtual void Hide() override {

    }
};

static PluginDisgaea1 gPlugin;

IPlugin* WINAPI GetPlugin() {
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
    return &gPlugin;
}
