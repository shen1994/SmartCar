#include "stdafx.h"
#include "Image.h"

unsigned char ov7725_image[CAMERA_H][CAMERA_W] = { 0 };
unsigned char ov7725_image_show[CAMERA_H][CAMERA_W] = { 0 };
////////////////////////////////////////////
/**
* @brief：左边界/右边界/中线
*       ： 定义中间扫描值
**/
int LeftEdge[CAMERA_H] = { 0 };
int RightEdge[CAMERA_H] = { 0 };
int CenterGuide[CAMERA_H] = { 0 };
int CenterErr[CAMERA_H] = { 0 };
////////////////////////////////////////////
/**
* @brief：阈值
*       :判断点为黑或白
**/
#define JudgeWhite(point) ((point) >= 1)
#define JudgeBlack(point) ((point) <  1)
////////////////////////////////////////////
BeginLine _BeginLine;
TLSM LeftTLSM, RightTLSM, CenterTLSM;
TLSMXX CenterTLSMXX;
int ObIsObStacle = 0;    //Lock---存在障碍物
int ObValidLine = 0;     //Lock---障碍物搜索的最后一行，用作有效行
int StStartLine = 0; //Lock---起跑线标志位
static int TimeCount = 0;	//Lock---计数防止和起跑时重合，只判停使用
static int TimeOtherCount = 0; //Lock---计数防止和起跑时重合，只判停使用
int IsRamp = 0;//Lock---判断为上坡的标志位
#define LEFT_EDGE 3   //截止行---0 + 3
#define RIGHT_EDGE 76 //截止行---CAMERA_W - 1 - 3
#define CENTER_GUIDE 40 //(CAMERA_W / 2)	//Lock---中心搜索值
#define SEARCH_TIMES 7 //Lock---搜索一行二分法次数---CAMEARA_W/2/2/2/2/2/2=1
#define START_ROW 1	//Lock---起始搜索行
#define END_ROW 59 //(CAMERA_H - 1 - 6)	//Lock---终止搜索行
#define START_COL 2	//Lock---起始搜索列
#define END_COL 77 //(CAMERA_W - 2 - 1)	//Lock---终止搜索列
#define FILTER_MIN 0		//Lock---宽度滤波最小值
#define FILTER_MAX 20	//Lock---宽度滤波最大值
#define StVStart 1	//Lock---搜索起跑线起始行
#define StVEnd 51   //Lock---搜索起跑线终止行
const int LoseEdgeWidth[60] = {	//补线宽度
	36,35,34,34,33,33,32,32,31,31,
	30,30,29,29,28,28,27,27,26,26,
	25,24,24,23,23,22,21,21,20,20,
	20,19,18,18,17,17,16,15,15,14,
	13,13,12,12,11,11,10,10, 9, 9,
	 8, 7, 7, 6, 6, 5, 5, 4, 4, 2
};//Lock---单边丢线补线宽度
int CenterGuideK = 0; //Lock---中心线斜率计算
int ValidEndShow = _BeginLine.ValidEndLine; //Unlock---终止有效行
int LeftStartShow = _BeginLine.LeftValidLine;	//UnLock---左起始有效行
int RightStartShow = _BeginLine.RightValidLine; //UnLock---右起始有效行
int CenterControl = 0;	//UnLock---舵机误差
////////////////////////////////////////////
//Lock
/**
*	@brief   图像采集完后处理函数
*	@return  状态
*/
Status ImageFinshed(void){
	ProcessImage();
	CenterGuideK = DetectWayType(&_BeginLine);
	CenterControl = GetSteerError(&_BeginLine);
	ValidEndShow = _BeginLine.ValidEndLine;
	LeftStartShow = _BeginLine.LeftValidLine;
	RightStartShow = _BeginLine.RightValidLine;
	return 'T';
}
////////////////////////////////////////////
//Lock
/**
* @breif 舵机转向偏差值
**/
Status GetSteerError(BeginLine *_BeginLine) {
	//1.0m-44
	//1.2m-47
	//1.5m-49
	//2.0m-50
	//2.2m-51
	//2.5m-52
	int		EveryPartNum = 0;
	int IntCenterK = 0;
	int		PartCount = 5, OtherCount = 0, i = 0, j = 0;
	float	CenterSum = 0, CenterWeight = 0, CenterData = 0;
	int CenterValidLine = _BeginLine->ValidEndLine;
	static float LastErr = 0;
	//最后一项 195 严重切
	//125 刚刚擦边
	//75 有点空隙过弯道
	//int PartWeight[8] = { 1, 1, 1, 1, 1, 1,1, 1 };
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 155, 185 };	//125 195
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 155, 125 };
	 int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 155, 95 };
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 75, 65 };
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 75,  1 };

	/*不足6行则保持原控制量*/
	if (CenterValidLine < 6) {
		return (int)LastErr;
	}

	/*断点处人为延长*/
#if 0
	IntCenterK = abs(CenterGuideK);
	//小弯处人为延长
	if ((CenterValidLine > 45 && IntCenterK > 65 && IntCenterK < 120)
		) {	//截至行长度不够
		LinearLine(&CenterTLSM, CenterValidLine - 5, 5, Center);
		_BeginLine->ValidEndLine = END_ROW - 1;
		for (int n = 1; n < (END_ROW - CenterValidLine); n++) {
			CenterGuideGo(CenterValidLine + n) = \
				ForecastTHSM(CenterValidLine + n, CenterTLSM.k, CenterTLSM.b);
			if (CenterGuideGo(CenterValidLine + n) <= START_COL) CenterGuideGo(CenterValidLine + n) = START_COL;
			if (CenterGuideGo(CenterValidLine + n) >= END_COL) CenterGuideGo(CenterValidLine + n) = END_COL;
		}
	}
