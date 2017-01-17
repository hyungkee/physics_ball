#define EXT_GLOBAL
#include "Phybal.h"

int gy = 800;

CBall* Balls[1000];
int B_Count;
CBlock* Objects[1000];	// nature가 지각하는 객체들
CBlock* StartPoint;
CBlock* EndPoint;
int O_Count;
int Time = -1;
int FPS = 0;
int FPS_Count = 0;

int iskeydown = 0;
int keydown = 0;   // 0001, 0010, 0100, 1000
int recentkey = 0; // LEFT, RIGHT,  UP, DOWN
int isloading = 1;
int Goal=0;
int forbid = 0;	// -3 : LEFT, -4: RIGHT

int Level = 0;
int LevelLoading = 1;
//////////////////////////////////////////////////////////////////////////////
////////////////////////           BLOCK          ////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//  Block		|	-1 : 시작점		| -2 : 끝점		| 1 ~ : 일반블록		//
//				|	-3 : POWER_LEFT	| -4 : POWER_RIGHT						//
//////////////////////////////////////////////////////////////////////////////

RECT crt;
RECT block_rect;

void LoadLevel(int level){
	int a,b;
	double t;
	char filename[100];

	sprintf(filename,"Map%d.mpm",level);

	FILE *in=fopen(filename,"r");

	if(in == NULL){
		PostQuitMessage(0);
		return;
	}

	block_rect.left = 0;
	block_rect.right = 0;
	block_rect.top = 0;
	block_rect.bottom = 0;

	while(fscanf(in,"%lf %d %d",&t,&a,&b)!=EOF){

		a++;
		b++;

		//화면크기 자동조정을 위한 조사
		if(block_rect.left == 0)	block_rect.left = a*50, block_rect.right = (a+1)*50, block_rect.top = b*50, block_rect.bottom = (b+1)*50;
		if(block_rect.left > a*50)			block_rect.left = a*50;
		if(block_rect.right < (a+1)*50)		block_rect.right = (a+1)*50;
		if(block_rect.top > b*50)			block_rect.top = b*50;
		if(block_rect.bottom < (b+1)*50)	block_rect.bottom = (b+1)*50;
		
		a--;
		b--;


		if(t==-1){//시작점
			StartPoint = new CBlock(t,50*a,50*b);
		}else if(t==-2){//끝점
			EndPoint = new CBlock(t,50*a,50*b);
		}else{//블록
			CBlock *New_Block;
			New_Block = new CBlock(t,50*a,50*b,50,50);
			Objects[O_Count++]=New_Block;
		}
	}

	CBall *ball;
	ball = new CBall(StartPoint->x+25,StartPoint->y+25,0,-100);
	Balls[B_Count++]=ball;


	fclose(in);

	LevelLoading=1;
	Level = level;
	Goal = 0;

	int deltaA,deltaB;
	deltaA = ( (crt.right - crt.left) - (block_rect.right - block_rect.left))/2 - block_rect.left;
	deltaB = ( (crt.bottom - crt.top) - (block_rect.bottom - block_rect.top))/2 - block_rect.top;
	block_rect.left += deltaA;
	block_rect.right += deltaA;
	block_rect.top += deltaB;
	block_rect.bottom += deltaB;
}
void CloseLevel(){
	delete StartPoint;
	delete EndPoint;
	int i;
	for(i=0;i<O_Count;i++){
		delete Objects[i];
		Objects[i]=NULL;
	}
	O_Count=0;
	for(i=0;i<B_Count;i++){
		delete Balls[i];
		Balls[i]=NULL;
	}
	B_Count=0;
}

void Init(){
	Bwidth=50;
	Bheight=50;
	hBlock1 = (HBITMAP)LoadImage(NULL,"Image\\Block1.bmp",NULL,Bwidth,Bheight,LR_LOADFROMFILE);
	hBlock2 = (HBITMAP)LoadImage(NULL,"Image\\Block2.bmp",NULL,Bwidth,Bheight,LR_LOADFROMFILE);
	hBlock3 = (HBITMAP)LoadImage(NULL,"Image\\Block3.bmp",NULL,Bwidth,Bheight,LR_LOADFROMFILE);
	hBlock4 = (HBITMAP)LoadImage(NULL,"Image\\Block4.bmp",NULL,Bwidth,Bheight,LR_LOADFROMFILE);
	hBlock5 = (HBITMAP)LoadImage(NULL,"Image\\Block5.bmp",NULL,Bwidth,Bheight,LR_LOADFROMFILE);
	B_Count=0;
	O_Count=0;

	LoadLevel(1);
}


