#include <iostream>
#include<graphics.h>
#include<filesystem>

#include"tools.h"

using namespace std;
#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum{ WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT };

IMAGE imgBg;  //表示背景图片
IMAGE imaBar;
IMAGE imgCards[ZHI_WU_COUNT];  //表示卡片图片
IMAGE *imgZhiWu[ZHI_WU_COUNT][20];  //表示植物图片

int curX, curY;//当前选中的植物，在移动过程中位置
int curZhiWu; //0:表示没有选中植物，1表示选中了第一个植物，2表示选中了第二个植物

struct zhiwu
{
	int type;        //0:没有植物，1：第一种植物
	int frameIndex;  //序列帧的序号
};

struct zhiwu zhiwuMap[5][9]; //表示植物的二维数组

bool fileExist(const char* name) {
	return filesystem::exists(name);
}

void gameInit() {

	//加载背景图片
	//把字符集修改为“多字节字符集”，否则会报错
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imaBar, "res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(zhiwuMap, 0, sizeof(zhiwuMap));

	//初始化植物卡片图片
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//生成植物卡片的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0;j < 20;j++) {
			//生成植物的文件名
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i , j + 1);
			//先判断这个文件是否存在
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name);
			}
			else {
				break;
			}
		}

	}

	curZhiWu = 0;
	//创建游戏图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT,1);
};

void updataWindow() {
	BeginBatchDraw();//开始缓冲

	//更新窗口内容
	putimage(0, 0, &imgBg);
	//putimage(250, 0, &imaBar);
	putimagePNG(250, 0, &imaBar);
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
	
	//渲染 拖动过程中的植物
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	for(int i=0;i<3;i++){
		for (int j = 0;j < 9;j++) {
			if (zhiwuMap[i][j].type > 0) {
				int x = 256 + j * 81;
				int y = 179 + i * 102 + 14;
				int zhiwuType = zhiwuMap[i][j].type - 1;
				int index = zhiwuMap[i][j].frameIndex;
				putimagePNG(x,y,imgZhiWu[zhiwuType][index]);
			}
		}

	}

	EndBatchDraw();//结束双缓冲
};

void userClick() {
	ExMessage msg;
	static int status = 0; //0表示没有点击，1表示点击了卡片
	if(peekmessage(&msg)){
		if (msg.message == WM_LBUTTONDOWN) {
			//左键按下事件
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {
				int index = (msg.x - 338) / 65;
				status = 1;
				curZhiWu = index + 1;
			}
		}
		else if(msg.message == WM_MOUSEMOVE && status == 1){
			//鼠标移动事件
			curX = msg.x;
			curY = msg.y;
		}
		else if(msg.message == WM_LBUTTONUP){
			if (msg.x > 256 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				if (zhiwuMap[row][col].type == 0) {
					zhiwuMap[row][col].type = curZhiWu;
					zhiwuMap[row][col].frameIndex = 0;
				}
			}

			curZhiWu = 0;
			status = 0;

		}
	}
};

int main(void) {
	gameInit();

	while (1) {
		userClick();


		//更新窗口内容
		updataWindow();
	}

	system("pause");
	return 0;
}