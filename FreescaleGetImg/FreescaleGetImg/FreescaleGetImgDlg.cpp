
// FreescaleGetImgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FreescaleGetImg.h"
#include "FreescaleGetImgDlg.h"
#include "afxdialogex.h"
#include <atlimage.h>


unsigned char ControlUI = 0;
unsigned char ImageDealUI = 0;
unsigned char BroseImageUI = 0;
unsigned char ClearImageUI = 0;
unsigned char SaveFlagEdge[3] = { 0 };//确认保存图像颜色
unsigned char isImage = 0;
unsigned char isCommOpen = 0;
#define ImagePix 6
BYTE ImageBuffer[CAMERA_H*CAMERA_W + 1] = { 0 };

CString SavePath(_T("ImageSave/"));
char PathArray[100] = { 0 };
int ImageNum = 0;
CImage ImageSrc;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()
// CFreescaleGetImgDlg dialog
CFreescaleGetImgDlg::CFreescaleGetImgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFreescaleGetImgDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFreescaleGetImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO3, SeriesCom);
	DDX_Control(pDX, IDC_COMBO4, SeriesBaud);
	DDX_Control(pDX, IDC_MSCOMM1, ComLink);
	DDX_Control(pDX, IDC_LIST1, StateShow);
	DDX_Control(pDX, IDC_LIST2, PathShow);
	DDX_Control(pDX, IDC_BUTTON7, Button_ImageDeal);
	DDX_Control(pDX, IDC_EDIT1, ImageNumPtr);
	DDX_Control(pDX, IDC_EDIT6, IsObstaclePtr);
	DDX_Control(pDX, IDC_EDIT3, PicErrorPtr);
	DDX_Control(pDX, IDC_EDIT4, IsStartPtr);
	DDX_Control(pDX, IDC_EDIT5, IsRampPtr);
	DDX_Control(pDX, IDC_STATIC1, LeftStartPtr);
	DDX_Control(pDX, IDC_STATIC2, RightStartPtr);
	DDX_Control(pDX, IDC_STATIC3, EndLinePtr);
	DDX_Control(pDX, IDC_BUTTON14, ButtonRunPtr);
	DDX_Control(pDX, IDC_IMG, PicControlPtr);
	DDX_Control(pDX, IDC_EDIT7, SlopePtr);
}

BEGIN_MESSAGE_MAP(CFreescaleGetImgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON5, &CFreescaleGetImgDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON2, &CFreescaleGetImgDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON7, &CFreescaleGetImgDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON4, &CFreescaleGetImgDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON3, &CFreescaleGetImgDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON6, &CFreescaleGetImgDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON1, &CFreescaleGetImgDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON8, &CFreescaleGetImgDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CFreescaleGetImgDlg::OnBnClickedButton9)
	ON_STN_CLICKED(IDC_IMG, &CFreescaleGetImgDlg::OnStnClickedImg)
	ON_BN_CLICKED(IDC_BUTTON10, &CFreescaleGetImgDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CFreescaleGetImgDlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, &CFreescaleGetImgDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &CFreescaleGetImgDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &CFreescaleGetImgDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON16, &CFreescaleGetImgDlg::OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON17, &CFreescaleGetImgDlg::OnBnClickedButton17)
END_MESSAGE_MAP()

