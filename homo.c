#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <tchar.h>

char *data="";
int befnum=0;
HANDLE arduino;
BOOL Ret;
int i=0;
int leng=0;
BOOL start=FALSE;

#define state	0
#define x	1
#define y	2
#define order	3
#define lr	4
#define pic	5

int iRT=0;
int iLT=0;
int iUP=0;
int iDN=0;
int iA=0;
int iZ=0;
int iET=0;
BOOL ard=FALSE;
BOOL rmt=FALSE;
int btn=0;
const int width=80;
const int col=10;

int input(void);
int crease(int,int,int,int);
int bottun(void);
	
int main(void){
	int i,j,s;
	int hp=1000;
	clock_t start=clock();
	int scr=0;
	int bl[4]={0,0,1,0};//0:状態,1:X,2:Y,3:描画順番,
	const int btxt=8;
	int hm[6]={0,0,1,0,1,0};//1:X,2:Y,3:描画順番,4:左右,5:絵
	const int ftxt=19;
	int size[4]={0,0,0,0};
	int rnd=0;
	int num=0;
	int mode=0;
	char tmp[100];
	char str[200];
	int spd=70;

	//arduino設定
	arduino = CreateFile(_T("COM4"),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(arduino==INVALID_HANDLE_VALUE){
		printf("could not open handle\n");
	}else{
		Ret=SetupComm(arduino,1024,1024);
		if(!Ret){
			printf("could not setup\n");
			CloseHandle(arduino);
		}else{
			Ret=PurgeComm(arduino,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
			if(!Ret){
				printf("could not clear\n");
				CloseHandle(arduino);	
			}else{
				DCB dcb;
				GetCommState(arduino,&dcb);
				dcb.BaudRate=CBR_57600;
				dcb.ByteSize=8;
				dcb.fParity=NOPARITY;
				dcb.StopBits=ONESTOPBIT;
				dcb.fOutxCtsFlow=TRUE;
				dcb.fRtsControl=RTS_CONTROL_ENABLE;

				Ret=SetCommState(arduino,&dcb);
				if(!Ret){
					printf("could not setcommstate\n");
					CloseHandle(arduino);
				}else{
					ard=TRUE;
				}
			}
		}
	}
	/*if(ard){
		printf("remote or pad?\nremote\tpad\n←\t→\n");
		ard=FALSE;
		while(1){
			input();
			if(iLT){rmt=TRUE;ard=TRUE;break;}else if(iRT){rmt=FALSE;ard=TRUE;break;}
		}
	}else{
		printf("key only\n");
	}*/
	rmt=FALSE;ard=TRUE;
	Sleep(500);

	//画面サイズ設定
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE); //ハンドルの取得
	COORD dwCoord={width+ftxt+1,25}; //文字表示サイズ
	SMALL_RECT rctWindowRect={0,0,width+ftxt,col}; //位置{Left,Top,Right,Bottom}
	SetConsoleScreenBufferSize(handle,dwCoord);
	SetConsoleWindowInfo(handle,TRUE,&rctWindowRect);

	while(1){
		start=clock();
		GetConsoleScreenBufferInfo(handle,&csbi);
		size[0]=csbi.srWindow.Right-csbi.srWindow.Left;
		size[1]=csbi.srWindow.Bottom-csbi.srWindow.Top;
		if(size[0]!=size[2]||size[1]!=size[3]){
			SMALL_RECT rctWindowRect={0,0,width+ftxt,col};
			SetConsoleWindowInfo(handle,TRUE,&rctWindowRect);
			size[2]=size[0];
			size[3]=size[1];
		}

		//文字列初期化
		memset(str,0,sizeof(str));
		
		//入力キー判定
		input();
		
		/*itoa(mode,tmp,10);
		strcat(str,"Mode:");
		strcat(str,tmp);*/

		switch(mode){
		case 0:
			printf("←↓↑→:MOVE\nA:SPEED UP\nZ:SPEED DOWN\n\nPRESS ENTER\n");
			for(s=0;s<col-6;s++){strcat(str,"\n");}
			hp=1000;
			scr=0;
			spd=100;
			hm[x]=0;
			hm[y]=1;
			hm[pic]=0;
			hm[lr]=1;
			mode=1;
			break;
		case 1:
			if(iET){mode=2;spd=70;}
			break;
		case 2:
			//入力されたキーによって処理
			if(iRT){
				hm[lr]=1;
				hm[x]=crease(hm[x],1,0,width);
				hm[pic]=(hm[pic]+1)%2;
			}
			if(iLT){
				hm[lr]=0;
				hm[x]=crease(hm[x],-1,0,width);
				hm[pic]=(hm[pic]+1)%2;
			}
			if(iUP){
				hm[y]=crease(hm[y],-1,1,col-1);
			}
			if(iDN){
				hm[y]=crease(hm[y],1,1,col-1);
			}
			if(iA){
				spd=crease(spd,-5,15,100);
			}
			if(iZ){
				spd=crease(spd,5,15,100);
			}

			//BL本設定
			if(bl[state]==0){
				srand((unsigned)(time(NULL)*rand()));
				rnd=rand()%((width-btxt)*(col-1));
				bl[x]=rnd%width;
				bl[y]=((int)rnd/width)+1;
				bl[state]=1;
			//当たり判定
			}else if((hm[y]==bl[y])&&(bl[x]+btxt-1>=hm[x])&&(bl[x]-ftxt+1<=hm[x])){
				bl[state]=0;
				scr++;
			}

			//順番
			if((bl[state]==1)&&(hm[y]==bl[y])&&(hm[x]>bl[x])){hm[order]=2;}else{hm[order]=1;}
			if(bl[state]==0){bl[order]=0;}else if((hm[y]==bl[y])&&(bl[x]>hm[x])){bl[order]=2;}else{bl[order]=1;}
			
			//一行目・HP
			hp--;
			if(hp<=0){mode=3;break;}
			strcat(str,"Time:");
			for(s=0;s<hp/100+1;s++){strcat(str,"■");}
			for(s=0;s<10-(hp/100+1);s++){strcat(str,"□");}
			//スピード
			strcat(str,"　Speed:");
			for(s=0;s<(105-spd)/5;s++){strcat(str,"|");}
			for(s=0;s<(spd-15)/5;s++){strcat(str," ");}
			//スコア
			itoa(scr,tmp,10);
			strcat(str,"　Score:");
			strcat(str,tmp);
			strcat(str,"\n");
			
			//二～六行目以降・ホモォ/BL本
			for(i=1;i<=col-1;i++){
				for(j=1;j<=2;j++){
					if((i==hm[y])&&(j==hm[order])){
						if(hm[order]==1){for(s=0;s<hm[x];s++){strcat(str," ");}
						}else{for(s=0;s<hm[x]-bl[x]-btxt;s++){strcat(str," ");}}
						if(hm[lr]==1){ //左右
							if(hm[pic]==0){strcat(str,"三┌(┌ ^o^)┐ﾎﾓｫ…");
							}else{strcat(str,"三へ(へ ^o^)へﾎﾓｫ…");}
						}else{
							if(hm[pic]==0){strcat(str,"ﾎﾓｫ…┌(^o^ ┐)┐三");
							}else{strcat(str,"ﾎﾓｫ…へ(^o^ へ)へ三");}
						}
					}
					if((i==bl[y])&&(j==bl[order])){
						if(bl[order]==1){for(s=0;s<bl[x];s++){strcat(str," ");}
						}else{for(s=0;s<bl[x]-hm[x]-ftxt;s++){strcat(str," ");}}
						strcat(str,"【BL本】");
					}
				}
				strcat(str,"\n");
			}
			break;
		case 3:
			strcat(str,"Result: ");
			itoa(scr,tmp,10);
			strcat(str,tmp);
			strcat(str,"pts\n\n");
			if(i){strcat(str,"┌(┌ ^o^)┐\n\n\n");}
			else{strcat(str,"へ(へ ^o^)へ\n\n\n");}
			strcat(str,"PRESS ENTER\n");
			for(s=0;s<col-6;s++){strcat(str,"\n");}
			i=(i+1)%2;
			spd=1500-scr*20;
			break;
		}
		printf(str);
		while(clock()-start<spd){
			input();
			if(mode==3&&iET){mode=0;spd=1000;break;}
		}
	}

	if(ard){CloseHandle(arduino);}
	return 0;

}

int crease(int dim,int change,int min,int max){
	dim+=change;
	if(dim<min){dim=min;}
	if(max<dim){dim=max;}
	return dim;
}
int bottun(void){
	int num=0;
	DWORD dwError;
	DWORD dwGetSize;
	COMSTAT comstat;

	Ret=ClearCommError(arduino,&dwError,&comstat);
	if(!Ret){
		printf("could not clear error\n");
	}else{
		leng=comstat.cbInQue;
	}
		
	Ret=ReadFile(arduino,&data,leng,&dwGetSize,NULL);
	if(!Ret){
		printf("could not get data\n");
	}else{
		if(data==""){num=befnum;}
		num=(int)data-808464432;
		befnum=num;
	}

	return num;
}
int input(void){
	//入力キー判定
	if(ard){
		btn=bottun();
		//printf("%d\n",btn);
		if(rmt){
			if(btn==61){iET=1;}else{iET=0;}
			if(btn==62){iUP=1;}else{iUP=0;}
			if(btn==63){iLT=1;}else{iLT=0;}
			if(btn==64){iDN=1;}else{iDN=0;}
			if(btn==65){iRT=1;}else{iRT=0;}	
		}else{
			if(btn>=17){btn-=7;}
			if(btn>=42){btn-=6;}
			if(btn>=32){btn-=32;iA=1;iET=1;}else{iA=0;iET=0;}
			if(btn>=16){btn-=16;iZ=1;}else{iZ=0;}
			if(btn>=8){btn-=8;iDN=1;}else{iDN=0;}
			if(btn>=4){btn-=4;iRT=1;}else{iRT=0;}
			if(btn>=2){btn-=2;iUP=1;}else{iUP=0;}
			if(btn>=1){btn-=1;iLT=1;}else{iLT=0;}
		}
	}else{
		if(GetAsyncKeyState(VK_RIGHT)<0){iRT=1;}else{iRT=0;}
		if(GetAsyncKeyState(VK_LEFT)<0){iLT=1;}else{iLT=0;}
		if(GetAsyncKeyState(VK_UP)<0){iUP=1;}else{iUP=0;}
		if(GetAsyncKeyState(VK_DOWN)<0){iDN=1;}else{iDN=0;}
		if(GetAsyncKeyState(VK_RETURN)<0){iET=1;}else{iET=0;}
		if(GetAsyncKeyState('A')<0){iA=1;}else{iA=0;}
		if(GetAsyncKeyState('Z')<0){iZ=1;}else{iZ=0;}
	}
}

