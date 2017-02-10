#include "pch.cpp"

#include "IPlugin.h"

#include "../Disgaea1/Unit.h"

#include "skills.inl"

#include <atlscrl.h>
#include <set>

#define IDC_CHARLIST 4001
#define IDC_RELOAD 4002
#define IDC_SAVE 4003
#define IDC_CHAREDITBASE 4050

struct UnitField {
	const wchar_t* name;
	uint8_t type; // 0-string 1-uint8 2-uint16 3-uint32 4-uint64 5-int8 6-int16 7-int32  +0x10 = padding
	uint32_t offset;
	uint8_t arraysize;
	void* ptr;
	uint32_t row;
};

UnitField unitfields[] = {
	{ L"名字", 0, offsetof(UnitInfo, name), 1, nullptr, 0 },
	{ L"职业", 0, offsetof(UnitInfo, job), 1, nullptr, 0 },
	{ L"RANK", 2, offsetof(UnitInfo, Rank), 1, nullptr, 1 },
	{ L"Mana", 3, offsetof(UnitInfo, Mana), 1, nullptr, 1 },
	{ L"等级", 2, offsetof(UnitInfo, lvl), 1, nullptr, 1 },
	{ L"当前HP", 3, offsetof(UnitInfo, HP), 1, nullptr, 2 },
	{ L"当前SP", 3, offsetof(UnitInfo, SP), 1, nullptr, 2 },
	{ L"经验", 4, offsetof(UnitInfo, exp), 1, nullptr, 2 },
	{ L"最大HP", 3, offsetof(UnitInfo, HPMax), 1, nullptr, 3 },
	{ L"基础HP", 3, offsetof(UnitInfo, HPBase), 1, nullptr, 3 },
	{ L"HP成长", 1, offsetof(UnitInfo, HPGrowth), 1, nullptr, 3 },
	{ L"Jm", 1, offsetof(UnitInfo, Jm), 1, nullptr, 3 },
	{ L"PSN", 2, offsetof(UnitInfo, PSN), 1, nullptr, 3 },
	{ L"最大SP", 3, offsetof(UnitInfo, SPMax), 1, nullptr, 4 },
	{ L"基础SP", 3, offsetof(UnitInfo, SPBase), 1, nullptr, 4 },
	{ L"SP成长", 1, offsetof(UnitInfo, SPGrowth), 1, nullptr, 4 },
	{ L"最大Jm", 1, offsetof(UnitInfo, JmMx), 1, nullptr, 4 },
	{ L"SLP", 2, offsetof(UnitInfo, SLP), 1, nullptr, 4 },
	{ L"ATK", 3, offsetof(UnitInfo, ATK), 1, nullptr, 5 },
	{ L"基础ATK", 3, offsetof(UnitInfo, ATKBase), 1, nullptr, 5 },
	{ L"ATK成长", 1, offsetof(UnitInfo, ATKGrowth), 1, nullptr, 5 },
	{ L"Mv", 1, offsetof(UnitInfo, Mv), 1, nullptr, 5 },
	{ L"PAR", 2, offsetof(UnitInfo, PAR), 1, nullptr, 5 },
	{ L"DEF", 3, offsetof(UnitInfo, DEF), 1, nullptr, 6 },
	{ L"基础DEF", 3, offsetof(UnitInfo, DEFBase), 1, nullptr, 6 },
	{ L"DEF成长", 1, offsetof(UnitInfo, DEFGrowth), 1, nullptr, 6 },
	{ L"最大Mv", 1, offsetof(UnitInfo, MvMx), 1, nullptr, 6 },
	{ L"FGT", 2, offsetof(UnitInfo, FGT), 1, nullptr, 6 },
	{ L"INT", 3, offsetof(UnitInfo, INT), 1, nullptr, 7 },
	{ L"基础INT", 3, offsetof(UnitInfo, INTBase), 1, nullptr, 7 },
	{ L"INT成长", 1, offsetof(UnitInfo, INTGrowth), 1, nullptr, 7 },
	{ L"反击", 1, offsetof(UnitInfo, Counter), 1, nullptr, 7 },
	{ L"DPR", 2, offsetof(UnitInfo, DPR), 1, nullptr, 7 },
	{ L"SPD", 3, offsetof(UnitInfo, SPD), 1, nullptr, 8 },
	{ L"基础SPD", 3, offsetof(UnitInfo, SPDBase), 1, nullptr, 8 },
	{ L"SPD成长", 1, offsetof(UnitInfo, SPDGrowth), 1, nullptr, 8 },
	{ L"最大反击", 1, offsetof(UnitInfo, CountMx), 1, nullptr, 8 },
	{ L"火抗性", 5, offsetof(UnitInfo, FireRes), 1, nullptr, 8 },
	{ L"HIT", 3, offsetof(UnitInfo, HIT), 1, nullptr, 9 },
	{ L"基础HIT", 3, offsetof(UnitInfo, HITBase), 1, nullptr, 9 },
	{ L"HIT成长", 1, offsetof(UnitInfo, HITGrowth), 1, nullptr, 9 },
	{ L"", 0x11, 0, 0, nullptr, 9 },
	{ L"风抗性", 5, offsetof(UnitInfo, WindRes), 1, nullptr, 9 },
	{ L"Res", 3, offsetof(UnitInfo, RES), 1, nullptr, 10 },
	{ L"基础RES", 3, offsetof(UnitInfo, RESBase), 1, nullptr, 10 },
	{ L"RES成长", 1, offsetof(UnitInfo, RESGrowth), 1, nullptr, 10 },
	{ L"", 0x11, 0, 0, nullptr, 10 },
	{ L"冰抗性", 5, offsetof(UnitInfo, IceRes), 1, nullptr, 10 },
	{ L"拳经验", 3, offsetof(UnitInfo, FistExp), 1, nullptr, 11 },
	{ L"拳等级", 1, offsetof(UnitInfo, FistLevel), 1, nullptr, 11 },
	{ L"拳适性", 1, offsetof(UnitInfo, FistClass), 1, nullptr, 11 },
	{ L"剑经验", 3, offsetof(UnitInfo, SwordExp), 1, nullptr, 12 },
	{ L"剑等级", 1, offsetof(UnitInfo, SwordLevel), 1, nullptr, 12 },
	{ L"剑适性", 1, offsetof(UnitInfo, SwordClass), 1, nullptr, 12 },
	{ L"枪经验", 3, offsetof(UnitInfo, SpearExp), 1, nullptr, 13 },
	{ L"枪等级", 1, offsetof(UnitInfo, SpearLevel), 1, nullptr, 13 },
	{ L"枪适性", 1, offsetof(UnitInfo, SpearClass), 1, nullptr, 13 },
	{ L"弓经验", 3, offsetof(UnitInfo, BowExp), 1, nullptr, 14 },
	{ L"弓等级", 1, offsetof(UnitInfo, BowLevel), 1, nullptr, 14 },
	{ L"弓适性", 1, offsetof(UnitInfo, BowClass), 1, nullptr, 14 },
	{ L"铳经验", 3, offsetof(UnitInfo, GunExp), 1, nullptr, 15 },
	{ L"铳等级", 1, offsetof(UnitInfo, GunLevel), 1, nullptr, 15 },
	{ L"铳适性", 1, offsetof(UnitInfo, GunClass), 1, nullptr, 15 },
	{ L"斧经验", 3, offsetof(UnitInfo, AxeExp), 1, nullptr, 16 },
	{ L"斧等级", 1, offsetof(UnitInfo, AxeLevel), 1, nullptr, 16 },
	{ L"斧适性", 1, offsetof(UnitInfo, AxeClass), 1, nullptr, 16 },
	{ L"杖经验", 3, offsetof(UnitInfo, StaffExp), 1, nullptr, 17 },
	{ L"杖等级", 1, offsetof(UnitInfo, StaffLevel), 1, nullptr, 17 },
	{ L"杖适性", 1, offsetof(UnitInfo, StaffClass), 1, nullptr, 17 },
// 	{ L"技能ID", 10, offsetof(UnitInfo, skillID), 0x60, &skill_names },
// 	{ L"等级", 1, offsetof(UnitInfo, skillLevel), 0x60, nullptr },
// 	{ L"经验", 3, offsetof(UnitInfo, skillExp), 0x60, nullptr },
	{ L"", 0xFF }
};

