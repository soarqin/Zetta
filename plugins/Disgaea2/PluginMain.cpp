#include "pch.cpp"

#include "IPlugin.h"

#include "Unit.h"

#define IDH_F1 1900
#define IDH_F2 1901
#define IDH_F3 1902
#define IDH_F4 1903
#define IDH_F5 1904
#define IDH_F6 1905
#define IDH_F7 1906
#define IDH_F8 1907
#define IDH_F9 1908
#define IDC_ALLJOBS 1899

const uint16_t summons[] = {
    1010, 1020, 1030, 1040, 1050, 1060, 1070, 1090, 1100,
    1110, 1120, 1130, 1140, 1150, 1160, 1170, 1180, 1190, 1200,
    2010, 2030, 2040, 2050, 2060, 2070, 2090,
    2120, 2130, 2140, 2150, 2160, 2170, 2180, 2190, 2200,
    2210, 2220, 2230, 2240
};

class CPluginPanel: public CWindowImpl<CPluginPanel, CStatic> {
public:
    void OnHotkey(int id, UINT, UINT);
    LRESULT OnAllJobs(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);

private:
    BEGIN_MSG_MAP_EX(CPluginPanel)
        MSG_WM_HOTKEY(OnHotkey)
        COMMAND_HANDLER(IDC_ALLJOBS, BN_CLICKED, OnAllJobs)
    END_MSG_MAP()
};

class PluginDisgaea2: public IPlugin {
public:
    virtual ~PluginDisgaea2() override {

    }

    virtual uint32_t Init(CAppModule* mod) override {
        module_ = mod;
        return 1;
    }

    virtual LPCWSTR GetName(uint32_t page) override {
        return L"高级";
    }

