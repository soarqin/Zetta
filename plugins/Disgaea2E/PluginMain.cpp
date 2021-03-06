#include "pch.cpp"

#include "IPlugin.h"

#include "../Disgaea2/Unit.h"
#include "Item.h"

#include "skills.inl"
#include "habits.inl"
#include "mitems.inl"

#include <atlscrl.h>
#include <atldlgs.h>

#define IDC_CHARLIST 4001
#define IDC_RELOAD 4002
#define IDC_SAVE 4003
#define IDC_BTNOK 4004
#define IDC_BTNCANCEL 4005
#define IDC_ITEMLIST 4006
#define IDC_ITEMLIST2 4007
#define IDC_BTNSADD 4010
#define IDC_BTNSDEL 4011
#define IDC_ITEMBTNREFRESH 4012
#define IDC_ITEMBTNEDIT 4013
#define IDC_ITEMBTNDEL 4014
#define IDC_ITEMBTNCLONE 4015
#define IDC_CHAREDITBASE 4050
#define IDC_COMBOSIDBASE 4150
#define IDC_EDITSLVLBASE 4250
#define IDC_EDITSEXPBASE 4350
#define IDC_ITEMEDITBASE 4450

#define item_empty_text (L"- 空 -")

inline void ConvUTF8(const uint8_t* str, wchar_t* out) {
    MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str, -1, out, 256);
}
inline void ConvUCS2(const wchar_t* str, uint8_t* out) {
    WideCharToMultiByte(CP_UTF8, 0, str, -1, (LPSTR)out, 0x3F, NULL, NULL);
}

struct UnitField {
	const wchar_t* name;
	uint8_t type; // 0-string 1-uint8 2-uint16 3-uint32 4-uint64 5-int8 6-int16 7-int32  +0x10 = padding
	uint32_t offset;
	uintptr_t ptr;
	uint32_t row;
    uintptr_t cptr;
    std::function<void(UnitField*, void*)> writecb;
};

UnitField unitfields[] = {
	{ L"名字", 0, offsetof(UnitInfo, name), 0, 0 },
	{ L"职业", 0, offsetof(UnitInfo, job), 0, 0 },
	{ L"Mana", 3, offsetof(UnitInfo, Mana), 0, 1 },
	{ L"等级", 2, offsetof(UnitInfo, lvl), 0, 1 },
	{ L"经验", 4, offsetof(UnitInfo, exp), 0, 1 },
	{ L"当前HP", 3, offsetof(UnitInfo, HP), 0, 2 },
	{ L"当前SP", 3, offsetof(UnitInfo, SP), 0, 2 },
	{ L"前科", 1, offsetof(UnitInfo, Crime), 0, 2 },
	{ L"累计前科", 2, offsetof(UnitInfo, TotalCrime), 0, 2 },
	{ L"最大HP", 3, offsetof(UnitInfo, HPMax), 0, 3 },
	{ L"基础HP", 3, offsetof(UnitInfo, HPBase), 0, 3 },
	{ L"HP成长", 1, offsetof(UnitInfo, HPGrowth), 0, 3 },
	{ L"Jm", 1, offsetof(UnitInfo, Jm), 0, 3 },
	{ L"最大SP", 3, offsetof(UnitInfo, SPMax), 0, 4 },
	{ L"基础SP", 3, offsetof(UnitInfo, SPBase), 0, 4 },
	{ L"SP成长", 1, offsetof(UnitInfo, SPGrowth), 0, 4 },
	{ L"最大Jm", 1, offsetof(UnitInfo, JmMx), 0, 4 },
	{ L"ATK", 3, offsetof(UnitInfo, ATK), 0, 5 },
	{ L"基础ATK", 3, offsetof(UnitInfo, ATKBase), 0, 5 },
	{ L"ATK成长", 1, offsetof(UnitInfo, ATKGrowth), 0, 5 },
	{ L"Mv", 1, offsetof(UnitInfo, Mv), 0, 5 },
	{ L"DEF", 3, offsetof(UnitInfo, DEF), 0, 6 },
	{ L"基础DEF", 3, offsetof(UnitInfo, DEFBase), 0, 6 },
	{ L"DEF成长", 1, offsetof(UnitInfo, DEFGrowth), 0, 6 },
	{ L"最大Mv", 1, offsetof(UnitInfo, MvMx), 0, 6 },
	{ L"INT", 3, offsetof(UnitInfo, INT), 0, 7 },
	{ L"基础INT", 3, offsetof(UnitInfo, INTBase), 0, 7 },
	{ L"INT成长", 1, offsetof(UnitInfo, INTGrowth), 0, 7 },
	{ L"反击", 1, offsetof(UnitInfo, Counter), 0, 7 },
	{ L"SPD", 3, offsetof(UnitInfo, SPD), 0, 8 },
	{ L"基础SPD", 3, offsetof(UnitInfo, SPDBase), 0, 8 },
	{ L"SPD成长", 1, offsetof(UnitInfo, SPDGrowth), 0, 8 },
	{ L"最大反击", 1, offsetof(UnitInfo, CountMx), 0, 8 },
	{ L"HIT", 3, offsetof(UnitInfo, HIT), 0, 9 },
	{ L"基础HIT", 3, offsetof(UnitInfo, HITBase), 0, 9 },
	{ L"HIT成长", 1, offsetof(UnitInfo, HITGrowth), 0, 9 },
	{ L"Res", 3, offsetof(UnitInfo, RES), 0, 10 },
	{ L"基础RES", 3, offsetof(UnitInfo, RESBase), 0, 10 },
	{ L"RES成长", 1, offsetof(UnitInfo, RESGrowth), 0, 10 },
	{ L"拳经验", 3, offsetof(UnitInfo, FistExp), 0, 11 },
	{ L"拳等级", 1, offsetof(UnitInfo, FistLevel), 0, 11 },
	{ L"拳适性", 1, offsetof(UnitInfo, FistClassBase), 0, 11 },
	{ L"剑经验", 3, offsetof(UnitInfo, SwordExp), 0, 12 },
	{ L"剑等级", 1, offsetof(UnitInfo, SwordLevel), 0, 12 },
	{ L"剑适性", 1, offsetof(UnitInfo, SwordClassBase), 0, 12 },
	{ L"矛经验", 3, offsetof(UnitInfo, SpearExp), 0, 13 },
	{ L"矛等级", 1, offsetof(UnitInfo, SpearLevel), 0, 13 },
	{ L"矛适性", 1, offsetof(UnitInfo, SpearClassBase), 0, 13 },
	{ L"弓经验", 3, offsetof(UnitInfo, BowExp), 0, 14 },
	{ L"弓等级", 1, offsetof(UnitInfo, BowLevel), 0, 14 },
	{ L"弓适性", 1, offsetof(UnitInfo, BowClassBase), 0, 14 },
	{ L"枪经验", 3, offsetof(UnitInfo, GunExp), 0, 15 },
	{ L"枪等级", 1, offsetof(UnitInfo, GunLevel), 0, 15 },
	{ L"枪适性", 1, offsetof(UnitInfo, GunClassBase), 0, 15 },
	{ L"斧经验", 3, offsetof(UnitInfo, AxeExp), 0, 16 },
	{ L"斧等级", 1, offsetof(UnitInfo, AxeLevel), 0, 16 },
	{ L"斧适性", 1, offsetof(UnitInfo, AxeClassBase), 0, 16 },
	{ L"杖经验", 3, offsetof(UnitInfo, StaffExp), 0, 17 },
	{ L"杖等级", 1, offsetof(UnitInfo, StaffLevel), 0, 17 },
	{ L"杖适性", 1, offsetof(UnitInfo, StaffClassBase), 0, 17 },
// 	{ L"技能ID", 10, offsetof(UnitInfo, skillID), 0x60, &skill_names },
// 	{ L"等级", 1, offsetof(UnitInfo, skillLevel), 0x60, nullptr },
// 	{ L"经验", 3, offsetof(UnitInfo, skillExp), 0x60, nullptr },
	{ L"", 0xFF }
};

