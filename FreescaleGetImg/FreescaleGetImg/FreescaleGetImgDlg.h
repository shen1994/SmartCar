
// FreescaleGetImgDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "mscomm1.h"
#include "Statement.h"
#include "Image.h"

// CFreescaleGetImgDlg dialog
class CFreescaleGetImgDlg : public CDialogEx
{
// Construction
public:
	CFreescaleGetImgDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FREESCALEGETIMG_DIALOG };

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
	DECLARE_MESSAGE_MAP()
public:
	void ShowBrowseImg(CImage ImageOrigin, CString LoadPath);
	void GetPicData(CImage &Image);
	void ShowImageFromArray(unsigned char** Image, int Width, int Height, CImage &ImageOrigin);
	void ImageScale(int Pix, CImage &SrcImage, CImage &DstImage);
	void ShowScaleImage(CImage &ImageOrigin);
	void ImageSaveToPath(CString Path);
	void DrawLeftEdge(CImage &Image);
	void DrawCenterEdge(CImage &Image);
	void DrawRightEdge(CImage &Image);
	void ChangeArray(CString SrcStr);
	void FloatToString(float Err, CString& Str);
	void ShowPicSomeData(void);
	void ReviseTransDistortion(unsigned char PicPoint[][CAMERA_W]);
	void CalLinearEquation(int Point[], float *a, float* b);
	void ReviseLongiDistortion(unsigned char PicPoint[][CAMERA_W]);
	CComboBox SeriesCom;
	CComboBox SeriesBaud;
	CMscomm1 ComLink;
	CListBox StateShow;
	CListBox PathShow;
	CButton Button_ImageDeal;
	CButton ButtonRunPtr;
	CEdit ImageNumPtr;
	CEdit PicErrorPtr;
	CEdit IsStartPtr;
	CEdit IsRampPtr;
	CEdit IsObstaclePtr;
	CEdit SlopePtr;
	CStatic LeftStartPtr;
	CStatic RightStartPtr;
	CStatic EndLinePtr;
	CStatic PicControlPtr;
	void OnCommMscomm1();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	afx_msg void OnBnClickedButton16();
	afx_msg void OnBnClickedButton17();
	afx_msg void OnStnClickedImg();
	DECLARE_EVENTSINK_MAP()
};