#endif

	for (int n = START_ROW; n <= _BeginLine->ValidEndLine; n++)
	{
		CenterErrGo(n) = CenterGuideGo(n) - CENTER_GUIDE;
	}//点的偏差值计算

	if (_BeginLine->ValidEndLine > 39)
		PartCount += (_BeginLine->ValidEndLine + 1 - 39) / 5 + 1;
	EveryPartNum = (_BeginLine->ValidEndLine + 1 - START_ROW) / PartCount;
	OtherCount = (_BeginLine->ValidEndLine + 1 - START_ROW) % EveryPartNum;
	
	if (OtherCount == 0 && PartCount < 8) PartCount++;
	if (PartCount > 8) {
		OtherCount += EveryPartNum * (PartCount - 8);
		PartCount = 8;
	}

	for (i = 0; i < PartCount; i++)
	{
		for (j = 0; j<EveryPartNum; j++)
		{
			CenterSum += (float)(CenterErrGo(i * EveryPartNum + j + START_ROW))* PartWeight[i];
			CenterWeight += PartWeight[i];
		}
	}//主体部分

	for (j = 0; j < OtherCount; j++)
	{
		CenterSum += (float)CenterErrGo(PartCount * EveryPartNum + j + START_ROW) * PartWeight[PartCount - 1];
	}//分部
	CenterWeight += OtherCount * PartWeight[PartCount - 1];
	CenterData = CenterSum / CenterWeight;
	LastErr = CenterData;
	return (int)CenterData;
}
////////////////////////////////////////////
//Lock
/**
* @brief:判断赛道类型---返回状态InWay
* @param[in]:起始搜索有效行号---VStart
* @param[in]:终止搜索有效行号---VEnd
* @param[in]:最小二乘用户自定义---CenterTLMS
* @return:状态
**/
Status DetectWayType(BeginLine *_BeginLine) {
	//动态性分成10段，每段取中点,最多不超过12段
	int PartCount = 8;
	int StraightCons = 59;	//直道上曲率常量
	int PointNum = 0, EveryPart = 0;
	float ThisSlope = 0.0f, LastSlope = 0.0f;
	int CenterPartX[8] = { 0 }, CenterPartY[8] = { 0 };
	int SamplePointX[3] = { 0 }, SamplePointY[3] = { 0 };
	double La = 0.0f, Lb = 0.0f, Lc = 0.0f;
	double Cabc = 0.0f, Sabc = 0.0f;
	PointNum = _BeginLine->ValidEndLine + 1 - START_ROW;
	//点采样
	if (PointNum < 10) return (int)(LastSlope * 100.0f) - StraightCons;
	EveryPart = PointNum / PartCount;
	for (int i = 0; i < PartCount; i++) {
		CenterPartX[i] = START_ROW + i*EveryPart + EveryPart / 2 + 1;
		CenterPartY[i] = CenterGuideGo(CenterPartX[i]);
	}
	//拟合曲线
	CalculateTLSMXX(&CenterTLSMXX, CenterPartX, CenterPartY, PartCount);
	//计算曲率
	SamplePointX[0] = START_ROW;
	SamplePointY[0] = ForecastTHSMXX(SamplePointX[0], CenterTLSMXX.a, CenterTLSMXX.c);
	SamplePointX[2] = _BeginLine->ValidEndLine;
	SamplePointY[2] = ForecastTHSMXX(SamplePointX[2], CenterTLSMXX.a, CenterTLSMXX.c);
	SamplePointX[1] = (SamplePointX[0] + SamplePointX[2]) / 2;
	SamplePointY[1] = ForecastTHSMXX(SamplePointX[1], CenterTLSMXX.a, CenterTLSMXX.c);
	//三角形边长计算
	La = sqrt(double((SamplePointX[0] - SamplePointX[1]) * (SamplePointX[0] - SamplePointX[1])
		+ (SamplePointY[0] - SamplePointY[1]) * (SamplePointY[0] - SamplePointY[1])));
	Lb = sqrt(double((SamplePointX[0] - SamplePointX[2]) * (SamplePointX[0] - SamplePointX[2])
		+ (SamplePointY[0] - SamplePointY[2]) * (SamplePointY[0] - SamplePointY[2])));
	Lc = sqrt(double((SamplePointX[1] - SamplePointX[2]) * (SamplePointX[1] - SamplePointX[2])
		+ (SamplePointY[1] - SamplePointY[2]) * (SamplePointY[1] - SamplePointY[2])));
	Cabc = La + Lb + Lc;
	Sabc = sqrt(Cabc*(Cabc - La)*(Cabc - Lb)*(Cabc - Lc));
	ThisSlope = (float)((4 * Sabc) / (La*Lb*Lc));
	LastSlope = ThisSlope;
	return (int)(ThisSlope * 100.0f) - StraightCons;
}
////////////////////////////////////////////
//Lock               
/**
* @brief:图象处理过程
* @return:状态
**/
Status ProcessImage(void){
	/*---时间判别加起跑线识别---*/
	TimeOtherCount++;
	if (TimeOtherCount >= 50){	//1s 50帧
		TimeOtherCount = 0;
		TimeCount++;
	}
	if (TimeCount >= 80) TimeCount = 80;
	if (TimeCount >= 3 && 0 == StStartLine){
		StStartLine = DetectStartLine1(StVStart, StVEnd, CENTER_GUIDE);
	}
	/*----引导线识别-----------*/
	ObIsObStacle = 0;
	IsRamp = 0;
	StStartLine = 0;
	//===决定是否存在障碍物
	InitMainVariables();
	SearchDoubleEdge(START_ROW, END_ROW, START_COL, END_COL, &_BeginLine);
	ReviseStartValidLine(START_COL, END_COL, &_BeginLine);
	SearchEndLine(&_BeginLine);
	//===决定是否存在坡道
	IsRamp = DetectRampOK(&_BeginLine);
	//===决定是否存在起跑线
	StStartLine = DetectStartLine1(StVStart, StVEnd, CENTER_GUIDE);
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:四舍五入
* @param[in]:输入小数---每用一次函数，代码自动增加
* @return:整型数
**/
int myround(float f)
{
	if ((int)f + 0.5f>f)
		return (int)f;
	else return (int)f + 1;
}
////////////////////////////////////////////
//Lock
/**
* @brief:初始化最小二乘参数
* @param[in]:坐标点和坐标点个数
* @return:状态
**/
Status InitTLSMVaribles(TLSM *_TLSM, int _PointX[], int _PointY[], int _Length){
	_TLSM->XIntegral = 0;
	_TLSM->YIntegral = 0;
	_TLSM->XXIntegral = 0;
	_TLSM->XYIntegral = 0;
	_TLSM->k = 0.0f;
	_TLSM->b = 0.0f;
	for (int i = 0; i<_Length; i++){
		_TLSM->PointX[i] = _PointX[i];
		_TLSM->PointY[i] = _PointY[i];
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:计算最小二乘，跟新斜率k,b
* @param[in]:坐标点和坐标点个数
* @return:状态
**/
Status CalculateTLSM(TLSM *_TLSM, int _PointX[], int _PointY[], int _Length){
	int Denominator = 0;
	InitTLSMVaribles(_TLSM, _PointX, _PointY, _Length);
	for (int i = 0; i < _Length; i++)
	{
		_TLSM->XIntegral += _TLSM->PointX[i];
		_TLSM->YIntegral += _TLSM->PointY[i];
		_TLSM->XYIntegral += _TLSM->PointX[i] * _TLSM->PointY[i];
		_TLSM->XXIntegral += _TLSM->PointX[i] * _TLSM->PointX[i];
	}
	Denominator = _Length * _TLSM->XXIntegral - _TLSM->XIntegral*_TLSM->XIntegral;
	if (abs(Denominator) < 0.00001) {
		return 1;	//垂直于X轴的直线
	}
	_TLSM->k = (float)(_Length * _TLSM->XYIntegral - _TLSM->XIntegral*_TLSM->YIntegral) / (float)Denominator;	//斜率K
	_TLSM->b = (float)(_TLSM->XXIntegral*_TLSM->YIntegral - _TLSM->XIntegral *_TLSM->XYIntegral) / (float)Denominator;	//截距b
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:初始化最小二乘参数
* @param[in]:坐标点和坐标点个数
* @return:状态
**/
Status InitTLSMXXVaribles(TLSMXX *_TLSMXX, int _PointX[], int _PointY[], int _Length) {
	_TLSMXX->XIntegral = 0;
	_TLSMXX->YIntegral = 0;
	_TLSMXX->XXIntegral = 0;
	_TLSMXX->XYIntegral = 0;
	_TLSMXX->XXXIntegral = 0;
	_TLSMXX->a = 0.0f;
	_TLSMXX->c = 0.0f;
	for (int i = 0; i<_Length; i++) {
		_TLSMXX->PointX[i] = _PointX[i];
		_TLSMXX->PointY[i] = _PointY[i];
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:计算最小二乘，跟新斜率k,b
* @param[in]:坐标点和坐标点个数
* @return:状态
**/
Status CalculateTLSMXX(TLSMXX *_TLSMXX, int _PointX[], int _PointY[], int _Length) {
	float D = 0.0f, Dx = 0.0f, Dy = 0.0f;
	InitTLSMXXVaribles(_TLSMXX, _PointX, _PointY, _Length);
	for (int i = 0; i < _Length; i++)
	{
		_TLSMXX->XIntegral += _TLSMXX->PointX[i];
		_TLSMXX->YIntegral += _TLSMXX->PointY[i];
		_TLSMXX->XYIntegral += _TLSMXX->PointX[i] * _TLSMXX->PointY[i];
		_TLSMXX->XXIntegral += _TLSMXX->PointX[i] * _TLSMXX->PointX[i];
		_TLSMXX->XXXIntegral += _TLSMXX->PointX[i] * _TLSMXX->PointX[i] * _TLSMXX->PointX[i];
	}
	D = (float)(_Length * _TLSMXX->XXXIntegral - _TLSMXX->XXIntegral * _TLSMXX->XIntegral);
	if ((fabs(D)-0.00001f) < 0) {
		return 1;	//垂直于X轴的直线
	}
	Dx = (float)(_Length * _TLSMXX->XYIntegral - _TLSMXX->XIntegral*_TLSMXX->YIntegral);
	Dy = (float)(_TLSMXX->YIntegral * _TLSMXX->XXXIntegral - _TLSMXX->XYIntegral * _TLSMXX->XXIntegral);
	_TLSMXX->a = Dx / D;
	_TLSMXX->c = Dy / D;
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:根据边线判断是否跟新最小二乘参数
* @param[in]:最小二乘用户自定义
* @param[in]:当前行---CurrentLine
* @param[in]:点的个数---Length
* @param[in]:搜索位置---Flag
* @return:状态
*/
Status LinearLine(TLSM *_TLSM, int CurrentLine, int Length, Site Flag){
	int PointX[TLSMLength] = { 0 };
	int PointY[TLSMLength] = { 0 };
	if (Left == Flag){
		for (int i = 0; i < Length; i++){
			PointX[i] = CurrentLine + i;
			PointY[i] = LeftEdgeGo(CurrentLine + i);
		}
	}
	else if (Right == Flag){
		for (int i = 0; i < Length; i++){
			PointX[i] = CurrentLine + i;
			PointY[i] = RightEdgeGo(CurrentLine + i);
		}
	}
	else{
		for (int i = 0; i < Length; i++){
			PointX[i] = CurrentLine + i;
			PointY[i] = CenterGuideGo(CurrentLine + i);
		}
	}
	CalculateTLSM(_TLSM, PointX, PointY, Length);	//跟新最小二乘k,b
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif 搜索一列，并判断该列是否存在连续跳变
**/
Status StartLineFind(int Col, int VStart, int VEnd, Site LocSite, \
	int *FirstFindFlag, int *LastFindFlag, \
	int *Bottom, int *Top, int *MidLineValue, int *VLineValue){
	int WhiteToBlackFlag = 0, BlackToWhiteFlag = 0;
	int BottomWidth = 0, TopWidth = 0, ToToWidth = 0, MidLine = 0;
	int VLoc = 0;

	for (int Rol = VStart; Rol < VEnd; Rol++){
		if (0 == WhiteToBlackFlag && 0 == *FirstFindFlag)
			if (JudgeWhite(img(Rol, Col)))
				if (JudgeBlack(img(Rol + 1, Col))){
					WhiteToBlackFlag = 1;
					*Bottom = Rol + 1;
				}
		if (1 == WhiteToBlackFlag && 0 == BlackToWhiteFlag && 0 == *FirstFindFlag)
			if (JudgeBlack(img(Rol, Col)))
				if (JudgeWhite(img(Rol + 1, Col))){
					BlackToWhiteFlag = 1;
					*Top = Rol;
					*VLineValue = Col;
					*MidLineValue = myround((float)(*Bottom + *Top) / 2.0f);
					//===寻找第一个起始列的限制条件
					if ((*Top - *Bottom) > 6 && *MidLineValue <= 40) return 0;
					if ((*Top - *Bottom) > 2 && *MidLineValue > 40) return 0;
					//===检验下一列有无黑白跳变
					if (Left == LocSite) VLoc = Col - 1;
					else VLoc = Col + 1;
					//===白色则默认有起跑线存在
					if (JudgeWhite(img(*MidLineValue, VLoc))) {
						*FirstFindFlag = 1;
						return 1;
					}
					//===不需要检验白色，第一列之后很可能就有白色
					if (JudgeBlack(img(*MidLineValue, VLoc))
						|| JudgeWhite(img(*MidLineValue, VLoc))){
						int ToBlack = 0, ToWhite = 0;
						int Bottom = *MidLineValue - 5;
						if (Bottom < VStart) Bottom = VStart;
						int Top = *MidLineValue + 5;
						if (Top > VEnd) Top = VEnd;

						for (int n = Bottom; n <= Top; n++){
							if (0 == ToBlack)
								if (JudgeWhite(img(n, VLoc)))
									if (JudgeBlack(img(n + 1, VLoc))){
										ToBlack = 1;
										BottomWidth = n;
									}
							if (1 == ToBlack && 0 == ToWhite){
								if (JudgeBlack(img(n, VLoc)))
									if (JudgeWhite(img(n + 1, VLoc))){
										ToWhite = 1;
										TopWidth = n;
									}
							}
						}
						if (ToBlack == 1 && ToWhite == 1){
							ToToWidth = TopWidth - BottomWidth;
							MidLine = (BottomWidth + TopWidth) / 2;
							if ((abs(*MidLineValue - MidLine) <= 3)
								|| (ToToWidth <= 6 && MidLine <= 40)
								|| (ToToWidth <= 2 && MidLine <= 40)
								){
								*FirstFindFlag = 1;
								return 0;
							}
						}
						if (ToBlack == 0 || ToWhite == 0) {
							return 0;
						}
					}
				}
		if (0 == WhiteToBlackFlag && 0 == *LastFindFlag && 1 == *FirstFindFlag)
			if (JudgeWhite(img(Rol, Col)))
				if (JudgeBlack(img(Rol + 1, Col))){
					WhiteToBlackFlag = 1;
					*Bottom = Rol + 1;
				}
		if (1 == WhiteToBlackFlag && 0 == BlackToWhiteFlag && 0 == *LastFindFlag && 1 == *FirstFindFlag)
			if (JudgeBlack(img(Rol, Col)))
				if (JudgeWhite(img(Rol + 1, Col))){
					BlackToWhiteFlag = 1;
					*Top = Rol;
					*VLineValue = Col;
					*MidLineValue = myround((float)(*Bottom + *Top) / 2.0f);
					//===检验下一列有无黑白跳变
					if (Left == LocSite) VLoc = Col - 1;
					else VLoc = Col + 1;
					if (JudgeWhite(img(*MidLineValue, VLoc))){
						*LastFindFlag = 1;
						return 1;
					}
					if (JudgeBlack(img(*MidLineValue, VLoc))){
						int ToBlack = 0, ToWhite = 0;
						int Bottom = *MidLineValue - 5;
						if (Bottom < VStart) Bottom = VStart;
						int Top = *MidLineValue + 5;
						if (Top > VEnd) Top = VEnd;

						for (int n = Bottom; n <= Top; n++){
							if (0 == ToBlack)
								if (JudgeWhite(img(n, VLoc)))
									if (JudgeBlack(img(n + 1, VLoc))){
										ToBlack = 1;
										BottomWidth = n;
									}
							if (1 == ToBlack && 0 == ToWhite){
								if (JudgeBlack(img(n, VLoc)))
									if (JudgeWhite(img(n + 1, VLoc))){
										ToWhite = 1;
										TopWidth = n;
									}
							}
						}
						if (ToBlack == 1 && ToWhite == 1){
							ToToWidth = TopWidth - BottomWidth;
							MidLine = (BottomWidth + TopWidth) / 2;
							if ((abs(*MidLineValue - MidLine) > 3)
								|| (ToToWidth > 6 && MidLine <= 40)
								|| (ToToWidth > 2 && MidLine > 40)
								){
								*LastFindFlag = 1;
								return 1;
							}
						}
						if (ToBlack == 0 || ToWhite == 0) {
							*LastFindFlag = 1;
							return 1;
						}
					}
				}
	}
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @breif 识别起跑线
**/
Status DetectStartLine1(int VStart, int VEnd, int HCenter){
#define LineLimit 40	//这个之后必须要有终止的连续跳变
	int PicMode = 0;
	int WhiteCount = 0;
	int BlackToWhiteFlag = 0, WhiteToBlackFlag = 0;
	int ToToLine[2] = { 0 }, ToToLineWidth = 0;
	int OneFirstFindFlag = 0, OneLastFindFlag = 0;
	int TwoFirstFindFlag = 0, TwoLastFindFlag = 0;
	int OneFirstOneTime = 0, OneLastOneTime = 0;
	int TwoFirstOneTime = 0, TwoLastOneTime = 0;
	int StartLineFlag = 0;
	int _Top = 0, _Bottom = 0, _MidLineValue = 0, _VLineValue = 0;
	int LeftHLineValue[4] = { 0 }, LeftVLineValue[2] = { 0 }, LeftMidHLineValue[2] = { 0 };
	int RightHLineValue[4] = { 0 }, RightVLineValue[2] = { 0 }, RightMidHLineValue[2] = { 0 };

	/*决定图像类型*/
	for (int i = VStart; i < VEnd; i++){
		if (JudgeWhite(img(i, HCenter))) WhiteCount++;
		if (0 == BlackToWhiteFlag)
			if (JudgeWhite(img(i, HCenter)))
				if (JudgeBlack(img(i + 1, HCenter))){
					BlackToWhiteFlag = 1;
					ToToLine[0] = i + 1;
				}
		if (1 == BlackToWhiteFlag && 0 == WhiteToBlackFlag)
			if (JudgeBlack(img(i, HCenter)))
				if (JudgeWhite(img(i + 1, HCenter))){
					WhiteToBlackFlag = 1;
					ToToLine[1] = i + 1;
				}
		if (i == (VEnd - VStart) && 1 == WhiteToBlackFlag && 1 == BlackToWhiteFlag)
			ToToLineWidth = ToToLine[1] - ToToLine[0];
		if (i == (VEnd - VStart)){
			if (WhiteCount == (VEnd - VStart)){
				PicMode = 1;
				break;
			}
			if (0 != ToToLineWidth && ToToLineWidth < 8) {
				PicMode = 4;
				break;
			}
			if ((0 == WhiteToBlackFlag && 1 == BlackToWhiteFlag)
				|| (0 != ToToLineWidth && ToToLineWidth > 10)){
				if (JudgeBlack(img(ToToLine[0], HCenter - 1))) {
					PicMode = 2;
					break;
				}
				if (JudgeBlack(img(ToToLine[0], HCenter + 1))) {
					PicMode = 3;
					break;
				}
			}
		}

	}
	//===对经常发生的进行再判断
	if (1 == PicMode){
		int GoLeft = 0, GoRight = 0;
		int GoCenterToRight = 0, GoCenterToLeft = 0;
		//===左边搜索一段特殊的边界
		for (int i = CENTER_GUIDE; i < END_COL; i++){
			if (JudgeWhite(img(VEnd - 1, i)))
				if (JudgeBlack(img(VEnd - 1, i + 1))){
					GoRight = i + 1;
					break;
				}
		}
		//===右边搜索一段特殊的边界
		for (int i = CENTER_GUIDE; i > START_COL; i--){
			if (JudgeWhite(img(VEnd - 1, i)))
				if (JudgeBlack(img(VEnd - 1, i - 1))){
					GoLeft = i - 1;
					break;
				}
		}
		if (0 != GoLeft && 0 != GoRight){
			GoCenterToRight = GoRight - CENTER_GUIDE;
			GoCenterToLeft = CENTER_GUIDE - GoLeft;
			if (GoLeft < 40 && GoRight > 40){
				if ((GoCenterToRight - GoCenterToLeft) > 12){
					PicMode = 2;
				}
				if ((GoCenterToLeft - GoCenterToRight) > 12){
					PicMode = 3;
				}
			}
		}
	}
	if (0 == PicMode) return 0;
	/*找出每一种图像类型的起跑线四个角*/
	//===车身很正，起跑线均匀分布在中心线两侧
	if (1 == PicMode){
		//===左搜索四个角
		for (int j = HCenter; j > START_COL; j--){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Left,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 0 == OneLastFindFlag && 0 == OneFirstOneTime){
				OneFirstOneTime = 1;
				LeftHLineValue[2] = _Bottom;
				LeftHLineValue[3] = _Top;
				LeftVLineValue[1] = _VLineValue;
				LeftMidHLineValue[1] = _MidLineValue;
				if (1 == StartLineFlag && 0 == OneLastOneTime && 1 == OneFirstOneTime){
					OneLastOneTime = 1;
					OneLastFindFlag = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				LeftHLineValue[0] = _Bottom;
				LeftHLineValue[1] = _Top;
				LeftVLineValue[0] = _VLineValue;
				LeftMidHLineValue[0] = _MidLineValue;
			}
			//没有找到跳变就默认
			if (j == (START_COL + 1) && 1 == OneFirstFindFlag && 0 == OneLastFindFlag
				&& 0 != _Bottom && 0 != _Top && LeftMidHLineValue[1] < LineLimit) {
				OneLastFindFlag = 1;
				LeftHLineValue[0] = _Bottom;
				LeftHLineValue[1] = _Top;
				LeftVLineValue[0] = _VLineValue;
				LeftMidHLineValue[0] = _MidLineValue;
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag) break;
		}
		//===左边没有这样的跳变特征退出
		if (0 == OneFirstFindFlag || 0 == OneLastFindFlag) return 0;
		//===右搜索四个角
		for (int j = HCenter; j < END_COL; j++){
			//===为了抗玩到干扰，第二次搜索限制
			int KeepVStart = 0, KeepVEnd = 0;
			int MidTemp = (LeftMidHLineValue[0] + LeftMidHLineValue[1]) / 2;
			KeepVStart = MidTemp - 15;
			if (KeepVStart < VStart) KeepVStart = VStart;
			KeepVEnd = MidTemp + 15;
			if (KeepVEnd > VEnd) KeepVEnd = VEnd;
			StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Right,
				&TwoFirstFindFlag, &TwoLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
				TwoFirstOneTime = 1;
				RightHLineValue[0] = _Bottom;
				RightHLineValue[1] = _Top;
				RightVLineValue[0] = _VLineValue;
				RightMidHLineValue[0] = _MidLineValue;
				if (1 == StartLineFlag && 0 == TwoLastOneTime && 1 == TwoFirstOneTime){
					TwoLastOneTime = 1;
					TwoLastFindFlag = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
			}
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
				TwoLastOneTime = 1;
				RightHLineValue[2] = _Bottom;
				RightHLineValue[3] = _Top;
				RightVLineValue[1] = _VLineValue;
				RightMidHLineValue[1] = _MidLineValue;
			}
			//没有找到跳变就默认
			if (j == (END_COL - 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
				&& 0 != _Bottom && 0 != _Top && RightMidHLineValue[0] < LineLimit) {
				TwoLastFindFlag = 1;
				RightHLineValue[2] = _Bottom;
				RightHLineValue[3] = _Top;
				RightVLineValue[1] = _VLineValue;
				RightMidHLineValue[1] = _MidLineValue;
			}
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) break;
		}
		//===右边没有这样的跳变特征退出
		if (0 == TwoFirstFindFlag || 0 == TwoLastFindFlag) return 0;
	}
	//===车身特别偏，起跑线在中心线一侧
	//===起跑线偏右
	if (2 == PicMode){
		for (int j = HCenter; j < END_COL; j++){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Right,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 0 == OneLastFindFlag && 0 == OneFirstOneTime){
				OneFirstOneTime = 1;
				LeftHLineValue[0] = _Bottom;
				LeftHLineValue[1] = _Top;
				LeftVLineValue[0] = _VLineValue;
				LeftMidHLineValue[0] = _MidLineValue;
				if (JudgeBlack(img(LeftMidHLineValue[0], LeftVLineValue[0] - 1))) return 0;
				if (1 == StartLineFlag && 0 == OneLastOneTime && 1 == OneFirstOneTime){
					OneLastOneTime = 1;
					OneLastFindFlag = 1;
					j += 5; //空5列继续找
					LeftHLineValue[2] = _Bottom;
					LeftHLineValue[3] = _Top;
					LeftVLineValue[1] = _VLineValue;
					LeftMidHLineValue[1] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				j += 5; //空5列继续找
				LeftHLineValue[2] = _Bottom;
				LeftHLineValue[3] = _Top;
				LeftVLineValue[1] = _VLineValue;
				LeftMidHLineValue[1] = _MidLineValue;
			}
			//在左边起边线找到的情况下找右边线
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag
				&& 1 == OneFirstOneTime && 1 == OneLastOneTime){
				//===为了抗玩到干扰，第二次搜索限制
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = (LeftMidHLineValue[0] + LeftMidHLineValue[1]) / 2;
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Right,
					&TwoFirstFindFlag, &TwoLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
					TwoFirstOneTime = 1;
					RightHLineValue[0] = _Bottom;
					RightHLineValue[1] = _Top;
					RightVLineValue[0] = _VLineValue;
					RightMidHLineValue[0] = _MidLineValue;
					if (1 == StartLineFlag && 0 == TwoLastOneTime && 1 == TwoFirstOneTime){
						TwoLastOneTime = 1;
						TwoLastFindFlag = 1;
						RightHLineValue[2] = _Bottom;
						RightHLineValue[3] = _Top;
						RightVLineValue[1] = _VLineValue;
						RightMidHLineValue[1] = _MidLineValue;
					}
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
					TwoLastOneTime = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
				//没有找到跳变就默认
				if (j == (END_COL - 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
					&& 0 != _Bottom && 0 != _Top && RightMidHLineValue[0] < LineLimit) {
					TwoLastFindFlag = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag \
				&& 1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) break;
		}
		if (0 == OneFirstFindFlag || 0 == OneLastFindFlag \
			|| 0 == TwoFirstFindFlag || 0 == TwoLastFindFlag) return 0;
	}
	//===起跑线偏左
	if (3 == PicMode){
		for (int j = HCenter; j > START_COL; j--){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Left,
				&TwoFirstFindFlag, &TwoLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
				TwoFirstOneTime = 1;
				RightHLineValue[2] = _Bottom;
				RightHLineValue[3] = _Top;
				RightVLineValue[1] = _VLineValue;
				RightMidHLineValue[1] = _MidLineValue;
				if (JudgeBlack(img(RightMidHLineValue[1], RightVLineValue[1] + 1))) return 0;
				if (1 == StartLineFlag && 0 == TwoLastOneTime && 1 == TwoFirstOneTime){
					TwoLastOneTime = 1;
					TwoLastFindFlag = 1;
					j -= 5; //空5列继续找
					RightHLineValue[0] = _Bottom;
					RightHLineValue[1] = _Top;
					RightVLineValue[0] = _VLineValue;
					RightMidHLineValue[0] = _MidLineValue;
				}
			}
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
				TwoLastOneTime = 1;
				j -= 5; //空5列继续找
				RightHLineValue[0] = _Bottom;
				RightHLineValue[1] = _Top;
				RightVLineValue[0] = _VLineValue;
				RightMidHLineValue[0] = _MidLineValue;
			}
			//在右边起边线找到的情况下找左边线
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag
				&& 1 == TwoFirstOneTime && 1 == TwoLastOneTime){
				//===为了抗玩到干扰，第二次搜索限制
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = (RightMidHLineValue[0] + RightMidHLineValue[1]) / 2;
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Left,
					&OneFirstFindFlag, &OneLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == OneFirstFindFlag && 0 == OneLastFindFlag && 0 == OneFirstOneTime){
					OneFirstOneTime = 1;
					LeftHLineValue[2] = _Bottom;
					LeftHLineValue[3] = _Top;
					LeftVLineValue[1] = _VLineValue;
					LeftMidHLineValue[1] = _MidLineValue;
					if (1 == StartLineFlag && 0 == OneLastOneTime && 1 == OneFirstOneTime){
						OneLastOneTime = 1;
						OneLastFindFlag = 1;
						LeftHLineValue[0] = _Bottom;
						LeftHLineValue[1] = _Top;
						LeftVLineValue[0] = _VLineValue;
						LeftMidHLineValue[0] = _MidLineValue;
					}
				}
				if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
					OneLastOneTime = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
				//没有找到跳变就默认
				if (j == (START_COL + 1) && 1 == OneFirstFindFlag && 0 == OneLastFindFlag
					&& 0 != _Bottom && 0 != _Top && LeftMidHLineValue[1] < LineLimit) {
					OneLastFindFlag = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag \
				&& 1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) break;
		}
		if (0 == OneFirstFindFlag || 0 == OneLastFindFlag \
			|| 0 == TwoFirstFindFlag || 0 == TwoLastFindFlag) return 0;
	}
	//===车身稍微偏一点，起跑线压在中心线上
	if (4 == PicMode){
		//===左半边搜索
		int HLineValueTemp[4] = { 0 }, VLineValueTemp[2] = { 0 }, MidHLineValueTemp[2] = { 0 };
		OneFirstFindFlag = 1;
		for (int j = HCenter; j > START_COL; j--){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Left,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				HLineValueTemp[0] = _Bottom;
				HLineValueTemp[1] = _Top;
				VLineValueTemp[0] = _VLineValue;
				MidHLineValueTemp[0] = _MidLineValue;
			}
		}
		OneLastFindFlag = 0;
		OneLastOneTime = 0;
		//===右半边搜索
		for (int j = HCenter; j < END_COL; j++){
			OneFirstFindFlag = 1;
			StartLineFlag = StartLineFind(j, VStart, VEnd, Right,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				HLineValueTemp[2] = _Bottom;
				HLineValueTemp[3] = _Top;
				VLineValueTemp[1] = _VLineValue;
				MidHLineValueTemp[1] = _MidLineValue;
			}
		}
		int isLeftSearch = 0, isRightSearch = 0;
		int GoLeftSearch = 0, GoRightSearch = 0;
		//===执行双边搜，判断是偏左还是偏右
		int GoRightV = END_COL - RightEdgeGo(VEnd);
		int GoLeftV = LeftEdgeGo(VEnd) - START_COL;
		if ((GoRightV - GoLeftV)>20)
			GoLeftSearch = 1;
		else if ((GoRightV - GoLeftV)<-20)
			GoRightSearch = 1;
		else{
			LinearLine(&LeftTLSM, VEnd - 10, 10, Left);
			LinearLine(&RightTLSM, VEnd - 10, 10, Right);
			if ((0.03f - (fabs(LeftTLSM.k) - fabs(RightTLSM.k))) > 0)
				GoRightSearch = 1;
			else if ((0.03f - (fabs(RightTLSM.k) - fabs(LeftTLSM.k))) > 0)
				GoLeftSearch = 1;
			else if ((fabs(LeftTLSM.k) - fabs(RightTLSM.k)) > 0)
				GoRightSearch = 1;
			else if ((fabs(RightTLSM.k) - fabs(LeftTLSM.k)) > 0)
				GoLeftSearch = 1;
			else;
		}
		//===假设是在左边
		int LeftValue = 0, RightValue = 0;
		int GoToToMid = 0;
		GoToToMid = myround((float)(MidHLineValueTemp[0] + MidHLineValueTemp[1]) / 2.0f);
		if (GoToToMid < 30)
			LeftValue = VLineValueTemp[0] - 8;
		else
			LeftValue = VLineValueTemp[0] - 4;
		if (LeftValue < START_COL) return 0;
		if (1 == GoLeftSearch){
			for (int j = LeftValue; j > START_COL; j--){
				//===为了抗玩到干扰，第二次搜索限制
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = myround((float)(MidHLineValueTemp[0] + MidHLineValueTemp[1]) / 2.0f);
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Left,
					&TwoFirstFindFlag, &TwoLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
					TwoFirstOneTime = 1;
					LeftHLineValue[2] = _Bottom;
					LeftHLineValue[3] = _Top;
					LeftVLineValue[1] = _VLineValue;
					LeftMidHLineValue[1] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
					TwoLastOneTime = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
				//没有找到跳变就默认
				if (j == (START_COL + 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
					&& 0 != _Bottom && 0 != _Top && LeftMidHLineValue[1] < LineLimit) {
					TwoLastFindFlag = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) {
					isLeftSearch = 1;
					break;
				}
			}
		}
		if (1 == isLeftSearch && 1 == GoLeftSearch){
			for (int n = 0; n < 4; n++){
				RightHLineValue[n] = HLineValueTemp[n];
				if (n < 2){
					RightVLineValue[n] = VLineValueTemp[n];
					RightMidHLineValue[n] = MidHLineValueTemp[n];
				}
			}
		}
		TwoFirstFindFlag = 0;
		TwoLastFindFlag = 0;
		TwoFirstOneTime = 0;
		TwoLastOneTime = 0;
		//===假设是在右边
		if (GoToToMid < 30)
			RightValue = VLineValueTemp[1] + 8;
		else
			RightValue = VLineValueTemp[1] + 4;
		if (RightValue > END_COL) return 0;
		if (1 == GoRightSearch){
			for (int j = RightValue; (j < END_COL && 0 == isLeftSearch); j++){
				//===为了抗玩到干扰，第二次搜索限制
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = (MidHLineValueTemp[0] + MidHLineValueTemp[1]) / 2;
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Right,
					&TwoFirstFindFlag, &TwoLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
					TwoFirstOneTime = 1;
					RightHLineValue[0] = _Bottom;
					RightHLineValue[1] = _Top;
					RightVLineValue[0] = _VLineValue;
					RightMidHLineValue[0] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
					TwoLastOneTime = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
				//没有找到跳变就默认
				if (j == (END_COL - 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
					&& 0 != _Bottom && 0 != _Top && RightMidHLineValue[0] < LineLimit) {
					TwoLastFindFlag = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) {
					isRightSearch = 1;
					break;
				}
			}
		}
		if (1 == isRightSearch && 1 == GoRightSearch){
			for (int n = 0; n < 4; n++){
				LeftHLineValue[n] = HLineValueTemp[n];
				if (n < 2){
					LeftVLineValue[n] = VLineValueTemp[n];
					LeftMidHLineValue[n] = MidHLineValueTemp[n];
				}
			}
		}
	}
	/*对找出的四个角进行判断，从而决定是否为起跑线*/
	//===寻找类似于质心的东西
	int KeepLeftHMid = (LeftMidHLineValue[0] + LeftMidHLineValue[1]) / 2;
	int KeepRightHMid = (RightMidHLineValue[0] + RightMidHLineValue[1]) / 2;
	int KeepMidHMid = (KeepLeftHMid + KeepRightHMid) / 2;
	//===左起跑线与右起跑线边界的距离
	int LeftRightDiff = abs(RightVLineValue[0] - LeftVLineValue[1]);
	//===起跑线的长度
	int LeftDiff = LeftVLineValue[1] - LeftVLineValue[0];
	int RightDiff = RightVLineValue[1] - RightVLineValue[0];
	//===起跑线的宽度值
	int LeftDiffWidth_Left = LeftHLineValue[1] - LeftHLineValue[0];
	int LeftDiffWidth_Right = LeftHLineValue[3] - LeftHLineValue[2];
	int RightDiffWidth_Left = RightHLineValue[1] - RightHLineValue[0];
	int RightDiffWidth_Right = RightHLineValue[3] - RightHLineValue[2];
	if (abs(KeepLeftHMid - KeepRightHMid) <= 10
		&& (LeftRightDiff < 20 && LeftRightDiff > 5)
		&& ((LeftDiff >= 2 && RightDiff >= 1) || (LeftDiff >= 1 && RightDiff >= 2))
		&& (LeftDiffWidth_Left <= 6 && LeftDiffWidth_Right <= 6)
		&& (RightDiffWidth_Left <= 6 && RightDiffWidth_Right <= 6)
		&& (KeepMidHMid <= 30)
		)
		return 1;
	if (abs(KeepLeftHMid - KeepRightHMid) <= 10
		&& (LeftRightDiff < 15 && LeftRightDiff > 5)
		&& ((LeftDiff >= 2 && RightDiff >= 1) || (LeftDiff >= 1 && RightDiff >= 2))
		&& (LeftDiffWidth_Left <= 3 && LeftDiffWidth_Right <= 3)
		&& (RightDiffWidth_Left <= 3 && RightDiffWidth_Right <= 3)
		&& (KeepMidHMid > 30 && KeepMidHMid <= 40)
		)
		return 1;
	if (abs(KeepLeftHMid - KeepRightHMid) <= 10
		&& (LeftRightDiff < 12 && LeftRightDiff > 5)
		&& ((LeftDiff >= 0 && RightDiff >= 1) || (LeftDiff >= 1 && RightDiff >= 0))
		&& (LeftDiffWidth_Left <= 1 && LeftDiffWidth_Right <= 1)
		&& (RightDiffWidth_Left <= 1 && RightDiffWidth_Right <= 1)
		&& (KeepMidHMid > 40)
		)
		return 1;
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief 根据两点计算一条直线
* @param[out] 返回直线斜率和斜矩
**/
Status CalculateOneLine(int OneLinePoint[], float *OneLinek, float *OneLineb){
	*OneLinek = 0.0f;
	*OneLineb = 0.0f;
	if (0 == (OneLinePoint[2] - OneLinePoint[0]))	return 1;
	*OneLinek = (float)(OneLinePoint[3] - OneLinePoint[1]) / (float)(OneLinePoint[2] - OneLinePoint[0]);
	*OneLineb = (float)OneLinePoint[1] - *OneLinek * (float)OneLinePoint[0];
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:初始化全局变量
**/
Status InitMainVariables(void){
	for (int i = 0; i < CAMERA_H; i++){
		LeftEdgeGo(i) = START_COL;
		CenterGuideGo(i) = CENTER_GUIDE;
		RightEdgeGo(i) = END_COL;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:初始化起始搜索
**/
Status InitBeginLine(BeginLine  *_BeginLine){
	_BeginLine->LeftBeginFilter = START_COL;
	_BeginLine->RightBeginFilter = END_COL;
	_BeginLine->LeftValidLine = 0;
	_BeginLine->RightValidLine = 0;
	_BeginLine->ValidEndLine = 0;
	_BeginLine->LoseLineFlag = 0;
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief 搜索单边界，自定义中心搜索,二分法搜索
* @param[in] 当前行---CurrentLine
* @param[in] 列的起始点、终止点、中心点----HStart/HEnd/HCenter
* @param[in] 搜索的当前位置---LocSite
* @return 0---未搜索到   1---搜索到 2---搜索中心点出错
**/
Status SearchOneHorrizonEdge(int CurrentLine, int HStart, int HEnd, int HCenter, Site LocSite){
	int KeepLeft = HStart, KeepRight = HEnd, KeepMid = HCenter;

	/*限制边界*/
	if (KeepLeft > KeepRight) return 0;
	if (KeepMid < KeepLeft || KeepMid > KeepRight)
		KeepMid = (KeepLeft + KeepRight) / 2;
	if (KeepMid < HStart)	KeepMid = HStart;
	if (KeepMid > HEnd) KeepMid = HEnd;

	if (Left == LocSite){	//左搜索
		for (int i = 0; i < SEARCH_TIMES; i++){
			if (JudgeWhite(img(CurrentLine, KeepMid))){
				KeepRight = KeepMid;
			}
			else{
				KeepLeft = KeepMid;
			}
			KeepMid = (KeepLeft + KeepRight) / 2;
			if ((KeepRight - KeepLeft) <= 1){
				break;
			}
		}
		if (JudgeWhite(img(CurrentLine, KeepRight)))
			if (JudgeBlack(img(CurrentLine, KeepLeft))){
				LeftEdgeGo(CurrentLine) = KeepLeft;
				return 1;
			}
		return 0;
	}
	else{	//右搜索
		for (int i = 0; i < SEARCH_TIMES; i++){
			if (JudgeWhite(img(CurrentLine, KeepMid))){
				KeepLeft = KeepMid;
			}
			else{
				KeepRight = KeepMid;
			}
			KeepMid = (KeepLeft + KeepRight) / 2;
			if ((KeepRight - KeepLeft) <= 1){
				break;
			}
		}
		if (JudgeBlack(img(CurrentLine, KeepRight)))
			if (JudgeWhite(img(CurrentLine, KeepLeft))){
				RightEdgeGo(CurrentLine) = KeepRight;
				return 1;
			}
		return 0;
	}
}
////////////////////////////////////////////
//Lock
/**
* @brief 双边搜索边线
**/
Status SearchDoubleEdge(int VStart, int VEnd, int HStart, int HEnd, BeginLine *_BeginLine){
	int CountLine = 0;
	int LeftSearchFlag[5] = { 0 };
	int RightSearchFlag[5] = { 0 };	//存储找点标志位
	int LeftSearchEdge[5] = { 0 };
	int RightSearchEdge[5] = { 0 };	//存储点
	int LoseLeftFlag = 0, LoseRightFlag = 0;
	int LastLoseLeft = 0, LastLoseRight = 0;
	int DiffDistance = 0;
	int LeftEdgeCount = 0, RightEdgeCount = 0;	//左右搜索使用
	int LeftClimbSite = 0, RightClimbSite = 0;	//0---左右搜索 1---沿着边界爬
	int DetectLeftFlag = 0, DetectRightFlag = 0; //基础行搜索不正
	int BaseLineFlag = 0;
	int OverLeftFlag = 0, OverRightFlag = 0; //修补过度，出图像边界
	int LastOverLeft = 0, LastOverRight = 0;
	int ReviseLeftOutFlag = 0, ReviseRightOutFlag = 0;
	int KeepReviseLeftLine = 0, KeepReviseRightLine = 0;
	int IsObstacle = 0, ObOneTime = 0;
	int ObStartSearchLine = 0, ObEndSearchLine = 0;  //障碍物搜索起始和终止
	int ObSearchCount = 0; //障碍物边缘计数
	Site ObComeOn = None;	//记录搜索的是左边还是右边
	int ObSaveEdge[CAMERA_H] = { 0 };	//存放障碍物边界值
	int ObSaveOtherEdge[CAMERA_H] = { 0 }; //存放障碍物边界值
	int ObSearchLeftEdge = 0, ObSearchRightEdge = 0;
	int _Center = CENTER_GUIDE;	//初始默认值, 记录上一行列值搜索
	InitBeginLine(_BeginLine);

	for (int i = VStart; i < VEnd; i++){
		//=====执行左搜索
		if (0 == LeftClimbSite){
			//=====找到右边界线就跟换边界线搜索，预防左边界>右边界
			if (0 == _BeginLine->RightValidLine)	{
				LoseLeftFlag = SearchOneHorrizonEdge(i, _BeginLine->LeftBeginFilter, \
					HEnd, _Center, Left);
			}
			else{
				LoseLeftFlag = SearchOneHorrizonEdge(i, _BeginLine->LeftBeginFilter, \
					RightEdgeGo(i - 1), _Center, Left);
			}
		}
		else{
			//=====找到连续的起始5行之后的搜索
			LoseLeftFlag = SupplyLineDotLoc(i, HStart, HEnd, Left, &OverLeftFlag);
		}
		//=====执行右搜索
		if (0 == RightClimbSite){
			//=====找到左边界线就跟换边界线搜索，预防右边界大于左边界
			if (0 == _BeginLine->LeftValidLine){
				if (1 == LoseLeftFlag){
					LoseRightFlag = SearchOneHorrizonEdge(i, LeftEdgeGo(i), \
						_BeginLine->RightBeginFilter, _Center, Right);
				}
				else{
					LoseRightFlag = SearchOneHorrizonEdge(i, HStart, \
						_BeginLine->RightBeginFilter, _Center, Right);
				}
			}
			//=====右边界未找到连续的起始5行，但是左边界却已经找到了，进行一次宽度滤波
			else{
				LoseRightFlag = SearchOneHorrizonEdge(i, LeftEdgeGo(i - 1), \
					_BeginLine->RightBeginFilter, _Center, Right);
			}
		}
		else{
			//=====找到连续的起始5行之后的搜索
			LoseRightFlag = SupplyLineDotLoc(i, HStart, HEnd, Right, &OverRightFlag);
		}

		//=====执行中心线搜索
		if (0 == LeftClimbSite && 1 == RightClimbSite){
			//=====记录下左边界丢失到右边线丢失的行值
			if (1 == LastLoseRight && 0 == LoseRightFlag){
				KeepReviseRightLine = i - 1;
			}
			//=====左边线丢失到右边线丢失
			if (0 == LastLoseLeft && 1 == LoseLeftFlag && 0 == LoseRightFlag && 0 != KeepReviseRightLine){
				ReviseLeftOutFlag = 1;
			}
			//=====左边丢线到双边找到
			if (0 == LastLoseLeft && 1 == LoseLeftFlag && 1 == LoseRightFlag){
				int GoCenter = myround((float)(LeftEdgeGo(i) + RightEdgeGo(i)) / 2.0f);
				DiffDistance = RightEdgeGo(i) - GoCenter - LoseEdgeWidth[i];
				for (int n = _BeginLine->RightValidLine; n < i; n++){
					LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Right);
				}
			}
		}
		if (1 == LeftClimbSite && 0 == RightClimbSite){
			if (1 == LastLoseLeft && 0 == LoseLeftFlag){
				KeepReviseLeftLine = i - 1;
			}
			if (0 == LastLoseRight && 1 == LoseRightFlag && 0 == LoseLeftFlag && 0 != KeepReviseLeftLine){
				ReviseRightOutFlag = 1;
			}
			if (0 == LastLoseRight && 1 == LoseRightFlag && 1 == LoseLeftFlag){
				int GoCenter = myround((float)(LeftEdgeGo(i) + RightEdgeGo(i)) / 2.0f);
				DiffDistance = GoCenter - LeftEdgeGo(i) - LoseEdgeWidth[i];
				for (int n = _BeginLine->LeftValidLine; n < i; n++){
					LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Left);
				}
			}
		}
		//=====左边线丢失到右边线丢失的中心线修补
		if (1 == LeftClimbSite && 1 == RightClimbSite && 1 == ReviseLeftOutFlag){
			int LeftEdgeTemp = 0, RightEdgeTemp = 0;
			ReviseLeftOutFlag = 0;
			LinearLine(&LeftTLSM, _BeginLine->LeftValidLine, 5, Left);
			for (int n = _BeginLine->RightValidLine; n <= KeepReviseRightLine; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeTemp + RightEdgeGo(n)) / 2.0f);
			}
			LinearLine(&RightTLSM, KeepReviseRightLine - 5, 5, Right);
			for (int n = KeepReviseRightLine + 1; n < _BeginLine->LeftValidLine; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeTemp + RightEdgeTemp) / 2.0f);
			}
			for (int n = _BeginLine->LeftValidLine; n <= _BeginLine->LeftValidLine + 5; n++){
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeGo(n) + RightEdgeTemp) / 2.0f);
			}
			DiffDistance = CenterGuideGo(_BeginLine->LeftValidLine) \
				- LeftEdgeGo(_BeginLine->LeftValidLine)	\
				- LoseEdgeWidth[_BeginLine->LeftValidLine];
		}
		//=====右边线丢失到左边线丢失的中心线修补
		if (1 == LeftClimbSite && 1 == RightClimbSite && 1 == ReviseRightOutFlag){
			int LeftEdgeTemp = 0, RightEdgeTemp = 0;
			ReviseRightOutFlag = 0;
			LinearLine(&RightTLSM, _BeginLine->RightValidLine, 5, Right);
			for (int n = _BeginLine->LeftValidLine; n <= KeepReviseLeftLine; n++){
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(RightEdgeTemp + LeftEdgeGo(n)) / 2.0f);
			}
			LinearLine(&LeftTLSM, KeepReviseLeftLine - 5, 5, Left);
			for (int n = KeepReviseLeftLine + 1; n < _BeginLine->RightValidLine; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeTemp + RightEdgeTemp) / 2.0f);
			}
			for (int n = _BeginLine->RightValidLine; n <= _BeginLine->RightValidLine + 5; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				CenterGuideGo(n) = myround((float)(RightEdgeGo(n) + LeftEdgeTemp) / 2.0f);
			}
			DiffDistance = RightEdgeGo(_BeginLine->RightValidLine)\
				- CenterGuideGo(_BeginLine->RightValidLine)	\
				- LoseEdgeWidth[_BeginLine->RightValidLine];
		}
		//=====左右双线找到后单边丢失的中心线修补
		if (1 == LeftClimbSite && 1 == RightClimbSite){
			if (1 == OverLeftFlag && 0 == LastOverLeft){
				DiffDistance = RightEdgeGo(i - 1) - CenterGuideGo(i - 1) - LoseEdgeWidth[i - 1];
			}
			if (1 == OverRightFlag && 0 == LastOverRight){
				DiffDistance = CenterGuideGo(i - 1) - LeftEdgeGo(i - 1) - LoseEdgeWidth[i - 1];
			}
		}
		//======普通中心线补偿
		//======未超出图像边界但是边线找不到，经过补线认为这次有效
		if (0 == OverLeftFlag && 0 == LoseLeftFlag && 1 == LeftClimbSite) LoseLeftFlag = 1;
		if (0 == OverRightFlag && 0 == LoseRightFlag && 1 == RightClimbSite) LoseRightFlag = 1;
		//======超出图像边界，无法补线认为这次无效
		if (1 == OverLeftFlag) LoseLeftFlag = !OverLeftFlag;
		if (1 == OverRightFlag)LoseRightFlag = !OverRightFlag;
		//======根据边线丢失情况补上中心线
		SupplyMidLine(i, DiffDistance, LoseLeftFlag, LoseRightFlag, HStart, HEnd);
		//======记录上一次搜索的丢边值以及出图像外丢边值
		LastLoseLeft = LoseLeftFlag;
		LastLoseRight = LoseRightFlag;
		LastOverLeft = OverLeftFlag;
		LastOverRight = OverRightFlag;

		/*搜索中心列值*/
		_Center = CenterGuideGo(i - 1);

		/*障碍物搜索,从中心线往两边搜索*/
		//为了抗干扰，搜索的边界应该是左右搜索到的边界线
		//ObComeOn的使用是为了保证搜索能够一直在一侧搜索，这可以增加搜索的速度
		//仅仅是第一次在中心线双边搜索
		if (1 == LeftClimbSite) ObSearchLeftEdge = LeftEdgeGo(i);
		else ObSearchLeftEdge = HStart;
		if (1 == RightClimbSite) ObSearchRightEdge = RightEdgeGo(i);
		else ObSearchRightEdge = HEnd;
		IsObstacle = SearchObStacleFromMid(i,
			ObSearchLeftEdge, ObSearchRightEdge, VEnd,
			&ObComeOn, ObSaveEdge, ObSaveOtherEdge,
			&ObStartSearchLine, &ObEndSearchLine, &ObSearchCount);
		if (1 == IsObstacle && 0 == ObOneTime){	//仅仅修补一次
			ObOneTime = 1;
			ObIsObStacle = 1;
			ObValidLine = ObEndSearchLine;	//记录障碍物对应的有效行
			ReviseObStacleFromMid(ObComeOn, ObSaveEdge, ObStartSearchLine, ObEndSearchLine);
			break;
		}

		/*动态存储丢失点的标志位和点*/
		if (i <= (VStart + 4)){	//第一次将数组装满
			LeftSearchFlag[CountLine] = LoseLeftFlag;
			RightSearchFlag[CountLine] = LoseRightFlag;
			LeftSearchEdge[CountLine] = LeftEdgeGo(i);
			RightSearchEdge[CountLine] = RightEdgeGo(i);
			CountLine++;
			if (CountLine >= 5) CountLine = 0; //重新存储
		}
		if (i > (VStart + 4) && 0 == LeftClimbSite){
			for (int n = 0; n < 4; n++){
				LeftSearchFlag[n] = LeftSearchFlag[n + 1];
				LeftSearchEdge[n] = LeftSearchEdge[n + 1];
			}
			LeftSearchFlag[4] = LoseLeftFlag;
			LeftSearchEdge[4] = LeftEdgeGo(i);
		}
		if (i >(VStart + 4) && 0 == RightClimbSite){	//跟新数组
			for (int n = 0; n < 4; n++){	//每增加一个点动态变化数组
				RightSearchFlag[n] = RightSearchFlag[n + 1];
				RightSearchEdge[n] = RightSearchEdge[n + 1];
			}
			RightSearchFlag[4] = LoseRightFlag;
			RightSearchEdge[4] = RightEdgeGo(i);
		}

		/*在未找到起始行前，动态存储的点计数，方便后面进行判断*/
		if (0 == LeftClimbSite){	/*左点搜索计数*/
			LeftEdgeCount = 0;
			for (int n = 0; n < 5; n++){
				if (1 == LeftSearchFlag[n]){
					LeftEdgeCount++;
				}
			}
		}
		if (0 == RightClimbSite){	/*右点搜索计数*/
			RightEdgeCount = 0;
			for (int n = 0; n < 5; n++){
				if (1 == RightSearchFlag[n]){
					RightEdgeCount++;
				}
			}
		}
		/********************************图像修补***********************************************/
		/*十字不正检测*/
		if (i == (VStart + 4)){
			DetectLeftFlag = DetectStartErr(i, _BeginLine, Left);
			if (1 == DetectLeftFlag){	//有夹角
				ReviseStartErr(i, Left, _BeginLine);
				LeftEdgeCount = 0;
				LeftClimbSite = 1;
			}
			if (DetectLeftFlag > 1){	//斜入十字
				LeftEdgeCount = 0;
				for (int n = 0; n < 5; n++){
					LeftSearchFlag[n] = 0;
				}
			}
			DetectRightFlag = DetectStartErr(i, _BeginLine, Right);
			if (1 == DetectRightFlag){
				ReviseStartErr(i, Right, _BeginLine);
				RightEdgeCount = 0;
				RightClimbSite = 1;
			}
			if (DetectRightFlag > 1){
				RightEdgeCount = 0;
				for (int n = 0; n < 5; n++){
					RightSearchFlag[n] = 0;
				}
			}
		}
		/***************************************************************************************/
		/*连续5行进行判断---是否连续*/
		if (i >= (VStart + 4) && (0 == LeftClimbSite || 0 == RightClimbSite)){	//至少存在5个点
			if (5 == LeftEdgeCount && 0 == LeftClimbSite){
				BaseLineFlag = ReviseBaseLine(i, Left);
				if (0 == BaseLineFlag){
					_BeginLine->LeftValidLine = 0;
					LeftClimbSite = 0;
					for (int n = 0; n < 5; n++){
						LeftSearchFlag[n] = 0;
					}
				}
				else{
					_BeginLine->LeftValidLine = i - 4;
					LeftClimbSite = 1;
				}
			}
			if (5 == RightEdgeCount && 0 == RightClimbSite){
				BaseLineFlag = ReviseBaseLine(i, Right);
				if (0 == BaseLineFlag){
					_BeginLine->RightValidLine = 0;
					RightClimbSite = 0;
					for (int n = 0; n < 5; n++){
						RightSearchFlag[n] = 0;
					}
				}
				else{
					_BeginLine->RightValidLine = i - 4;
					RightClimbSite = 1;
				}
			}
		}
		/*连续基础5行搜索结束后*/
		if (DetectLeftFlag > 1){
			if (0 == RightClimbSite && 4 == DetectLeftFlag){
				_BeginLine->LeftBeginFilter = LeftEdgeGo(i - 4) + FILTER_MAX;
			}
		}
		if (DetectRightFlag > 1){
			if (0 == LeftClimbSite && 4 == DetectRightFlag){
				_BeginLine->RightBeginFilter = RightEdgeGo(i - 4) - FILTER_MAX;
			}
		}
	}
	/**********************************************************************************************/
	/*边线搜索完成后进行丢线判断*/
	if ((0 == _BeginLine->LeftValidLine) && (0 == _BeginLine->RightValidLine)){
		_BeginLine->LoseLineFlag = DoubleLose;//双边找不到
	}
	else if ((0 != _BeginLine->LeftValidLine) && (0 == _BeginLine->RightValidLine)){
		_BeginLine->LoseLineFlag = RightLose;	//找到边线
		int KeepJumpLine = 0, KeepJumpCow = 0;
		for (int n = CENTER_GUIDE; n < (CAMERA_W - 1); n++){
			if (JudgeWhite(img(START_ROW + 7, n)))
				if (JudgeBlack(img(START_ROW + 7, n + 1))){
					KeepJumpCow = n + 1;
					break;
				}
		}
		if (0 != KeepJumpCow){
			DiffDistance = myround((float)(LeftEdgeGo(START_ROW + 7) + KeepJumpCow) / 2.0f)	\
				- LeftEdgeGo(START_ROW + 7) - LoseEdgeWidth[START_ROW + 7];
		}
		else{
			KeepJumpCow = END_COL - 1;
			for (int n = _BeginLine->LeftValidLine; n<END_ROW; n++){
				if (JudgeWhite(img(n, KeepJumpCow)))
					if (JudgeBlack(img(n + 1, KeepJumpCow))){
						KeepJumpLine = n;
						break;
					}
			}
			int Point[4] = { 0 };
			float k = 0.0f, b = 0.0f;
			Point[0] = START_ROW;
			Point[1] = LeftEdgeGo(Point[0]);
			Point[2] = KeepJumpLine;
			Point[3] = LeftEdgeGo(Point[2]);
			CalculateOneLine(Point, &k, &b);
			k = (float)fabs(k);
			DiffDistance = (int)(8.7f * k - 3.5f);
			if (START_ROW == KeepJumpLine) DiffDistance = 0;
		}
		//===修补中心线
		if (0 == IsObstacle){
			for (int n = _BeginLine->LeftValidLine; n < END_ROW; n++){
				LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Left);
			}
		}
	}
	else if ((0 == _BeginLine->LeftValidLine) && (0 != _BeginLine->RightValidLine)){
		_BeginLine->LoseLineFlag = LeftLose; //找到边线
		int KeepJumpLine = 0, KeepJumpCow = 0;
		for (int n = CENTER_GUIDE; n > 0; n--){
			if (JudgeWhite(img(START_ROW + 7, n)))
				if (JudgeBlack(img(START_ROW + 7, n - 1))){
					KeepJumpCow = n - 1;
					break;
				}
		}
		if (0 != KeepJumpCow){
			DiffDistance = RightEdgeGo(START_ROW + 7)	\
				- myround((float)(KeepJumpCow + RightEdgeGo(START_ROW + 7)) / 2.0f)
				- LoseEdgeWidth[START_ROW + 7];
		}
		else{
			KeepJumpCow = START_COL + 1;
			for (int n = _BeginLine->RightValidLine; n<END_ROW; n++){
				if (JudgeWhite(img(n, KeepJumpCow)))
					if (JudgeBlack(img(n + 1, KeepJumpCow))){
						KeepJumpLine = n;
						break;
					}
			}
			int Point[4] = { 0 };
			float k = 0.0f, b = 0.0f;
			Point[0] = START_ROW;
			Point[1] = RightEdgeGo(Point[0]);
			Point[2] = KeepJumpLine;
			Point[3] = RightEdgeGo(Point[2]);
			CalculateOneLine(Point, &k, &b);
			k = (float)fabs(k);
			DiffDistance = (int)(8.7f * k - 3.5f);
			if (START_ROW == KeepJumpLine) DiffDistance = 0;
		}
		//===修补中心线
		if (0 == IsObstacle){
			for (int n = _BeginLine->RightValidLine; n < END_ROW; n++){
				LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Right);
			}
		}
	}
	else{
		_BeginLine->LoseLineFlag = NoneLose; //找到边线
	}
	return OK;
}
/////////////////////////////////////////////
//Lock
/**
* @breif 修补搜索到的起始基础行
**/
Status ReviseBaseLine(int CurrentLine, Site LocSite){
	int Point[4] = { 0 };
	float k = 0.0f, b = 0.0f;
	int IsDo[3] = { 0 };
	if (Left == LocSite){
		int LeftStartDiff[4] = { 0 };
		LeftStartDiff[0] = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 1);
		LeftStartDiff[1] = LeftEdgeGo(CurrentLine - 1) - LeftEdgeGo(CurrentLine - 2);
		LeftStartDiff[2] = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 3);
		LeftStartDiff[3] = LeftEdgeGo(CurrentLine - 3) - LeftEdgeGo(CurrentLine - 4);
		if (
			(LeftStartDiff[0] == 0 && LeftStartDiff[1] == 0 && LeftStartDiff[2] == 0 && abs(LeftStartDiff[3]) == 1)
			|| (LeftStartDiff[0] == 0 && LeftStartDiff[1] == 0 && abs(LeftStartDiff[2]) == 1 && LeftStartDiff[3] == 0)
			|| (LeftStartDiff[0] == 0 && abs(LeftStartDiff[1]) == 1 && LeftStartDiff[2] == 0 && LeftStartDiff[3] == 0)
			|| (abs(LeftStartDiff[0]) == 1 && LeftStartDiff[1] == 0 && LeftStartDiff[2] == 0 && LeftStartDiff[3] == 0)
			|| (LeftStartDiff[0] == 0 && LeftStartDiff[1] == -1 && LeftStartDiff[2] == 1 && LeftStartDiff[3] == 1)
			){
			LeftTLSM.k = 0.0f;
			LeftTLSM.b = 0.0f;
			return 0;
		}
		/*十字畸变矫正*/
		if (
			((LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine)) >= 4)
			|| ((LeftEdgeGo(CurrentLine - 4) - LeftEdgeGo(CurrentLine) >= 7))
			|| LeftStartDiff[0] >= 6
			|| LeftStartDiff[1] >= 6
			|| LeftStartDiff[2] >= 6
			|| LeftStartDiff[3] >= 6
			){
			LeftTLSM.k = 0.0f;
			LeftTLSM.b = 0.0f;
			_BeginLine.LeftValidLine = LeftEdgeGo(CurrentLine - 4) + FILTER_MAX;
			return 0;
		}
		else if (LeftStartDiff[0] == 0 && LeftStartDiff[1] == 0
			&& LeftStartDiff[2] == 0 && LeftStartDiff[3] == 0){
			LeftTLSM.k = 0.0f;
			LeftTLSM.b = 0.0f;
			return 0;
		}
		/*起始拐点左矫正*/
		else{
			int LeftStartDiff1 = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 2);
			int LeftStartDiff2 = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 4);
			IsDo[0] = ((LeftStartDiff2 >= 1 && LeftStartDiff1 < -1)	\
				|| (LeftStartDiff2 > 1 && LeftStartDiff1 <= -1));
			IsDo[1] = (LeftStartDiff[0] <= 0 && LeftStartDiff[1] <= 0	\
				&& LeftStartDiff[2] >= 0 && LeftStartDiff[3] >= 0);
			IsDo[2] = (LeftStartDiff[0] <= -2 && LeftStartDiff[1] == 1	\
				&& LeftStartDiff[2] == 0 && LeftStartDiff[3] == 1);
			if ((IsDo[0]&& IsDo[1]) || IsDo[2]){
				Point[0] = CurrentLine - 4;
				Point[1] = LeftEdgeGo(Point[0]);
				Point[2] = CurrentLine - 2;
				Point[3] = LeftEdgeGo(Point[2]);
				CalculateOneLine(Point, &k, &b);
				for (int n = CurrentLine - 2; n <= CurrentLine; n++){
					LeftEdgeGo(n) = ForecastTHSM(n, k, b);
				}
				LeftTLSM.k = k;
				LeftTLSM.b = b;
			}
			else{
				LinearLine(&LeftTLSM, CurrentLine - 4, 5, Left);		//跟新最小二乘
			}
			return 1;
		}
	}
	if (Right == LocSite){
		int RightStartDiff[4] = { 0 };
		RightStartDiff[0] = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 1);
		RightStartDiff[1] = RightEdgeGo(CurrentLine - 1) - RightEdgeGo(CurrentLine - 2);
		RightStartDiff[2] = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 3);
		RightStartDiff[3] = RightEdgeGo(CurrentLine - 3) - RightEdgeGo(CurrentLine - 4);
		if ((RightStartDiff[0] == 0 && RightStartDiff[1] == 0 && RightStartDiff[2] == 0 && abs(RightStartDiff[3]) == 1)
			|| (RightStartDiff[0] == 0 && RightStartDiff[1] == 0 && abs(RightStartDiff[2]) == 1 && RightStartDiff[3] == 0)
			|| (RightStartDiff[0] == 0 && abs(RightStartDiff[1]) == 1 && RightStartDiff[2] == 0 && RightStartDiff[3] == 0)
			|| (abs(RightStartDiff[0]) == 1 && RightStartDiff[1] == 0 && RightStartDiff[2] == 0 && RightStartDiff[3] == 0)
			|| (RightStartDiff[0] == 0 && RightStartDiff[1] == 1 && RightStartDiff[2] == -1 && RightStartDiff[3] == -1)
			){
			RightTLSM.k = 0.0f;
			RightTLSM.b = 0.0f;
			return 0;
		}
		/*十字畸变矫正*/
		if (
			((RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2)) >= 4)
			|| ((RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 4) >= 7))
			|| RightStartDiff[0] <= -6
			|| RightStartDiff[1] <= -6
			|| RightStartDiff[2] <= -6
			|| RightStartDiff[3] <= -6
			){
			RightTLSM.k = 0.0f;
			RightTLSM.b = 0.0f;
			_BeginLine.RightBeginFilter = RightEdgeGo(CurrentLine - 4) - FILTER_MAX;
			return 0;
		}
		else if (RightStartDiff[0] == 0 && RightStartDiff[1] == 0
			&& RightStartDiff[2] == 0 && RightStartDiff[3] == 0){
			RightTLSM.k = 0.0f;
			RightTLSM.b = 0.0f;
			return 0;
		}
		else{
			/*起始拐点右矫正*/
			int RightStartDiff1 = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2);
			int RightStartDiff2 = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 4);
			IsDo[0] = ((RightStartDiff1 >= 1 && RightStartDiff2 < -1)		\
				|| (RightStartDiff1 > 1 && RightStartDiff2 <= -1));
			IsDo[1] = (RightStartDiff[0] >= 0 && RightStartDiff[1] >= 0	\
				&& RightStartDiff[2] <= 0 && RightStartDiff[3] <= 0);
			IsDo[2] = (RightStartDiff[0] >= 2 && RightStartDiff[1] == -1	\
				&& RightStartDiff[2] == 0 && RightStartDiff[3] == -1);
			if ((IsDo[0] && IsDo[1]) || IsDo[2]){
				Point[0] = CurrentLine - 4;
				Point[1] = RightEdgeGo(Point[0]);
				Point[2] = CurrentLine - 2;
				Point[3] = RightEdgeGo(Point[2]);
				CalculateOneLine(Point, &k, &b);
				for (int n = CurrentLine - 2; n <= CurrentLine; n++){
					RightEdgeGo(n) = ForecastTHSM(n, k, b);
				}
				RightTLSM.k = k;
				RightTLSM.b = b;
			}
			else{
				LinearLine(&RightTLSM, CurrentLine - 4, 5, Right);		//跟新最小二乘
			}
			return 1;
		}
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif 搜索障碍物
**/
Status SearchObStacleFromMid(int CurrentLine,
	int HStart, int HEnd, int VEnd,
	Site* SearchFlag, int* ObEdge, int* ObOtherEdge,
	int* ObFirstLine, int* ObEndLine, int* ObCount){
	//搜索到的障碍物边缘连续，并且靠近中心线一定的距离
	//由于锲型物体的一面反光严重，所以单行判断是不准确的 
	int ObActualCenterWidth = 0, ObRealCenterWidth = 0;
	int EdgeColTemp[2] = { 0 };
	int ObCenterDot[2] = { 0 };
	int WhiteToBlackFlag = 0, BlackToWhiteFlag = 0;
	static int FirstComeIn = 0;
	int DoubleEdge[2] = { 0 };
	int MidCow = CenterGuideGo(CurrentLine);

	//第一次搜索，决定是执行左搜索还是右搜索，不做任何特征判断
	if (None == *SearchFlag){
		//第一次搜索的时候，中线线上不可能是黑色障碍物
		if (JudgeBlack(img(CurrentLine, MidCow)))
			return 0;
		//执行左搜索
		for (int n = MidCow; n > HStart; n--){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n - 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n - 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n - 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n - 1;
					}
			//必须要有白黑，黑白的跳变才可能作为障碍物
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[0] - DoubleEdge[1]) <= 20
				&& (DoubleEdge[0] - DoubleEdge[1]) >= 5){
				//第一次装填边界值,决定左搜索
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObFirstLine = CurrentLine;
				*ObCount = 0;	//第一次清零
				*ObCount += 1;
				*SearchFlag = Left;
				FirstComeIn = 1;
				return 0;
			}
		}
		//左边搜索到就不必继续搜索
		if (*SearchFlag == Left) return 0;
		WhiteToBlackFlag = 0;
		BlackToWhiteFlag = 0;
		DoubleEdge[0] = 0;
		DoubleEdge[1] = 0;
		//搜索右边界
		for (int n = MidCow; n < HEnd; n++){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n + 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n + 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n + 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n + 1;
					}
			//必须要有白黑，黑白的跳变才可能作为障碍物
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[1] - DoubleEdge[0]) <= 20
				&& (DoubleEdge[1] - DoubleEdge[0]) >= 5){
				//第一次装填边界值,决定右边界
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObFirstLine = CurrentLine;
				*ObCount = 0;	//第一次清零
				*ObCount += 1;
				*SearchFlag = Right;
				FirstComeIn = 1;
				return 0;
			}
		}
	}
	else if (Left == *SearchFlag){
		for (int n = MidCow; n > HStart; n--){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n - 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n - 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n - 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n - 1;
					}
			//找到最后一列还是找不到就默认
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag && n == (HStart + 1)){
				if (JudgeBlack(img(CurrentLine, n))){
					BlackToWhiteFlag = 1;
					DoubleEdge[1] = n - 1;
				}
			}
			//必须要有白黑，黑白的跳变才可能作为障碍物
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[0] - DoubleEdge[1]) <= 20){
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObCount += 1;
				break;	//跳出搜索作为判断
			}
		}
		//第一次进来可能会有严重畸变需要滤除
		if (1 == FirstComeIn
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			&& (abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (CurrentLine == (*ObFirstLine + 1))
			){
			FirstComeIn = 0;
			*ObCount = 0;	//重新计数
			*SearchFlag = None;	//重新搜索第一行
			return 0;
		}
		//决定终止行
		if (CurrentLine == (VEnd - 1)	//搜索到最后一列还是搜索到，这属于特殊情况
			&& 0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag){
			*ObEndLine = VEnd - 1;
		}
		//畸变严重
		if ((abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			){
			*ObEndLine = CurrentLine - 1;
		}
		//下一行找不到
		if (0 == WhiteToBlackFlag || 0 == BlackToWhiteFlag){
			*ObEndLine = CurrentLine - 1;
		}

		//不足3行重新找
		if (0 != *ObFirstLine && 0 != *ObEndLine && *ObCount < 5){
			*ObCount = 0;	//重新计数
			*SearchFlag = None;	//重新搜索第一行
			return 0;
		}
		//看来这次是真的找到障碍物了，但是还是要判断一下
		if (0 != *ObFirstLine && 0 != *ObEndLine){
			//获取障碍物的中心点
			ObCenterDot[0] = (*ObFirstLine + *ObEndLine) / 2;
			EdgeColTemp[0] = (*(ObEdge + *ObFirstLine) + *(ObEdge + *ObEndLine)) / 2;
			EdgeColTemp[1] = (*(ObOtherEdge + *ObFirstLine) + *(ObOtherEdge + *ObEndLine)) / 2;
			ObCenterDot[1] = (EdgeColTemp[0] + EdgeColTemp[1]) / 2;
			//与中心线的距离要固定---与中心线的距离是5cm
			//图像因为有畸变，其宽度值应当满足一个线性关系
			ObActualCenterWidth = 25 - (int)(0.15f * ObCenterDot[0]);
			ObRealCenterWidth = (LeftEdgeGo(ObCenterDot[0]) \
				+ RightEdgeGo(ObCenterDot[0])) / 2 - ObCenterDot[1];
			//最后做一个判断，是否具有障碍物特征，否则视为无效
			if (ObRealCenterWidth <= ObActualCenterWidth){
				return 1;
			}
			else{
				*ObCount = 0;	//重新计数
				*SearchFlag = None;	//重新搜索第一行
				return 0;
			}
		}
	}
	else if (Right == *SearchFlag){
		for (int n = MidCow; n < HEnd; n++){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n + 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n + 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n + 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n + 1;
					}
			//找到最后一列还是找不到就默认
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag && n == (HEnd - 1)){
				if (JudgeBlack(img(CurrentLine, n))){
					BlackToWhiteFlag = 1;
					DoubleEdge[1] = n + 1;
				}
			}
			//必须要有白黑，黑白的跳变才可能作为障碍物
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[1] - DoubleEdge[0]) <= 20){
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObCount += 1;
				break;	//跳出搜索作为判断
			}
		}
		//第一次进来可能会有严重畸变需要滤除
		if (1 == FirstComeIn
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			&& (abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (CurrentLine == (*ObFirstLine + 1))
			){
			FirstComeIn = 0;
			*ObCount = 0;	//重新计数
			*SearchFlag = None;	//重新搜索第一行
			return 0;
		}
		//决定终止行
		if (CurrentLine == (VEnd - 1)	//搜索到最后一列还是搜索到，这属于特殊情况
			&& 0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag){
			*ObEndLine = VEnd - 1;
		}
		//畸变严重
		if ((abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			){
			*ObEndLine = CurrentLine - 1;
		}
		if (0 == WhiteToBlackFlag || 0 == BlackToWhiteFlag){
			*ObEndLine = CurrentLine - 1;
		}
		//不足5行重新找
		if (0 != *ObFirstLine && 0 != *ObEndLine && *ObCount < 5){
			*ObCount = 0;	//重新计数
			*SearchFlag = None;	//重新搜索第一行
			return 0;
		}
		//看来这次是真的找到障碍物了，但是还是要判断一下
		if (0 != *ObFirstLine && 0 != *ObEndLine){
			//获取障碍物的中心点
			ObCenterDot[0] = (*ObFirstLine + *ObEndLine) / 2;
			EdgeColTemp[0] = (*(ObEdge + *ObFirstLine) + *(ObEdge + *ObEndLine)) / 2;
			EdgeColTemp[1] = (*(ObOtherEdge + *ObFirstLine) + *(ObOtherEdge + *ObEndLine)) / 2;
			ObCenterDot[1] = (EdgeColTemp[0] + EdgeColTemp[1]) / 2;
			//与中心线的距离要固定---与中心线的距离是5cm
			//图像因为有畸变，其宽度值应当满足一个线性关系
			//50-7 35-10
			ObActualCenterWidth = 25 - (int)(0.15f * ObCenterDot[0]); //20 0.2
			ObRealCenterWidth = ObCenterDot[1] - (LeftEdgeGo(ObCenterDot[0]) \
				+ RightEdgeGo(ObCenterDot[0])) / 2;
			//最后做一个判断，是否具有障碍物特征，否则视为无效
			if (ObRealCenterWidth <= ObActualCenterWidth){
				return 1;
			}
			else{
				*ObCount = 0;	//重新计数
				*SearchFlag = None;	//重新搜索第一行
				return 0;
			}
		}
	}
	else;

	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @bref 修补由障碍物引发的中心线
**/
Status ReviseObStacleFromMid(Site LocSite, int* ObEdge, int ObFirstLine, int ObEndLine){
	int Point_X[5] = { 0 };
	int Point_Y[5] = { 0 };
	//===筛选出五组等份值
	Point_X[0] = ObFirstLine + 1;
	Point_X[4] = ObEndLine;
	Point_X[2] = myround((float)(Point_X[0] + Point_X[4]) / 2.0f);
	Point_X[1] = myround((float)(Point_X[0] + Point_X[2]) / 2.0f);
	Point_X[3] = myround((float)(Point_X[2] + Point_X[4]) / 2.0f);

	//决定列项
	if (Left == LocSite){
		for (int n = 0; n < 5; n++){
			Point_Y[n] = *(ObEdge + Point_X[n]) + 0;
		}
		//===根据障碍特征拟合修补过去的点
		CalculateTLSM(&LeftTLSM, Point_X, Point_Y, 5);
		ReviseSetDot(START_ROW, END_ROW - START_ROW, START_COL, END_COL, Left);
	}
	else if (Right == LocSite){
		for (int n = 0; n < 5; n++){
			Point_Y[n] = *(ObEdge + Point_X[n]) - 0;
		}
		CalculateTLSM(&RightTLSM, Point_X, Point_Y, 5);
		ReviseSetDot(START_ROW, END_ROW - START_ROW, START_COL, END_COL, Right);
	}
	else;

	//===还原中线
	for (int n = START_ROW; n <= Point_X[4]; n++){
		CenterGuideGo(n) = (LeftEdgeGo(n) + RightEdgeGo(n)) / 2;
	}
	Point_X[0] = START_ROW;
	Point_X[4] = ObEndLine;
	Point_X[2] = myround((float)(Point_X[0] + Point_X[4]) / 2.0f);
	Point_X[1] = myround((float)(Point_X[0] + Point_X[2]) / 2.0f);
	Point_X[3] = myround((float)(Point_X[2] + Point_X[4]) / 2.0f);
	for (int n = 0; n < 5; n++){
		Point_Y[n] = CenterGuideGo(Point_X[n]);
	}
	CalculateTLSM(&CenterTLSM, Point_X, Point_Y, 5);
	ReviseSetDot(Point_X[4], END_ROW - Point_X[4], START_COL, END_COL, Center);
	//还原乱修补的点
	for (int i = (ObEndLine + 1); i < END_ROW; i++) {
		LeftEdgeGo(i) = START_COL;
		RightEdgeGo(i) = END_COL;
		CenterGuideGo(i) = CENTER_GUIDE;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif 向上修补点
**/
Status ReviseSetDot(int CurrentLine, int Len, int HStart, int HEnd, Site LocSite){
	int DataTemp = 0;
	int isSearchFlag = 0;
	if (Len == 0) return 0;
	if (Left == LocSite){
		for (int i = 0; i < Len; i++){
			DataTemp = ForecastTHSM(CurrentLine + i, LeftTLSM.k, LeftTLSM.b);
			if (DataTemp > HStart && DataTemp < HEnd){
				LeftEdgeGo(CurrentLine + i) = DataTemp;
				isSearchFlag = 0;
			}
			else{
				LeftEdgeGo(CurrentLine + i) = LeftEdgeGo(CurrentLine + i - 1);
				isSearchFlag = 1;
			}
		}
	}
	else if (Right == LocSite){
		for (int i = 0; i < Len; i++){
			DataTemp = ForecastTHSM(CurrentLine + i, RightTLSM.k, RightTLSM.b);
			if (DataTemp > HStart && DataTemp < HEnd){
				RightEdgeGo(CurrentLine + i) = DataTemp;
				isSearchFlag = 0;
			}
			else{
				RightEdgeGo(CurrentLine + i) = RightEdgeGo(CurrentLine + i - 1);
				isSearchFlag = 1;
			}
		}
	}
	else{
		for (int i = 0; i < Len; i++){
			DataTemp = ForecastTHSM(CurrentLine + i, CenterTLSM.k, CenterTLSM.b);
			if (DataTemp > HStart && DataTemp < HEnd){
				CenterGuideGo(CurrentLine + i) = DataTemp;
				isSearchFlag = 0;
			}
			else{
				CenterGuideGo(CurrentLine + i) = CenterGuideGo(CurrentLine + i - 1);
				isSearchFlag = 1;
			}
		}
	}
	return isSearchFlag;
}
////////////////////////////////////////////
//Lock
/**
* @brief:在一个点的附近搜索边界
* @param[in]:起始点---HStart和终止点---HEnd，限定搜索边界
* @param[in]:搜索位置标志---LocFlag
* @param[out]:是否搜索到的标志---SearchFalg 0---搜索不到 1---搜索到
* @return:状态
**/
Status SupplyOneHorrizon(int CurrentLine, int HStart, int HEnd, Site LocFlag, int *SearchFlag){
	int NearByDis = 0;
	if (CurrentLine <= 10){
		NearByDis = 5;
	}
	else if (CurrentLine <= 40){
		NearByDis = 8;
	}
	else{
		NearByDis = 10;
	}
	int PreEdge = 0;
	int Point[2] = { 0 };
	int DesPoint = 0;
	if (Left == LocFlag){
		PreEdge = LeftEdgeGo(CurrentLine - 1);	//左边界搜索的边界限定
		Point[0] = PreEdge - NearByDis;
		Point[1] = PreEdge + NearByDis;
		if (Point[0] < HStart) Point[0] = HStart;
		if (Point[1]  > HEnd) Point[1] = HEnd;

		if (JudgeWhite(img(CurrentLine, PreEdge))){	//白色执行左搜索
			for (int i = PreEdge; i > Point[0]; i--){
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i - 1))){
						DesPoint = i - 1;
						break;
					}
			}
		}
		if (JudgeBlack(img(CurrentLine, PreEdge))){	//黑色执行右搜索
			for (int i = PreEdge; i < Point[1]; i++){
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i + 1))){
						DesPoint = i;
						break;
					}
			}
		}
		if (0 == DesPoint) *SearchFlag = 0;
		else{
			*SearchFlag = 1;
			LeftEdgeGo(CurrentLine) = DesPoint;
		}
	}
	else if (Right == LocFlag){
		PreEdge = RightEdgeGo(CurrentLine - 1);
		Point[0] = PreEdge - NearByDis;
		Point[1] = PreEdge + NearByDis;
		if (Point[0] < HStart) Point[0] = HStart;
		if (Point[1]  > HEnd) Point[1] = HEnd;

		if (JudgeWhite(img(CurrentLine, PreEdge))){	//白色执行右搜索
			for (int i = PreEdge; i < Point[1]; i++){
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i + 1))){
						DesPoint = i + 1;
						break;
					}
			}
		}
		if (JudgeBlack(img(CurrentLine, PreEdge))){	//黑色执行左搜索
			for (int i = PreEdge; i>Point[0]; i--){
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i - 1))){
						DesPoint = i;
						break;
					}
			}
		}
		if (0 == DesPoint) *SearchFlag = 0;
		else{
			*SearchFlag = 1;
			RightEdgeGo(CurrentLine) = DesPoint;
		}
	}
	else if (Center == LocFlag){
		PreEdge = CenterGuideGo(CurrentLine - 1);
		Point[0] = PreEdge - NearByDis;
		Point[1] = PreEdge + NearByDis;
		if (Point[0] < HStart) Point[0] = HStart;
		if (Point[1]  > HEnd) Point[1] = HEnd;

		if (CurrentLine < 6 || CurrentLine > 45) return OK;

		if (JudgeWhite(img(CurrentLine, PreEdge))){
			for (int i = PreEdge; i > Point[0]; i--){	//修改左边界
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i - 1))){
						LeftEdgeGo(CurrentLine) = i + 2; //i - 1;
						break;
					}
			}
			for (int i = PreEdge; i < Point[1]; i++){   //修改右边界
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i + 1))){
						RightEdgeGo(CurrentLine) = i - 2;//i + 1;
						break;
					}
			}
		}
		if (JudgeBlack(img(CurrentLine, PreEdge))){
			for (int i = PreEdge; i < Point[1]; i++){   //修改左边界
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i + 1))){
						LeftEdgeGo(CurrentLine) = i + 3;//i;
						break;
					}
			}
			for (int i = PreEdge; i > Point[0]; i--){	//修改右边界
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i - 1))){
						RightEdgeGo(CurrentLine) = i - 3;//i;
						break;
					}
			}
		}
		return ERROR;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief 修补边界
