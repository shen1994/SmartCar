#pragma once
#include "afxwin.h"


// Statement dialog

class Statement : public CDialogEx
{
	DECLARE_DYNAMIC(Statement)

public:
	Statement(CWnd* pParent = NULL);   // standard constructor
	virtual ~Statement();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
public:
	CListBox StatementPtr;
};
