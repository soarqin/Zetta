#include "pch.cpp"

#include "IPlugin.h"

#include "../Disgaea1/Unit.h"

#include "skills.inl"

#define IDC_CHARLIST 4001
#define IDC_CHAREDITBASE 4050

class CPluginPanel: public CWindowImpl<CPluginPanel, CStatic> {
public:
//    void OnHotkey(int id, UINT, UINT);
    LRESULT OnCharList(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);

private:
    BEGIN_MSG_MAP_EX(CPluginPanel)
        COMMAND_ID_HANDLER(IDC_CHARLIST, OnCharList)
        COMMAND_RANGE_HANDLER(IDC_CHAREDITBASE, IDC_CHAREDITBASE + 150, OnEdit)
    END_MSG_MAP()
};

class PluginDisgaea1: public IPlugin {
public:
    virtual ~PluginDisgaea1() override {

    }

    virtual void Init(CAppModule* mod) override {
        module_ = mod;
    }

    virtual LPCWSTR GetName() override {
        return L"½ÇÉ«ÐÞ¸Ä";
    }

    virtual void Enable(IProcEdit* proc, void* tc) override {
        if (proc->GetVersion() == "1.0.0.3") {
            enemyCountOff_ = 0x9F57A;
            enemyStartOff_ = 0x1204F4;
            currentUnitOff_ = 0x1AA46C;
            congressOff_ = 0x12066A;
        } else if (proc->GetVersion() == "1.0.0.5") {
            enemyCountOff_ = 0x9F57E;
			switch (proc->GetCRC()) {
			case 0x4E7169ADu: /* U12 */
			case 0xCDC33CBBu: /* U13 */
				enemyStartOff_ = 0x120864;
                congressOff_ = 0x1209DA;
				break;
			case 0xD8AE5E17u: /* U14 */
				enemyStartOff_ = 0x120994;
                congressOff_ = 0x120B0A;
				break;
			default:
				enemyStartOff_ = 0;
                congressOff_ = 0;
				break;
			}
			currentUnitOff_ = 0x1AE474;
        } else return;
        proc_ = proc;
        tabCtrl_ = (HWND)tc;
        CFontHandle fnt;
        fnt.CreatePointFont(90, _T("Arial"), 0);
        CRect trc, rc;
        tabCtrl_.GetItemRect(0, trc);
        tabCtrl_.GetClientRect(rc);
        rc.DeflateRect(8, trc.Height() + 8, 8, 8);
        panel_.Create(tabCtrl_.m_hWnd, rc, 0, WS_CHILD | WS_BORDER);
        LoadChars();
        charlist_.Create(panel_.m_hWnd, CRect(8, 8, 128, 8 + rc.Height()), 0, WS_CHILD | WS_BORDER | WS_VISIBLE | LBS_NOTIFY, 0, IDC_CHARLIST);
        charlist_.SetFont(fnt);
        for (uint16_t i = 0; i < count_; ++i) {
            int index = charlist_.AddString(units_[i].name);
            charlist_.SetItemData(index, (DWORD_PTR)i);
        }
        for (int i = 0; i < 5; ++i) {
            editbox_[i].Create(panel_.m_hWnd, CRect(150, 8 + 30 * i, 250, 27 + 30 * i), 0, WS_CHILD | WS_BORDER | WS_VISIBLE, 0, IDC_CHAREDITBASE + i);
            editbox_[i].SetFont(fnt);
        }
    }

    virtual void Disable() override {
        if (panel_.IsWindow()) {
            panel_.DestroyWindow();
        }
        panel_.m_hWnd = (HWND)0;
        proc_ = nullptr;
        tabCtrl_ = (HWND)0;
        count_ = 0;
        units_.clear();
    }

    virtual void Show() override {
        panel_.ShowWindow(SW_SHOW);
    }

    virtual void Hide() override {
        panel_.ShowWindow(SW_HIDE);
    }

    virtual void Tick() override {
    }

    void OnSelChange() {
        int n = charlist_.GetCurSel();
        if (n < 0) return;
        wchar_t txt[128];
        uint16_t idx = (uint16_t)charlist_.GetItemData(n);
        for (int i = 0; i < 5; ++i) {
            auto ite = skill_names.find(units_[idx].skillID[i]);
            if (ite == skill_names.end()) {
                wsprintf(txt, L"%u", units_[idx].skillID[i]);
                editbox_[i].SetWindowText(txt);
            } else {
                editbox_[i].SetWindowText(ite->second);
            }
        }
        /*
        wchar_t name[32];
        charlist_.GetText(n, name);
        MessageBox(NULL, name, NULL, 0);
        */
    }

private:
    void LoadChars() {
        proc_->Read(true, 0x45B10, &count_, 2);
        units_.resize(count_);
        if (count_ == 0) return;
        proc_->Read(true, 0x1E98, &units_[0], sizeof(UnitInfo) * count_);
    }

private:
    CAppModule* module_ = nullptr;
    IProcEdit* proc_ = nullptr;
    CTabCtrl tabCtrl_;
    CPluginPanel panel_;
    CListBox charlist_;
    CEdit editbox_[100];
    uint16_t count_ = 0;
    std::vector<UnitInfo> units_;
    uintptr_t enemyStartOff_ = 0;
    uintptr_t enemyCountOff_ = 0;
    uintptr_t currentUnitOff_ = 0;
    uintptr_t currentUnit_ = 0;
    uintptr_t congressOff_ = 0;
};

static PluginDisgaea1 gPlugin;

LRESULT CPluginPanel::OnCharList(WORD notifyCode, WORD id, HWND hwnd, BOOL & bHandled) {
    switch (notifyCode) {
    case LBN_SELCHANGE:
        gPlugin.OnSelChange();
        break;
    }
    return 0;
}

LRESULT CPluginPanel::OnEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL & bHandled) {
    return 0;
}

IPlugin* WINAPI GetPlugin() {
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
    return &gPlugin;
}
