#include<Windows.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include"resource.h"
#define BMPS 10
#define N_MARTIRX 16//N*M��������(̰���ߵĻ����)
#define GAMETIMER 1
#define MAX_LENGTH  600

/*********ʳ��ṹ��**********
* ������ʶ��ǰʳ���״̬�Լ�λ��
* foodLocation��ʳ���λ��
* state:ʳ���״̬���Ƿ񱻳Ե�
****************************/
struct FOOD{
	POINT foodLocation;
	bool state ;//�Ƿ񱻳Ե�
	FOOD(){
		state = false;
		foodLocation.x = 32 + 96;
		foodLocation.y = 32 + 96;
	}
};
//ȫ�ֱ���
int GameState = 0;//��Ϸ״̬0:��ϷΪ��ʼ��1����Ϸ�����У�2����Ϸ����
int SnakeLength = 1;//�߳�
int GameSpeed = 200;
int GameLevel = 1;//��Ϸ�ȼ�,�ȼ�1���߳�>=20�� ��Ϸ�ٶ�Ϊ 200���ȼ�2���߳�>=40����Ϸ�ٶ�Ϊ100 ���ȼ�3 (�߳�>= 80)��Ϸ�ٶ�Ϊ 50 ���ȼ�4���߳�>=200����Ϸ�ٶ�Ϊ25
BOOL Flag = TRUE;//ֻ���ڵ�һ����Ϸ��ʱ���п�����������ʾ��ture��ʾ�Ǹ�����Ӧ�ó���
BOOL Pause = FALSE;//��Ϸ��ͣ��ѡ��
FOOD food;
int MoveDirection = 1;//��ǰ���ߵ��ƶ�����1��ʾ����2��ʾ���ң�3��ʾ���ϣ�4��ʾ����
POINT Snake[MAX_LENGTH];//�洢��ǰ��̰���������λ����Ϣ,Snake[0]ʼ�մ洢��ͷ��������Ϣ
RECT rectSnake;//��Ϸ��
RECT rectClient;
//����������

LRESULT CALLBACK  windProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK AboutProc(HWND,UINT,WPARAM,LPARAM);
void SetClientRect(HWND	hwnd,int width,int height);//�ı�ͻ����Ĵ�С������������Ҫ�Ĵ��ڵĴ�С
void Draw(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectClient);//������Ϸ��ʼ����
void DrawAnimate(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectSnake);//���ƿ�������
void DrawButton(HWND hwnd,HDC hdc,HBITMAP hBitMap[],RECT rectStartButton,int index);//���ƿ�ʼ��Ϸ��ť
void Food();//�������һ��ʳ��
bool SnakeEatFood(int x,int y);//�ж�̰�����Ƿ�Ե���ʳ��
bool GameOver(int x,int y);//�ж���Ϸ�Ƿ����
void SnakeMove(int x,int y);//̰���ߵ��ƶ�
void DrawGameOver(HWND hwnd,HDC hdc,HBITMAP hBitMap[],int index);//����gameoverͼƬ

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd )
{
	static TCHAR szAppName[] = TEXT("̰����");
	HWND	hwnd;
	MSG		msg;
	WNDCLASS		wndClass;
	HICON hIcon;

	wndClass.style					= CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra			= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.hCursor				= LoadCursor(NULL,IDC_ARROW);
	wndClass.hIcon					= LoadIcon(NULL,MAKEINTRESOURCE(IDI_APP));//�޸�Ӧ�ó���ͼ��ʱʹ��
	wndClass.hInstance			= hInstance;
	wndClass.lpfnWndProc	=	windProc;
	wndClass.lpszClassName	=  szAppName;
	wndClass.lpszMenuName	=	MAKEINTRESOURCE(IDR_SNAKE);

	//ע�ᴰ����
	if(!RegisterClass(&wndClass))
	{
		MessageBox(NULL,TEXT("This program requires Windows NT!"),szAppName,MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,szAppName,
											WS_OVERLAPPEDWINDOW,/*& ~WS_MAXIMIZEBOX &~WS_MINIMIZEBOX&~WS_THICKFRAME,//ȥ�������С�������Ź���*/
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											NULL,
											LoadMenu(hInstance,MAKEINTRESOURCE(IDR_SNAKE)),
											hInstance,
											NULL);

	//�޸Ĵ������Ͻǵ�ͼ��
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
	static RECT rectStartButton;//��ʼ��Ϸ��ť��
	static PAINTSTRUCT ps;
	static POINT mouseLocation;
	static int x,y;
	
	hBrush = CreateSolidBrush(RGB(255,255,255));
	switch (Message)
	{
	case WM_CREATE:		
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		hBitMap[0] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BRICK));//ǽ
		hBitMap[1] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_NUMERS));//����
		hBitMap[2] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SCORE));//�÷�����
		hBitMap[3] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMELEVEL));//��Ϸ�ȼ�����
		hBitMap[4] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_ANIMATE));//������������ͼ
		hBitMap[5] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMESTART));//��Ϸ��ʼ��ť
		hBitMap[6] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_WORDSNAKE));//snake�ĸ�Ӣ����ĸ
		hBitMap[7] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SNAKE));//�ߵ������Լ�ʳ��
		hBitMap[8] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_STARTCHANGE));//�����ڿ�ʼ��ť��ʱ�ı���ɫ
		hBitMap[9] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMEOVER));//GAMEOVER�ĸ���ĸ
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
					Snake[0].x = rectSnake.left+160;//��ʼ����ͷ������Ϣ
					Snake[0].y = rectSnake.left+160;
					x = Snake[0].x;
					y = Snake[0].y;
				}
				SetTimer(hwnd,GAMETIMER,GameSpeed,NULL);//����ʱ���ʱ��
			}
			else KillTimer(hwnd,GAMETIMER);
			break;
		case VK_ESCAPE:
			int result;
			result = MessageBox(NULL,TEXT("��ȷ��Ҫ�˳���Ϸ��?"),TEXT("Warning!"),MB_YESNO|MB_ICONWARNING);
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
		case ID_GAMESTART://����˵��еĿ�ʼ��Ϸ��ť��ʼ��Ϸ
			GameLevel = 1;
			GameState = 1;
			SnakeLength = 1;
			GameSpeed = 200;
			SetTimer(hwnd,GAMETIMER,GameSpeed,NULL);//����ʱ���ʱ��
			Snake[0].x = rectSnake.left+160;//��ʼ����ͷ������Ϣ
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
			DrawAnimate(hwnd,hdc,hBitMap,rectSnake);//��������
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
			if(PtInRect(&rectStartButton,mouseLocation))//�����ʼ��Ϸ��ť��ʼ��Ϸ
			{
				GameLevel = 1;
				GameState = 1;
				SnakeLength = 1;
				GameSpeed = 200;
				SetTimer(hwnd,GAMETIMER,GameSpeed,NULL);//����ʱ���ʱ��
				Snake[0].x = rectSnake.left+160;//��ʼ����ͷ������Ϣ
				Snake[0].y = rectSnake.left+160;
				x = Snake[0].x;
				y = Snake[0].y;
				Pause = TRUE;
			}
		}
		return 0;

		//��ťЧ��ʵ��
	case WM_MOUSEMOVE:
		TRACKMOUSEEVENT TrackME;
		mouseLocation.x = LOWORD(lParam);
		mouseLocation.y = HIWORD(lParam);
		TrackME.cbSize = sizeof(TrackME);  
		TrackME.dwFlags = TME_LEAVE | TME_HOVER;  
		TrackME.hwndTrack = hwnd;  
		TrackME.dwHoverTime = 1;//�����೤��HOVER  
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