class CPluginPanel: public CWindowImpl<CPluginPanel, CStatic> {
public:
    LRESULT OnCharList(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnReload(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnSave(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);

private:
    BEGIN_MSG_MAP_EX(CPluginPanel)
        COMMAND_HANDLER(IDC_CHARLIST, LBN_SELCHANGE, OnCharList)
        COMMAND_HANDLER(IDC_RELOAD, BN_CLICKED, OnReload)
        COMMAND_HANDLER(IDC_SAVE, BN_CLICKED, OnSave)
    END_MSG_MAP()
};

class CPluginPanel2 : public CWindowImpl<CPluginPanel2, CStatic> {
public:
    LRESULT OnEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);

private:
    BEGIN_MSG_MAP_EX(CPluginPanel2)
        COMMAND_RANGE_CODE_HANDLER(IDC_CHAREDITBASE, IDC_CHAREDITBASE + 150, EN_CHANGE, OnEdit)
    END_MSG_MAP()
};

class PluginDisgaea1E: public IPlugin {
public:
    virtual ~PluginDisgaea1E() override {

    }

    virtual uint32_t Init(CAppModule* mod) override {
        module_ = mod;
        // return 2;
        return 1;
    }

    virtual LPCWSTR GetName(uint32_t page) override {
        const wchar_t* names[2] = { L"角色修改", L"技能修改" };
        return names[page];
    }

