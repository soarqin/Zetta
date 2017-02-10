#include "pch.h"

#include "MainWnd.h"

#include "ProcEdit.h"
#include "Zetta!.h"

std::wstring U2W(const std::string& n) {
    std::wstring r;
    int sz = ::MultiByteToWideChar(CP_UTF8, 0, n.c_str(), n.length(), NULL, 0);
    r.resize(sz);
    ::MultiByteToWideChar(CP_UTF8, 0, n.c_str(), n.length(), &r[0], sz);
    return r;
}


LRESULT CCommonPanel::OnBytes(WORD nNotifyCode, WORD nID, HWND hWnd, BOOL &) {
    PatchAddr* p = (PatchAddr*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (p == NULL) return 0;
    CButton btn = hWnd;
    if (btn.GetCheck()) {
        for (auto& pp : p->bytes) {
            gProcEdit.MakePatch(pp.search, pp.searchMask, pp.patch, pp.patchMask, pp.skip, pp.poff);
        }
    } else {
        for (auto& pp : p->bytes) {
            gProcEdit.CancelPatch(pp.poff);
        }
    }
    return 0;
}

LRESULT CCommonPanel::OnEdit(WORD nNotifyCode, WORD nID, HWND hWnd, BOOL &) {
    PatchAddr* p = (PatchAddr*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (p == NULL) return 0;
    char n[256];
    ::GetDlgItemTextA(m_hWnd, nID, n, 256);
    switch (p->type) {
    case PT_U8:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint8_t)strtoul(n, nullptr, 10));
        break;
    case PT_U16:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint16_t)strtoul(n, nullptr, 10));
        break;
    case PT_U32:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint32_t)strtoul(n, nullptr, 10));
        break;
    case PT_U64:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint64_t)strtoull(n, nullptr, 10));
        break;
    case PT_I8:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint8_t)strtol(n, nullptr, 10));
        break;
    case PT_I16:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint16_t)strtol(n, nullptr, 10));
        break;
    case PT_I32:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint32_t)strtol(n, nullptr, 10));
        break;
    case PT_I64:
        gProcEdit.Write(p->pos == PP_MEM, p->offset, (uint64_t)strtoll(n, nullptr, 10));
        break;
    default:
        break;
    }
    return 0;
}


CMainWnd::CMainWnd() {
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    TCHAR path[256];
    GetModuleFileName(NULL, path, 256);
    PathRemoveFileSpec(path);
    PathAppend(path, _T("plugins"));
    TCHAR wc[256];
    lstrcpy(wc, path);
    PathAppend(wc, _T("*.dll"));

    hFind = FindFirstFile(wc, &ffd);

    if (INVALID_HANDLE_VALUE == hFind) return;

    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        TCHAR fname[256];
        lstrcpy(fname, path);
        PathAppend(fname, ffd.cFileName);
        HMODULE mod = LoadLibrary(fname);
        if (mod == nullptr) continue;
        PLUGINLOADPROC loadProc = (PLUGINLOADPROC)GetProcAddress(mod, "GetPlugin");
        if (loadProc == nullptr) {
            FreeLibrary(mod);
            continue;
        }
        IPlugin* plugin = loadProc();
        if (plugin != nullptr) {
            uint32_t n = plugin->Init(&_Module);
            plugins_.push_back(std::make_pair(plugin, n));
        } else
            FreeLibrary(mod);
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);
}

CMainWnd::~CMainWnd() {
}

LRESULT CMainWnd::OnCreate(LPCREATESTRUCT) {
    auto icon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MAINICON));
    SetIcon(icon, TRUE);
    SetIcon(icon, FALSE);
    // center the dialog on the screen
    CenterWindow();

    UpdateTitle();

    fnt_.CreatePointFont(90, _T("SimSun"));
    fnt2_.CreatePointFont(150, _T("SimSun"), 0, true);
    fnt3_.CreatePointFont(120, _T("SimSun"), 0, true);

    BuildForm();
    Update();

    SetTimer(IDT_UPDATE, 1000, NULL);

    return TRUE;
}

void CMainWnd::OnDestroy() {
    Clear();
}

void CMainWnd::OnCloseDialog() {
    DestroyWindow();
    ::PostQuitMessage(0);
}