int Enable2Go(double xp,double yp){
	int i;
	for(i=0;i<O_Count;i++){
		if(Objects[i]->x<xp+10 && xp<Objects[i]->x+Objects[i]->width && Objects[i]->y<yp+10 && yp<Objects[i]->y+Objects[i]->height)
			return 0;
	}
	return 1;
}

double X_Reflect(int k,double xp, double yp){
	int i,j;
	int speed=200;
	double x,y;
	x = Balls[k]->x;
	y = Balls[k]->y;

	for(i=0;i<O_Count;i++){
		if(Objects[i]->x<xp+8 && xp<Objects[i]->x+Objects[i]->width && Objects[i]->y<yp+8 && yp<Objects[i]->y+Objects[i]->height){

			//(xp,yp)가 벽 안에 있음
			if(xp <= Objects[i]->x +Objects[i]->width && Objects[i]->x + Objects[i]->width < x){//왼쪽벽에 충돌
				for(j=0;j<O_Count;j++)//충돌한 벽에 오른쪽이 존재할때. -> 오류
					if(Objects[i]->x+Objects[i]->width == Objects[j]->x && Objects[i]->y == Objects[j]->y)
						break;
				if(j==O_Count){
					forbid = 0;
					xp=2*(Objects[i]->x+Objects[i]->width)-xp;
					if((keydown&2)!=0){//0010 - 오른쪽
						Balls[k]->vy=-300;
						speed=200;
						if(recentkey==1)	recentkey=2;
					}else{
						speed=100;
					}
					return -speed*sign(Balls[k]->vx);
				}
			}
			if(x+8 < Objects[i]->x && Objects[i]->x <= xp+8){//오른쪽벽에 충돌
				for(j=0;j<O_Count;j++)//충돌한 벽에 왼쪽이 존재할때. -> 오류
					if(Objects[i]->x-Objects[i]->width == Objects[j]->x && Objects[i]->y == Objects[j]->y)
						break;
				if(j==O_Count){
					forbid = 0;
					xp=2*Objects[i]->x-xp;
					if((keydown&1)!=0){//0001 - 왼쪽
						Balls[k]->vy=-300;
						speed=200;
						if(recentkey==2)	recentkey=1;
					}else{
						speed=100;
					}
					return -speed*sign(Balls[k]->vx);
				}
			}
		}
	}
	return Balls[k]->vx;
}

double Y_Reflect(int k,double xp, double yp){
	int i,j;
	double x,y;
	x = Balls[k]->x;
	y = Balls[k]->y;
	for(i=0;i<O_Count;i++){
		if(Objects[i]->x<=xp+8 && xp<=Objects[i]->x+Objects[i]->width && Objects[i]->y<=yp+8 && yp<=Objects[i]->y+Objects[i]->height){

			//(xp,yp)가 벽 안에 있음
			if(yp <= Objects[i]->y + Objects[i]->height && Objects[i]->y + Objects[i]->height <= y){//위쪽벽에 충돌
				for(j=0;j<O_Count;j++)//충돌한 벽에 아래쪽이 존재할때. -> 오류
					if(Objects[i]->y+Objects[i]->height == Objects[j]->y && Objects[i]->x == Objects[j]->x)
						break;

				if(j==O_Count){
					forbid = 0;
					yp=2*(Objects[i]->y+Objects[i]->height)-yp;
					if(Objects[i]->Type>0)
						return -1*Balls[k]->vy*sqrt(Objects[i]->Type);
					else
						return -1*Balls[k]->vy;
				}
			}
			if(y+8 <= Objects[i]->y && Objects[i]->y <= yp+8){//아래쪽벽에 충돌
				for(j=0;j<O_Count;j++)//충돌한 벽에 위쪽이 존재할때. -> 오류
					if(Objects[i]->y-Objects[i]->height == Objects[j]->y && Objects[i]->x == Objects[j]->x)
						break;
				if(j==O_Count){
					forbid = 0;
					yp=2*Objects[i]->y-yp;
					if(Objects[i]->Type > 0)
						return -300*sign(Balls[k]->vy)*sqrt(Objects[i]->Type);
					else{
						if((int)(Objects[i]->Type+0.5) == -2 ){ // 왼쪽으로
							Balls[k]->x = Objects[i]->x;
							Balls[k]->y = Objects[i]->y + Objects[i]->height / 2;
							forbid = -3;
						}
						if((int)(Objects[i]->Type+0.5) == -3 ){ // 오른쪽으로
							Balls[k]->x = Objects[i]->x + Objects[i]->width;
							Balls[k]->y = Objects[i]->y + Objects[i]->height / 2;
							forbid = -4;
						}
					}
				}
			}
		}
	}
	return Balls[k]->vy;
}
char STR[128];