    virtual bool Enable(IProcEdit* proc, void* tc) override {
        if (proc->ClassName() != "DISGAEA") return false;
        proc_ = proc;
        tabCtrl_ = (HWND)tc;
        CFontHandle fnt;
        fnt.CreatePointFont(90, _T("Arial"), 0);
        CRect trc, rc;
        tabCtrl_.GetItemRect(0, trc);
        tabCtrl_.GetClientRect(rc);
        rc.DeflateRect(8, trc.Height() + 8, 8, 8);
        panell_.Create(tabCtrl_.m_hWnd, CRect(rc.left, rc.top, rc.left + 140, rc.bottom), 0, WS_CHILD);
        CRect src(rc.left + 140, rc.top, rc.right, rc.bottom);
        spanel_[0].Create(tabCtrl_.m_hWnd, src, 0, WS_CHILD | WS_BORDER);
        spanel_[1].Create(tabCtrl_.m_hWnd, src, 0, WS_CHILD | WS_BORDER);
        spanel_[0].GetClientRect(src);
        src.right -= 30;
        panel_[0].Create(spanel_[0].m_hWnd, src, 0, WS_CHILD | WS_VISIBLE);
        panel_[1].Create(spanel_[1].m_hWnd, src, 0, WS_CHILD | WS_VISIBLE);

        charlist_.Create(panell_.m_hWnd, CRect(8, 0, 128, rc.Height() - 12), 0, WS_CHILD | WS_BORDER | WS_VISIBLE | LBS_NOTIFY, 0, IDC_CHARLIST);
        charlist_.SetFont(fnt, false);
        CButton btn[2];
        btn[0].Create(panell_.m_hWnd, CRect(8, rc.Height() - 22, 62, rc.Height()), L"刷新", WS_CHILD | WS_BORDER | WS_VISIBLE, 0, IDC_RELOAD);
        btn[1].Create(panell_.m_hWnd, CRect(72, rc.Height() - 22, 128, rc.Height()), L"写入", WS_CHILD | WS_BORDER | WS_VISIBLE, 0, IDC_SAVE);
        btn[0].SetFont(fnt, false);
        btn[1].SetFont(fnt, false);
        LoadChars();
		uint32_t count = 0;
		uint32_t position = 0;
		uint32_t row = 0;
		for (auto* u = unitfields; u->type != 0xFF; ++u) {
			if (u->row != row) {
				position = 0; row = u->row;
			}
			if (!(u->type & 0x10)) {
				editname_[count].Create(spanel_[0].m_hWnd, CRect(3 + position, 4 + 23 * row, 58 + position, 22 + 23 * row), u->name, WS_CHILD | WS_VISIBLE | SS_RIGHT);
				editname_[count].SetFont(fnt, false);
			}
			uint32_t width = 0;
            UINT extraFlag = ES_NUMBER | ES_NOIME;
            switch (u->type & 0x07) {
			case 0:
				width = 120;
                extraFlag = 0;
				break;
			case 1:
			case 5:
				width = 30;
				break;
			case 2:
			case 6:
				width = 60;
				break;
			case 3:
			case 7:
				width = 60;
				break;
			case 4:
				width = 120;
				break;
			}
			if (!(u->type & 0x10)) {
				editbox_[count].Create(panel_[0].m_hWnd, CRect(60 + position, 2 + 23 * row, 60 + width + position, 23 + 23 * row), 0, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_AUTOHSCROLL | WS_DISABLED | extraFlag, 0, IDC_CHAREDITBASE + count);
				editbox_[count].SetFont(fnt, false);
                editbox_[count].SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)u);
			}
			position += 60 + width;
			++count;
		}
        panel_[0].GetWindowRect(src);
        src.bottom = 26 + 23 * row + src.top;
        panel_[0].MoveWindow(src);
        spanel_[0].SetClient(panel_[0]);
        spanel_[1].SetClient(panel_[1]);
        spanel_[0].ShowWindow(SW_HIDE);
        spanel_[1].ShowWindow(SW_HIDE);
        /*
        for (int i = 0; i < 5; ++i) {
            editbox_[i].Create(panel_.m_hWnd, CRect(150, 8 + 30 * i, 250, 27 + 30 * i), 0, WS_CHILD | WS_BORDER | WS_VISIBLE, 0, IDC_CHAREDITBASE + i);
            editbox_[i].SetFont(fnt);
        }
		*/
		return true;
    }

    virtual void Disable() override {
        if (panell_.IsWindow()) {
            panell_.DestroyWindow();
        }
        panell_.m_hWnd = (HWND)0;
        for (uint32_t i = 0; i < 2; ++i) {
            if (spanel_[i].IsWindow()) {
                spanel_[i].DestroyWindow();
            }
            spanel_[i].m_hWnd = (HWND)0;
        }
        proc_ = nullptr;
        tabCtrl_ = (HWND)0;
        count_ = 0;
        units_.clear();
    }

    virtual void Show(uint32_t page) override {
        panell_.ShowWindow(SW_SHOW);
        spanel_[page].ShowWindow(SW_SHOW);
    }

    virtual void Hide(uint32_t page) override {
        panell_.ShowWindow(SW_HIDE);
        spanel_[page].ShowWindow(SW_HIDE);
    }

    virtual void Tick(uint32_t page) override {
    }

    void OnSelChange() {
        changing_ = true;
        dirty_.clear();
        int n = charlist_.GetCurSel();
		if (n < 0) {
			uint32_t count = 0;
			for (auto* u = unitfields; u->type != 0xFF; ++u) {
                if (!(u->type & 0x10)) {
                    editbox_[count].EnableWindow(FALSE);
                    editbox_[count].Clear();
                }
				++count;
			}
            changing_ = false;
            return;
		}
		uint32_t count = 0;
		wchar_t txt[128];
		for (auto* u = unitfields; u->type != 0xFF; ++u) {
			if (!(u->type & 0x10)) {
				editbox_[count].EnableWindow(TRUE);
				uint8_t* ptr = (uint8_t*)&units_[n] + u->offset;
				switch (u->type & 0x07) {
				case 0: {
					wchar_t name[32];
					lstrcpy(name, (wchar_t*)ptr);
					editbox_[count].SetWindowText(name);
					break;
				}
				case 1:
					wsprintf(txt, L"%u", *ptr);
					editbox_[count].SetWindowText(txt);
					break;
				case 2:
					wsprintf(txt, L"%u", *(uint16_t*)ptr);
					editbox_[count].SetWindowText(txt);
					break;
				case 3:
					wsprintf(txt, L"%u", *(uint32_t*)ptr);
					editbox_[count].SetWindowText(txt);
					break;
				case 4:
					wsprintf(txt, L"%I64u", *(uint64_t*)ptr);
					editbox_[count].SetWindowText(txt);
					break;
				case 5:
					wsprintf(txt, L"%d", *(int8_t*)ptr);
					editbox_[count].SetWindowText(txt);
					break;
				case 6:
					wsprintf(txt, L"%d", *(int16_t*)ptr);
					editbox_[count].SetWindowText(txt);
					break;
				case 7:
					wsprintf(txt, L"%d", *(int32_t*)ptr);
					editbox_[count].SetWindowText(txt);
					break;
				}
			}
			++count;
		}
        changing_ = false;
        /*
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
		*/
        /*
        wchar_t name[32];
        charlist_.GetText(n, name);
        MessageBox(NULL, name, NULL, 0);
        */
    }

    void OnReload() {
        int n = charlist_.GetCurSel();
        wchar_t name[256];
        wchar_t newname[256];
        charlist_.GetText(n, name);
        LoadChars();
        bool check_name = false;
        int newsize = (int)units_.size();
        if (n >= newsize) {
            n = newsize - 1;
            check_name = true;
        } else {
            charlist_.GetText(n, newname);
            if (lstrcmp(newname, name) != 0)
                check_name = true;
        }
        for (int i = 0; i < newsize; ++i) {
            charlist_.GetText(n, newname);
            if (lstrcmp(newname, name) != 0) {
                n = i;
                break;
            }
        }
        charlist_.SetCurSel(n);
        OnSelChange();
    }

    void OnSave() {
        int sel = charlist_.GetCurSel();
        if (sel < 0) return;
        uint16_t idx = (uint16_t)charlist_.GetItemData(sel);
        if (idx >= (uint16_t)units_.size()) return;
        for (auto& n : dirty_) {
            auto* uf = (UnitField*)editbox_[n].GetWindowLongPtr(GWLP_USERDATA);
            if (!(uf->type & 0x10)) {
                wchar_t name[256];
                uint8_t* ptr = (uint8_t*)&units_[idx] + uf->offset;
                switch (uf->type & 0x07) {
                case 0:
                    editbox_[n].GetWindowText(name, 256);
                    lstrcpy((wchar_t*)ptr, name);
                    break;
                case 1:
                    editbox_[n].GetWindowText(name, 256);
                    *(uint8_t*)ptr = (uint8_t)wcstoul(name, nullptr, 10);
                    break;
                case 2:
                    editbox_[n].GetWindowText(name, 256);
                    *(uint16_t*)ptr = (uint16_t)wcstoul(name, nullptr, 10);
                    break;
                case 3:
                    editbox_[n].GetWindowText(name, 256);
                    *(uint32_t*)ptr = (uint32_t)wcstoul(name, nullptr, 10);
                    break;
                case 4:
                    editbox_[n].GetWindowText(name, 256);
                    *(uint64_t*)ptr = (uint64_t)wcstoull(name, nullptr, 10);
                    break;
                case 5:
                    editbox_[n].GetWindowText(name, 256);
                    *(int8_t*)ptr = (int8_t)wcstol(name, nullptr, 10);
                    break;
                case 6:
                    editbox_[n].GetWindowText(name, 256);
                    *(int16_t*)ptr = (int16_t)wcstol(name, nullptr, 10);
                    break;
                case 7:
                    editbox_[n].GetWindowText(name, 256);
                    *(int32_t*)ptr = (int32_t)wcstol(name, nullptr, 10);
                    break;
                }
            }
        }
        dirty_.clear();
        proc_->Write(true, 0x1E98 + sizeof(UnitInfo) * idx, &units_[idx], sizeof(UnitInfo));
    }

    void OnChanged(uint32_t n) {
        if (changing_) return;
        dirty_.insert(n);
    }

