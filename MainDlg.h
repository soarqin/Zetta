#pragma once

#include "Zetta!.h"

#include "Patch.h"

#define IDT_UPDATE 0x2001
#define IDC_BYTESBASE 0x2801
#define IDC_EDITBASE 0x3001

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter
{
public:
	enum { IDD = IDD_DIS1MAIN };

    CMainDlg();

    ~CMainDlg();

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return ::IsDialogMessage(m_hWnd, pMsg);
	}

    LRESULT OnInitDialog(HWND /*wParam*/, LPARAM /*lParam*/);

    void OnCloseDialog();

    void OnTimer(UINT_PTR);

    LRESULT OnBytes(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
    void Clear();
    void BuildForm();
    void Update();
    bool CheckProcess();

private:
    std::vector<CStatic*> pn;
    std::vector<CStatic*> lb;
    std::vector<CEdit*> tb;
    std::vector<CButton*> cb;
    std::map<uint32_t, CEdit*> assigns;
    PatchSpec* spec = nullptr;
    CFont fnt, fnt2;

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnCloseDialog)
        MSG_WM_TIMER(OnTimer)
        COMMAND_RANGE_HANDLER(IDC_BYTESBASE, IDC_BYTESBASE + 0x7FF, OnBytes);
        COMMAND_RANGE_HANDLER(IDC_EDITBASE, IDC_EDITBASE + 0x7FF, OnEdit);
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