void RealTime(double TimeDelta=(double)1/60){
	int i;
	double xp, yp;

	if(LevelLoading!=0)	return;

	//KEY로인한 이동
	for(i=0;i<B_Count;i++){
		sprintf(STR,"%d,%d,%g",iskeydown,keydown,Balls[i]->vx);
		{
			switch(recentkey){
			case 1:
				for(i=0;i<B_Count;i++){
					if(Balls[i]->vx>-180)	Balls[i]->vx-=10;
				}
				break;
			case 2:
				for(i=0;i<B_Count;i++){
					if(Balls[i]->vx<180)	Balls[i]->vx+=10;
				}
				break;
			case 3:
				break;
			case 4:
				break;
			}
		}
	}



	for(i=0;i<B_Count;i++){

		//Ball이 이동할 경로
		xp = Balls[i]->x + Balls[i]->vx * TimeDelta;
		yp = Balls[i]->y + Balls[i]->vy * TimeDelta;

		if( !Enable2Go( xp , yp ) ){
			//벽의 충돌
			Balls[i]->vx = X_Reflect(i, xp, yp);
			Balls[i]->vy = Y_Reflect(i, xp, yp);

			if(forbid==-3){
				Balls[i]->vy = 0;
				Balls[i]->vx = -500;
			}
			if(forbid==-4){
				Balls[i]->vy = 0;
				Balls[i]->vx = 500;
			}
		}

		
		xp = Balls[i]->x + Balls[i]->vx * TimeDelta;
		yp = Balls[i]->y + Balls[i]->vy * TimeDelta;
		
		//속도로 인한 이동

		Balls[i]->x = xp;
		Balls[i]->y = yp;

		//중력에 의한 효과
		if(forbid==0)	Balls[i]->vy += gy * TimeDelta;

		if(Balls[i]->y > block_rect.bottom)
			Balls[i]->reset();
	}


	//끝점 도달
	int static goal_time;

	for(i=0;i<B_Count;i++)
		if(EndPoint->x<Balls[i]->x && Balls[i]->x<EndPoint->x+EndPoint->width)
			if(EndPoint->y<Balls[i]->y && Balls[i]->y<EndPoint->y+EndPoint->height)
				if(!Goal)
					Goal=1, goal_time = GetTickCount();
	
	if(Goal==1){
		if(GetTickCount()-goal_time>2000){
			for(i=0;i<B_Count;i++)
				if(Balls[i]!=NULL){
					delete Balls[i];
					for(int j=i;j<B_Count;j++)
						Balls[j]=Balls[j+1];
					B_Count--;
				}
			CloseLevel();
			LoadLevel(Level+1);
			return;
		}
	}

}