private:
    void LoadChars() {
        proc_->Read(true, 0x45B10, &count_, 2);
        units_.resize(count_);
        if (count_ == 0) return;
        proc_->Read(true, 0x1E98, &units_[0], sizeof(UnitInfo) * count_);
        int n = charlist_.GetCount();
        while (--n >= 0)
            charlist_.DeleteString(n);
        for (uint16_t i = 0; i < count_; ++i) {
            int index = charlist_.AddString(units_[i].name);
            charlist_.SetItemData(index, (DWORD_PTR)i);
        }
    }

private:
    CAppModule* module_ = nullptr;
    IProcEdit* proc_ = nullptr;
    CTabCtrl tabCtrl_;
    CPluginPanel panell_;
    CScrollContainer spanel_[2];
    CPluginPanel2 panel_[2];
    CListBox charlist_;
	CStatic editname_[100];
    CEdit editbox_[100];
    uint16_t count_ = 0;
    std::vector<UnitInfo> units_;
    std::set<uint32_t> dirty_;
    bool changing_ = false;
};

static PluginDisgaea1E gPlugin;

LRESULT CPluginPanel::OnCharList(WORD notifyCode, WORD id, HWND hwnd, BOOL & bHandled) {
    gPlugin.OnSelChange();
    return 0;
}

LRESULT CPluginPanel::OnReload(WORD notifyCode, WORD id, HWND hwnd, BOOL & bHandled) {
    gPlugin.OnReload();
    return 0;
}

LRESULT CPluginPanel::OnSave(WORD notifyCode, WORD id, HWND hwnd, BOOL & bHandled) {
    gPlugin.OnSave();
    return 0;
}

LRESULT CPluginPanel2::OnEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL & bHandled) {
    gPlugin.OnChanged(id - IDC_CHAREDITBASE);
    return 0;
}

IPlugin* WINAPI GetPlugin() {
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
    return &gPlugin;
}
