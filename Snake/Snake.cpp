#include<Windows.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include"resource.h"
#define BMPS 10
#define N_MARTIRX 16//N*M螺旋矩阵(贪吃蛇的活动区域)
#define GAMETIMER 1
#define MAX_LENGTH  600

/*********食物结构体**********
* 用来标识当前食物的状态以及位置
* foodLocation：食物的位置
* state:食物的状态，是否被吃掉
****************************/
struct FOOD{
	POINT foodLocation;
	bool state ;//是否被吃掉
	FOOD(){
		state = false;
		foodLocation.x = 32 + 96;
		foodLocation.y = 32 + 96;
	}
};
//全局变量
int GameState = 0;//游戏状态0:游戏为开始，1：游戏进行中，2：游戏结束
int SnakeLength = 1;//蛇长
int GameSpeed = 200;
int GameLevel = 1;//游戏等级,等级1（蛇长>=20） 游戏速度为 200，等级2（蛇长>=40）游戏速度为100 ，等级3 (蛇长>= 80)游戏速度为 50 ，等级4（蛇长>=200）游戏速度为25
BOOL Flag = TRUE;//只有在第一次游戏的时候有开机动画的显示，ture表示是刚启动应用程序
BOOL Pause = FALSE;//游戏暂停的选项
FOOD food;
int MoveDirection = 1;//当前的蛇的移动方向，1表示向左，2表示向右，3表示向上，4表示向下
POINT Snake[MAX_LENGTH];//存储当前的贪吃蛇身体的位置信息,Snake[0]始终存储蛇头的坐标信息
RECT rectSnake;//游戏区
RECT rectClient;
//函数声明区