BOOL CMainWnd::OnEraseBkGnd(HDC hdc) {
    CDCHandle dc = hdc;

    // Save old brush
    CBrush brush;
    brush.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
    CBrushHandle oldBrush = dc.SelectBrush(brush);

    CRect rect;
    dc.GetClipBox(&rect);     // Erase the area needed

    dc.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(),
        PATCOPY);
    dc.SelectBrush(oldBrush);
    return TRUE;
}

LRESULT CMainWnd::OnTabChange(int, LPNMHDR, BOOL&) {
    auto sel = tabctrl_.GetCurSel();
    if (sel < 0) return 0;
    if (lastsel_ > 0) {
        if ((size_t)lastsel_ <= pluginsEnabled_.size()) {
            auto& plugin = pluginsEnabled_[lastsel_ - 1];
            plugin.first->Hide(plugin.second);
        }
    }
    else if (lastsel_ == 0) {
        for (auto& c : cpanels_)
            c->ShowWindow(SW_HIDE);
    }
    if (sel > 0) {
        if ((size_t)sel <= pluginsEnabled_.size()) {
            auto& plugin = pluginsEnabled_[sel - 1];
            plugin.first->Show(plugin.second);
        }
    } else {
        for (auto& c : cpanels_)
            c->ShowWindow(SW_SHOW);
    }
    lastsel_ = sel;
    Update();
    return 0;
}

void CMainWnd::OnTimer(UINT_PTR) {
    Update();
}

void CMainWnd::Clear() {
    lastsel_ = -1;
    UnloadPlugins();
    for (auto& l : labels_) {
        l->DestroyWindow();
        delete l;
    }
    labels_.clear();
    for (auto& t : textboxes_) {
        t->DestroyWindow();
        delete t;
    }
    textboxes_.clear();
    for (auto& c : checkboxes_) {
        c->DestroyWindow();
        delete c;
    }
    checkboxes_.clear();
    for (auto& p : cpanels_) {
        p->DestroyWindow();
        delete p;
    }
    cpanels_.clear();
    if (tabctrl_.IsWindow())
        tabctrl_.DestroyWindow();
}