/*************About�Ի�����Ϣ������******
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


/***********���ÿͻ����Ĵ�С*************
*����: width:�ͻ����Ŀ��
*			 height:�ͻ����ĸ߶�
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

/*************������Ϸ��ʼ����***********
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
	
	//���Ƹ�ǽ
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

	//���Ƶ÷�������
	SelectObject(hdcDes,hBitMap[2]);
	GetObject(hBitMap[2],sizeof(BITMAP),&bitMap);
	BitBlt(hdc,576,3*cy,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);

	//���Ʒ���
	SelectObject(hdcDes,hBitMap[1]);
	GetObject(hBitMap[1],sizeof(BITMAP),&bitMap);
	for(i =0 ;i<3;i++)
	{
		BitBlt(hdc,576+i*31,cy*5,31,32,hdcDes,0,(temp/j)*32,SRCCOPY);
		temp = temp%j;
		j = j/10;
	}

	//������Ϸ�ȼ�
	BitBlt(hdc,576+31,13*cy,31,32,hdcDes,0,(GameLevel)*32,SRCCOPY);
	//������Ϸ�ȼ��ĸ���
	SelectObject(hdcDes,hBitMap[3]);
	GetObject(hBitMap[3],sizeof(BITMAP),&bitMap);
	BitBlt(hdc,576,11*cy,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
	
	
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}

/*****************************************************
* ��Ϸ��ʼ��ʾ��ʼ����,������ʾ��������
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

		if(Flag)//������������
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

			//���ƿ�ʼ��Ϸ��ť
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

/**************ͨ���������ʳ��***************
*
*
*********************************************/

void Food()
{
	int i;
	BOOL foodee = FALSE;//������ʶ������ʳ���Ƿ����ߵ�������
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

/*************�ж�̰�����Ƿ�Ե���ʳ��*********
* x,y��ʾ��ͷ�ƶ��������
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

/*************�ж���Ϸ�Ƿ����****************
* x��y��ʾ��ǰ��ͷ����һ���ƶ��������
* ��Ϸ��������true�����򷵻�false
********************************************/
bool GameOver(int x,int y)
{
	int i;
	if(x < rectSnake.left || x >= rectSnake.right || y <rectSnake.top  || y >= rectSnake.bottom )//�ж��Ƿ������˱߽�
	return true;
	if(SnakeLength>4)//���߳�����4��ʱ���ж��Ƿ�Ե�������
	{
		for(i = 1;i<SnakeLength;i++)
			if(x == Snake[i].x&&y == Snake[i].y) return true;
	}
	return false;
}

/****************�ߵ��ƶ�********************
*
*�㷨��Ҫ�Ľ�
********************************************/
void SnakeMove(int x,int y)
{
	int i;
	//���ݵ�ǰ�ƶ�����ȷ����һ��������
	
	if(GameOver(x,y))
	{
		GameState = 2;
		Pause = FALSE;
		return;
	}
	//�ж��Ƿ�Ե���ʳ��
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
	DrawText(hdc,TEXT("���Ѿ����걬�����Ҽ�����?(��ESC�˳���Ϸ����SPACE���¿�ʼ��Ϸ)"),-1,&rectSnake,DT_SINGLELINE | DT_TOP);
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}
