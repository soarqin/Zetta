#include "pch.h"

#include "MainDlg.h"

#include "ProcEdit.h"

std::wstring U2W(const std::string& n) {
    std::wstring r;
    int sz = ::MultiByteToWideChar(CP_UTF8, 0, n.c_str(), n.length(), NULL, 0);
    r.resize(sz);
    ::MultiByteToWideChar(CP_UTF8, 0, n.c_str(), n.length(), &r[0], sz);
    return r;
}

CMainDlg::CMainDlg() {
}

CMainDlg::~CMainDlg() {
}

LRESULT CMainDlg::OnInitDialog(HWND, LPARAM) {
    // center the dialog on the screen
    CenterWindow();

    TCHAR path[256];
    TCHAR ver[256];
    GetModuleFileName(NULL, path, 256);
    GetFileVersion(path, ver);
    {
        TCHAR title[256];
        GetWindowText(title, 256);
        lstrcat(title, _T(" v"));
        lstrcat(title, ver);
        SetWindowText(title);
    }

    // set icons
    /*
    HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON),
    IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    SetIcon(hIcon, TRUE);
    HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON),
    IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    SetIcon(hIconSmall, FALSE);
    */

    CFontHandle fh = this->GetFont();
    CLogFont lf;
    fh.GetLogFont(&lf);
    lstrcpy(lf.lfFaceName, _T("SimSun"));
    lf.lfHeight = -12;
    fnt.CreateFontIndirect(&lf);
    lf.lfHeight = -16;
    lf.lfWeight = FW_BOLD;
    fnt2.CreateFontIndirect(&lf);

    BuildForm();
    Update();

    SetTimer(IDT_UPDATE, 1000, NULL);

    return TRUE;
}

void CMainDlg::OnCloseDialog() {
    DestroyWindow();
    ::PostQuitMessage(0);
}

void CMainDlg::OnTimer(UINT_PTR) {
    Update();
}

LRESULT CMainDlg::OnBytes(WORD nNotifyCode, WORD nID, HWND hWnd, BOOL &) {
    if (nNotifyCode != BN_CLICKED) return 0;
    PatchAddr* p = (PatchAddr*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (p == NULL) return 0;
    if (IsDlgButtonChecked(nID)) {
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

LRESULT CMainDlg::OnEdit(WORD nNotifyCode, WORD nID, HWND hWnd, BOOL &) {
    if (nNotifyCode != EN_CHANGE) return 0;
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

void CMainDlg::Clear() {
    for (auto& p : pn) {
        p->DestroyWindow();
        delete p;
    }
    pn.clear();
    for (auto& l : lb) {
        l->DestroyWindow();
        delete l;
    }
    lb.clear();
    for (auto& t : tb) {
        t->DestroyWindow();
        delete t;
    }
    tb.clear();
    for (auto& c : cb) {
        c->DestroyWindow();
        delete c;
    }
    cb.clear();
}

void CMainDlg::BuildForm() {
    Clear();
    if (spec == nullptr) {
        CDC dc = CreateCompatibleDC(GetDC());
        dc.SelectFont(fnt);
        
        auto* lab = new CStatic;
        const TCHAR* caption = _T("Waiting for game...");
        lab->Create(m_hWnd, 0, caption, WS_VISIBLE | WS_CHILD | SS_CENTER);
        lab->SetFont(fnt);
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
        lb.push_back(lab);
    } else {
        CDC dc = CreateCompatibleDC(GetDC()), dc2 = CreateCompatibleDC(GetDC());
        dc.SelectFont(fnt);
        dc2.SelectFont(fnt2);

        unsigned maxheight = 0, maxwidth = 0;
        unsigned currleft = 0, currtop = 8;
        for (auto& b : spec->blocks) {
            if (b.newColumn) {
                currleft += maxwidth + 8;
                currtop = 8;
                maxwidth = 0;
            }
            auto lasttop = currtop;
            auto* pan = new CStatic;
            std::wstring bname = U2W(b.name);
            pan->Create(m_hWnd, 0, _T(""), WS_VISIBLE | WS_CHILD | WS_BORDER);
            pan->SetFont(fnt2);
            pan->MoveWindow(CRect(currleft, currtop, currleft + 500, currtop + 500));
            pn.push_back(pan);

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
            lab->Create(m_hWnd, 0, bname.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER);
            lab->SetFont(fnt2);
            lab->MoveWindow(CRect(currleft + 8, currtop + 8, currleft + 8 + maxwidth - 16, currtop + 8 + 30));
            lb.push_back(lab);
            currtop += 30 + 4;
            unsigned cleft = 8 + cwidth + 8;
            for (auto& p : b.patches) {
                auto* lab2 = new CStatic;
                auto pname = U2W(p.name);
                lab2->Create(m_hWnd, 0, pname.c_str(), WS_VISIBLE | WS_CHILD | SS_RIGHT);
                lab2->SetFont(fnt);
                lab2->MoveWindow(CRect(currleft + 8, currtop, currleft + 8 + cwidth, currtop + 20));
                lb.push_back(lab2);
                if (p.type == PT_BYTES) {
                    auto* chb = new CButton;
                    chb->Create(m_hWnd, CRect(currleft + 8 + cwidth + 8, currtop - 2, currleft + 8 + cwidth + 8 + 20, currtop - 2 + 20), _T(""), WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, IDC_BYTESBASE + p.offset);
                    chb->SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)&p);
                    cb.push_back(chb);
                } else {
                    auto* teb = new CEdit;
                    teb->Create(m_hWnd, CRect(currleft + 8 + cwidth + 8, currtop - 2, currleft + 8 + cwidth + 8 + 100, currtop - 2 + 17), _T(""), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | ES_LEFT | ES_NOIME, 0, IDC_EDITBASE + p.index);
                    teb->SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)&p);
                    teb->SetFont(fnt);
                    tb.push_back(teb);
                    assigns[p.index] = teb;
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
        CRect rc;
        GetWindowRect(rc);
        rc.right = rc.left + currleft + maxwidth + 8;
        rc.bottom = rc.top + maxheight + 16;
        AdjustWindowRectEx(rc, GetStyle() & ~WS_OVERLAPPED, FALSE, GetExStyle());
        MoveWindow(rc);
    }
}

void CMainDlg::Update() {
    if (!CheckProcess()) return;
    for (auto& b : spec->blocks) {
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
            auto* lab = assigns[p.index];
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

bool CMainDlg::CheckProcess() {
    int r = gProcEdit.Check();
    if (r > 0) return true;
    spec = nullptr;
    gProcEdit.Find(gPatch.className);
    if (!gProcEdit.Found()) {
        if (r < 0) BuildForm();
        return false;
    }
    auto ite = gPatch.versions.find(gProcEdit.GetVersion());
    if (ite == gPatch.versions.end()) {
        if (!lb.empty())
            SetWindowTextA(lb.back()->m_hWnd, ("Unsupported version: " + gProcEdit.GetVersion()).c_str());
        return false;
    }
    spec = &ite->second;
    gProcEdit.UpdateAddr(spec->memAddr);
#ifdef _DEBUG
    gProcEdit.DumpMemory();
#endif
    BuildForm();
    return true;
}
