#include<windows.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include<mmsystem.h>
#pragma comment (lib,"winmm.lib")
#pragma comment(lib, "Msimg32.lib" ) //TransparentBlt用
#define Pi 3.1415926
//全局变量声明
HWND	hWnd;
HDC		hdc, mdc, bufdc;
DWORD	tPre, tNow;
HFONT hf;
SIZE size;
TEXTMETRIC tm;
RECT rect;

static int xm, ym;//鼠标坐标
POINT mouse[7];//鼠标拖尾点数组
int num;//鼠标拖尾点数组下标
int lflag;//控制鼠标拖尾

int flagstart = 0;//待机等待游戏开始
int A_weight[6] = { 66,126,64,68,98,66 };
int	A_hight[6] = { 66,50,59,72,85,68 };
double ntime = 0;//水果运动计时
double x, y;//水果坐标
double g = 0.058;//重力加速度
int gl;//判断一次弹射几个水果
HBITMAP apple;//苹果 1
HBITMAP apple1, apple2;
HBITMAP banana;//香蕉 2
HBITMAP banana1, banana2;
HBITMAP peach;//桃子 3
HBITMAP peach1, peach2;
HBITMAP basaha;//草莓 4
HBITMAP basaha1, basaha2;
HBITMAP sandia;//西瓜 5 
HBITMAP sandia1, sandia2;
HBITMAP boom;//炸弹 6
HBITMAP flash;//刀光
HBITMAP gameover;//游戏失败

HBITMAP bgstart;//起始背景
int flagpeach = 0;//启动界面桃子
double xp1, xp2, yp, pt;//开始界面桃子坐标
int flagsandia = 0;//启动界面西瓜
double xs1, xs2, ys, st;//开始界面西瓜坐标
int flagboom = 0;//启动界面炸弹

HBITMAP gamebg;//普通模式背景
HBITMAP x_, xf_;
HBITMAP xx_, xxf_;
HBITMAP xxx_, xxxf_;
HBITMAP lose;//水果掉落未切到
int fnormal = 0;//普通模式界面
int losenormal;///普通模式失败
int score;//得分
char score_s[5];//得分字符输出数组
int flag_x;//生命值
int fexplain_nor;//普通模式游戏说明
char nor[3][70];
const char nor_1[3][70] = { "玩家每切一个水果加一分",
						"切到炸弹或漏切三次水果则游戏失败",
						"点击屏幕任意处开始游戏" };

int losedojo;//道场模式失败
int dojogame;//道场模式
int fruitnum = 30;//待切水果个数
int flag_y;//落下水果
int dojotime;//计时
int puttime;//用时
char b[20];
const char* a = "挑战成功";
int fexplain_jodo;//道场模式游戏说明
char jodo[5][80];
const char jodo_1[5][80] = { "玩家每切一个水果则水果总数减一",
							"玩家每漏一个水果则水果总数加三",
							"切到炸弹或水果总数不低于40则游戏失败",
							"水果总数(30个)减为0则挑战成功",
							"点击屏幕任意处开始游戏", };
//声音
int fvoice_menu;//主菜单声音
int fvoice_start;//开始声音
typedef struct
{
	int no;//编号
	int weight;//宽
	int hight;//高
	double x;//x坐标
	double y;//y坐标
	double vx;//x方向速度
	double vy;//y方向速度
	double ntime;//计时变量
	HBITMAP p;//图片
	int flagcut;//判断是否被切
	double x1, x2;//切开后两片的横坐标
}fruit;
fruit f[2];//水果