/**
* @breif 控件初始化
**/
BOOL CFreescaleGetImgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//串口配置
	int ComCount = 0;
	CString ComNum;
	CString ComStr("COM");
	CString ComAll;
	for (int i = 0; i < 15; i++){
		ComCount++;
		ComNum.Format(_T("%d"), ComCount);
		ComAll = ComStr + ComNum;
		SeriesCom.InsertString(i, ComAll);
	}
	SeriesCom.SetCurSel(0);

	SeriesBaud.InsertString(0,_T("115200"));
	SeriesBaud.InsertString(1,_T("9600"));
	SeriesBaud.SetCurSel(0);

	//图像序号
	CFont font;
	font.CreatePointFont(280, _T("宋体"));
	ImageNumPtr.SetFont(&font);
	ImageNumPtr.SetWindowText(_T("0"));
	EndLinePtr.SetFont(&font);
	EndLinePtr.SetWindowText(_T("0"));
	LeftStartPtr.SetFont(&font);
	LeftStartPtr.SetWindowText(_T("0"));
	RightStartPtr.SetFont(&font);
	RightStartPtr.SetWindowText(_T("0"));
	PicErrorPtr.SetFont(&font);
	PicErrorPtr.SetWindowText(_T("0"));
	IsStartPtr.SetFont(&font);
	IsStartPtr.SetWindowText(_T("0"));
	IsRampPtr.SetFont(&font);
	IsRampPtr.SetWindowText(_T("0"));
	IsObstaclePtr.SetFont(&font);
	IsObstaclePtr.SetWindowText(_T("0"));
	SlopePtr.SetFont(&font);
	SlopePtr.SetWindowText(_T("0"));
	//字体大小
	StateShow.SetFont(&font);
	PathShow.SetFont(&font);
	PathShow.ResetContent();
	PathShow.InsertString(0, SavePath);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFreescaleGetImgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFreescaleGetImgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	if (ControlUI == 1){
		ControlUI = 0;
		CImage ImageDes;
		if (!ImageDes.IsNull()){
			ImageDes.Destroy();
		}
		ShowImageFromArray((unsigned char**)ov7725_image, CAMERA_W, CAMERA_H, ImageSrc);
		if (1 == SaveFlagEdge[0]) DrawLeftEdge(ImageSrc);
		if (1 == SaveFlagEdge[1]) DrawCenterEdge(ImageSrc);
		if (1 == SaveFlagEdge[2]) DrawRightEdge(ImageSrc);
		ImageScale(ImagePix, ImageSrc, ImageDes);
		ShowScaleImage(ImageDes);
	}

	if (ImageDealUI == 1){
		ImageDealUI = 0;
		DrawLeftEdge(ImageSrc);
		DrawCenterEdge(ImageSrc);
		DrawRightEdge(ImageSrc);
		CImage ImageDes;
		if (!ImageDes.IsNull()){
			ImageDes.Destroy();
		}
		ImageScale(ImagePix, ImageSrc, ImageDes);
		ShowScaleImage(ImageDes);
	}

	if (BroseImageUI == 1){
		BroseImageUI = 0;
		CImage ImageDes;
		if (!ImageDes.IsNull()){
			ImageDes.Destroy();
		}
		ShowImageFromArray((unsigned char**)ov7725_image, CAMERA_W, CAMERA_H, ImageSrc);
		ImageScale(ImagePix, ImageSrc, ImageDes);
		ShowScaleImage(ImageDes);
	}

	if (1 == ClearImageUI){
		ClearImageUI = 0;
		CImage ImageDes;
		if (!ImageDes.IsNull()){
			ImageDes.Destroy();
		}
		ShowImageFromArray((unsigned char**)ov7725_image, CAMERA_W, CAMERA_H, ImageSrc);
		if (1 == SaveFlagEdge[0]) DrawLeftEdge(ImageSrc);
		if (1 == SaveFlagEdge[1]) DrawCenterEdge(ImageSrc);
		if (1 == SaveFlagEdge[2]) DrawRightEdge(ImageSrc);
		ImageScale(ImagePix, ImageSrc, ImageDes);
		ShowScaleImage(ImageDes);
	}
}
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFreescaleGetImgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*------------控件接口-----------------*/
/////////////////////////////////////////////
/**
* @breif 串口接收中断
**/
BEGIN_EVENTSINK_MAP(CFreescaleGetImgDlg, CDialogEx)
	ON_EVENT(CFreescaleGetImgDlg, IDC_MSCOMM1, 1, CFreescaleGetImgDlg::OnCommMscomm1, VTS_NONE)