static std::map<uint16_t, int> mitem_map, elite_map, habit_map;
const std::map<uint16_t, const wchar_t*> elite_types = {
    {0, L"普通"},
    {1, L"稀有"},
    {2, L"精英"},
};

UnitField itemfields[] = {
    { L"名称", 0, offsetof(ItemInfo, name), 0, 0 },
    { L"型号", 9, offsetof(ItemInfo, id), (uintptr_t)&mitem_names, 0, (uintptr_t)&mitem_map },
    { L"等级", 1, offsetof(ItemInfo, level), 0, 0 },
    // { L"经验", 4, offsetof(ItemInfo, exp), 0, 0 },
    { L"类型", 1, offsetof(ItemInfo, type), 0, 1 },
    { L"图标", 1, offsetof(ItemInfo, icon), 0, 1 },
    { L"稀有度", 8, offsetof(ItemInfo, elite), (uintptr_t)&elite_types, 1, (uintptr_t)&elite_map, [](UnitField* uf, void* ptr) {
        auto* ii = (ItemInfo*)ptr;
        switch (ii->elite) {
        case 0:
            ii->dungeonMax = 29;
            break;
        case 1:
            ii->dungeonMax = 59;
            break;
        case 2:
            ii->dungeonMax = 99;
            break;
        }
    }},
    { L"RARITY", 1, offsetof(ItemInfo, rarity), 0, 1 },
    { L"MV", 1, offsetof(ItemInfo, mv), 0, 2 },
    { L"JM", 1, offsetof(ItemInfo, jm), 0, 2 },
    { L"当前层数", 1, offsetof(ItemInfo, dungeonCur), 0, 2 },
    { L"最大住人", 1, offsetof(ItemInfo, popMax), 0, 2 },
    { L"HP", 7, offsetof(ItemInfo, hp), 0, 3 },
    { L"基础HP", 7, offsetof(ItemInfo, hpBase), 0, 3 },
    { L"住人1", 9, offsetof(ItemInfo, habits[0]) + 4, (uintptr_t)&habit_names, 3, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[0]), 0, 3 },
    { L"SP", 7, offsetof(ItemInfo, sp), 0, 4 },
    { L"基础SP", 7, offsetof(ItemInfo, spBase), 0, 4 },
    { L"住人2", 9, offsetof(ItemInfo, habits[1]) + 4, (uintptr_t)&habit_names, 4, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[1]), 0, 4 },
    { L"ATK", 7, offsetof(ItemInfo, atk), 0, 5 },
    { L"基础ATK", 7, offsetof(ItemInfo, atkBase), 0, 5 },
    { L"住人3", 9, offsetof(ItemInfo, habits[2]) + 4, (uintptr_t)&habit_names, 5, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[2]), 0, 5 },
    { L"DEF", 7, offsetof(ItemInfo, def), 0, 6 },
    { L"基础DEF", 7, offsetof(ItemInfo, defBase), 0, 6 },
    { L"住人4", 9, offsetof(ItemInfo, habits[3]) + 4, (uintptr_t)&habit_names, 6, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[3]), 0, 6 },
    { L"INT", 7, offsetof(ItemInfo, inte), 0, 7 },
    { L"基础INT", 7, offsetof(ItemInfo, inteBase), 0, 7 },
    { L"住人5", 9, offsetof(ItemInfo, habits[4]) + 4, (uintptr_t)&habit_names, 7, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[4]), 0, 7 },
    { L"SPD", 7, offsetof(ItemInfo, spd), 0, 8 },
    { L"基础SPD", 7, offsetof(ItemInfo, spdBase), 0, 8 },
    { L"住人6", 9, offsetof(ItemInfo, habits[5]) + 4, (uintptr_t)&habit_names, 8, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[5]), 0, 8 },
    { L"HIT", 7, offsetof(ItemInfo, hit), 0, 9 },
    { L"基础HIT", 7, offsetof(ItemInfo, hitBase), 0, 9 },
    { L"住人7", 9, offsetof(ItemInfo, habits[6]) + 4, (uintptr_t)&habit_names, 9, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[6]), 0, 9 },
    { L"RES", 7, offsetof(ItemInfo, res), 0, 10 },
    { L"基础RES", 7, offsetof(ItemInfo, resBase), 0, 10 },
    { L"住人8", 9, offsetof(ItemInfo, habits[7]) + 4, (uintptr_t)&habit_names, 10, (uintptr_t)&habit_map },
    { L"", 3, offsetof(ItemInfo, habits[7]), 0, 10 },
    { L"", 0xFF }
};

class CItemDlg: public CIndirectDialogImpl<CItemDlg> {
public:
    using CIndirectDialogImpl<CItemDlg>::CIndirectDialogImpl;