//全局函数声明
void srand_fruit();//随机生成水果
void fruitdown(int n, int i);//水果被切下落函数
HBITMAP put_fruit(int n);//返回位图类型名
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void				MyPaint(HDC hdc);
//第三方函数（掩图函数）
void MyTransparentBlt(HDC hdcDest,      // 目标DC
	int nXOriginDest,   // 目标X偏移
	int nYOriginDest,   // 目标Y偏移
	int nWidthDest,     // 目标宽度
	int nHeightDest,    // 目标高度
	HDC hdcSrc,         // 源DC
	int nXOriginSrc,    // 源X起点
	int nYOriginSrc,    // 源Y起点
	int nWidthSrc,      // 源宽度
	int nHeightSrc,     // 源高度
	UINT crTransparent  // 透明色,COLORREF类型
)
{
	HBITMAP hOldImageBMP, hImageBMP = CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);	// 创建兼容位图
	HBITMAP hOldMaskBMP, hMaskBMP = CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);			// 创建单色掩码位图
	HDC		hImageDC = CreateCompatibleDC(hdcDest);
	HDC		hMaskDC = CreateCompatibleDC(hdcDest);
	hOldImageBMP = (HBITMAP)SelectObject(hImageDC, hImageBMP);
	hOldMaskBMP = (HBITMAP)SelectObject(hMaskDC, hMaskBMP);

	// 将源DC中的位图拷贝到临时DC中
	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
		BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
	else
		StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
			hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);

	// 设置透明色
	SetBkColor(hImageDC, crTransparent);

	// 生成透明区域为白色，其它区域为黑色的掩码位图
	BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);

	// 生成透明区域为黑色，其它区域保持不变的位图
	SetBkColor(hImageDC, RGB(0, 0, 0));
	SetTextColor(hImageDC, RGB(255, 255, 255));
	BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	// 透明部分保持屏幕不变，其它部分变成黑色
	SetBkColor(hdcDest, RGB(0xff, 0xff, 0xff));
	SetTextColor(hdcDest, RGB(0, 0, 0));
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	// "或"运算,生成最终效果
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCPAINT);

	SelectObject(hImageDC, hOldImageBMP);
	DeleteDC(hImageDC);
	SelectObject(hMaskDC, hOldMaskBMP);
	DeleteDC(hMaskDC);
	DeleteObject(hImageBMP);
	DeleteObject(hMaskBMP);
}
//****WinMain函数，程序入口点函数***********************
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	MSG msg;
	MyRegisterClass(hInstance);
	//初始化
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	//初始化msg 
	GetMessage(&msg, NULL, NULL, NULL);
	//游戏循环
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tNow = GetTickCount64();
			if (tNow - tPre >= 50)
				MyPaint(hdc);
		}
	}
	return msg.wParam;
}
//****设计一个窗口类****
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "canvas";
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}
//****初始化函数****
// 加载位图并设定各种初始值
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hWnd = CreateWindow("canvas", "水果忍者", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}
	MoveWindow(hWnd, 100, 50, 640, 480, true);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	////////////////////////////////////////
	HBITMAP fullmap;
	hdc = GetDC(hWnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);
	SetBkColor(mdc, RGB(255, 255, 255));
	//建立空的位图（虚拟位图）并置入mdc中
	fullmap = CreateCompatibleBitmap(hdc, 640, 480);
	SelectObject(mdc, fullmap);

	bgstart = (HBITMAP)LoadImage(NULL, "picture/bgstart.bmp", IMAGE_BITMAP, 640, 480, LR_LOADFROMFILE);//起始背景
	gamebg = (HBITMAP)LoadImage(NULL, "picture/gamebg.bmp", IMAGE_BITMAP, 640, 480, LR_LOADFROMFILE);//普通模式背景
	apple = (HBITMAP)LoadImage(NULL, "picture/apple.bmp", IMAGE_BITMAP, 66, 66, LR_LOADFROMFILE);//苹果
	apple1 = (HBITMAP)LoadImage(NULL, "picture/apple-1.bmp", IMAGE_BITMAP, 66, 66, LR_LOADFROMFILE);//苹果
	apple2 = (HBITMAP)LoadImage(NULL, "picture/apple-2.bmp", IMAGE_BITMAP, 66, 66, LR_LOADFROMFILE);//苹果
	banana = (HBITMAP)LoadImage(NULL, "picture/banana.bmp", IMAGE_BITMAP, 126, 50, LR_LOADFROMFILE);//香蕉
	banana1 = (HBITMAP)LoadImage(NULL, "picture/banana-1.bmp", IMAGE_BITMAP, 126, 50, LR_LOADFROMFILE);//香蕉
	banana2 = (HBITMAP)LoadImage(NULL, "picture/banana-2.bmp", IMAGE_BITMAP, 126, 50, LR_LOADFROMFILE);//香蕉
	peach = (HBITMAP)LoadImage(NULL, "picture/peach.bmp", IMAGE_BITMAP, 64, 59, LR_LOADFROMFILE);//桃子
	peach1 = (HBITMAP)LoadImage(NULL, "picture/peach-1.bmp", IMAGE_BITMAP, 64, 59, LR_LOADFROMFILE);//桃子
	peach2 = (HBITMAP)LoadImage(NULL, "picture/peach-2.bmp", IMAGE_BITMAP, 64, 59, LR_LOADFROMFILE);//桃子
	basaha = (HBITMAP)LoadImage(NULL, "picture/basaha.bmp", IMAGE_BITMAP, 68, 72, LR_LOADFROMFILE);//草莓
	basaha1 = (HBITMAP)LoadImage(NULL, "picture/basaha-1.bmp", IMAGE_BITMAP, 68, 72, LR_LOADFROMFILE);//草莓
	basaha2 = (HBITMAP)LoadImage(NULL, "picture/basaha-2.bmp", IMAGE_BITMAP, 68, 72, LR_LOADFROMFILE);//草莓
	sandia = (HBITMAP)LoadImage(NULL, "picture/sandia.bmp", IMAGE_BITMAP, 98, 85, LR_LOADFROMFILE);//西瓜
	sandia1 = (HBITMAP)LoadImage(NULL, "picture/sandia-1.bmp", IMAGE_BITMAP, 98, 85, LR_LOADFROMFILE);//西瓜
	sandia2 = (HBITMAP)LoadImage(NULL, "picture/sandia-2.bmp", IMAGE_BITMAP, 98, 85, LR_LOADFROMFILE);//西瓜
	boom = (HBITMAP)LoadImage(NULL, "picture/boom.bmp", IMAGE_BITMAP, 66, 68, LR_LOADFROMFILE);//炸弹
	flash = (HBITMAP)LoadImage(NULL, "picture/flash.bmp", IMAGE_BITMAP, 358, 20, LR_LOADFROMFILE);//刀光
	x_ = (HBITMAP)LoadImage(NULL, "picture/x.bmp", IMAGE_BITMAP, 22, 19, LR_LOADFROMFILE);
	xf_ = (HBITMAP)LoadImage(NULL, "picture/xf.bmp", IMAGE_BITMAP, 22, 19, LR_LOADFROMFILE);
	xx_ = (HBITMAP)LoadImage(NULL, "picture/xx.bmp", IMAGE_BITMAP, 27, 26, LR_LOADFROMFILE);
	xxf_ = (HBITMAP)LoadImage(NULL, "picture/xxf.bmp", IMAGE_BITMAP, 27, 26, LR_LOADFROMFILE);
	xxx_ = (HBITMAP)LoadImage(NULL, "picture/xxx.bmp", IMAGE_BITMAP, 32, 32, LR_LOADFROMFILE);
	xxxf_ = (HBITMAP)LoadImage(NULL, "picture/xxxf.bmp", IMAGE_BITMAP, 32, 32, LR_LOADFROMFILE);
	lose = (HBITMAP)LoadImage(NULL, "picture/lose.bmp", IMAGE_BITMAP, 54, 50, LR_LOADFROMFILE);//水果掉落的'X'
	gameover = (HBITMAP)LoadImage(NULL, "picture/gameover.bmp", IMAGE_BITMAP, 490,85, LR_LOADFROMFILE);//游戏失败

	GetClientRect(hWnd, &rect);
	//初始化开始界面水果
	xp1 = 82;
	xp2 = 82;
	yp = 305;
	xs1 = 270;
	xs2 = 270;
	ys = 294;
	MyPaint(hdc);
	return TRUE;
}
//****返回水果位图名函数****
HBITMAP put_fruit(int n)
{
	HBITMAP x=NULL;
	switch (n)
	{
	case 0:x = apple; break;
	case 1:x = banana; break;
	case 2:x = peach; break;
	case 3:x = basaha; break;
	case 4:x = sandia; break;
	case 5:x = boom; break;
	}
	return x;
}
//****水果被切下落函数****
void fruitdown(int n, int i)
{
	switch (n)
	{
	case 0:
	{
		SelectObject(bufdc, apple1);
		MyTransparentBlt(mdc, f[i].x1, f[i].y, 66, 66, bufdc, 0, 0, 66, 66, RGB(255, 255, 255));
		SelectObject(bufdc, apple2);
		MyTransparentBlt(mdc, f[i].x2, f[i].y, 66, 66, bufdc, 0, 0, 66, 66, RGB(255, 255, 255));
		break;
	}
	case 1:
	{
		SelectObject(bufdc, banana1);
		MyTransparentBlt(mdc, f[i].x1, f[i].y, 126, 50, bufdc, 0, 0, 126, 50, RGB(255, 255, 255));
		SelectObject(bufdc, banana2);
		MyTransparentBlt(mdc, f[i].x2, f[i].y, 126, 50, bufdc, 0, 0, 126, 50, RGB(255, 255, 255));
		break;
	}
	case 2:
	{
		SelectObject(bufdc, peach1);
		MyTransparentBlt(mdc, f[i].x1, f[i].y, 64, 59, bufdc, 0, 0, 64, 59, RGB(255, 255, 255));
		SelectObject(bufdc, peach2);
		MyTransparentBlt(mdc, f[i].x2, f[i].y, 64, 59, bufdc, 0, 0, 64, 59, RGB(255, 255, 255));
		break;
	}
	case 3:
	{
		SelectObject(bufdc, basaha1);
		MyTransparentBlt(mdc, f[i].x1, f[i].y, 68, 72, bufdc, 0, 0, 68, 72, RGB(255, 255, 255));
		SelectObject(bufdc, basaha2);
		MyTransparentBlt(mdc, f[i].x2, f[i].y, 68, 72, bufdc, 0, 0, 68, 72, RGB(255, 255, 255));
		break;
	}
	case 4:
	{
		SelectObject(bufdc, sandia1);
		MyTransparentBlt(mdc, f[i].x1, f[i].y, 98, 85, bufdc, 0, 0, 98, 85, RGB(255, 255, 255));
		SelectObject(bufdc, sandia2);
		MyTransparentBlt(mdc, f[i].x2, f[i].y, 98, 85, bufdc, 0, 0, 98, 85, RGB(255, 255, 255));
		break;
	}
	}
}
//****随机生成水果函数****
void srand_fruit()
{
	srand((int)time(0));
	gl = 0;
	gl = rand() % (10);
	if (gl < 5)gl = 1;//gl小于7一个水果，否者两个水果
	else gl = 0;
	int n;//水果编号
	n = rand() % (6);
	for (int i = 0; i < 2; i++)
	{
		f[i].no = n;
		f[i].hight = A_hight[n];
		f[i].weight = A_weight[n];
		f[i].p = put_fruit(n);
		if (gl) continue;
		n = rand() % (6);
	}
	if (gl)
	{
		f[0].x = f[1].x = rand() % (500) + 50.0;
		f[0].y = f[1].y = 480.0;
		if (f[0].x > 340)
			f[0].vx = f[1].vx = -1.0 * (rand() % (3) + 8.0);
		else
			f[0].vx = f[1].vx = rand() % (3) + 8.0;
		f[0].vy = f[1].vy = -1.0 * (rand() % (3) + 30.0);
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			f[i].x = rand() % (500) + 50.0;
			f[i].y = 480.0;
			if (f[i].x > 340)
				f[i].vx = -1.0 * (rand() % (3) + 8.0);
			else
				f[i].vx = rand() % (3) + 8.0;
			f[i].vy = -1.0 * (rand() % (3) + 30.0);
		}
	}
	f[0].ntime = f[1].ntime = 0;
	f[0].flagcut = f[1].flagcut = 1;
}
//****自定义绘图函数****
void MyPaint(HDC hdc)
{
	if (fnormal)	//普通模式
	{
		if (!fexplain_nor)
		{
			if (!fvoice_start)
			{
				mciSendString("play sound/start.mp3", 0, 0, 0);	//游戏开始声音
				fvoice_start = 1;
			}
			SelectObject(bufdc, gamebg);			//简单游戏窗口
			BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);
			for(int i=0;i<3;i++)
				sprintf(nor[i], "%s", nor_1[i]);
			hf = CreateFont(
				25, 0, 0, 0, FW_HEAVY, 0, 0, 0, GB2312_CHARSET, OUT_CHARACTER_PRECIS
				, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				"粗字体"
			);
			SetTextColor(mdc, RGB(30, 125, 220));
			SetBkMode(mdc, TRANSPARENT);
			SelectObject(mdc, hf);
			GetTextMetrics(mdc, &tm);
			for(int i=0;i<3;i++)
				TextOut(mdc, 50, 50+40*i, nor[i], strlen(nor[i]));
			DeleteObject(hf);
		}
		else
		{
			if (!losenormal)
			{
				if (flag_x >= 3)
				{
					losenormal = 1;
					mciSendString("play sound/over.mp3", 0, 0, 0);	//游戏失败
				}
				SelectObject(bufdc, gamebg);			//简单游戏窗口
				BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);
				if (flagstart <= 5)		//贴水果
				{
					if (flagstart == 2) mciSendString("play sound/normal.mp3", 0, 0, 0);	//普通模式背景音乐
					if (!flagstart)srand_fruit();
					flagstart++;
				}
				else
				{
					int flag = 0;
					for (int i = 0; i < 2; i++)
					{
						if (f[i].y >= 480.0 && f[i].vy > 0)
							flag++;
						//水果落下位未被切开
						if (f[i].y > 450 && f[i].flagcut && f[i].vy > 0 && f[i].no != 5 || f[i].x < 0 && f[i].flagcut && f[i].vy > 0 && f[i].no != 5 || f[i].x > 640 && f[i].flagcut && f[i].vy > 0 && f[i].no != 5)
						{
							if (f[i].x > 0 && f[i].x < 640 && f[i].y > 480 && f[i].y < 535 || f[i].x < 0 && f[i].x > -11 || f[i].x > 640 && f[i].x < 651)
							{
								flag_x++;
								if (gl && i == 1) flag_x--;
							}
							SelectObject(bufdc, lose);
							if (f[i].x > 50 && f[i].x < 590)
								MyTransparentBlt(mdc, f[i].x, 400, 40, 40, bufdc, 0, 0, 54, 50, RGB(255, 255, 255));
							else if (f[i].x < 50)
								MyTransparentBlt(mdc, 0, 400, 40, 40, bufdc, 0, 0, 54, 50, RGB(255, 255, 255));
							else if (f[i].x > 590)
								MyTransparentBlt(mdc, 590, 400, 40, 40, bufdc, 0, 0, 54, 50, RGB(255, 255, 255));
						}
					}
					if(flag == 2)srand_fruit();//两个水果都落下重新生成水果
					flag = 0;
				}
				for (int i = 0; i < 2; i++)
				{
					if (f[i].flagcut)			//贴水果
					{
						SelectObject(bufdc, f[i].p);
						MyTransparentBlt(mdc, f[i].x, f[i].y, f[i].weight, f[i].hight, bufdc, 0, 0, f[i].weight, f[i].hight, RGB(255, 255, 255));
					}
					else
					{
						fruitdown(f[i].no, i);
						continue;
					}
					//判断水果是否被切
					if (f[i].flagcut&&lflag && xm > f[i].x + 3 && xm < f[i].x + f[i].weight - 3 && ym > f[i].y + 3 && ym < f[i].y + f[i].hight - 3)
					{
						f[i].flagcut = 0;
						if (f[i].no == 5)
						{
							mciSendString("play sound/boom.mp3", 0, 0, 0);//切炸弹爆炸声
							losenormal = 1;
							break;
						}
						SelectObject(bufdc, flash);
						MyTransparentBlt(mdc, f[i].x - 75 + f[i].weight / 2, f[i].y + f[i].hight / 2, 150, 10, bufdc, 0, 0, 358, 20, RGB(255, 255, 255));
						f[i].x1 = f[i].x2 = f[i].x;
						score++;
						if (gl && i == 1) score--;
					}
				}
			}
			else      //显示游戏失败
			{
				mciSendString("stop sound/normal.mp3", 0, 0, 0);
				SelectObject(bufdc, gamebg);
				BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);
				SelectObject(bufdc, gameover);
				MyTransparentBlt(mdc, 75, 200, 490, 75, bufdc, 0, 0, 490, 75, RGB(255, 255, 255));
				flagsandia = 0;
				st = 0;
				flagstart = 0;
				xs1 = 278;
				xs2 = 278;
				ys = 294;
			}
			if (flag_x > 0)				//贴生命值
				SelectObject(bufdc, xf_);
			else
				SelectObject(bufdc, x_);
			MyTransparentBlt(mdc, rect.right - 100, 10, 22, 19, bufdc, 0, 0, 22, 19, RGB(255, 255, 255));
			if (flag_x > 1)
				SelectObject(bufdc, xxf_);
			else
				SelectObject(bufdc, xx_);
			MyTransparentBlt(mdc, rect.right - 73, 10, 27, 26, bufdc, 0, 0, 27, 26, RGB(255, 255, 255));
			if (flag_x > 2)
				SelectObject(bufdc, xxxf_);
			else
				SelectObject(bufdc, xxx_);
			MyTransparentBlt(mdc, rect.right - 40, 10, 32, 32, bufdc, 0, 0, 32, 32, RGB(255, 255, 255));
			sprintf(score_s, "%d", score);	//得分
			hf = CreateFont(
				50, 0, 0, 0, FW_HEAVY, 0, 0, 0, GB2312_CHARSET, OUT_CHARACTER_PRECIS
				, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				"粗字体"
			);
			SetTextColor(mdc, RGB(220, 0, 0));
			SetBkMode(mdc, TRANSPARENT);
			SelectObject(mdc, hf);
			GetTextMetrics(mdc, &tm);
			TextOut(mdc, 47, 0, score_s, strlen(score_s));
			DeleteObject(hf);
		}
	}
	else if (dojogame)   //道场模式
	{
		if (!fexplain_jodo)
		{
			if (!fvoice_start)
			{
				mciSendString("play sound/start.mp3", 0, 0, 0);	//游戏开始声音
				fvoice_start = 1;
			}
			SelectObject(bufdc, gamebg);			//道场游戏窗口
			BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);
			for(int i=0;i<5;i++)
				sprintf(jodo[i], "%s", jodo_1[i]);
			hf = CreateFont(
				25, 0, 0, 0, FW_HEAVY, 0, 0, 0, GB2312_CHARSET, OUT_CHARACTER_PRECIS
				, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				"粗字体"
			);
			SetTextColor(mdc, RGB(30, 125, 220));
			SetBkMode(mdc, TRANSPARENT);
			SelectObject(mdc, hf);
			GetTextMetrics(mdc, &tm);
			for(int i=0;i<5;i++)
				TextOut(mdc, 50, 50+40*i, jodo[i], strlen(jodo[i]));
			DeleteObject(hf);
		}
		else
		{
			if (!losedojo)
			{
				SelectObject(bufdc, gamebg);			//道场游戏窗口
				BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);
				if (flagstart <= 5)		//贴水果
				{
					if (flagstart == 2) mciSendString("play sound/dojo.mp3", 0, 0, 0);	//dojo模式背景音乐
					if (!flagstart)
						srand_fruit();
					flagstart++;
				}
				else
				{
					int flag = 0;
					for (int i = 0; i < 2; i++)
					{
						if (f[i].y >= 480.0 && f[i].vy > 0)
							flag++;
						//水果落下位未被切开
						if (f[i].y > 450 && f[i].flagcut && f[i].vy > 0 && f[i].no != 5 || f[i].x < 0 && f[i].flagcut && f[i].vy > 0 && f[i].no != 5 || f[i].x > 640 && f[i].flagcut && f[i].vy > 0 && f[i].no != 5)
						{
							if (f[i].x > 0 && f[i].x < 640 && f[i].y > 480 && f[i].y < 535 || f[i].x < 0 && f[i].x > -11 || f[i].x > 640 && f[i].x < 651)
							{
								flag_y++;
								if (gl && i == 1) flag_y--;
							}
							SelectObject(bufdc, lose);
							if (f[i].x > 50 && f[i].x < 590)
								MyTransparentBlt(mdc, f[i].x, 400, 40, 40, bufdc, 0, 0, 54, 50, RGB(255, 255, 255));
							else if (f[i].x < 50)
								MyTransparentBlt(mdc, 0, 400, 40, 40, bufdc, 0, 0, 54, 50, RGB(255, 255, 255));
							else if (f[i].x > 590)
								MyTransparentBlt(mdc, 590, 400, 40, 40, bufdc, 0, 0, 54, 50, RGB(255, 255, 255));
						}
					}
					if (flag == 2) srand_fruit();//两个水果都落下重新生成水果
					flag = 0;
				}
				for (int i = 0; i < 2; i++)
				{
					if (f[i].flagcut)			//贴水果
					{
						SelectObject(bufdc, f[i].p);
						MyTransparentBlt(mdc, f[i].x, f[i].y, f[i].weight, f[i].hight, bufdc, 0, 0, f[i].weight, f[i].hight, RGB(255, 255, 255));
					}
					else
					{
						fruitdown(f[i].no, i);
						continue;
					}
					//判断水果是否被切
					if (f[i].flagcut&&lflag && xm > f[i].x + 3 && xm < f[i].x + f[i].weight - 3 && ym > f[i].y + 3 && ym < f[i].y + f[i].hight - 3)
					{
						f[i].flagcut = 0;
						if (f[i].no == 5)
						{
							mciSendString("play sound/boom.mp3", 0, 0, 0);//切炸弹爆炸声
							losedojo = 1;
							break;
						}
						SelectObject(bufdc, flash);
						MyTransparentBlt(mdc, f[i].x - 75 + f[i].weight / 2, f[i].y + f[i].hight / 2, 150, 10, bufdc, 0, 0, 358, 20, RGB(255, 255, 255));
						f[i].x1 = f[i].x2 = f[i].x;
						fruitnum--;
						if (gl && i == 1) fruitnum++;
						if (fruitnum <= 0)
						{
							mciSendString("play sound/victory.mp3 repeat", 0, 0, 0); //游戏胜利
							losedojo = 1;
							num = 0;
							break;
						}
					}
					if (fruitnum >= 40)
					{
						mciSendString("play sound/over.mp3", 0, 0, 0); //游戏结束
						losedojo = 1;
						break;
					}
				}
			}
			else      //显示挑战成功或游戏失败
			{
				mciSendString("stop sound/dojo.mp3", 0, 0, 0);
				if (fruitnum <= 0)
				{
					hf = CreateFont(
						80, 0, 0, 0, FW_HEAVY, 0, 0, 0, GB2312_CHARSET, OUT_CHARACTER_PRECIS
						, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
						"粗字体"
					);
					SetTextColor(mdc, RGB(100, 220, 50));
					SetBkMode(mdc, TRANSPARENT);
					SelectObject(mdc, hf);
					GetTextMetrics(mdc, &tm);
					GetTextExtentPoint32(mdc, a, strlen(a), &size);
					TextOut(mdc, rect.right / 2 - size.cx / 2, rect.bottom / 3, a, strlen(a));
					DeleteObject(hf);
				}
				else
				{
					SelectObject(bufdc, gamebg);
					BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);
					SelectObject(bufdc, gameover);
					MyTransparentBlt(mdc, 75, 200, 490, 75, bufdc, 0, 0, 490, 75, RGB(255, 255, 255));
				}
				flagpeach = 0;
				pt = 0;
				flagstart = 0;
				xp1 = 82;
				xp2 = 82;
				yp = 305;
			}
			if (flag_y == 1)
			{
				fruitnum += 3;
				flag_y = 0;
			}
			else if (flag_y == 2)
			{
				fruitnum += 3 * 2;
				flag_y = 0;
			}
			flag_y = 0;
			sprintf(score_s, "%d", fruitnum);	//得分
			sprintf(b, "%ds", puttime);	//计时
			hf = CreateFont(
				50, 0, 0, 0, FW_HEAVY, 0, 0, 0, GB2312_CHARSET, OUT_CHARACTER_PRECIS
				, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				"粗字体"
			);
			SetTextColor(mdc, RGB(220, 0, 0));
			SetBkMode(mdc, TRANSPARENT);
			SelectObject(mdc, hf);
			GetTextMetrics(mdc, &tm);
			TextOut(mdc, 47, 0, score_s, strlen(score_s));
			TextOut(mdc, 500, 0, b, strlen(b));
			DeleteObject(hf);
		}
	}
	else       //开始菜单
	{
		if (!fvoice_menu)   //游戏菜单背景音乐
		{
			mciSendString("stop sound/over.mp3", 0, 0, 0);
			mciSendString("stop sound/victory.mp3", 0, 0, 0);
			mciSendString("play sound/menu.mp3 repeat", 0, 0, 0);
			fvoice_menu = 1;
		}
		SelectObject(bufdc, bgstart);
		BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);
		if (flagpeach || lflag && xm > 82 + 3 && xm < 82 + 64 - 3 && ym > 305 + 3 && ym < 305 + 59 - 3)  //jodo模式
		{
			SelectObject(bufdc, peach1);
			MyTransparentBlt(mdc, xp1, yp, 64, 59, bufdc, 0, 0, 64, 59, RGB(255, 255, 255));
			SelectObject(bufdc, peach2);
			MyTransparentBlt(mdc, xp2, yp, 64, 59, bufdc, 0, 0, 64, 59, RGB(255, 255, 255));
			if (!flagpeach)
			{
				SelectObject(bufdc, flash);
				MyTransparentBlt(mdc, 82 - 75 + 32, 305 + 30, 150, 10, bufdc, 0, 0, 358, 20, RGB(255, 255, 255));
				mciSendString("play sound/splatter1.mp3", 0, 0, 0); //水果分裂声音
			}
			flagpeach = 1;
			xp1 -= 5;
			xp2 += 5;
			yp += 7 * g * pt * pt;
			pt++;
			if ((int)yp > rect.bottom)
				dojogame = 1;
		}
		else 
		{
			SelectObject(bufdc, peach);
			MyTransparentBlt(mdc, 82, 305, 64, 59, bufdc, 0, 0, 64, 59, RGB(255, 255, 255));
		}
		if (flagsandia || lflag && xm > 278 + 3 && xm < 278 + 98 - 3 && ym > 294 + 3 && ym < 294 + 85 - 3) //普通模式
		{
			SelectObject(bufdc, sandia1);
			MyTransparentBlt(mdc, xs1, ys, 98, 85, bufdc, 0, 0, 98, 85, RGB(255, 255, 255));
			SelectObject(bufdc, sandia2);
			MyTransparentBlt(mdc, xs2, ys, 98, 85, bufdc, 0, 0, 98, 85, RGB(255, 255, 255));
			if (!flagsandia)
			{
				SelectObject(bufdc, flash);
				MyTransparentBlt(mdc, 270 - 75 + 49, 294 + 42, 150, 10, bufdc, 0, 0, 358, 20, RGB(255, 255, 255));
				mciSendString("play sound/splatter1.mp3", 0, 0, 0); //水果分裂声音
			}
			flagsandia = 1;
			xs1 -= 5;
			xs2 += 5;
			ys += 7 * g * st * st;
			st++;
			if ((int)ys > rect.bottom)
				fnormal = 1;
		}
		else
		{
			SelectObject(bufdc, sandia);
			MyTransparentBlt(mdc, 270, 294, 98, 85, bufdc, 0, 0, 98, 85, RGB(255, 255, 255));
		}
		SelectObject(bufdc, boom);  //退出
		MyTransparentBlt(mdc, 480, 308, 66, 68, bufdc, 0, 0, 66, 68, RGB(255, 255, 255));
		if (lflag && xm > 480 + 3 && xm < 480 + 66 - 3 && ym > 308 + 3 && ym < 308 + 68 - 3)
			PostQuitMessage(0);
		if (fnormal || dojogame)    //关闭背景音乐
			mciSendString("stop sound/menu.mp3", 0, 0, 0);
	}
	HPEN hp;	//绘制鼠标拖尾
	hp = CreatePen(PS_SOLID, 4, RGB(192, 192, 192));
	SelectObject(mdc, hp);
	for (int i = num - 1; i >= 0; i--)
	{
		if (i == num - 1)MoveToEx(mdc, mouse[num - 1].x, mouse[num - 1].y, NULL);
		LineTo(mdc, mouse[i].x, mouse[i].y);
	}
	DeleteObject(hp);
	BitBlt(hdc, 0, 0, 640, 480, mdc, 0, 0, SRCCOPY);
	tPre = GetTickCount64();	//记录此次绘图时间
	if (dojogame&& flagstart > 5)
	{
		if (!losedojo)
		{
			dojotime++;
			puttime = dojotime / 20;
		}
	}
	if (flagstart > 5)	//水果抛物运动
	{
		for (int i = 0; i < 2; i++)
		{
			if (f[i].flagcut)
			{
				if(f[i].y<=480.0)
					f[i].x += f[i].vx;
			}
			else
			{
				f[i].x1 -= 3;
				f[i].x2 += 3;
			}
			f[i].y = 546.0+ f[i].vy * f[i].ntime;
			f[i].vy += g * f[i].ntime;
			f[i].ntime++;
		}
	}
	if (lflag)	//控制鼠标拖尾
	{
		mouse[num] = { xm,ym };
		num++;
		num %= 7;
	}
	
}
//****消息处理函数***********************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:	     //按下键盘消息
		switch (wParam)
		{
		case VK_ESCAPE:           //按下【Esc】键
			PostQuitMessage(0);  //结束程序
			break;
		}
		break;
	case WM_MOUSEMOVE:
		xm = LOWORD(lParam);
		ym = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		if (losenormal|| losedojo)
			lflag = 0;
		else lflag = 1;//鼠标拖尾
		if (!fexplain_nor&&fnormal)
			fexplain_nor = 1;
		if (!fexplain_jodo && dojogame)
			fexplain_jodo = 1;
		if (losenormal)
		{
			fnormal = 0;
			score = 0;
			flag_x = 0;
			losenormal = 0;
			fexplain_nor = 0;
			fvoice_menu = 0;
			fvoice_start = 0;
		}
		if (losedojo)
		{
			dojogame = 0;
			fruitnum = 30;
			losedojo = 0;
			puttime = 0;
			dojotime = 0;
			flag_x = 0;
			fexplain_jodo = 0;
			fvoice_menu = 0;
			fvoice_start = 0;
		}
		break;
	case WM_LBUTTONUP:	//重置鼠标拖尾点数组
		lflag = 0;
		num = 0;
		for (int i = 0; i < 7; i++)
		{
			mouse[i].x = 0;
			mouse[i].y = 0;
		}
		break;
	case WM_DESTROY:	//窗口结束消息
		DeleteDC(mdc);
		DeleteDC(bufdc);
		ReleaseDC(hWnd, hdc);
		PostQuitMessage(0);
		break;
	default:							//其他消息
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}