END_EVENTSINK_MAP()
void CFreescaleGetImgDlg::OnCommMscomm1()
{
#define BufferSize 1024
#define Image_H CAMERA_H
#define Image_W CAMERA_W
	//数据超出引发中断,重新计数,发送进中断
	static long ImageBufferCount = 0;
	static long FullCount = 0, OtherCount = 0;
	int Count = 0;
	char FullFlag = 0;

	if (ComLink.get_CommEvent() == 2){
		unsigned char StrBuffer[BufferSize] = { 0 };
		VARIANT InputData = ComLink.get_Input();
		COleSafeArray C_InputData = InputData;
			
		/*缓冲区存满*/
		if (C_InputData.GetOneDimSize() == BufferSize){
			for (long i = 0; i < BufferSize; i++){
				C_InputData.GetElement(&i, StrBuffer + i);
				ImageBuffer[ImageBufferCount] = StrBuffer[i];
				ImageBufferCount++;
			}
			FullCount++;
		}

		/*缓冲区未满*/
		if (C_InputData.GetOneDimSize() != BufferSize){
			for (long i = 0; i < (long)(C_InputData.GetOneDimSize()); i++){
				C_InputData.GetElement(&i, StrBuffer + i);
				ImageBuffer[ImageBufferCount] = StrBuffer[i];
				ImageBufferCount++;
				OtherCount++;
			}
		}
	
		/*读取图像数组*/
		if ((FullCount*BufferSize + OtherCount) >= (Image_H*Image_W)){

			for (int i = 0; i < Image_H; i++){
				for (int j = 0; j < Image_W; j++){
					ov7725_image[i][j] = ImageBuffer[Count];
					Count++;
				}
			}
			//清零
			ImageBufferCount = 0;
			FullCount = 0;
			OtherCount = 0;
			
			//跟新图像
			ControlUI = 1;
			Invalidate();
			UpdateWindow();
		}
	}
}
/////////////////////////////////////////////
/**
* @breif Picture点击触发事件
**/
void CFreescaleGetImgDlg::OnStnClickedImg()
{
	CPoint Point;
	CRect PosRect;
	GetCursorPos(&Point);
	PicControlPtr.GetWindowRect(PosRect);
	Point.y = (Point.y - PosRect.top) / ImagePix;
	Point.x = (Point.x - PosRect.left) / ImagePix;
	CString PosX, PosY;
	PosY.Format(_T("%d"), CAMERA_H - 1 - Point.y);
	PosX.Format(_T("%d"), Point.x);
	PosY = _T("Row------") + PosY;
	PosX = _T("Col------") + PosX;
	StateShow.ResetContent();
	StateShow.InsertString(0, PosY);
	StateShow.InsertString(1, PosX);
}
/////////////////////////////////////////////
/**
* @breif 图像处理
**/
void CFreescaleGetImgDlg::OnBnClickedButton7()
{
	if (0 == isImage) return;
	if (1 == SaveFlagEdge[0]
		|| 1 == SaveFlagEdge[1]
		|| 1 == SaveFlagEdge[2])
		return;
	GetPicData(ImageSrc);
	ImageFinshed();

	ShowPicSomeData();

	//跟新图像
	ImageDealUI = 1;
	Invalidate();
	UpdateWindow();

	SaveFlagEdge[0] = 1;	
	SaveFlagEdge[1] = 1;
	SaveFlagEdge[2] = 1;//保存图像处理后的图像
}
/////////////////////////////////////////////
/**
* @breif 加载图像保存路径
**/
void CFreescaleGetImgDlg::OnBnClickedButton4()
{
	ImageNum = 0;
	BROWSEINFO binfo;
	ZeroMemory(&binfo, sizeof(binfo));
	binfo.lpszTitle = _T("Choose One Path!");
	binfo.ulFlags = BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST lpDlist;
	lpDlist = SHBrowseForFolder(&binfo);
	if (NULL != lpDlist){
		TCHAR SavePathTemp[200] = { 0 };
		SHGetPathFromIDList(lpDlist, SavePathTemp);
		SavePath = SavePathTemp;
		SavePath.Replace('\\', '/');
		SavePath += '/';
		PathShow.ResetContent();
		PathShow.InsertString(0, SavePath);
	}
}
/////////////////////////////////////////////
/**
* @breif 程序说明
**/
void CFreescaleGetImgDlg::OnBnClickedButton3()
{
	Statement StatementWindow;
	StatementWindow.DoModal();
}
/////////////////////////////////////////////
/**
* @brief 清除图像显示
**/
void CFreescaleGetImgDlg::OnBnClickedButton6()
{
	if (0 == isImage) return;

	SaveFlagEdge[0] = 0;
	SaveFlagEdge[1] = 0;
	SaveFlagEdge[2] = 0;

	ClearImageUI = 1;
	Invalidate();
	UpdateWindow();
}
/////////////////////////////////////////////
/**
* @breif 清除左边界线
**/
void CFreescaleGetImgDlg::OnBnClickedButton10()
{
	if (0 == isImage) return;
	SaveFlagEdge[0] = 0;
	ClearImageUI = 1;
	Invalidate();
	UpdateWindow();
}
/////////////////////////////////////////////
/**
* @breif 清除中间边界线
**/
void CFreescaleGetImgDlg::OnBnClickedButton11()
{
	if (0 == isImage) return;
	SaveFlagEdge[1] = 0;
	ClearImageUI = 1;
	Invalidate();
	UpdateWindow();
}
/////////////////////////////////////////////
/**
* @breif 清除右边界线
**/
void CFreescaleGetImgDlg::OnBnClickedButton1()
{
	if (0 == isImage) return;
	SaveFlagEdge[2] = 0;
	ClearImageUI = 1;
	Invalidate();
	UpdateWindow();
}
/////////////////////////////////////////////
/**
* @breif 获取图像
**/
void CFreescaleGetImgDlg::OnBnClickedButton5()
{
	if (0 == isCommOpen) return;

	isImage = 1;
	SaveFlagEdge[0] = 0;
	SaveFlagEdge[1] = 0;
	SaveFlagEdge[2] = 0;
	for (int i = 0; i < (CAMERA_H*CAMERA_W + 1); i++)
		ImageBuffer[i] = 0;
	ComLink.put_Output(COleVariant(_T("S")));
}
/////////////////////////////////////////////
/**
* @breif 打开串口
**/
void CFreescaleGetImgDlg::OnBnClickedButton2()
{
	static char OpenFlag = 0;
	CString Button_OpenStr;
	GetDlgItemText(IDC_BUTTON2, Button_OpenStr);
	CWnd *Button_OpenPtr = NULL;
	Button_OpenPtr = GetDlgItem(IDC_BUTTON2);
	if (!ComLink.get_PortOpen()){
		CString Com_ConfigStr;
		CString BaudboxStr;
		int ComxboxNum;

		SeriesBaud.GetLBText(SeriesBaud.GetCurSel(), BaudboxStr);
		ComxboxNum = SeriesCom.GetCurSel() + 1;

		Com_ConfigStr = BaudboxStr + ',' + 'n' + '8' + ',' + '1';
		ComLink.put_CommPort(ComxboxNum);
		ComLink.put_InputMode(1);	//二进制
		ComLink.put_Settings(Com_ConfigStr);
		ComLink.put_InputLen(1024);
		ComLink.put_RThreshold(1); //一个字节引发中断
		ComLink.put_RTSEnable(1);

		ComLink.put_PortOpen(true);
		Button_OpenStr = _T("Close");
		UpdateData(true);
		Button_OpenPtr->SetWindowText(Button_OpenStr);
		isCommOpen = 1;
	}
	else{
		ComLink.put_PortOpen(false);
		Button_OpenStr = _T("Open");
		UpdateData(true);
		Button_OpenPtr->SetWindowText(Button_OpenStr);
		isCommOpen = 0;
	}
}
/////////////////////////////////////////////
/*
* @breif 加载Image
**/
void CFreescaleGetImgDlg::OnBnClickedButton8()
{
	CString BrowsePath;
	CString PicName;
	CString PicNameDo;
	ImageNumPtr.GetWindowText(PicName);
	PicNameDo = _T("BipMap") + PicName + _T(".bmp");
	BrowsePath = SavePath + PicNameDo;

	CImage ImageTemp;
	if (!ImageTemp.IsNull()){
		ImageTemp.Destroy();
	}
	ImageTemp.Load(BrowsePath);	
	if (ImageTemp.IsNull()) return;
	GetPicData(ImageTemp);

	BroseImageUI = 1;
	Invalidate();
	UpdateWindow();


	isImage = 1;

	SaveFlagEdge[0] = 0;
	SaveFlagEdge[1] = 0;
	SaveFlagEdge[2] = 0;
}
/////////////////////////////////////////////
/**
* @breif 保存图像
**/
void CFreescaleGetImgDlg::OnBnClickedButton9()
{
	if (0 == isImage) return;
	/*跟新图像显示控件*/
	ShowImageFromArray((unsigned char**)ov7725_image, CAMERA_W, CAMERA_H, ImageSrc);
	if (1 == SaveFlagEdge[0]) DrawLeftEdge(ImageSrc);
	if (1 == SaveFlagEdge[1]) DrawCenterEdge(ImageSrc);
	if (1 == SaveFlagEdge[2]) DrawRightEdge(ImageSrc);
	ShowScaleImage(ImageSrc);

	CString Num;
	ImageNumPtr.GetWindowText(Num);
	CString PathTemp = SavePath + _T("BipMap") + Num + _T(".bmp");
	ImageNum = _ttoi(Num);
	ImageNum++;
	Num.Format(_T("%d"), ImageNum);
	ImageNumPtr.SetWindowTextW(Num);

	ImageSaveToPath(PathTemp);
	//跟新图像
	ControlUI = 1;
	Invalidate();
	UpdateWindow();
}
/////////////////////////////////////////////
/**
* @breif 横向畸变矫正
**/
void CFreescaleGetImgDlg::OnBnClickedButton12()
{
	//跟新图像
	SaveFlagEdge[0] = 0;
	SaveFlagEdge[1] = 0;
	SaveFlagEdge[2] = 0;

	ReviseTransDistortion(ov7725_image);

	ControlUI = 1;
	Invalidate();
	UpdateWindow();


}
/////////////////////////////////////////////
/**
* @breif 发送修正的PWM控制值
**/
void CFreescaleGetImgDlg::OnBnClickedButton13()
{
	SaveFlagEdge[0] = 0;
	SaveFlagEdge[1] = 0;
	SaveFlagEdge[2] = 0;

	ReviseLongiDistortion(ov7725_image);

	ControlUI = 1;
	Invalidate();
	UpdateWindow();
}
/////////////////////////////////////////////
/**
* @breif 控制小车跑和停
**/
void CFreescaleGetImgDlg::OnBnClickedButton14()
{
	static int isRun = 0;
	if (0 == isCommOpen) return;
	if (1 == isRun){
		isRun = 0;
		ComLink.put_Output(COleVariant(_T("R")));
		ButtonRunPtr.SetWindowText(_T("Stop"));
	}
	else{
		isRun = 1;
		ComLink.put_Output(COleVariant(_T("T")));
		ButtonRunPtr.SetWindowText(_T("Run"));
	}
}
/////////////////////////////////////////////
/**
* @breif 加载上一张图片
**/
void CFreescaleGetImgDlg::OnBnClickedButton17()
{
	CString BrowsePath;
	CString PicName;
	CString PicNameDo;
	int PicNum = 0;

	ImageNumPtr.GetWindowText(PicName);
	PicNum = _ttoi(PicName);
	PicNum--;
	if (PicNum < 0) PicNum = 0; //最小为第0张
	PicName.Empty();
	PicName.Format(_T("%d"), PicNum);
	ImageNumPtr.SetWindowTextW(PicName);

	PicNameDo = _T("BipMap") + PicName + _T(".bmp");
	BrowsePath = SavePath + PicNameDo;

	CImage ImageTemp;
	if (!ImageTemp.IsNull()){
		ImageTemp.Destroy();
	}
	ImageTemp.Load(BrowsePath);
	if (ImageTemp.IsNull()) return;
	GetPicData(ImageTemp);

	BroseImageUI = 1;
	Invalidate();
	UpdateWindow();


	isImage = 1;

	SaveFlagEdge[0] = 0;
	SaveFlagEdge[1] = 0;
	SaveFlagEdge[2] = 0;
}
/////////////////////////////////////////////
/**
* @breif 加载下一张图片
**/
void CFreescaleGetImgDlg::OnBnClickedButton16()
{
	CString BrowsePath;
	CString PicName;
	CString PicNameDo;
	int PicNum = 0;

	ImageNumPtr.GetWindowText(PicName);
	PicNum = _ttoi(PicName);
	PicNum++;
	if (PicNum > 500) PicNum = 500; //限制图片数量
	PicName.Empty();
	PicName.Format(_T("%d"), PicNum);
	ImageNumPtr.SetWindowTextW(PicName);
	
	PicNameDo = _T("BipMap") + PicName + _T(".bmp");
	BrowsePath = SavePath + PicNameDo;

	CImage ImageTemp;
	if (!ImageTemp.IsNull()){
		ImageTemp.Destroy();
	}
	ImageTemp.Load(BrowsePath);
	if (ImageTemp.IsNull()) return;
	GetPicData(ImageTemp);

	BroseImageUI = 1;
	Invalidate();
	UpdateWindow();


	isImage = 1;

	SaveFlagEdge[0] = 0;
	SaveFlagEdge[1] = 0;
	SaveFlagEdge[2] = 0;
}
/*-------------自定义函数-----------------*/
/////////////////////////////////////////////
/**
* @breif 提取像素点
**/
void CFreescaleGetImgDlg::GetPicData(CImage &Image){
	int Width = Image.GetWidth();
	int Height = Image.GetHeight();
	COLORREF colr;
	int PicR=0, PicG=0, PicB=0;
	int PicAvg;

	for (int i = 0; i < Height; i++){
		for (int j = 0; j < Width; j++){
			colr = Image.GetPixel(j, i);//注意j 和i 不能交换---重要
			PicR = GetRValue(colr);
			PicG = GetGValue(colr);
			PicB = GetBValue(colr);
			PicAvg = (PicR + PicG + PicB) / 3;
			if (PicAvg > 120){	//阈值
				ov7725_image[i][j] = 0XFF;
			}
			else{
				ov7725_image[i][j] = 0X00;
			}
		}
	}
}
/////////////////////////////////////////////
/**
* @breif 加载固定图像
**/
void CFreescaleGetImgDlg::ShowBrowseImg(CImage ImageOrigin,CString LoadPath){
	CRect WindowRect;

	ImageOrigin.Load(LoadPath);
	if (ImageOrigin.IsNull()) return;

	int ImgWidth = ImageOrigin.GetWidth();
	int ImgHeight = ImageOrigin.GetHeight();

	//获取控件大小
	GetDlgItem(IDC_IMG)->GetWindowRect(&WindowRect);
	//将客户区选中到控件表示的矩形区域内 
	ScreenToClient(&WindowRect);
	GetDlgItem(IDC_IMG)->MoveWindow(WindowRect.left, WindowRect.top, ImgWidth, ImgHeight, TRUE);
	CWnd *pWnd = NULL;
	pWnd = GetDlgItem(IDC_IMG);//获取控件句柄  
	pWnd->GetClientRect(&WindowRect);//获取句柄指向控件区域的大小  
	CDC *pDc = NULL;
	pDc = pWnd->GetDC();//获取picture的DC  
	ImageOrigin.Draw(pDc->m_hDC, WindowRect);//将图片绘制到picture表示的区域内  
	ReleaseDC(pDc);
}
/////////////////////////////////////////////
/**
* @breif 图像放大
**/
void CFreescaleGetImgDlg::ImageScale(int Pix, CImage &SrcImage, CImage &DstImage){
	int SrcWidth = SrcImage.GetWidth();
	int SrcHeight = SrcImage.GetHeight();

	int DstWidth = SrcWidth*Pix;
	int DstHeiht = SrcHeight*Pix;

	if (!DstImage.IsNull()){
		DstImage.Destroy();
	}

	DstImage.Create(DstWidth, DstHeiht, 24);

	BYTE* DstPtr = (BYTE*)DstImage.GetBits();
	int DstPitch = DstImage.GetPitch();
	BYTE* SrcPtr = (BYTE*)SrcImage.GetBits();
	int SrcPitch = SrcImage.GetPitch();


	for (int i = 0; i < DstHeiht; i++){
		for (int j = 0; j < DstWidth; j++){
			*(DstPtr + i*DstPitch + j * 3 + 0) =
				*(SrcPtr + (i / Pix)*SrcPitch + (j / Pix) * 3 + 0);
			*(DstPtr + i*DstPitch + j * 3 + 1) =
				*(SrcPtr + (i / Pix)*SrcPitch + (j / Pix) * 3 + 1);
			*(DstPtr + i*DstPitch + j * 3 + 2) =
				*(SrcPtr + (i / Pix)*SrcPitch + (j / Pix) * 3 + 2);

		}
	}
}
/////////////////////////////////////////////
/**
* @breif 根据一个数组读取一个图像
**/
void CFreescaleGetImgDlg::ShowImageFromArray(unsigned char** Image, \
	int Width, int Height, CImage &ImageOrigin){
	if (!ImageOrigin.IsNull()){
		ImageOrigin.Destroy();
	}
	ImageOrigin.Create(Width, Height, 24);

	unsigned char* pImg = (unsigned char*)ImageOrigin.GetBits();
	int Step = ImageOrigin.GetPitch();
	for (int i = 0; i < Height; i++){
		for (int j = 0; j < Width; j++){
			if (((unsigned char*)Image + (i*Width))[j] == 0x00){
				*(pImg + i*Step + j * 3 + 0) = 0;
				*(pImg + i*Step + j * 3 + 1) = 0;
				*(pImg + i*Step + j * 3 + 2) = 0;
			}
			else{
				*(pImg + i*Step + j * 3 + 0) = 255;
				*(pImg + i*Step + j * 3 + 1) = 255;
				*(pImg + i*Step + j * 3 + 2) = 255;
			}
		}
	}
}
/////////////////////////////////////////////
/**
* @breif 显示图像
**/
void CFreescaleGetImgDlg::ShowScaleImage(CImage &ImageOrigin){
	int ImageHeight = ImageOrigin.GetHeight();
	int ImageWidth = ImageOrigin.GetWidth();
	CRect WindowRect;
	//获取控件大小
	GetDlgItem(IDC_IMG)->GetWindowRect(&WindowRect);
	//将客户区选中到控件表示的矩形区域内 
	ScreenToClient(&WindowRect);
	GetDlgItem(IDC_IMG)->MoveWindow(WindowRect.left, WindowRect.top, ImageWidth, ImageHeight, TRUE);
	CWnd *pWnd = NULL;
	pWnd = GetDlgItem(IDC_IMG);//获取控件句柄  
	pWnd->GetClientRect(&WindowRect);//获取句柄指向控件区域的大小  
	CDC *pDc = NULL;
	pDc = pWnd->GetDC();//获取picture的DC  
	ImageOrigin.Draw(pDc->m_hDC, WindowRect);//将图片绘制到picture表示的区域内  
	ReleaseDC(pDc);
}
/////////////////////////////////////////////
/**
* @breif 保存图像
**/
void CFreescaleGetImgDlg::ImageSaveToPath(CString Path){
	CImage Image;
	HWND hwnd = GetDlgItem(IDC_IMG)->GetSafeHwnd();
	HDC hDC = ::GetWindowDC(hwnd);
	RECT rect;
	::GetWindowRect(hwnd, &rect);
	HDC hDCMem = ::CreateCompatibleDC(hDC);
	HBITMAP hBitMap = ::CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
	if (NULL == hBitMap){
		MessageBox(_T("图片创建失败！"));
	}
	HBITMAP HoldMap = (HBITMAP)::SelectObject(hDCMem, hBitMap);
	::BitBlt(hDCMem, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hDC, 0, 0, SRCCOPY);
	Image.Attach(hBitMap);
	Image.Save(Path);
	Image.Detach();
	::SelectObject(hDCMem, HoldMap); //选入上次的返回值     
	//释放     
	::DeleteObject(hBitMap);
	::DeleteDC(hDCMem);
	::DeleteDC(hDC);
	Image.Destroy();
}
/////////////////////////////////////////////
/**
* @breif 左边界线
**/
void CFreescaleGetImgDlg::DrawLeftEdge(CImage &Image){
	unsigned char* pImg = (unsigned char*)Image.GetBits();
	int Step = Image.GetPitch();
	for (int i = 0; i < CAMERA_H; i++){
		*(pImg + i*Step + LeftEdge[i] * 3 + 0) = 255;
		*(pImg + i*Step + LeftEdge[i] * 3 + 1) = 0;
		*(pImg + i*Step + LeftEdge[i] * 3 + 2) = 0;
	}
}
/////////////////////////////////////////////
/**
* @breif 中间边界线
**/
void CFreescaleGetImgDlg::DrawCenterEdge(CImage &Image){
	unsigned char* pImg = (unsigned char*)Image.GetBits();
	int Step = Image.GetPitch();
	for (int i = 0; i < CAMERA_H; i++){
		*(pImg + i*Step + CenterGuide[i] * 3 + 0) = 0;
		*(pImg + i*Step + CenterGuide[i] * 3 + 1) = 255;
		*(pImg + i*Step + CenterGuide[i] * 3 + 2) = 0;
	}
}
/////////////////////////////////////////////
/**
* @breif 右边界线
**/
void CFreescaleGetImgDlg::DrawRightEdge(CImage &Image){
	unsigned char* pImg = (unsigned char*)Image.GetBits();
	int Step = Image.GetPitch();
	for (int i = 0; i < CAMERA_H; i++){
		*(pImg + i*Step + RightEdge[i] * 3 + 0) = 0;
		*(pImg + i*Step + RightEdge[i] * 3 + 1) = 0;
		*(pImg + i*Step + RightEdge[i] * 3 + 2) = 255;
	}
}
/////////////////////////////////////////////
/**
* @breif 浮点转换为字符串
**/
void CFreescaleGetImgDlg::FloatToString(float Err, CString& Str){
	if (Err >= 0) Str = _T("+");
	if (Err < 0) {
		Err = (float)fabs(Err);
		Str = _T("-");
	}
	int ErrScrop = (int)(Err * 100);
	int NumErr[5] = { 0 };
	NumErr[0] = ErrScrop / 10000;
	NumErr[1] = ErrScrop % 10000 / 1000;
	NumErr[2] = ErrScrop % 1000 / 100;
	NumErr[3] = ErrScrop % 100 / 10;
	NumErr[4] = ErrScrop % 10;
	CString NumTemp;
	for (int i = 0; i < 5; i++){
		NumTemp.Format(_T("%d"), NumErr[i]);
		Str += NumTemp;
		if (2 == i) Str += _T(".");
	}
}
/////////////////////////////////////////////
/**
* @breif 处理图像后显示图像相关参数
**/
void CFreescaleGetImgDlg::ShowPicSomeData(void){
	//显示左起始有效行
	CString LeftStartStr;
	LeftStartStr.Format(_T("%d"), LeftStartShow);
	LeftStartPtr.SetWindowText(LeftStartStr);
	//显示右起始有效行
	CString RightStartStr;
	RightStartStr.Format(_T("%d"), RightStartShow);
	RightStartPtr.SetWindowText(RightStartStr);
	//显示截止行
	CString EndLineStr;
	EndLineStr.Format(_T("%d"), ValidEndShow);
	EndLinePtr.SetWindowText(EndLineStr);
	//显示图像误差
	CString PicErrorStr;
	PicErrorStr.Format(_T("%d"), CenterControl);
	PicErrorPtr.SetWindowText(PicErrorStr);
	//显示图像斜率
	CString SlopeStr;
	SlopeStr.Format(_T("%d"), CenterGuideK);
	SlopePtr.SetWindowText(SlopeStr);
	//识别起跑线
	CString IsStartStr;
	IsStartStr.Format(_T("%d"), StStartLine);
	IsStartPtr.SetWindowText(IsStartStr);
	//识别坡道
	CString IsRampStr;
	IsRampStr.Format(_T("%d"), IsRamp);
	IsRampPtr.SetWindowText(IsRampStr);
	//识别障碍物
	CString IsObstacleStr;
	IsObstacleStr.Format(_T("%d"), ObIsObStacle);
	IsObstaclePtr.SetWindowText(IsObstacleStr);
}
/////////////////////////////////////////////
/**
* @breif 横向畸变矫正
**/
void CFreescaleGetImgDlg::ReviseTransDistortion(unsigned char PicPoint[][CAMERA_W]){
	//u->Col v->row M->RowNum
	//U = u*(1+(AB/CD-1)*(v/(M-1)))	//矫正公式
	//x=(CD/(N-1))(u-N/2) //矫正后图像坐标与真实坐标对应关系
	float Pro = -0.18f;	//AB/CD-1
	int Loc = 0;

	unsigned char PicTemp[CAMERA_H][CAMERA_W] = { 0 };
	for (int i = 0; i < CAMERA_H; i++){
		for (int j = 0; j < CAMERA_W; j++){
			PicTemp[i][j] = PicPoint[i][j];
		}
	}

	for (int i = 0; i < CAMERA_H; i++){
		for (int j = 0; j < CAMERA_W; j++){
			Loc = (int)(j*(1 + (Pro*((float)i / (float)(CAMERA_H - 1)))));
			PicPoint[i][Loc] = PicTemp[i][j];
		}
	}
	return ;
}
////////////////////////////////////////////
/**
* @breif 求解二元一次方程
**/
void CFreescaleGetImgDlg::CalLinearEquation(int Point[], float *a, float* b){
	//ax+by=e
	//cx+dy=f
	//x=(de-bf)/(ad-bc)
	//y=(af-ce)/(ad-bc)
	int PointX_X1 = Point[0] * Point[0];
	int PointX_X2 = Point[2] * Point[2];
	float denominator = (float)(PointX_X1 * Point[2] - Point[0] * PointX_X2);
	if (fabs(denominator) < 0.00001) return;    //垂直于X轴
	*a = (Point[2] * Point[1] - Point[0] * Point[3]) / denominator;
	*b = (PointX_X1 * Point[3] - PointX_X2 * Point[1]) / denominator;
	return;
}
////////////////////////////////////////////
void CFreescaleGetImgDlg::ReviseLongiDistortion(unsigned char PicPoint[][CAMERA_W]){
	//y=vd
	float RealY = 260.0f;	//260cm
	float ActualY = 0.0f;
	float a = 0.0, b = 0.0f;
	int Point[4] = { 14, 16, 33, 150 };
	CalLinearEquation(Point, &a, &b);

	unsigned char PicTemp[CAMERA_H][CAMERA_W] = { 0 };
	for (int i = 0; i < CAMERA_H; i++){
		for (int j = 0; j < CAMERA_W; j++){
			PicTemp[i][j] = PicPoint[i][j];
		}
	}

	int LocLoc[CAMERA_H] = { 0 };
	for (int i = 0; i < CAMERA_H; i++){
		ActualY = a*(float)(i*i) + b*(float)i;
		float Pro = ActualY / RealY;
		if (Pro < 0)Pro = 0;
		if (Pro > 1)Pro = 1;
		LocLoc[i] = (int)(Pro * (float)(CAMERA_H - 1));
		for (int j = 0; j < CAMERA_W; j++){
			PicPoint[LocLoc[i]][j] = PicTemp[i][j];
		}
	}
	for (int i = 0; i < CAMERA_H; i++){
		if (LocLoc[i] != LocLoc[i + 1]){
			for (int j = LocLoc[i] + 1; j < LocLoc[i + 1]; j++){
				for (int k = 0; k < CAMERA_W; k++){
					PicPoint[j][k] = PicTemp[i][k];
				}
			}
		}
	}
	return ;
}
////////////////////////////////////////////