    void SetItem(const ItemInfo& info) {
        memcpy(&info_, &info, sizeof(info));
    }
    void DoInitTemplate() {
        m_Template.Create(false, L"道具编辑", 0, 0, 0, 0, WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_CENTER, WS_EX_DLGMODALFRAME);
    }
    void DoInitControls() {
    }
    inline const ItemInfo& Info() const { return info_; }

private:
    BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
        fnt_.CreatePointFont(90, _T("Arial"), 0);
        uint32_t count = 0, ccount = 0;
        uint32_t position = 8;
        uint32_t row = 0;
        for (auto* u = itemfields; u->type != 0xFF; ++u) {
            if (u->row != row) {
                position = 8; row = u->row;
            }
			if (!(u->type & 0x10) && u->name[0] != 0) {
                CStatic editname;
                editname.Create(m_hWnd, CRect(3 + position, 4 + 23 * row, 58 + position, 22 + 23 * row), u->name, WS_CHILD | WS_VISIBLE | SS_RIGHT);
                editname.SetFont(fnt_, FALSE);
            }
            uint32_t width = 0;
            UINT extraFlag = ES_NUMBER | ES_NOIME;
            switch (u->type & 0x0F) {
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
            case 8:
            case 9:
                width = 120;
                break;
            }
            uint32_t prefix = u->name[0] == 0 ? 0 : 60;
            if (!(u->type & 0x10)) {
                if (u->type & 0x08) {
                    auto& cb = combobox_[ccount];
                    cb.Create(m_hWnd, CRect(prefix + position, 2 + 23 * row, prefix + width + position, 302 + 23 * row), 0, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 0, IDC_COMBOSIDBASE + ccount);
                    cb.SetFont(fnt_, FALSE);
                    cb.SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)u);
                    const auto* ptr = (const std::map<uint16_t, const wchar_t*>*)u->ptr;
                    auto* cptr = (std::map<uint16_t, int>*)u->cptr;
                    bool deal = cptr->empty();
                    for (auto& p : *ptr) {
                        int idx = cb.AddString(p.second);
                        cb.SetItemData(idx, p.first);
                        if (deal) (*cptr)[p.first] = idx;
                    }
                    ++ccount;
                } else {
                    editbox_[count].Create(m_hWnd, CRect(prefix + position, 2 + 23 * row, prefix + width + position, 23 + 23 * row), 0, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_AUTOHSCROLL | extraFlag, 0, IDC_ITEMEDITBASE + count);
                    editbox_[count].SetFont(fnt_, FALSE);
                    editbox_[count].SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)u);
                    ++count;
                }
            }
            position += prefix + width;
        }
        count = 0; ccount = 0;
        wchar_t txt[256];
        for (auto* u = itemfields; u->type != 0xFF; ++u) {
            if (!(u->type & 0x10)) {
                editbox_[count].EnableWindow(TRUE);
                uint8_t* ptr = (uint8_t*)&info_ + u->offset;
                auto t = u->type & 0x0F;
                switch (t) {
                case 0:
                {
                    wchar_t name[256];
                    ConvUTF8(ptr, name);
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
                case 8:
                case 9: {
                    auto* cptr = (std::map<uint16_t, int>*)u->cptr;
                    auto ite = cptr->find(t == 8 ? *(uint8_t*)ptr : *(uint16_t*)ptr);
                    if (ite == cptr->end())
                        combobox_[ccount].SetCurSel(0);
                    else
                        combobox_[ccount].SetCurSel(ite->second);
                    break;
                }
                }
                if (u->writecb) u->writecb(u, &info_);
                if (u->type & 0x08) ++ccount; else ++count;
            }
        }
        CRect rc;
        GetWindowRect(rc);
        rc.right = rc.left + 510;
        rc.bottom = rc.top + 315;
        MoveWindow(rc);
        CenterWindow(GetParent());
        GetClientRect(rc);
        CButton btn[2];
        btn[0].Create(m_hWnd, CRect((rc.right - rc.left) / 2 - 80, rc.bottom - rc.top - 28, (rc.right - rc.left) / 2 - 20, rc.bottom - rc.top - 6), L"确定", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, IDC_BTNOK);
        btn[0].SetFont(fnt_, FALSE);
        btn[1].Create(m_hWnd, CRect((rc.right - rc.left) / 2 + 20, rc.bottom - rc.top - 28, (rc.right - rc.left) / 2 + 80, rc.bottom - rc.top - 6), L"取消", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_BTNCANCEL);
        btn[1].SetFont(fnt_, FALSE);
        return TRUE;
    }
    LRESULT OnOK(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
        uint32_t count = 0, ccount = 0;
        for (auto* u = itemfields; u->type != 0xFF; ++u) {
            if (!(u->type & 0x10)) {
                wchar_t name[256];
                uint8_t* ptr = (uint8_t*)&info_ + u->offset;
                switch (u->type & 0x0F) {
                case 0:
                    editbox_[count].GetWindowText(name, 256);
                    ConvUCS2(name, ptr);
                    break;
                case 1:
                    editbox_[count].GetWindowText(name, 256);
                    *(uint8_t*)ptr = (uint8_t)wcstoul(name, nullptr, 10);
                    break;
                case 2:
                    editbox_[count].GetWindowText(name, 256);
                    *(uint16_t*)ptr = (uint16_t)wcstoul(name, nullptr, 10);
                    break;
                case 3:
                    editbox_[count].GetWindowText(name, 256);
                    *(uint32_t*)ptr = (uint32_t)wcstoul(name, nullptr, 10);
                    break;
                case 4:
                    editbox_[count].GetWindowText(name, 256);
                    *(uint64_t*)ptr = (uint64_t)wcstoull(name, nullptr, 10);
                    break;
                case 5:
                    editbox_[count].GetWindowText(name, 256);
                    *(int8_t*)ptr = (int8_t)wcstol(name, nullptr, 10);
                    break;
                case 6:
                    editbox_[count].GetWindowText(name, 256);
                    *(int16_t*)ptr = (int16_t)wcstol(name, nullptr, 10);
                    break;
                case 7:
                    editbox_[count].GetWindowText(name, 256);
                    *(int32_t*)ptr = (int32_t)wcstol(name, nullptr, 10);
                    break;
                case 8: {
                    int sel = combobox_[ccount].GetCurSel();
                    if (sel < 0) break;
                    *(uint8_t*)ptr = (uint8_t)combobox_[ccount].GetItemData(sel);
                    break;
                }
                case 9: {
                    int sel = combobox_[ccount].GetCurSel();
                    if (sel < 0) break;
                    *(uint16_t*)ptr = (uint16_t)combobox_[ccount].GetItemData(sel);
                    break;
                }
                }
                if (u->writecb) u->writecb(u, &info_);
                if (u->type & 0x08) ++ccount;
                else ++count;
            }
        }
        EndDialog(1);
        return 0;
    }
    LRESULT OnCancel(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
        EndDialog(0);
        return 0;
    }
    void OnClose() {
        EndDialog(0);
    }
