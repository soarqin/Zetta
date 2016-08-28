#pragma once

#include "Patch.h"
#include "IPlugin.h"

#define IDT_UPDATE 1200
#define IDC_TABS 1201
#define IDC_BYTESBASE 1501
#define IDC_EDITBASE 2001

class CCommonPanel : public CWindowImpl<CCommonPanel, CStatic> {
public:
    LRESULT OnBytes(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    BEGIN_MSG_MAP_EX(CMainWnd)
        COMMAND_RANGE_HANDLER(IDC_BYTESBASE, IDC_BYTESBASE + 500, OnBytes)
        COMMAND_RANGE_HANDLER(IDC_EDITBASE, IDC_EDITBASE + 500, OnEdit)
    END_MSG_MAP()
};

class CMainWnd : public CWindowImpl<CMainWnd>, public CMessageFilter
{
public:
    DECLARE_WND_CLASS(_T("ZettaWnd"))

    CMainWnd();

    ~CMainWnd();

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return ::IsDialogMessage(m_hWnd, pMsg);
	}

    LRESULT OnCreate(LPCREATESTRUCT);
    void OnDestroy();
    void OnCloseDialog();
    BOOL OnEraseBkGnd(HDC);
    LRESULT OnTabChange(int, LPNMHDR, BOOL&);

    void OnTimer(UINT_PTR);

private:
    void Clear();
    void BuildForm();
    void Update();
    bool CheckProcess();
    void LoadPlugins();
    void UnloadPlugins();
    void ShowPlugin(size_t index, bool show);

private:
    std::vector<CCommonPanel*> cpanels_;
    std::vector<CStatic*> labels_;
    std::vector<CEdit*> textboxes_;
    std::vector<CButton*> checkboxes_;
    CTabCtrl tabctrl_;
    std::map<uint32_t, CEdit*> assigns_;
    PatchSpec* spec_ = nullptr;
    CFont fnt_, fnt2_, fnt3_;
    int lastsel_ = -1;
    std::vector<IPlugin*> plugins_;

    BEGIN_MSG_MAP_EX(CMainWnd)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_ERASEBKGND(OnEraseBkGnd)
        MSG_WM_CLOSE(OnCloseDialog)
        MSG_WM_TIMER(OnTimer)
        NOTIFY_HANDLER(IDC_TABS, TCN_SELCHANGE, OnTabChange);
        /*
        COMMAND_ID_HANDLER(IDC_BTN_SELFILE, OnSelFile)
        COMMAND_ID_HANDLER(IDC_BTN_DELFILE, OnDelFile)
        COMMAND_ID_HANDLER(IDC_BTN_MOVEUP, OnMoveUp)
        COMMAND_ID_HANDLER(IDC_BTN_MOVEDOWN, OnMoveDown)
        COMMAND_ID_HANDLER(IDC_BTN_CLEAR, OnClear)
        COMMAND_ID_HANDLER(IDC_BTN_GO, OnStart)
        COMMAND_HANDLER_EX(IDC_FILELIST, LBN_SELCHANGE, OnListSelChanged)
        */
    END_MSG_MAP()

};