LRESULT CALLBACK  windProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK AboutProc(HWND,UINT,WPARAM,LPARAM);
void SetClientRect(HWND	hwnd,int width,int height);//改变客户区的大小，用于设置想要的窗口的大小
void Draw(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectClient);//绘制游戏初始界面
void DrawAnimate(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectSnake);//绘制开机动画
void DrawButton(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectStartButton,int index);//绘制开始游戏按钮
void Food();//随机产生一个食物
bool SnakeEatFood(int x,int y);//判断贪吃蛇是否吃到了食物
bool GameOver(int x,int y);//判断游戏是否结束
void SnakeMove(int x,int y);//贪吃蛇的移动
void DrawGameOver(HWND hwnd,HDC hdc,HBITMAP hBitMap[],int index);//绘制gameover图片

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd )
{
	static TCHAR szAppName[] = TEXT("贪吃蛇");
	HWND	hwnd;
	MSG		msg;
	WNDCLASS		wndClass;
	HICON hIcon;

	wndClass.style					= CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra			= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.hCursor				= LoadCursor(NULL,IDC_ARROW);
	wndClass.hIcon					= LoadIcon(NULL,MAKEINTRESOURCE(IDI_APP));//修改应用程序图标时使用
	wndClass.hInstance			= hInstance;
	wndClass.lpfnWndProc	=	windProc;
	wndClass.lpszClassName	=  szAppName;
	wndClass.lpszMenuName	=	MAKEINTRESOURCE(IDR_SNAKE);

	//注册窗口类
	if(!RegisterClass(&wndClass))
	{
		MessageBox(NULL,TEXT("This program requires Windows NT!"),szAppName,MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,szAppName,
											WS_OVERLAPPEDWINDOW,/*& ~WS_MAXIMIZEBOX &~WS_MINIMIZEBOX&~WS_THICKFRAME,//去除最大化最小化和缩放功能*/
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											NULL,
											LoadMenu(hInstance,MAKEINTRESOURCE(IDR_SNAKE)),
											hInstance,
											NULL);

	//修改窗口左上角的图标
	hIcon = LoadIcon((HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE) ,MAKEINTRESOURCE(IDI_APP));
	SendMessage(hwnd, WM_SETICON, TRUE,  (LPARAM)hIcon);

	ShowWindow(hwnd,nShowCmd);
	UpdateWindow(hwnd);
	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK  windProc(HWND	hwnd,UINT		Message,WPARAM		wParam,LPARAM		lParam)
{
	int i;
	static HDC	hdc;
	static HDC hdcDes;
	static HBITMAP hBitMap[BMPS];
	static BITMAP bitMap;
	static HINSTANCE hInstance;
	static HBRUSH hBrush;
	static RECT rectStartButton;//开始游戏按钮区
	static PAINTSTRUCT ps;
	static POINT mouseLocation;
	static int x,y;
	
	hBrush = CreateSolidBrush(RGB(255,255,255));
	switch (Message)
	{
	case WM_CREATE:		
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		hBitMap[0] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BRICK));//墙
		hBitMap[1] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_NUMERS));//分数
		hBitMap[2] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SCORE));//得分字样
		hBitMap[3] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMELEVEL));//游戏等级字样
		hBitMap[4] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_ANIMATE));//开机动画的贴图
		hBitMap[5] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMESTART));//游戏开始按钮
		hBitMap[6] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_WORDSNAKE));//snake四个英文字母
		hBitMap[7] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SNAKE));//蛇的身体以及食物
		hBitMap[8] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_STARTCHANGE));//鼠标放在开始按钮上时改变颜色
		hBitMap[9] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMEOVER));//GAMEOVER四个字母
		srand((unsigned)time(NULL));
		SetClientRect(hwnd,704,576);
		
		return 0;

	case WM_SIZE:
		GetClientRect(hwnd,&rectClient);
		rectSnake.left = rectClient.left+32;
		rectSnake.top = rectClient.top+32;
		rectSnake.bottom = rectClient.bottom-32;
		rectSnake.right = rectClient.left+17*32;

		rectStartButton.left = rectSnake.left+150;
		rectStartButton.right = rectStartButton.left+236;
		rectStartButton.top = rectSnake.bottom-100;
		rectStartButton.bottom = rectStartButton.top+79;
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			Pause = !Pause;
			if(Pause)
			{
				if(GameState != 1)
				{
					GameLevel = 1;
					GameState = 1;
					SnakeLength = 1;
					GameSpeed = 200;
					Snake[0].x = rectSnake.left+160;//初始化蛇头坐标信息
					Snake[0].y = rectSnake.left+160;
					x = Snake[0].x;
					y = Snake[0].y;
				}
				SetTimer(hwnd,GAMETIMER,GameSpeed,NULL);//设置时间计时器
			}
			else KillTimer(hwnd,GAMETIMER);
			break;
		case VK_ESCAPE:
			int result;
			result = MessageBox(NULL,TEXT("你确定要退出游戏吗?"),TEXT("Warning!"),MB_YESNO|MB_ICONWARNING);
			switch (result)
			{
			case IDYES:
				DestroyWindow(hwnd);
				break;
			case IDNO:
				break;
			default: break;
			}
			break;
		case VK_LEFT:
		    if(MoveDirection != 2)
			MoveDirection = 1;
			return 0;
		case VK_RIGHT:
		    if(MoveDirection != 1)
			MoveDirection = 2;
			return 0;
		case VK_UP:
		    if(MoveDirection != 4)
			MoveDirection = 3;
			return 0;
		case VK_DOWN:
		    if(MoveDirection != 3)
			MoveDirection = 4;
			return 0;
		}
		return 0;
	
	
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_ABOUT:
			DialogBox(hInstance,MAKEINTRESOURCE(IDD_DIABOUT),NULL,AboutProc);
			break;
		case ID_GAMESTART://点击菜单中的开始游戏按钮开始游戏
			GameLevel = 1;
			GameState = 1;
			SnakeLength = 1;
			GameSpeed = 200;
			SetTimer(hwnd,GAMETIMER,GameSpeed,NULL);//设置时间计时器
			Snake[0].x = rectSnake.left+160;//初始化蛇头坐标信息
			Snake[0].y = rectSnake.left+160;
			x = Snake[0].x;
			y = Snake[0].y;
			Pause = TRUE;
			break;
		case ID_GAMEQUIT:
			KillTimer(hwnd,GAMETIMER);
			ReleaseDC(hwnd,hdc);
			DestroyWindow(hwnd);
			exit(0);
			break ;
		}	
		return 0;
	
	case	 WM_PAINT:
		
		hdc = GetDC(hwnd);
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd,&rectClient);
		hdcDes = CreateCompatibleDC(hdc);
		Draw(hwnd,hdc,hBitMap,rectClient);	
		if(GameState == 0)
		{
			DrawAnimate(hwnd,hdc,hBitMap,rectSnake);//开机动画
		}
		else if(GameState == 1)
		{	
			FillRect(hdc,&rectSnake,hBrush);
			SelectObject(hdcDes,hBitMap[7]);
			GetObject(hBitMap[7],sizeof(BITMAP),&bitMap);
			if(!food.state)
				BitBlt(hdc, food.foodLocation.x, food.foodLocation.y, bitMap.bmWidth, bitMap.bmHeight/2,hdcDes,0,16,SRCCOPY);
			for(i =0;i<SnakeLength;i++)
			{
				BitBlt(hdc, Snake[i].x, Snake[i].y, bitMap.bmWidth, bitMap.bmHeight/2,hdcDes,0,0,SRCCOPY);
			}
		}
		else if(GameState == 2)
		{
			DrawGameOver(hwnd,hdc,hBitMap,9);
			KillTimer(hwnd,GAMETIMER);
		}
		DeleteDC(hdcDes);
		EndPaint(hwnd,&ps);
		Flag = FALSE;
		return 0;
	case WM_TIMER:
		if(GameState == 1)
		{
			if(MoveDirection == 4)
			{
				x= x;
				y = y +16;
			}
			else if(MoveDirection == 3)
			{
				x = x;
				y = y - 16;
			}
			else if(MoveDirection == 2)
			{
				x = x+16;
				y = y;
			}
			else if(MoveDirection == 1)
			{
				x = x -16;
				y = y;
			}
			SnakeMove(x,y);
			InvalidateRect(hwnd,&rectSnake,FALSE);
		}
		
		return 0;
	case WM_LBUTTONDOWN:
		mouseLocation.x = LOWORD(lParam);
		mouseLocation.y = HIWORD(lParam);
		if(GameState == 0)
		{
			if(PtInRect(&rectStartButton,mouseLocation))//点击开始游戏按钮开始游戏
			{
				GameLevel = 1;
				GameState = 1;
				SnakeLength = 1;
				GameSpeed = 200;
				SetTimer(hwnd,GAMETIMER,GameSpeed,NULL);//设置时间计时器
				Snake[0].x = rectSnake.left+160;//初始化蛇头坐标信息
				Snake[0].y = rectSnake.left+160;
				x = Snake[0].x;
				y = Snake[0].y;
				Pause = TRUE;
			}
		}
		return 0;

		//按钮效果实现
	case WM_MOUSEMOVE:
		TRACKMOUSEEVENT TrackME;
		mouseLocation.x = LOWORD(lParam);
		mouseLocation.y = HIWORD(lParam);
		TrackME.cbSize = sizeof(TrackME);  
		TrackME.dwFlags = TME_LEAVE | TME_HOVER;  
		TrackME.hwndTrack = hwnd;  
		TrackME.dwHoverTime = 1;//持续多长算HOVER  
		TrackMouseEvent(&TrackME);  
		if(GameState == 0)
		{
			if(PtInRect(&rectStartButton,mouseLocation))
			{
				FillRect(hdc,&rectStartButton,hBrush);
				SetCursor(LoadCursor(NULL,IDC_HAND));
				DrawButton(hwnd,hdc,hBitMap,rectStartButton,8);
			}
			else 
			{
				FillRect(hdc,&rectStartButton,hBrush);
				DrawButton(hwnd,hdc,hBitMap,rectStartButton,5);
			}
		}		
		return 0;
	
	case WM_DESTROY:	
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,Message,wParam,lParam);
}

