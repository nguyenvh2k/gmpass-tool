
// gmpasstoolDlg.h : header file
//

#pragma once


// CgmpasstoolDlg dialog
class CgmpasstoolDlg : public CDialogEx
{
// Construction
public:
	CgmpasstoolDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GMPASSTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedEncrypt();
	afx_msg void OnBnClickedDecrypt();
	afx_msg void OnBnClickedClear();
	afx_msg void OnEnSetfocusOutput();
	afx_msg void OnEnSetfocusMd5();
	afx_msg LRESULT OnSelectAllText(WPARAM wParam, LPARAM lParam);

	void RunPasswordAction(bool encode);
	DECLARE_MESSAGE_MAP()
};