void MSG_PAINT(HWND hWnd)
{
	char LStr[128];

	PAINTSTRUCT ps;
	HDC hdc, hMemDC;
	HDC SurfaceDC;
	HBITMAP hBitmap, OldBitmap;
	HBRUSH BGBrush, BallBrush;
    HFONT Font, OldFont;

	hdc = BeginPaint(hWnd,&ps);
	hMemDC = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	OldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

		SurfaceDC = CreateCompatibleDC(hMemDC);

	Font = CreateFont(13,0,0,0,400,0,0,0,HANGEUL_CHARSET,0,0,0,0,TEXT("돋움체"));
	OldFont = (HFONT)SelectObject(hMemDC,Font);
//	SetBkMode(hMemDC, 1);
	SetTextColor(hMemDC, 0x000000);	// 폰트색
	SetBkColor(hMemDC, 0xffffff);	// 배경색

		BGBrush = CreateSolidBrush(0x000000);
		FillRect(hMemDC, &block_rect, BGBrush);
		DeleteObject(BGBrush);

		if(LevelLoading==0){	//배경을 알파블랜딩
			BLENDFUNCTION blendInfo;
			static BYTE byAlpha = 180; // 중간부터 시작 // 127
// 			static BYTE byAlpha = 0; // 중간부터 시작 // 127
 
		 //// 블랜드구조체 설정
			blendInfo.BlendOp    = AC_SRC_OVER;
			blendInfo.BlendFlags   = 0;
			blendInfo.AlphaFormat   = 0;  // 항상 0
			blendInfo.SourceConstantAlpha = byAlpha; // 알파값 조절
 
		 //// 출력
			int a, b, c, d;
			for(int i=0;i<B_Count;i++){
				a = Balls[i]->x-200 + block_rect.left;
				b = Balls[i]->y-200 + block_rect.top;

				AlphaBlend(	hMemDC,	max(a,crt.left), max(b,crt.top), min(400,crt.right-max(a,crt.left)), min(400,crt.bottom-max(b,crt.top)),
							hdc,	max(a,crt.left), max(b,crt.top), min(400,crt.right-max(a,crt.left)), min(400,crt.bottom-max(b,crt.top)),
							blendInfo);
			}
		}


		if(LevelLoading==0){	//블록 출력
			for(int i=0;i<O_Count;i++){

				int B_Type=(int)(Objects[i]->Type+0.5); // 0.2 = EPSILON
				if(B_Type == 1)		OldBlock = (HBITMAP)SelectObject(SurfaceDC, hBlock1);
				if(B_Type == 2)		OldBlock = (HBITMAP)SelectObject(SurfaceDC, hBlock2);
				if(B_Type == -2)	OldBlock = (HBITMAP)SelectObject(SurfaceDC, hBlock4);
				if(B_Type == -3)	OldBlock = (HBITMAP)SelectObject(SurfaceDC, hBlock5);

				BitBlt(hMemDC, block_rect.left+Objects[i]->x, block_rect.top+Objects[i]->y, Objects[i]->width, Objects[i]->height, SurfaceDC, 0, 0, SRCCOPY);

				SelectObject(SurfaceDC, OldBlock);

			}
		}
		if(LevelLoading==0)	if(!Goal){//End Point
			OldBlock = (HBITMAP)SelectObject(SurfaceDC, hBlock3);

			BitBlt(hMemDC, block_rect.left+EndPoint->x, block_rect.top+EndPoint->y, EndPoint->width, EndPoint->height, SurfaceDC, 0, 0, SRCCOPY);

			SelectObject(SurfaceDC, OldBlock);
		}
		if(LevelLoading==0){	//Ball 출력
			RECT ball;
			OldBlock = (HBITMAP)SelectObject(SurfaceDC, hBlock1);
			for(int i=0;i<B_Count;i++){
				ball.left=block_rect.left+(int)Balls[i]->x;
				ball.right=block_rect.left+(int)Balls[i]->x+8;
				ball.top=block_rect.top+(int)Balls[i]->y;
				ball.bottom=block_rect.top+(int)Balls[i]->y+8;
				if(Goal)	BallBrush = CreateSolidBrush(0x00ffff);
				if(!Goal)	BallBrush = CreateSolidBrush(0xffffff);
				FillRect(hMemDC, &ball, BallBrush);
				DeleteObject(BallBrush);
			}
			SelectObject(SurfaceDC, OldBlock);
		}

		//FPS출력
		if(Time == -1){	Time = GetTickCount();FPS_Count=0;}
		if(GetTickCount()-Time>=1000)	FPS = FPS_Count, Time = -1;
		else	FPS_Count++;

		if(LevelLoading!=0){
			LevelLoading++;
			isloading = 1;
			sprintf(LStr,"LEVEL %d",Level);
			TextOutA(hMemDC,(block_rect.right+block_rect.left)/2-50,(block_rect.bottom+block_rect.top)/2-20,LStr,lstrlenA(LStr));
		}
		if(LevelLoading>50)	LevelLoading=0, isloading = 0;

		while((double)(GetTickCount()-Time)<FPS_Count*1000/60);//60FPS 맞추기


		sprintf(LStr,"FPS : %d",FPS);
		TextOutA(hMemDC,10,10,LStr,lstrlenA(LStr));


		if(isloading){
			if(FPS>=60)	isloading=0;
			else{
				sprintf(LStr,"Loading...");
				TextOutA(hMemDC,(block_rect.right+block_rect.left)/2-60,(block_rect.bottom+block_rect.top)/2,LStr,lstrlenA(LStr));
			}
		}


	BitBlt(hdc, crt.left, crt.top, crt.right, crt.bottom, hMemDC, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(hMemDC, OldFont));
	DeleteObject(SelectObject(hMemDC, OldBitmap));
	DeleteDC(hMemDC);
	DeleteDC(SurfaceDC);
	EndPaint(hWnd,&ps);
}