/*************About对话框消息处理函数******
*
*
*****************************************/
BOOL CALLBACK AboutProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd,0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


/***********设置客户区的大小*************
*参数: width:客户区的宽度
*			 height:客户区的高度
*
****************************************/
void SetClientRect(HWND	hwnd,int width,int height)
{
	RECT rectWindow;
	RECT rectClient;
	int windowSizeX,windowSizeY;

	GetWindowRect(hwnd,&rectWindow);
	GetClientRect(hwnd,&rectClient);

	windowSizeX = rectWindow.right-rectWindow.left-(rectClient.right-rectClient.left)+width;
	windowSizeY = rectWindow.bottom-rectWindow.top-(rectClient.bottom-rectClient.top)+height;

	MoveWindow(hwnd,
		GetSystemMetrics(SM_CXSCREEN)/2-windowSizeX/2,
		GetSystemMetrics(SM_CYSCREEN)/2-windowSizeY/2-100,
		windowSizeX,
		windowSizeY,
		FALSE);

}

/*************绘制游戏初始界面***********
*
*
*
*
****************************************/
void Draw(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectClient)
{
	int i,j,temp;
	HDC hdcDes;
	int cx,cy;
	BITMAP bitMap;
	j =100;
	temp = SnakeLength-1;

	hdc = GetDC(hwnd);
	hdcDes = CreateCompatibleDC(hdc);
	SelectObject(hdcDes,hBitMap[0]);
	GetObject(hBitMap[0],sizeof(BITMAP),&bitMap);
	cx = bitMap.bmWidth;
	cy = bitMap.bmHeight;
	
	//绘制刚墙
	for(i =0;i*cx<=rectClient.right;i++ )
	{
		BitBlt(hdc,i*cx,0,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,i*cx,rectClient.bottom-cy,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,rectClient.bottom-cy*2,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,cy,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,2*cy,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,cy*7,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,cy*8,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,cy*9,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,cy*10,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,576+i*cx,cy*15,cx,cy,hdcDes,0,0,SRCCOPY);
	}
	for(i = 0;i*cx<=rectClient.bottom-cy;i++)
	{
		BitBlt(hdc,0,i*cx+cx,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.right-cx,i*cy+cy,cx,cy,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,544,i*cy+cy,cx,cy,hdcDes,0,0,SRCCOPY);
	}

	//绘制得分两个字
	SelectObject(hdcDes,hBitMap[2]);
	GetObject(hBitMap[2],sizeof(BITMAP),&bitMap);
	BitBlt(hdc,576,3*cy,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);

	//绘制分数
	SelectObject(hdcDes,hBitMap[1]);
	GetObject(hBitMap[1],sizeof(BITMAP),&bitMap);
	for(i =0 ;i<3;i++)
	{
		BitBlt(hdc,576+i*31,cy*5,31,32,hdcDes,0,(temp/j)*32,SRCCOPY);
		temp = temp%j;
		j = j/10;
	}

	//绘制游戏等级
	BitBlt(hdc,576+31,13*cy,31,32,hdcDes,0,(GameLevel)*32,SRCCOPY);
	//绘制游戏等级四个字
	SelectObject(hdcDes,hBitMap[3]);
	GetObject(hBitMap[3],sizeof(BITMAP),&bitMap);
	BitBlt(hdc,576,11*cy,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
	
	
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}

/*****************************************************
* 游戏开始显示开始动画,绘制显示螺旋矩阵
* 
* 
* 
******************************************************/
void DrawAnimate(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectSnake)
{
		HDC hdcDes;
		int i,k;
		HBRUSH hBrush;
		BITMAP bitMap;
		hdc = GetDC(hwnd);
		hdcDes = CreateCompatibleDC(hdc);
		hBrush = CreateSolidBrush(RGB(255,255,255));

		if(Flag)//绘制螺旋矩阵
		{
			SelectObject(hdcDes,hBitMap[4]);
			GetObject(hBitMap[4],sizeof(BITMAP),&bitMap);
			for(i = 0;i<N_MARTIRX/2;i++)
			{
				for(k = i;k<=N_MARTIRX-i-1;k++)
				{
					BitBlt(hdc,rectSnake.left+k*32,rectSnake.top+i*32,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
					Sleep(10);
				}
				for(k = i+1;k<N_MARTIRX-i-1;k++)
				{
					BitBlt(hdc,rectSnake.right-i*32-32,rectSnake.top+k*32,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
					Sleep(10);
				}
				for(k = N_MARTIRX -i-1;k>i;k--)
				{
					BitBlt(hdc,rectSnake.left+k*32,rectSnake.bottom-(i+1)*32,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
					Sleep(10);
				}
				for(k = N_MARTIRX -i-1;k>i;k--)
				{
					BitBlt(hdc,rectSnake.left+i*32,rectSnake.top+k*32,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
					Sleep(10);
				}
			}

			FillRect(hdc,&rectSnake,hBrush);
			SelectObject(hdcDes,hBitMap[5]);
			GetObject(hBitMap[5],sizeof(BITMAP),&bitMap);

			//绘制开始游戏按钮
			BitBlt(hdc,rectSnake.left+150,rectSnake.bottom-100,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
			SelectObject(hdcDes,hBitMap[6]);
			GetObject(hBitMap[6],sizeof(BITMAP),&bitMap);
			for(i = 5;i>0;i--)
			{
				BitBlt(hdc,rectSnake.left+56+(i-1)*80,rectSnake.top+100,bitMap.bmWidth,bitMap.bmHeight,hdcDes,(i-1)*80,0,SRCCOPY);
				Sleep(200);
			}
		}
		else 
		{
			SelectObject(hdcDes,hBitMap[6]);
			GetObject(hBitMap[6],sizeof(BITMAP),&bitMap);
			BitBlt(hdc,rectSnake.left+56,rectSnake.top+100,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
			SelectObject(hdcDes,hBitMap[5]);
			GetObject(hBitMap[5],sizeof(BITMAP),&bitMap);
			BitBlt(hdc,rectSnake.left+150,rectSnake.bottom-100,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
		}
		
		
		ReleaseDC(hwnd,hdc);
		DeleteDC(hdcDes);
}

/**************通过随机产生食物***************
*
*
*********************************************/

void Food()
{
	int i;
	BOOL foodee = FALSE;//用来标识产生的食物是否在蛇的身体上
	while(TRUE)
	{
		food.foodLocation.x = rand()%32*16+32;
		food.foodLocation.y = rand()%32*16+32;
		for(i = 0;i<SnakeLength;i++)
		{
			if(Snake[i].x == food.foodLocation.x&&Snake[i].y == food.foodLocation.y)foodee =TRUE;
		}
		if(foodee)continue;
		else break;
	}
	food.state = false;
}

/*************判断贪吃蛇是否吃到了食物*********
* x,y表示蛇头移动后的坐标
*
*********************************************/
bool SnakeEatFood(int x,int y)
{
	if(x == food.foodLocation.x && y == food.foodLocation.y)
	{
		food.state = true;
		return true;
	}
	return false;
}

/*************判断游戏是否结束****************
* x，y表示当前蛇头进行一次移动后的坐标
* 游戏结束返回true，否则返回false
********************************************/
bool GameOver(int x,int y)
{
	int i;
	if(x < rectSnake.left || x >= rectSnake.right || y <rectSnake.top  || y >= rectSnake.bottom )//判断是否碰到了边界
	return true;
	if(SnakeLength>4)//当蛇长大于4的时候判断是否吃到了自身
	{
		for(i = 1;i<SnakeLength;i++)
			if(x == Snake[i].x&&y == Snake[i].y) return true;
	}
	return false;
}

/****************蛇的移动********************
*
*算法需要改进
********************************************/
void SnakeMove(int x,int y)
{
	int i;
	//根据当前移动方向确定下一步的坐标
	
	if(GameOver(x,y))
	{
		GameState = 2;
		Pause = FALSE;
		return;
	}
	//判断是否吃到了食物
	if(SnakeEatFood(x,y))
	{
		for(i =SnakeLength;i>0;i--)
			Snake[i]= Snake[i-1];
		Snake[0].x = x;
		Snake[0].y = y;
		SnakeLength++;
		Food();
	}
	else 
	{
		for(i =SnakeLength-1;i>0;i--)
			Snake[i]= Snake[i-1];
		Snake[0].x = x;
		Snake[0].y = y;
	}
	if(SnakeLength>=20&&SnakeLength<40)
	{
		GameLevel = 2;
		GameSpeed = 100 ;
	}
	else if(SnakeLength>=40&&SnakeLength<80)
	{
		GameLevel = 3;
		GameSpeed = 80;
	}
	else if(SnakeLength>=80)
	{
		GameLevel = 4;
		GameSpeed = 50;
	}
}

void DrawButton(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectStartButton,int index)
{
	HDC hdcDes;
	BITMAP bitMap;
	hdc = GetDC(hwnd);
	hdcDes = CreateCompatibleDC(hdc);
	SelectObject(hdcDes,hBitMap[index]);
	GetObject(hBitMap[index],sizeof(BITMAP),&bitMap);
	BitBlt(hdc,rectStartButton.left,rectStartButton.top,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}
void DrawGameOver(HWND hwnd,HDC hdc,HBITMAP hBitMap[],int index)
{
	int i;
	HDC hdcDes;
	BITMAP bitMap;
	hdc = GetDC(hwnd);
	hdcDes = CreateCompatibleDC(hdc);
	SelectObject(hdcDes,hBitMap[index]);
	GetObject(hBitMap[index],sizeof(BITMAP),&bitMap);
	for(i =0;i<8;i++)
	{
		BitBlt(hdc,rectSnake.left+22+i*58,rectSnake.top+200,bitMap.bmWidth/8,bitMap.bmHeight,hdcDes,i*58,0,SRCCOPY);
		Sleep(300);
	}
	DrawText(hdc,TEXT("你已经被完爆，还敢继续吗?(按ESC退出游戏，按SPACE重新开始游戏)"),-1,&rectSnake,DT_SINGLELINE | DT_TOP);
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}