void CMainWnd::BuildForm() {
    Clear();
    if (spec_ == nullptr) {
        CDC dc = CreateCompatibleDC(GetDC());
        dc.SelectFont(fnt_);
        
        auto* lab = new CStatic;
        const TCHAR* caption = _T("Waiting for game...");
        lab->Create(m_hWnd, 0, caption, WS_VISIBLE | WS_CHILD | SS_CENTER);
        lab->SetFont(fnt_);
        SIZE sz;
        dc.GetTextExtent(caption, lstrlen(caption), &sz);
        auto w = std::max(400, (int)sz.cx + 16);
        auto h = std::max(250, (int)sz.cy + 16);
        CRect rc(w, h, w + w, h + h);
        AdjustWindowRectEx(rc, this->GetStyle() & ~WS_OVERLAPPED, FALSE, this->GetExStyle());
        MoveWindow(rc);
        CenterWindow();
        auto l = (w - sz.cx) / 2;
        auto t = (h - sz.cy) / 2;
        lab->MoveWindow(CRect(l, t, l + sz.cx, t + sz.cy));
        labels_.push_back(lab);
    } else {
        CDC dc = CreateCompatibleDC(GetDC()), dc2 = CreateCompatibleDC(GetDC());
        dc.SelectFont(fnt_);
        dc2.SelectFont(fnt2_);

        tabctrl_.Create(m_hWnd, CRect(0, 0, 100, 100), NULL, WS_VISIBLE | WS_CHILD | TCS_SINGLELINE, 0, IDC_TABS);
        tabctrl_.SetFont(fnt3_);
        tabctrl_.AddItem(_T("Í¨ÓÃ"));
        CRect trc;
        tabctrl_.GetItemRect(0, trc);

        unsigned maxheight = 0, maxwidth = 0;
        unsigned currleft = 0, currtop = trc.Height() + 8;
        for (auto& b : spec_->blocks) {
            if (b.newColumn) {
                currleft += maxwidth + 8;
                currtop = trc.Height() + 8;
                maxwidth = 0;
            }
            auto lasttop = currtop;
            auto* pan = new CCommonPanel;
            std::wstring bname = U2W(b.name);
            pan->Create(tabctrl_.m_hWnd, 0, _T(""), WS_VISIBLE | WS_CHILD | WS_BORDER);
            pan->SetFont(fnt2_);
            pan->MoveWindow(CRect(currleft, currtop, currleft + 500, currtop + 500));
            cpanels_.push_back(pan);

            SIZE sz;
            dc2.GetTextExtent(bname.c_str(), bname.length(), &sz);
            unsigned cwidth = sz.cx > 108 ? sz.cx - 108 : 0;
            cwidth = std::max(120, (int)cwidth);
            for (auto& p : b.patches) {
                SIZE lsz;
                auto pname = U2W(p.name);
                dc.GetTextExtent(pname.c_str(), pname.length(), &lsz);
                cwidth = std::max((unsigned)lsz.cx, cwidth);
            }

            maxwidth = std::max(maxwidth, cwidth + 16 + 100 + 8);
            auto* lab = new CStatic;
            lab->Create(pan->m_hWnd, 0, bname.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER);
            lab->SetFont(fnt2_);
            lab->MoveWindow(CRect(8, 8, 8 + maxwidth - 16, 8 + 30));
            labels_.push_back(lab);
            currtop += 30 + 10;
            unsigned cleft = 8 + cwidth + 8;
            for (auto& p : b.patches) {
                auto* lab2 = new CStatic;
                auto pname = U2W(p.name);
                lab2->Create(pan->m_hWnd, 0, pname.c_str(), WS_VISIBLE | WS_CHILD | SS_RIGHT);
                lab2->SetFont(fnt_);
                lab2->MoveWindow(CRect(8, currtop - lasttop, 8 + cwidth, currtop - lasttop + 20));
                labels_.push_back(lab2);
                if (p.type == PT_BYTES) {
                    auto* chb = new CButton;
                    chb->Create(pan->m_hWnd, CRect(8 + cwidth + 8, currtop - lasttop - 2, 8 + cwidth + 8 + 18, currtop - lasttop - 2 + 18), _T(""), WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, IDC_BYTESBASE + p.offset);
                    chb->SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)&p);
                    checkboxes_.push_back(chb);
                } else {
                    auto* teb = new CEdit;
                    teb->Create(pan->m_hWnd, CRect(8 + cwidth + 8, currtop - lasttop - 2, 8 + cwidth + 8 + 100, currtop - lasttop - 2 + 17), _T(""), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | ES_LEFT | ES_NOIME, 0, IDC_EDITBASE + p.index);
                    teb->SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)&p);
                    teb->SetFont(fnt_);
                    textboxes_.push_back(teb);
                    assigns_[p.index] = teb;
                }
                currtop += 20 + 4;
            }
            CRect rc;
            pan->GetWindowRect(rc);
            ScreenToClient(rc);
            rc.right = rc.left + maxwidth;
            rc.bottom = rc.top + currtop - lasttop;
            pan->MoveWindow(rc);
            maxheight = std::max(maxheight, currtop);
            currtop += 4;
        }
        CRect wrc, rc;
        GetWindowRect(wrc);
        GetClientRect(rc);
        rc.right = rc.left + currleft + maxwidth + 8;
        rc.bottom = rc.top + maxheight + 16;
        tabctrl_.MoveWindow(CRect(0, 0, rc.Width(), rc.Height()));
        AdjustWindowRectEx(rc, GetStyle() & ~WS_OVERLAPPED, FALSE, GetExStyle());
        MoveWindow(CRect(wrc.left, wrc.top, wrc.left + rc.Width(), wrc.top + rc.Height()));
        LoadPlugins();
        tabctrl_.SetCurSel(0);
        lastsel_ = 0;
    }
}