private:
    BEGIN_MSG_MAP_EX(CItemDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        COMMAND_HANDLER(IDC_BTNOK, BN_CLICKED, OnOK)
        COMMAND_HANDLER(IDC_BTNCANCEL, BN_CLICKED, OnCancel)
    END_MSG_MAP()

private:
    ItemInfo info_;
    CFont fnt_;
    CEdit editbox_[40];
    CComboBox combobox_[10];
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
    LRESULT OnSkillChange(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnBtnSAdd(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnBtnSDel(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnItemEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnItemListDoubleClick(int id, LPNMHDR hdr, BOOL& bHandled);
    LRESULT OnItemListItemChanged(int id, LPNMHDR hdr, BOOL& bHandled);
    LRESULT OnItemBtnRefresh(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnItemBtnEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnItemBtnDel(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnItemBtnClone(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled);

private:
    BEGIN_MSG_MAP_EX(CPluginPanel2)
        COMMAND_RANGE_CODE_HANDLER(IDC_CHAREDITBASE, IDC_CHAREDITBASE + 100, EN_CHANGE, OnEdit)
        COMMAND_RANGE_CODE_HANDLER(IDC_COMBOSIDBASE, IDC_COMBOSIDBASE + 0x60, CBN_SELCHANGE, OnSkillChange)
        COMMAND_RANGE_CODE_HANDLER(IDC_EDITSLVLBASE, IDC_EDITSLVLBASE + 0x60, EN_CHANGE, OnSkillChange)
        COMMAND_RANGE_CODE_HANDLER(IDC_EDITSEXPBASE, IDC_EDITSEXPBASE + 0x60, EN_CHANGE, OnSkillChange)
        COMMAND_RANGE_CODE_HANDLER(IDC_ITEMEDITBASE, IDC_ITEMEDITBASE + 4, BN_CLICKED, OnItemEdit)
        COMMAND_HANDLER(IDC_BTNSADD, BN_CLICKED, OnBtnSAdd)
        COMMAND_HANDLER(IDC_BTNSDEL, BN_CLICKED, OnBtnSDel)
        COMMAND_HANDLER(IDC_ITEMBTNREFRESH, BN_CLICKED, OnItemBtnRefresh)
        COMMAND_HANDLER(IDC_ITEMBTNEDIT, BN_CLICKED, OnItemBtnEdit)
        COMMAND_HANDLER(IDC_ITEMBTNDEL, BN_CLICKED, OnItemBtnDel)
        COMMAND_HANDLER(IDC_ITEMBTNCLONE, BN_CLICKED, OnItemBtnClone)
        NOTIFY_RANGE_CODE_HANDLER(IDC_ITEMLIST, IDC_ITEMLIST2, NM_DBLCLK, OnItemListDoubleClick)
        NOTIFY_RANGE_CODE_HANDLER(IDC_ITEMLIST, IDC_ITEMLIST2, LVN_ITEMCHANGED, OnItemListItemChanged)
    END_MSG_MAP()
};

class PluginDisgaea2E: public IPlugin {
public:
    virtual ~PluginDisgaea2E() override {

    }

    virtual uint32_t Init(CAppModule* mod) override {
        module_ = mod;
        return 3;
    }

    virtual LPCWSTR GetName(uint32_t page) override {
        const wchar_t* names[3] = { L"角色修改", L"技能修改", L"道具修改" };
        return names[page];
    }

    virtual bool Enable(IProcEdit* proc, void* tc) override {
        if (proc->ClassName() != "Disgaea2 PC") return false;
        if (proc->GetVersion() == "1.0.0.1") {
            selfStartOff_ = 0x2A0738;
            selfCountOff_ = 0x34D974;
            itemStartOff_ = 0x31B538;
        } else return false;
        itemsel_ = -1;
        proc_ = proc;
        tabCtrl_ = (HWND)tc;
        fnt_.CreatePointFont(90, _T("Arial"), 0);
        CRect trc, rc;
        tabCtrl_.GetItemRect(0, trc);
        tabCtrl_.GetClientRect(rc);
        rc.DeflateRect(8, trc.Height() + 8, 8, 8);
        panell_.Create(tabCtrl_.m_hWnd, CRect(rc.left, rc.top, rc.left + 140, rc.bottom), 0, WS_CHILD);
        panel_[2].Create(tabCtrl_.m_hWnd, rc, 0, WS_CHILD | WS_BORDER);
        CRect src(rc.left + 140, rc.top, rc.right, rc.bottom);
        spanel_[0].Create(tabCtrl_.m_hWnd, src, 0, WS_CHILD | WS_BORDER);
        spanel_[1].Create(tabCtrl_.m_hWnd, src, 0, WS_CHILD | WS_BORDER);
        panel_[0].Create(spanel_[0].m_hWnd, CRect(0, 0, 10, 10), 0, WS_CHILD | WS_VISIBLE);
        panel_[1].Create(spanel_[1].m_hWnd, CRect(0, 0, 10, 10), 0, WS_CHILD | WS_VISIBLE);

        charlist_.Create(panell_.m_hWnd, CRect(8, 0, 128, rc.Height() - 22), 0, WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 0, IDC_CHARLIST);
        charlist_.SetFont(fnt_, FALSE);
        CButton btn[2];
        btn[0].Create(panell_.m_hWnd, CRect(8, rc.Height() - 22, 62, rc.Height()), L"刷新", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_RELOAD);
        btn[1].Create(panell_.m_hWnd, CRect(72, rc.Height() - 22, 128, rc.Height()), L"写入", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_SAVE);
        btn[0].SetFont(fnt_, FALSE);
        btn[1].SetFont(fnt_, FALSE);
        LoadChars();
		uint32_t count = 0;
		uint32_t position = 0;
		uint32_t row = 0;
		for (auto* u = unitfields; u->type != 0xFF; ++u) {
			if (u->row != row) {
				position = 0; row = u->row;
			}
			if (!(u->type & 0x10)) {
                CStatic editname;
				editname.Create(spanel_[0].m_hWnd, CRect(3 + position, 4 + 23 * row, 58 + position, 22 + 23 * row), u->name, WS_CHILD | WS_VISIBLE | SS_RIGHT);
				editname.SetFont(fnt_, FALSE);
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
				editbox_[count].SetFont(fnt_, FALSE);
                editbox_[count].SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)u);
			}
			position += 60 + width;
			++count;
		}
        CStatic txt0;
        txt0.Create(panel_[0].m_hWnd, CRect(340, 280, 388, 298), L"装备", WS_CHILD | WS_VISIBLE);
        for (int i = 0; i < 4; ++i) {
            btnitem_[i].Create(panel_[0].m_hWnd, CRect(340, 301 + 23 * i, 510, 322 + 23 * i), L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_ITEMEDITBASE + i);
            btnitem_[i].SetFont(fnt_, FALSE);
        }
        txt0.SetFont(fnt_, FALSE);
        panel_[0].GetWindowRect(src);
        panel_[0].ScreenToClient(src);
        src.bottom = 26 + 23 * row + src.top;
        panel_[0].MoveWindow(src);
        btnsadd_.Create(panel_[1].m_hWnd, CRect(0, 0, 22, 22), L"+", WS_CHILD | BS_PUSHBUTTON, 0, IDC_BTNSADD);
        btnsdel_.Create(panel_[1].m_hWnd, CRect(0, 0, 22, 22), L"-", WS_CHILD | BS_PUSHBUTTON, 0, IDC_BTNSDEL);
        btnsadd_.SetFont(fnt_, FALSE);
        btnsdel_.SetFont(fnt_, FALSE);
        CStatic txt1, txt2, txt3;
        txt1.Create(panel_[1].m_hWnd, CRect(28, 8, 100, 30), L"技能", WS_CHILD | WS_VISIBLE);
        txt2.Create(panel_[1].m_hWnd, CRect(128, 8, 160, 30), L"等级", WS_CHILD | WS_VISIBLE);
        txt3.Create(panel_[1].m_hWnd, CRect(188, 8, 250, 30), L"经验", WS_CHILD | WS_VISIBLE);
        txt1.SetFont(fnt_, FALSE);
        txt2.SetFont(fnt_, FALSE);
        txt3.SetFont(fnt_, FALSE);
        for (uint32_t i = 0; i < 0x60; ++i) {
            if (i == 0) {
                combosid_[i].Create(panel_[1].m_hWnd, CRect(8, 32 + i * 25, 100, 254 + i * 25), 0, WS_CHILD | CBS_DROPDOWNLIST | CBS_DROPDOWNLIST | WS_VSCROLL, 0, IDC_COMBOSIDBASE + i);
                int index = combosid_[i].AddString(L"- 无 -");
                skillMap_[0] = 0;
                skillMap2_[0] = 0;
                for (auto& p : skill_names) {
                    index = combosid_[i].AddString(p.second);
                    skillMap_[index] = p.first;
                    skillMap2_[p.first] = index;
                }
                combosid_[i].SetFont(fnt_, FALSE);
            }
            editslvl_[i].Create(panel_[1].m_hWnd, CRect(108, 32 + i * 25, 160, 54 + i * 25), 0, WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER | ES_NOIME, 0, IDC_EDITSLVLBASE + i);
            editsexp_[i].Create(panel_[1].m_hWnd, CRect(168, 32 + i * 25, 250, 54 + i * 25), 0, WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER | ES_NOIME, 0, IDC_EDITSEXPBASE + i);
            editslvl_[i].SetFont(fnt_, FALSE);
            editsexp_[i].SetFont(fnt_, FALSE);
        }
        spanel_[0].SetClient(panel_[0]);
        spanel_[1].SetClient(panel_[1]);
        spanel_[0].ShowWindow(SW_HIDE);
        spanel_[1].ShowWindow(SW_HIDE);
        CStatic istatic[2];
        CButton ibtn[4];
        istatic[0].Create(panel_[2].m_hWnd, CRect(8, 2, 108, 20), L"携带袋", WS_CHILD | WS_VISIBLE);
        istatic[1].Create(panel_[2].m_hWnd, CRect(310, 2, 410, 20), L"仓库", WS_CHILD | WS_VISIBLE);
        istatic[0].SetFont(fnt_, FALSE);
        istatic[1].SetFont(fnt_, FALSE);
        itemlist_[0].Create(panel_[2].m_hWnd, CRect(8, 20, 308, rc.Height() - 8), 0, WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, IDC_ITEMLIST);
        itemlist_[1].Create(panel_[2].m_hWnd, CRect(310, 20, 618, rc.Height() - 8), 0, WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, IDC_ITEMLIST2);
        for (int i = 0; i < 2; ++i) {
            itemlist_[i].SetFont(fnt_, FALSE);
            itemlist_[i].SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
            itemlist_[i].InsertColumn(0, L"名称", LVCFMT_LEFT, 125);
            itemlist_[i].InsertColumn(1, L"型号", LVCFMT_LEFT, 125);
            itemlist_[i].InsertColumn(2, L"LV", LVCFMT_LEFT, 35);
        }
        ibtn[0].Create(panel_[2].m_hWnd, CRect(626, 20, 706, 41), L"刷新", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_ITEMBTNREFRESH);
        ibtn[1].Create(panel_[2].m_hWnd, CRect(626, 50, 706, 71), L"编辑", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_ITEMBTNEDIT);
        ibtn[2].Create(panel_[2].m_hWnd, CRect(626, 80, 706, 101), L"删除", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_ITEMBTNDEL);
        ibtn[3].Create(panel_[2].m_hWnd, CRect(626, 110, 706, 131), L"复制", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_ITEMBTNCLONE);
        for (int i = 0; i < 4; ++i) {
            ibtn[i].SetFont(fnt_, FALSE);
        }
        LoadItems();
        /*
        for (int i = 0; i < 5; ++i) {
            editbox_[i].Create(panel_.m_hWnd, CRect(150, 8 + 30 * i, 250, 27 + 30 * i), 0, WS_CHILD | WS_BORDER | WS_VISIBLE, 0, IDC_CHAREDITBASE + i);
            editbox_[i].SetFont(fnt_);
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
                spanel_[i].SetClient(NULL, false);
                spanel_[i].DestroyWindow();
            }
            spanel_[i].m_hWnd = (HWND)0;
            panel_[i].m_hWnd = (HWND)0;
        }
        panel_[2].m_hWnd = (HWND)0;
        charlist_.m_hWnd = (HWND)0;
        itemlist_[0].m_hWnd = (HWND)0;
        itemlist_[1].m_hWnd = (HWND)0;
        btnsadd_.m_hWnd = (HWND)0;
        btnsdel_.m_hWnd = (HWND)0;
        for (int i = 0; i < 100; ++i) {
            editbox_[i].m_hWnd = (HWND)0;
        }
        for (int i = 0; i < 0x60; ++i) {
            combosid_[i].m_hWnd = (HWND)0;
            editslvl_[i].m_hWnd = (HWND)0;
            editsexp_[i].m_hWnd = (HWND)0;
        }
        for (int i = 0; i < 4; ++i) {
            btnitem_[i].m_hWnd = (HWND)0;
        }
        proc_ = nullptr;
        tabCtrl_ = (HWND)0;
        count_ = 0;
        fnt_.DeleteObject();
        units_.clear();
    }

    virtual void Show(uint32_t page) override {
        switch (page) {
        case 0:
        case 1:
            panell_.ShowWindow(SW_SHOW);
            spanel_[page].ShowWindow(SW_SHOW);
            break;
        case 2:
            panel_[2].ShowWindow(SW_SHOW);
            break;
        }
    }

    virtual void Hide(uint32_t page) override {
        switch (page) {
        case 0:
        case 1:
            panell_.ShowWindow(SW_HIDE);
            spanel_[page].ShowWindow(SW_HIDE);
            break;
        case 2:
            panel_[2].ShowWindow(SW_HIDE);
            break;
        }
    }

    virtual void Tick(uint32_t page) override {
    }

    void OnSelChange() {
        changing_ = true;
        dirty_.clear();
        int n = charlist_.GetCurSel();
        uint16_t idx = 0;
        if (n >= 0) {
            idx = (uint16_t)charlist_.GetItemData(n);
            if (idx >= (uint16_t)units_.size()) {
                n = -1;
            }
        }
        if (n < 0) {
			uint32_t count = 0;
			for (auto* u = unitfields; u->type != 0xFF; ++u) {
                if (!(u->type & 0x10)) {
                    editbox_[count].EnableWindow(FALSE);
                    editbox_[count].Clear();
                }
				++count;
			}
            for (uint32_t i = 0; i < 0x60; ++i) {
                if(combosid_[i].IsWindow()) combosid_[i].ShowWindow(SW_HIDE);
                editslvl_[i].ShowWindow(SW_HIDE);
                editsexp_[i].ShowWindow(SW_HIDE);
            }
            btnsadd_.ShowWindow(SW_HIDE);
            btnsdel_.ShowWindow(SW_HIDE);
            changing_ = false;
            return;
		}
		uint32_t count = 0;
		wchar_t txt[128];
        auto &unit = units_[idx];
        for (auto* u = unitfields; u->type != 0xFF; ++u) {
			if (!(u->type & 0x10)) {
				editbox_[count].EnableWindow(TRUE);
				uint8_t* ptr = (uint8_t*)&unit + u->offset;
				switch (u->type & 0x07) {
				case 0: {
					wchar_t name[256];
					ConvUTF8(ptr, name);
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
        for (uint32_t i = 0; i < 4; ++i) {
            auto* ii = (ItemInfo*)(unit.equip + sizeof(ItemInfo) * i);
            wchar_t name[256];
            if (ii->id == 0) {
                btnitem_[i].SetWindowTextW(L"- 无 -");
            } else {
                ConvUTF8(ii->name, name);
                btnitem_[i].SetWindowTextW(name);
            }
        }
        RefreshSkillUI(unit);
        changing_ = false;
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
					ConvUCS2(name, ptr);
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
                if (uf->writecb) uf->writecb(uf, &units_[idx]);
            }
        }
        dirty_.clear();
        proc_->Write(false, selfStartOff_ + sizeof(UnitInfo) * idx, &units_[idx], sizeof(UnitInfo));
    }

    void OnChanged(uint32_t n) {
        if (changing_) return;
        dirty_.insert(n);
    }

    void OnSkillChanged(int type, int n) {
        if (changing_ || n >= 0x60) return;
        int sel = charlist_.GetCurSel();
        if (sel < 0) return;
        uint16_t idx = (uint16_t)charlist_.GetItemData(sel);
        if (idx >= (uint16_t)units_.size()) return;
        auto& unit = units_[idx];
        switch (type) {
        case 0: {
            int ssel = combosid_[n].GetCurSel();
            auto ite = skillMap_.find(ssel);
            if (ite == skillMap_.end()) unit.skillID[n] = 0;
            unit.skillID[n] = ite->second;
            break;
        }
        case 1: {
            wchar_t txt[128];
            editslvl_[n].GetWindowText(txt, 128);
            unit.skillExp[n] = (uint32_t)wcstoul(txt, 0, 10);
            break;
        }
        case 2: {
            wchar_t txt[128];
            editsexp_[n].GetWindowText(txt, 128);
            unit.skillExp[n] = (uint8_t)wcstoul(txt, 0, 10);
            break;
        }
        }
    }

    void OnSkillAdd() {
        int sel = charlist_.GetCurSel();
        if (sel < 0) return;
        uint16_t idx = (uint16_t)charlist_.GetItemData(sel);
        if (idx >= (uint16_t)units_.size()) return;
        auto& unit = units_[idx];
        if (unit.skillCount >= 0x60) return;
        changing_ = true;
        unit.skillID[unit.skillCount] = 0;
        unit.skillLevel[unit.skillCount] = 0;
        unit.skillExp[unit.skillCount] = 0;
        ++unit.skillCount;
        RefreshSkillUI(unit, unit.skillCount - 1, unit.skillCount);
        changing_ = false;
    }

    void OnSkillDel() {
        if (changing_) return;
        int sel = charlist_.GetCurSel();
        if (sel < 0) return;
        uint16_t idx = (uint16_t)charlist_.GetItemData(sel);
        if (idx >= (uint16_t)units_.size()) return;
        auto& unit = units_[idx];
        if (unit.skillCount == 0) return;
        changing_ = true;
        --unit.skillCount;
        RefreshSkillUI(unit, unit.skillCount, unit.skillCount + 1);
        changing_ = false;
    }

    void OnItemEdit(uint32_t index) {
        int sel = charlist_.GetCurSel();
        if (sel < 0) return;
        uint16_t idx = (uint16_t)charlist_.GetItemData(sel);
        if (idx >= (uint16_t)units_.size()) return;
        auto& unit = units_[idx];
        CItemDlg dlg;
        dlg.SetItem(*(ItemInfo*)(unit.equip + sizeof(ItemInfo) * index));
        if (dlg.DoModal(tabCtrl_)) {
            memcpy(unit.equip + sizeof(ItemInfo) * index, &dlg.Info(), sizeof(ItemInfo));
            wchar_t text[256];
            ConvUTF8(dlg.Info().name, text);
            btnitem_[index].SetWindowText(text);
        }
    }

    void DoItemRefresh() {
        LoadItems();
    }

    void DoItemEdit() {
        if (itemsel_ < 0) return;
        int type = itemsel_ < 24 ? 0 : 1;
        int sel = itemsel_ < 24 ? itemsel_ : itemsel_ - 24;
        uint32_t index = (uint32_t)itemlist_[type].GetItemData(sel);
        CItemDlg dlg;
        ItemInfo info;
        proc_->Read(false, itemStartOff_ + sizeof(ItemInfo) * index, &info, sizeof(ItemInfo));
        dlg.SetItem(info);
        if (dlg.DoModal(tabCtrl_)) {
            wchar_t text[256];
            auto& ii = dlg.Info();
            if (ii.id == 0)
                lstrcpy(text, item_empty_text);
            else
                ConvUTF8(ii.name, text);
            itemlist_[type].SetItemText(sel, 0, text);
            if (ii.id == 0) {
                itemlist_[type].SetItemText(sel, 1, L"");
                itemlist_[type].SetItemText(sel, 2, L"");
            } else {
                auto ite = mitem_names.find(ii.id);
                if (ite != mitem_names.end())
                    itemlist_[type].SetItemText(sel, 1, ite->second);
                wsprintf(text, L"%u", ii.level);
                itemlist_[type].SetItemText(sel, 2, text);
            }
            proc_->Write(false, itemStartOff_ + sizeof(ItemInfo) * index, &ii, sizeof(ItemInfo));
        }
    }

    void DoItemDel() {
        if (itemsel_ < 0) return;
        int type = itemsel_ < 24 ? 0 : 1;
        int sel = itemsel_ < 24 ? itemsel_ : itemsel_ - 24;
        uint32_t index = (uint32_t)itemlist_[type].GetItemData(sel);
        CItemDlg dlg;
        ItemInfo info;
        proc_->Read(false, itemStartOff_ + sizeof(ItemInfo) * index, &info, sizeof(ItemInfo));
        info.id = 0;
        proc_->Write(false, itemStartOff_ + sizeof(ItemInfo) * index, &info, sizeof(ItemInfo));
        itemlist_[type].SetItemText(sel, 0, item_empty_text);
        itemlist_[type].SetItemText(sel, 1, L"");
        itemlist_[type].SetItemText(sel, 2, L"");
    }

    void DoItemClone() {
        if (itemsel_ < 0) return;
        int type = itemsel_ < 24 ? 0 : 1;
        int sel = itemsel_ < 24 ? itemsel_ : itemsel_ - 24;
        uint32_t index = (uint32_t)itemlist_[type].GetItemData(sel);
        std::vector<ItemInfo> info;
        info.resize(536);
        proc_->Read(false, itemStartOff_, &info[0], sizeof(ItemInfo) * 536);
        for (uint32_t i = 0; i < 512 + 24; ++i) {
            if (i != index && info[i].id == 0) {
                proc_->Write(false, itemStartOff_ + sizeof(ItemInfo) * i, &info[index], sizeof(ItemInfo));
                type = i < 24 ? 0 : 1;
                sel = i < 24 ? i : i - 24;
                auto& il = itemlist_[type];
                wchar_t text[256];
                if (info[index].id == 0)
                    lstrcpy(text, item_empty_text);
                else
                    ConvUTF8(info[index].name, text);
                il.SetItemText(sel, 0, text);
                if (info[index].id == 0) return;
                auto ite = mitem_names.find(info[index].id);
                if (ite != mitem_names.end())
                    il.SetItemText(sel, 1, ite->second);
                wsprintf(text, L"%u", info[index].level);
                il.SetItemText(sel, 2, text);
                return;
            }
        }
        ::MessageBoxW(tabCtrl_, L"没有剩余的空位!", L"复制物品", MB_OK | MB_ICONERROR);
    }

    void OnItemSelChange(int sel) {
        itemsel_ = sel;
    }

private:
    void LoadChars() {
        proc_->Read(false, selfCountOff_, &count_, 2);
        units_.resize(count_);
        if (count_ == 0) return;
        proc_->Read(false, selfStartOff_, &units_[0], sizeof(UnitInfo) * count_);
        int n = charlist_.GetCount();
        while (--n >= 0)
            charlist_.DeleteString(n);
        for (uint16_t i = 0; i < count_; ++i) {
			wchar_t name[256];
			ConvUTF8(units_[i].name, name);
            int index = charlist_.AddString(name);
            charlist_.SetItemData(index, (DWORD_PTR)i);
        }
    }

    void LoadItems() {
        std::vector<ItemInfo> info;
        info.resize(536);
        proc_->Read(false, itemStartOff_, &info[0], sizeof(ItemInfo) * 536);
        for (int i = 0; i < 2; ++i) {
            itemlist_[i].SetRedraw(FALSE);
            itemlist_[i].DeleteAllItems();
        }
        for (uint32_t i = 0; i < 512 + 24; ++i) {
            wchar_t text[256];
            if (info[i].id == 0)
                lstrcpy(text, item_empty_text);
            else
                ConvUTF8(info[i].name, text);
            CListViewCtrl* il;
            uint32_t index;
            if (i < 24) {
                il = &itemlist_[0];
                index = i;
            } else {
                il = &itemlist_[1];
                index = i - 24;
            }
            index = il->AddItem(index, 0, text);
            if (info[i].id == 0) continue;
            auto ite = mitem_names.find(info[i].id);
            if (ite != mitem_names.end())
                il->SetItemText(index, 1, ite->second);
            wsprintf(text, L"%u", info[i].level);
            il->SetItemText(index, 2, text);
            il->SetItemData(index, i);
        }
        for (int i = 0; i < 2; ++i)
            itemlist_[i].SetRedraw(TRUE);
        itemsel_ = -1;
    }

    void RefreshSkillUI(UnitInfo& unit, int start = -1, int end = -1) {
        if (start < 0) start = 0;
        if (end < 0) end = 0x60;
        for (int i = start; i < unit.skillCount && i < end; ++i) {
            if (!combosid_[i].IsWindow()) {
                combosid_[i].Create(panel_[1].m_hWnd, CRect(8, 32 + i * 25, 100, 254 + i * 25), 0, WS_CHILD | CBS_DROPDOWNLIST | CBS_DROPDOWNLIST | WS_VSCROLL, 0, IDC_COMBOSIDBASE + i);
                combosid_[i].AddString(L"- 无 -");
                for (auto& p : skill_names)
                    combosid_[i].AddString(p.second);
                combosid_[i].SetFont(fnt_, FALSE);
            }
            combosid_[i].ShowWindow(SW_SHOW);
            auto ite = skillMap2_.find(unit.skillID[i]);
            if (ite == skillMap2_.end())
                combosid_[i].SetCurSel(0);
            else
                combosid_[i].SetCurSel(ite->second);
            editslvl_[i].ShowWindow(SW_SHOW);
            wchar_t txt[128];
            wsprintf(txt, L"%d", unit.skillLevel[i]);
            editslvl_[i].SetWindowText(txt);
            editsexp_[i].ShowWindow(SW_SHOW);
            wsprintf(txt, L"%d", unit.skillExp[i]);
            editsexp_[i].SetWindowText(txt);
        }
        if (unit.skillCount < 0x60) {
            btnsadd_.ShowWindow(SW_SHOW);
            btnsadd_.MoveWindow(CRect(8, 32 + unit.skillCount * 25, 30, 54 + unit.skillCount * 25));
        } else
            btnsadd_.ShowWindow(SW_HIDE);
        if (unit.skillCount > 0) {
            btnsdel_.ShowWindow(SW_SHOW);
            btnsdel_.MoveWindow(CRect(258, 32 + (unit.skillCount - 1) * 25, 280, 54 + (unit.skillCount - 1) * 25));
        } else
            btnsdel_.ShowWindow(SW_HIDE);
        for (int i = unit.skillCount; i < end; ++i) {
            if (combosid_[i].IsWindow()) combosid_[i].ShowWindow(SW_HIDE);
            editslvl_[i].ShowWindow(SW_HIDE);
            editsexp_[i].ShowWindow(SW_HIDE);
        }
        if (end >= unit.skillCount) {
            CRect rc;
            panel_[1].GetWindowRect(rc);
            spanel_[1].ScreenToClient(rc);
            rc.right = rc.left + 288;
            rc.bottom = rc.top + 62 + (unit.skillCount < 0x60 ? unit.skillCount : unit.skillCount - 1) * 25;
            panel_[1].MoveWindow(rc, FALSE);
            spanel_[1].SetScrollSize(rc.Width(), rc.Height(), TRUE, false);
			spanel_[1].UpdateLayout();
        }
    }

private:
    uintptr_t selfStartOff_ = 0;
    uintptr_t selfCountOff_ = 0;
    uintptr_t itemStartOff_ = 0;
    int itemsel_ = -1;

    CAppModule* module_ = nullptr;
    IProcEdit* proc_ = nullptr;
    CFont fnt_;
    CTabCtrl tabCtrl_;
    CPluginPanel panell_;
    CScrollContainer spanel_[2];
    CPluginPanel2 panel_[3];
    CListBox charlist_;
    CListViewCtrl itemlist_[2];
    CEdit editbox_[100];
    CComboBox combosid_[0x60];
    CEdit editslvl_[0x60];
    CEdit editsexp_[0x60];
    CButton btnitem_[4];
    CButton btnsadd_, btnsdel_;
    uint16_t count_ = 0;
    std::vector<UnitInfo> units_;
    std::set<uint32_t> dirty_;
    bool changing_ = false;
    std::map<int, uint16_t> skillMap_;
    std::map<uint16_t, int> skillMap2_;
};

static PluginDisgaea2E gPlugin;

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

LRESULT CPluginPanel2::OnSkillChange(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    if (id >= IDC_COMBOSIDBASE && id < IDC_COMBOSIDBASE + 0x60)
        gPlugin.OnSkillChanged(0, id - IDC_COMBOSIDBASE);
    if (id >= IDC_EDITSLVLBASE && id < IDC_EDITSLVLBASE + 0x60)
        gPlugin.OnSkillChanged(1, id - IDC_EDITSLVLBASE);
    if (id >= IDC_EDITSEXPBASE && id < IDC_EDITSEXPBASE + 0x60)
        gPlugin.OnSkillChanged(2, id - IDC_EDITSEXPBASE);
    return 0;
}

LRESULT CPluginPanel2::OnBtnSAdd(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    gPlugin.OnSkillAdd();
    return 0;
}

LRESULT CPluginPanel2::OnBtnSDel(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    gPlugin.OnSkillDel();
    return 0;
}

LRESULT CPluginPanel2::OnItemEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL & bHandled) {
    gPlugin.OnItemEdit(id - IDC_ITEMEDITBASE);
    return 0;
}

LRESULT CPluginPanel2::OnItemListDoubleClick(int id, LPNMHDR hdr, BOOL& bHandled) {
    gPlugin.DoItemEdit();
    return 0;
}

LRESULT CPluginPanel2::OnItemListItemChanged(int id, LPNMHDR hdr, BOOL& bHandled) {
    NMLISTVIEW* lpnmlv = (NMLISTVIEW*)hdr;
    if ((lpnmlv->uOldState & LVIS_SELECTED) == 0 && (lpnmlv->uNewState & LVIS_SELECTED) != 0) {
        switch (hdr->idFrom) {
        case IDC_ITEMLIST:
            gPlugin.OnItemSelChange(lpnmlv->iItem);
            break;
        case IDC_ITEMLIST2:
            gPlugin.OnItemSelChange(24 + lpnmlv->iItem);
            break;
        }
    } else if ((lpnmlv->uOldState & LVIS_SELECTED) != 0 && (lpnmlv->uNewState & LVIS_SELECTED) == 0) {
        gPlugin.OnItemSelChange(-1);
    }
    return 0;
}

LRESULT CPluginPanel2::OnItemBtnRefresh(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    gPlugin.DoItemRefresh();
    return 0;
}

LRESULT CPluginPanel2::OnItemBtnEdit(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    gPlugin.DoItemEdit();
    return 0;
}

LRESULT CPluginPanel2::OnItemBtnDel(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    gPlugin.DoItemDel();
    return 0;
}

LRESULT CPluginPanel2::OnItemBtnClone(WORD notifyCode, WORD id, HWND hwnd, BOOL& bHandled) {
    gPlugin.DoItemClone();
    return 0;
}

IPlugin* WINAPI GetPlugin() {
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
    return &gPlugin;
}