* @return 0---在临边未找到 1---在临边已经找到
**/
Status SupplyLineDotLoc(int CurrentLine, int HStart, int HEnd, Site LocSite, int* OverFlag){
	int isSearch = 0;
	int isJumpFlag = 0;
	if (Left == LocSite){
		SupplyOneHorrizon(CurrentLine, HStart, HEnd, Left, &isSearch);
		/*找到点跟新最小二乘并进行拐点检测*/
		if (1 == isSearch){
			isJumpFlag = isJumpDot(CurrentLine, Left);
			if (1 == isJumpFlag){	//检测到拐点不跟新最小二乘，补线
				if (CurrentLine <= 8){
					LinearLine(&LeftTLSM, START_ROW, 5, Left);
					ReviseSetDot(CurrentLine, 1, HStart, HEnd, Left);
				}
				else if (CurrentLine <= 35){
					if ((CurrentLine - 8) >= _BeginLine.LeftValidLine){
						LinearLine(&LeftTLSM, CurrentLine - 8, 5, Left);
					}
					else{
						LinearLine(&LeftTLSM, CurrentLine - 7, 5, Left);
					}
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Left);
				}
				else{
					LinearLine(&LeftTLSM, CurrentLine - 8, 5, Left);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Left);
				}
			}
			/*十字找不到点的特殊情况,区别于弯道拐点*/
			else if (2 == isJumpFlag && CurrentLine >= 6){
				int TopLine = 0;
				SupplyOneHorrizon(CurrentLine + 1, HStart, HEnd, Left, &TopLine);
				if (0 == TopLine){
					if (CurrentLine >= 6 && CurrentLine <= 10)
						LinearLine(&LeftTLSM, START_ROW, 5, Left);
					else
						LinearLine(&LeftTLSM, CurrentLine - 6, 5, Left);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Left);
				}
			}
			else{
				if (CurrentLine < 40)
					LinearLine(&LeftTLSM, CurrentLine - 5, 5, Left);
				else if (CurrentLine < 45)
					LinearLine(&LeftTLSM, CurrentLine - 3, 3, Left);
				else
					LinearLine(&LeftTLSM, CurrentLine - 2, 2, Left);
			}
			return 1;
		}
		else{
			if (0 != LeftTLSM.k){
				if ((LeftTLSM.k-0.45f) >= 0)
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart + 2, HEnd - 2, Left);
				else
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart, HEnd, Left);
			}
			else{
				LeftEdgeGo(CurrentLine) = LeftEdgeGo(CurrentLine - 1);
				*OverFlag = 1;
			}
			return 0;
		}
	}
	else if (Right == LocSite){
		SupplyOneHorrizon(CurrentLine, HStart, HEnd, Right, &isSearch);
		/*找到点跟新最小二乘并进行拐点检测和回拐判别*/
		if (1 == isSearch){
			isJumpFlag = isJumpDot(CurrentLine, Right);
			if (1 == isJumpFlag){	//检测到拐点不跟新最小二乘，补线
				if (CurrentLine <= 8){
					LinearLine(&RightTLSM, START_ROW, 5, Right);
					ReviseSetDot(CurrentLine, 1, HStart, HEnd, Right);
				}
				else if (CurrentLine <= 35){
					if ((CurrentLine - 8) >= _BeginLine.RightValidLine){
						LinearLine(&RightTLSM, CurrentLine - 8, 5, Right);
					}
					else{
						LinearLine(&RightTLSM, CurrentLine - 7, 5, Right);
					}
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Right);
				}
				else{
					LinearLine(&RightTLSM, CurrentLine - 8, 5, Right);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Right);
				}
			}
			/*十字找不到点的特殊情况*/
			if (2 == isJumpFlag && CurrentLine >= 6){
				int TopLine = 0;
				SupplyOneHorrizon(CurrentLine + 1, HStart, HEnd, Right, &TopLine);
				if (0 == TopLine){
					if (CurrentLine >= 6 && CurrentLine <= 10)
						LinearLine(&RightTLSM, START_ROW, 5, Right);
					else
						LinearLine(&RightTLSM, CurrentLine - 6, 5, Right);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Right);
				}
			}
			else{
				if (CurrentLine < 40)
					LinearLine(&RightTLSM, CurrentLine - 5, 5, Right);
				else if (CurrentLine < 45)
					LinearLine(&RightTLSM, CurrentLine - 3, 3, Right);
				else
					LinearLine(&RightTLSM, CurrentLine - 2, 2, Right);
			}
			return 1;
		}
		else{	
			if (0 != RightTLSM.k){
				if ((RightTLSM.k-0.45f) >= 0)
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart + 2, HEnd - 2, Right);
				else
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart, HEnd, Right);
			}
			else{
				RightEdgeGo(CurrentLine) = RightEdgeGo(CurrentLine - 1);
				*OverFlag = 1;
			}
			return 0;
		}
	}
	else;
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief:判断拐点多级判断
* @param[in]:当前行---CurrentLine
* @param[in]:搜索位置LocSite
* @return:是否是回拐点 0---不是 1----是
**/
Status isJumpDot(int CurrentLine, Site LocSite){
	int Diff1 = 0, Diff2 = 0, Diff3 = 0, Diff4 = 0;
	int DiffTop = 0, DiffBottom = 0;
	int Pro = 0;
	if (Left == LocSite){
		Diff1 = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 1);
		Diff2 = LeftEdgeGo(CurrentLine - 1) - LeftEdgeGo(CurrentLine - 2);
		Diff3 = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 3);
		Diff4 = LeftEdgeGo(CurrentLine - 3) - LeftEdgeGo(CurrentLine - 4);
		DiffTop = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine);
		DiffBottom = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 4);
	}
	else if (Right == LocSite){
		Diff1 = RightEdgeGo(CurrentLine - 1) - RightEdgeGo(CurrentLine);
		Diff2 = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 1);
		Diff3 = RightEdgeGo(CurrentLine - 3) - RightEdgeGo(CurrentLine - 2);
		Diff4 = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 3);
		DiffTop = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2);
		DiffBottom = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 2);
	}
	else;

	//特殊拐点处理
	if (Diff1 == 1 && Diff2 == -1 && Diff3 == 0 && Diff4 == 0) return 1;
	//小弯远处不处理
	if (CurrentLine > 45){
		if (Diff1 <= -2 && Diff2 == 0 && Diff3 == 1 && Diff4 == 0)
			return 0;
		if (Diff1 <= -3 && Diff2 <= -1 && Diff3 == 0 && Diff4 == 0)
			return 0;
	}
	

	if (CurrentLine < 40 && CurrentLine > 20){
		if (Diff1 <= 0 && Diff2 <= 0 && Diff3 >= 0){
			if (DiffTop >= 3 && DiffBottom >= 0)
				return 1;
		}

		if (Diff1 <= 0 && Diff2 >= 0 && Diff3 >= 0){
			if (DiffTop >= 5 && DiffBottom >= 0)
				return 1;
		}
	}

	if (Diff1 <= 0 && Diff2 <= 0	//可能有拐点存在
		&& Diff3 >= 0 && Diff4 >= 0){
		if (0 == DiffBottom) Pro = DiffTop;
		else Pro = DiffTop / DiffBottom;
		if ((DiffTop >= 1 && DiffBottom >= 1 && CurrentLine < 10)
			|| (DiffTop >= 2 && DiffBottom >= 1 && CurrentLine < 40 && CurrentLine >= 10)
			|| (DiffTop >= 3 && DiffBottom >= 2 && CurrentLine >= 40)
			|| (Pro >= 3 && CurrentLine <= 35)
			|| (Pro >= 5 && CurrentLine > 35)){
			return 1;
		}

		/*特殊情况十字和拐弯，分辨在于十字上一行找不到而拐弯找得到*/
		if (CurrentLine < 40){
			if (
				(DiffTop >= 1 && DiffBottom >= 1)
				|| (DiffTop >= 1 && Diff2 == 0 && Diff3 == 0 && Diff4 == 0)
				|| (DiffBottom >= 1 && Diff1 == 0 && Diff2 == 0 && Diff3 == 0)
				|| (Diff1 <= -2 && Diff2 == 1 && Diff3 == 0 && Diff4 == 1)
				){
				return 2;
			}
		}
	}
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief:补中心线
* @param[in]:搜索动态起始行自定义相关---_BeginLine
* @param[in]:终止结束行---VEnd
* @param[out]:搜索终止有效行
* @return:状态
**/
Status SupplyMidLine(int CurrentLine, int Diff,
	int LeftEdgeFlag, int RightEdgeFlag,
	int HStart, int HEnd){
	if (1 == LeftEdgeFlag && 1 == RightEdgeFlag){
		CenterGuideGo(CurrentLine) = \
			myround((float)(LeftEdgeGo(CurrentLine) + RightEdgeGo(CurrentLine)) / 2.0f);
	}
	else if (1 == LeftEdgeFlag && 0 == RightEdgeFlag){
		LoseEdgeDeal(CurrentLine, Diff, HStart, HEnd, Left);
	}
	else if (0 == LeftEdgeFlag && 1 == RightEdgeFlag){
		LoseEdgeDeal(CurrentLine, Diff, HStart, HEnd, Right);
	}
	else{
		CenterGuideGo(CurrentLine) = CenterGuideGo(CurrentLine - 1);
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:丢边界线处理
* @param[in]:当前行---CurrentLine
* @param[in]:起始搜索/终止搜索（HStart/HEnd)
* @param[in]:搜索位置 Left---根据左边线补中线 Right---根据右边线补线
* @return:异常情况下的行号---CurrentLine
**/
Status LoseEdgeDeal(int CurrentLine, int Diff, int HStart, int HEnd, Site LocFlag){
	int MidTemp = 0;
	if (Left == LocFlag){
		MidTemp = LeftEdgeGo(CurrentLine) + LoseEdgeWidth[CurrentLine] + Diff;
		if (MidTemp > HStart && MidTemp < HEnd){
			CenterGuideGo(CurrentLine) = MidTemp;
		}
		else{
			if (MidTemp <= HStart) CenterGuideGo(CurrentLine) = HStart;
			if (MidTemp >= HEnd)	CenterGuideGo(CurrentLine) = HEnd;
		}
	}
	else if (Right == LocFlag){
		MidTemp = RightEdgeGo(CurrentLine) - LoseEdgeWidth[CurrentLine] - Diff;
		if (MidTemp > HStart && MidTemp < HEnd){
			CenterGuideGo(CurrentLine) = MidTemp;
		}
		else{
			if (MidTemp <= HStart) CenterGuideGo(CurrentLine) = HStart;
			if (MidTemp >= HEnd)	CenterGuideGo(CurrentLine) = HEnd;
		}
	}
	else{
		return ERROR;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief 起始行十字畸变矫正
**/
Status DetectStartErr(int CurrentLine, BeginLine *_BeginLine, Site LocFlag){
	/*起始行畸变矫正*/
	int Diff[4] = { 0 };
	int DiffTop = 0, DiffBottom = 0;
	if (Left == LocFlag){
		Diff[0] = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 1);
		Diff[1] = LeftEdgeGo(CurrentLine - 1) - LeftEdgeGo(CurrentLine - 2);
		Diff[2] = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 3);
		Diff[3] = LeftEdgeGo(CurrentLine - 3) - LeftEdgeGo(CurrentLine - 4);
		DiffTop = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine);
		DiffBottom = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 4);
		//错误起始
		if (Diff[0] <= -2 && Diff[1] == 1 && Diff[2] == 0 && abs(Diff[3]) == 1) return 1;
		if (LeftEdgeGo(CurrentLine) == START_COL ||
			LeftEdgeGo(CurrentLine - 1) == START_COL ||
			LeftEdgeGo(CurrentLine - 2) == START_COL ||
			LeftEdgeGo(CurrentLine - 3) == START_COL ||
			LeftEdgeGo(CurrentLine - 4) == START_COL) return 2;
		if (Diff[0] == 0 && Diff[1] == 1 && Diff[2] == 1 && Diff[3] == 0) return 2;
		if (abs(Diff[0]) >= 1 && Diff[1] == 0 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && abs(Diff[1]) >= 1 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && abs(Diff[2]) >= 1 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) >= 1) return 2;
		//找不到边界
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) == 0) return 3;
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] >= 0 && Diff[3] >= 0){
			if (DiffTop >= 2 && DiffBottom >= 1) return 1;
			if (DiffTop <= 1 && DiffBottom == 1) return 2;
			if (DiffTop >= 3 && 0 == DiffBottom) return 2;
		}
		//畸变严重
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] <= 0 && Diff[3] <= 0){
			if ((LeftEdgeGo(CurrentLine - 4) - LeftEdgeGo(CurrentLine)) >= 4)
				return 4;
		}
	}

	if (Right == LocFlag){
		Diff[0] = RightEdgeGo(CurrentLine - 1) - RightEdgeGo(CurrentLine);
		Diff[1] = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 1);
		Diff[2] = RightEdgeGo(CurrentLine - 3) - RightEdgeGo(CurrentLine - 2);
		Diff[3] = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 3);
		DiffTop = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2);
		DiffBottom = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 2);
		if (Diff[0] <= -2 && Diff[1] == 1 && Diff[2] == 0 && abs(Diff[3]) == 1) return 1;
		if (RightEdgeGo(CurrentLine) == END_COL ||
			RightEdgeGo(CurrentLine - 1) == END_COL ||
			RightEdgeGo(CurrentLine - 2) == END_COL ||
			RightEdgeGo(CurrentLine - 3) == END_COL ||
			RightEdgeGo(CurrentLine - 4) == END_COL) return 2;
		if (Diff[0] == 0 && Diff[1] == 1 && Diff[2] == 1 && Diff[3] == 0) return 2;
		if (abs(Diff[0]) >= 1 && Diff[1] == 0 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && abs(Diff[1]) >= 1 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && abs(Diff[2]) >= 1 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) >= 1) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) == 0) return 3;
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] >= 0 && Diff[3] >= 0){
			if (DiffTop >= 2 && DiffBottom >= 1) return 1;
			if (DiffTop <= 1 && DiffBottom == 1) return 2;
			if (DiffTop >= 3 && 0 == DiffBottom) return 2;
		}
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] <= 0 && Diff[3] <= 0){
			if ((RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 4)) >= 4)
				return 4;
		}
	}
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief 修正起始行检测不正
**/
Status ReviseStartErr(int CurrentLine, Site LocSite, BeginLine *_BeginLine){
	float k = 0.0f, b = 0.0f;
	int Temp = 0;
	int Point[4] = { 0 };
	if (Left == LocSite){
		Point[0] = CurrentLine - 4;
		Point[1] = LeftEdgeGo(Point[0]);
		Point[2] = CurrentLine - 2;
		Point[3] = LeftEdgeGo(Point[2]);
		CalculateOneLine(Point, &k, &b);
		for (int i = (CurrentLine - 2); i <= CurrentLine; i++){
			Temp = ForecastTHSM(i, k, b);
			if (Temp > START_COL && Temp < END_COL){
				LeftEdgeGo(i) = Temp;
			}
			else{
				LeftEdgeGo(i) = LeftEdgeGo(i - 1);
			}
		}
		for (int i = CurrentLine; i >= (CurrentLine - 4); i--){
			Temp = LeftEdgeGo(i) + LoseEdgeWidth[i];
			if (Temp > START_COL && Temp < END_COL){
				CenterGuideGo(i) = Temp;
			}
			else{
				CenterGuideGo(i) = CenterGuideGo(i - 1);
			}
		}
		LeftTLSM.k = k;
		LeftTLSM.b = b;
		_BeginLine->LeftValidLine = CurrentLine - 4;
	}
	if (Right == LocSite){
		Point[0] = CurrentLine - 4;
		Point[1] = RightEdgeGo(Point[0]);
		Point[2] = CurrentLine - 2;
		Point[3] = RightEdgeGo(Point[2]);
		CalculateOneLine(Point, &k, &b);
		for (int i = (CurrentLine - 2); i <= CurrentLine; i++){
			Temp = ForecastTHSM(i, k, b);
			if (Temp > START_COL && Temp < END_COL){
				RightEdgeGo(i) = Temp;
			}
			else{
				RightEdgeGo(i) = RightEdgeGo(i - 1);
			}
		}
		for (int i = CurrentLine; i >= (CurrentLine - 4); i--){
			Temp = RightEdgeGo(i) - LoseEdgeWidth[i];
			if (Temp > START_COL && Temp < END_COL){
				CenterGuideGo(i) = Temp;
			}
			else{
				CenterGuideGo(i) = CenterGuideGo(i - 1);
			}
		}
		RightTLSM.k = k;
		RightTLSM.b = b;
		_BeginLine->RightValidLine = CurrentLine - 4;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif 最远前瞻2.5米判断
**/
Status SearchEndLine(BeginLine* _BeginLine){
	_BeginLine->ValidEndLine = END_ROW - 1;
	for (int i = 1; i <= (CAMERA_H - 6); i++){
		if (NoneLose == _BeginLine->LoseLineFlag){
			if (abs(RightEdgeGo(i) - LeftEdgeGo(i)) <= 10){
				_BeginLine->ValidEndLine = i;
				break;
			}
			if (CenterGuideGo(i) <= LEFT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
			if (CenterGuideGo(i) >= RIGHT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
		}
		if (LeftLose == _BeginLine->LoseLineFlag){
			if (CenterGuideGo(i) <= LEFT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
		}
		if (RightLose == _BeginLine->LoseLineFlag){
			if (CenterGuideGo(i) >= RIGHT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
		}
		if (i > 45 && abs(CenterGuideGo(i) - CenterGuideGo(i - 1)) > 13){
			_BeginLine->ValidEndLine = i - 1;
			break;
		}
	}

	if (1 == ObIsObStacle){
		_BeginLine->ValidEndLine = ObValidLine;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif 修补起始行
**/
Status ReviseStartValidLine(int HStart, int HEnd, BeginLine* _BeginLine){
	int ValidStartLine = 0;
	if (NoneLose == _BeginLine->LoseLineFlag){
		if (_BeginLine->LeftValidLine == _BeginLine->RightValidLine){
			ValidStartLine = _BeginLine->LeftValidLine;
			if (START_ROW != ValidStartLine){
				LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
				ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
			}
		}
		else if (_BeginLine->LeftValidLine > _BeginLine->RightValidLine){
			ValidStartLine = _BeginLine->RightValidLine;
			int Diff = CenterGuideGo(_BeginLine->LeftValidLine)
				- LeftEdgeGo(_BeginLine->LeftValidLine)
				- LoseEdgeWidth[_BeginLine->LeftValidLine];
			for (int i = _BeginLine->RightValidLine; i < _BeginLine->LeftValidLine; i++){
				LoseEdgeDeal(i, Diff, START_COL, END_COL, Right);
			}
			if (START_ROW != ValidStartLine){
				LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
				ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
			}
		}
		else{
			ValidStartLine = _BeginLine->LeftValidLine;
			int Diff = RightEdgeGo(_BeginLine->RightValidLine) \
				- CenterGuideGo(_BeginLine->RightValidLine)
				- LoseEdgeWidth[_BeginLine->RightValidLine];
			for (int i = _BeginLine->LeftValidLine; i < _BeginLine->RightValidLine; i++){
				LoseEdgeDeal(i, Diff, START_COL, END_COL, Left);
			}
			if (START_ROW != ValidStartLine){
				LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
				ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
			}
		}
	}
	else if (LeftLose == _BeginLine->LoseLineFlag){
		ValidStartLine = _BeginLine->RightValidLine;
		if (START_ROW != ValidStartLine){
			LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
			ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
		}
	}
	else if (RightLose == _BeginLine->LoseLineFlag){
		ValidStartLine = _BeginLine->LeftValidLine;
		if (START_ROW != ValidStartLine){
			LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
			ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
		}
	}
	else;


	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif 识别坡道
**/
Status DetectRampOK(BeginLine* _BeginLine){
	int Width[5] = { 0 }, WidthChange[4] = { 0 };
	int Slope[2] = { 0 };
	int WidthChangeSum = 0, SlopeSum = 0;
	if (NoneLose == _BeginLine->LoseLineFlag && _BeginLine->ValidEndLine >= 50){
		Width[0] = RightEdgeGo(10) - LeftEdgeGo(10);
		Width[1] = RightEdgeGo(20) - LeftEdgeGo(20);
		Width[2] = RightEdgeGo(30) - LeftEdgeGo(30);
		Width[3] = RightEdgeGo(40) - LeftEdgeGo(40);
		Width[4] = RightEdgeGo(50) - LeftEdgeGo(50);
		WidthChange[0] = Width[0] - Width[1];
		WidthChange[1] = Width[1] - Width[2];
		WidthChange[2] = Width[2] - Width[3];
		WidthChange[3] = Width[3] - Width[4];
		Slope[0] = LeftEdgeGo(50) - LeftEdgeGo(10);
		Slope[1] = RightEdgeGo(10) - RightEdgeGo(50);
		WidthChangeSum = WidthChange[0] + WidthChange[1] + WidthChange[2] + WidthChange[3];
		SlopeSum = Slope[0] + Slope[1];
		if ((WidthChange[0] <= 12 && WidthChange[1] <= 10 && WidthChange[2] <= 10 && WidthChange[3] <= 10)
			&& (WidthChange[0] >= 3 && WidthChange[1] >= 3 && WidthChange[2] >= 3 && WidthChange[3] >= 3)
			&& (WidthChangeSum < 40 && WidthChangeSum >= 20)
			&& (SlopeSum < 40 && SlopeSum >= 20)
			&& (Width[0] >= 65 && Width[4] >= 30))
			return 1;
	}
	return 0;
}
////////////////////////////////////////////
//UnLock
/**
* @breif 腐蚀滤波
**/
Status OneBlackDotFilter(void){
	for (int i = 0; i < CAMERA_H; i++){
		for (int j = 1; j < (CAMERA_W - 1); j++){
			if (JudgeWhite(img(i, j - 1)))
				if (JudgeBlack(img(i, j)))
					if (JudgeWhite(img(i, j + 1))){
						img(i,j) = 0xFF;
					}
		}
	}
	return OK;
}
////////////////////////////////////////////