void CMainWnd::Update() {
    if (!CheckProcess()) return;
    if (!tabctrl_.IsWindow()) return;
    auto sel = tabctrl_.GetCurSel();
    if (sel < 0) return;
    if (sel > 0) {
        if ((size_t)sel <= pluginsEnabled_.size()) {
            auto& plugin = pluginsEnabled_[sel - 1];
            plugin.first->Tick(plugin.second);
        }
        return;
    }
    for (auto& b : spec_->blocks) {
        for (auto& p : b.patches) {
            std::string val;
            switch (p.type) {
            case PT_U8:
                val = std::to_string(gProcEdit.Read<uint8_t>(p.pos == PP_MEM, p.offset));
                break;
            case PT_U16:
                val = std::to_string(gProcEdit.Read<uint16_t>(p.pos == PP_MEM, p.offset));
                break;
            case PT_U32:
                val = std::to_string(gProcEdit.Read<uint32_t>(p.pos == PP_MEM, p.offset));
                break;
            case PT_U64:
                val = std::to_string(gProcEdit.Read<uint64_t>(p.pos == PP_MEM, p.offset));
                break;
            case PT_I8:
                val = std::to_string((int32_t)gProcEdit.Read<int8_t>(p.pos == PP_MEM, p.offset));
                break;
            case PT_I16:
                val = std::to_string(gProcEdit.Read<int16_t>(p.pos == PP_MEM, p.offset));
                break;
            case PT_I32:
                val = std::to_string(gProcEdit.Read<int32_t>(p.pos == PP_MEM, p.offset));
                break;
            case PT_I64:
                val = std::to_string(gProcEdit.Read<int64_t>(p.pos == PP_MEM, p.offset));
                break;
            default:
                continue;
            }
            auto* lab = assigns_[p.index];
            if (lab == nullptr) continue;
            char n[256];
            GetWindowTextA(lab->m_hWnd, n, 256);
            if (val != n) {
                auto l = lab->GetWindowLongPtr(GWLP_USERDATA);
                lab->SetWindowLongPtrW(GWLP_USERDATA, 0);
                SetWindowTextA(lab->m_hWnd, val.c_str());
                lab->SetWindowLongPtrW(GWLP_USERDATA, l);
            }
        }
    }
}

bool CMainWnd::CheckProcess() {
    int r = gProcEdit.Check();
    if (r > 0) return true;
    spec_ = nullptr;
    std::map<std::string, PatchSpec>* ver = nullptr;
    for (auto& p : gPatch.versions) {
        gProcEdit.Find(p.first);
        if (gProcEdit.Found()) {
            ver = &p.second;
            break;
        }
    }
    if (!ver) {
        if (r < 0) BuildForm();
        UpdateTitle();
        return false;
    }
    auto ite = ver->find(gProcEdit.GetVersion());
    if (ite == ver->end()) {
        if (!labels_.empty())
            SetWindowTextA(labels_.back()->m_hWnd, ("Unsupported version: " + gProcEdit.GetVersion()).c_str());
        UpdateTitle();
        return false;
    }
    spec_ = &ite->second;
    UpdateTitle();
    gProcEdit.UpdateAddr(spec_->memAddr);
#ifdef _DEBUG
    gProcEdit.DumpMemory();
#endif
    BuildForm();
    return true;
}

void CMainWnd::LoadPlugins() {
	UnloadPlugins();
    for (auto& p : plugins_) {
		if (p.first->Enable(&gProcEdit, tabctrl_.m_hWnd)) {
            for (uint32_t i = 0; i < p.second; ++i) {
                tabctrl_.AddItem(p.first->GetName(i));
                pluginsEnabled_.push_back(std::make_pair(p.first, i));
            }
		}
    }
}

void CMainWnd::UnloadPlugins() {
    for (auto& p : pluginsEnabled_)
        if (p.second == 0)
            p.first->Disable();
	pluginsEnabled_.clear();
    if (!tabctrl_.IsWindow()) return;
    auto count = tabctrl_.GetItemCount();
    while (count > 1) {
        tabctrl_.DeleteItem(count - 1);
        count = tabctrl_.GetItemCount();
    }
}

void CMainWnd::UpdateTitle() {
    TCHAR path[256];
    TCHAR ver[256];
    GetModuleFileName(NULL, path, 256);
    GetFileVersion(path, ver);
    TCHAR title[256];
    if (spec_ == nullptr)
        wsprintf(title, L"Zetta! v%s", ver);
    else
        wsprintf(title, L"Zetta! v%s - %s", ver, spec_->name.c_str());
    SetWindowText(title);
}