int KEY( WPARAM wParam, LPARAM lParam ){//KEYDOWN이 계속 누르는거에 대해서는 해당x KEYUP까지 고려해주어야한다.
	int press=0;

	if(LevelLoading!=0)	return 0;
	if(GetAsyncKeyState(VK_LEFT)&0x8000){
		keydown |= 1;// 0001
		if(!iskeydown)	recentkey = 1;
		press=1;
	}else{
		keydown &= 14;// 1110
	}
	if(GetAsyncKeyState(VK_RIGHT)&0x8000){
		keydown |= 2;// 0010
		if(!iskeydown)	recentkey = 2;
		press=1;
	}else{
		keydown &= 13;// 1101
	}
	if(GetAsyncKeyState(VK_UP)&0x8000){
		keydown |= 4;// 0100
		if(!iskeydown)	recentkey = 3;
	}else{
		keydown &= 11;// 1011
	}
	if(GetAsyncKeyState(VK_DOWN)&0x8000){
		keydown |= 8;// 1000
		if(!iskeydown)	recentkey = 4;
	}else{
		keydown &= 7;// 0111
	}

	if(keydown != 0){
		if(forbid!=0){
			for(int i=0;i<B_Count;i++){
				if(Balls[i]->vx<-150)
					Balls[i]->vx=-150;
				else if(Balls[i]->vx>150)
					Balls[i]->vx=150;
			}
			forbid = 0;
		}
	}

	if(press==1)
		return 1;

	recentkey=0;

	
	if(GetAsyncKeyState(VK_ESCAPE)&0x80000){
		PostQuitMessage(0);
	}
	return 0;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam )
{
	int i;

	if(!KEY(wParam,lParam)){
		if(forbid==0){
			for(i=0;i<B_Count;i++)
				if(Balls[i]->vx!=0)
					Balls[i]->vx-=sign(Balls[i]->vx)*2;
		}
	}


	switch(iMessage) {
	case WM_TIMER:
						if(!isloading)	RealTime();
						InvalidateRect(hWnd, NULL, FALSE);	break;
	case WM_PAINT:		MSG_PAINT(hWnd);					break;
	case WM_CREATE:
						GetClientRect(hWnd, &crt);
						SetTimer(hWnd,0,10,NULL);			break;

	case WM_KEYDOWN:	iskeydown = 1;						break;
	case WM_KEYUP:		iskeydown = 0;						break;

	case WM_DESTROY:	PostQuitMessage(0);					break;
	}

	return DefWindowProc( hWnd, iMessage, wParam, lParam );
}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow )
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = CreateSolidBrush(RGB( 255,255,255 ) );
	WndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	WndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = ( WNDPROC )WndProc;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = TEXT( "Physics_Ball" );
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass( &WndClass );

/*	
	hWnd = CreateWindow( TEXT( "Physics_Ball" ), TEXT( "Physics_Ball" ),
	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
	800, 600, NULL, (HMENU)NULL, hInstance, NULL );
*/	
	hWnd=CreateWindow(TEXT( "Physics_Ball" ), TEXT( "Physics_Ball" ),WS_POPUP,
		  0,0,1280,800,
		  NULL,(HMENU)NULL,hInstance,NULL);

	ShowWindow( hWnd, nCmdShow );
 
	Init();

	DEVMODE dm;
	{
		ZeroMemory(&dm,sizeof(DEVMODE));
		dm.dmSize=sizeof(DEVMODE);
		dm.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		dm.dmBitsPerPel=32;
		dm.dmPelsWidth=1280;
		dm.dmPelsHeight=800;
		if( ChangeDisplaySettings(&dm,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
			ChangeDisplaySettings(&dm,0);
	}


	while( GetMessage( &Message, NULL, 0, 0 ) ) {
		TranslateMessage( &Message );
		DispatchMessage( &Message );
	}
	return (int)Message.wParam;
}