    virtual bool Enable(IProcEdit* proc, void* tc) override {
        if (proc->ClassName() != "Disgaea2 PC") return false;
        if (proc->GetVersion() == "1.0.0.1") {
            selfStartOff_ = 0x2A0738;
            selfCountOff_ = 0x34D974;
            unitStartOff_ = 0x35F620;
            currentUnitOff_ = 0x65AD80;
            congressOff_ = 0x35F938;
            congressCountOff_ = 0x6678F4;
        } else return false;
        proc_ = proc;
        tabCtrl_ = (HWND)tc;
        CFontHandle fnt;
        fnt.CreatePointFont(200, _T("Arial"), 0);
        CRect trc, rc;
        tabCtrl_.GetItemRect(0, trc);
        tabCtrl_.GetClientRect(rc);
        rc.DeflateRect(8, trc.Height() + 8, 8, 8);
        panel_.Create(tabCtrl_.m_hWnd, rc, 0, WS_CHILD | WS_BORDER);
        CStatic lab[8];
        lab[0].Create(panel_.m_hWnd, CRect(8, 58, 8 + rc.Width(), 88), _T("Ctrl+F1 我方HP/SP全满"), WS_VISIBLE | WS_CHILD);
        lab[1].Create(panel_.m_hWnd, CRect(8, 98, 8 + rc.Width(), 128), _T("Ctrl+F2 所有敌人生命为0"), WS_VISIBLE | WS_CHILD);
        lab[2].Create(panel_.m_hWnd, CRect(8, 138, 8 + rc.Width(), 168), _T("Ctrl+F3 当前目标HP/SP回满"), WS_VISIBLE | WS_CHILD);
        lab[3].Create(panel_.m_hWnd, CRect(8, 178, 8 + rc.Width(), 208), _T("Ctrl+F5 当前目标HP为0"), WS_VISIBLE | WS_CHILD);
        lab[4].Create(panel_.m_hWnd, CRect(8, 218, 8 + rc.Width(), 248), _T("Ctrl+F6 当前目标HP为1"), WS_VISIBLE | WS_CHILD);
        lab[5].Create(panel_.m_hWnd, CRect(8, 258, 8 + rc.Width(), 288), _T("Ctrl+F7 议会议员好感为高"), WS_VISIBLE | WS_CHILD);
        lab[6].Create(panel_.m_hWnd, CRect(8, 298, 8 + rc.Width(), 328), _T("Ctrl+F8 道具界下层为中转层"), WS_VISIBLE | WS_CHILD);
        lab[7].Create(panel_.m_hWnd, CRect(8, 338, 8 + rc.Width(), 368), _T("Ctrl+F9 道具界99层"), WS_VISIBLE | WS_CHILD);
        for (int i = 0; i < 8; ++i)
            lab[i].SetFont(fnt);
        CFontHandle fnt2;
        fnt2.CreatePointFont(120, _T("Arial"), 0, true);
        CButton btn1;
        btn1.Create(panel_.m_hWnd, CRect(8, 18, 188, 48), _T("开放全部可创建角色"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, IDC_ALLJOBS);
        btn1.SetFont(fnt2);

        for (int id = IDH_F1; id <= IDH_F9; ++id)
			if (id != IDH_F4)
				RegisterHotKey(panel_.m_hWnd, id, 0x4000 | MOD_CONTROL, VK_F1 + (id - IDH_F1));

		return true;
    }

    virtual void Disable() override {
        if (panel_.IsWindow()) {
            panel_.DestroyWindow();
            for (int id = IDH_F1; id <= IDH_F8; ++id)
                UnregisterHotKey(panel_.m_hWnd, id);
        }
        panel_.m_hWnd = (HWND)0;
        proc_ = nullptr;
        tabCtrl_ = (HWND)0;
        count_ = 0;
        units_.clear();
    }

    virtual void Show(uint32_t page) override {
        panel_.ShowWindow(SW_SHOW);
    }

    virtual void Hide(uint32_t page) override {
        panel_.ShowWindow(SW_HIDE);
    }

    virtual void Tick(uint32_t page) override {
    }

    bool OnF1() {
        uint16_t count;
        proc_->Read(false, selfCountOff_, &count, 2);
        if (count == 0) return false;
        units_.resize(count);
        proc_->Read(false, selfStartOff_, &units_[0], sizeof(UnitInfo) * count);
        for (uint16_t i = 0; i < count; ++i) {
            proc_->Write(false, selfStartOff_ + sizeof(UnitInfo) * i + offsetof(UnitInfo, HP), &units_[i].HPMax, 4);
            proc_->Write(false, selfStartOff_ + sizeof(UnitInfo) * i + offsetof(UnitInfo, SP), &units_[i].SPMax, 4);
        }
        return true;
    }

    bool OnF2() {
        uint32_t soff = unitStartOff_;
        bool run = false;
        while (1) {
            uint32_t tag;
            uint32_t off;
            uint8_t tp;
            proc_->Read(false, soff, &tag, 4);
            if (tag == 0) {
                soff += 0x388;
                continue;
            }
            proc_->Read(false, soff + 8, &off, 4);
            if (off == 0) break;
            proc_->Read(false, soff + 0x33A, &tp, 1);
            if (tp == 0) {
                uint32_t hp = 0;
                run = true;
                proc_->WriteRaw(off + offsetof(UnitInfo, HP), &hp, 4);
            }
            soff += 0x388;
        }
        return run;
    }

    bool OnF3() {
        uint32_t off;
        proc_->Read(false, currentUnitOff_, &off, 4);
        proc_->Read(false, unitStartOff_ + 8 + off * 0x388, &off, 4);
        if (off < 0x200000) return false;
        UnitInfo u;
        proc_->ReadRaw(off, &u, sizeof(UnitInfo));
        proc_->WriteRaw(off + offsetof(UnitInfo, HP), &u.HPMax, 4);
        proc_->WriteRaw(off + offsetof(UnitInfo, SP), &u.SPMax, 4);
        return true;
    }

    bool OnF5() {
        uint32_t off;
        proc_->Read(false, currentUnitOff_, &off, 4);
        proc_->Read(false, unitStartOff_ + 8 + off * 0x388, &off, 4);
        if (off < 0x200000) return false;
        uint32_t hp = 0;
        proc_->WriteRaw(off + offsetof(UnitInfo, HP), &hp, 4);
        return true;
    }

    bool OnF6() {
        uint32_t off;
        proc_->Read(false, currentUnitOff_, &off, 4);
        proc_->Read(false, unitStartOff_ + 8 + off * 0x388, &off, 4);
        if (off < 0x200000) return false;
        uint32_t hp = 1;
        proc_->WriteRaw(off + offsetof(UnitInfo, HP), &hp, 4);
        return true;
    }

    bool OnF7() {
        uint8_t count;
        int16_t n = 120;
        proc_->Read(false, congressCountOff_, &count, 1);
        for (uint8_t i = 0; i < count; ++i) {
            proc_->Write(false, congressOff_ + 0x388 * i, &n, 2);
        }
        return true;
    }

    bool OnF8() {
        uint8_t level;
        proc_->Read(true, 0xAE9AC, &level, 1);
        if (level >= 99) return false;
        if ((level + 1) % 10 == 0) level += 9;
        else level = level / 10 * 10 + 8;
        proc_->Write(true, 0xAE9AC, &level, 1);
        return true;
    }

    bool OnF9() {
        uint8_t level = 98;
        proc_->Write(true, 0xAE9AC, &level, 1);
        return true;
    }

    void OnAllJobs() {
        uint32_t count = 0x120;
        std::vector<uint64_t> jobs;
        std::set<uint16_t> alljobs;
        for (auto& s : summons) {
            for (uint16_t i = s; i < s + 6; ++i)
                alljobs.insert(i);
        }
        jobs.resize(count);
        proc_->Read(false, selfStartOff_ - count * 8, &jobs[0], count * 8);
        for (int i = (int)jobs.size() - 1; i >= 0; --i) {
            uint16_t oj = (uint16_t)(jobs[i] >> 16);
            if (oj != 0) alljobs.erase(oj);
        }
        if (alljobs.empty()) return;
        for (int i = (int)jobs.size() - 1; i >= 0 && !alljobs.empty(); --i) {
            uint16_t oj = (uint16_t)(jobs[i] >> 16);
            if (oj != 0) continue;
            uint16_t nj = *alljobs.begin();
            jobs[i] |= (uint64_t)nj << 16;
            alljobs.erase(alljobs.begin());
        }
        proc_->Write(false, selfStartOff_ - count * 8, &jobs[0], count * 8);
    }

private:
    CAppModule* module_ = nullptr;
    IProcEdit* proc_ = nullptr;
    CTabCtrl tabCtrl_;
    CPluginPanel panel_;
    uint16_t count_ = 0;
    std::vector<UnitInfo> units_;
    uintptr_t unitStartOff_ = 0;
    uintptr_t selfStartOff_ = 0;
    uintptr_t selfCountOff_ = 0;
    uintptr_t currentUnitOff_ = 0;
    uintptr_t congressOff_ = 0;
    uintptr_t congressCountOff_ = 0;
};

static PluginDisgaea2 gPlugin;

void CPluginPanel::OnHotkey(int id, UINT, UINT) {
    switch (id) {
    case IDH_F1:
        if (!gPlugin.OnF1()) return;
        break;
    case IDH_F2:
        if (!gPlugin.OnF2()) return;
        break;
    case IDH_F3:
        if (!gPlugin.OnF3()) return;
        break;
    case IDH_F5:
        if (!gPlugin.OnF5()) return;
        break;
    case IDH_F6:
        if (!gPlugin.OnF6()) return;
        break;
    case IDH_F7:
        if (!gPlugin.OnF7()) return;
        break;
    case IDH_F8:
        if (!gPlugin.OnF8()) return;
        break;
	case IDH_F9:
		if (!gPlugin.OnF9()) return;
		break;
	default:
        return;
    }
    Beep(300, 300);
}

LRESULT CPluginPanel::OnAllJobs(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    gPlugin.OnAllJobs();
    return 0;
}

IPlugin* WINAPI GetPlugin() {
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
    return &gPlugin;
}
