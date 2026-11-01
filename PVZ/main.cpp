#include <iostream>
#include <filesystem>
#include <time.h>
#include "plant.h"
#pragma comment(lib,"winmm.lib")

using namespace std;

IMAGE imgBg;  //表示背景图片
IMAGE imaBar;
IMAGE imgCards[ZHI_WU_COUNT];  //表示卡片图片
IMAGE *imgZhiWu[ZHI_WU_COUNT][20];  //表示植物图片
IMAGE imgSunshineBall[29]; //表示阳光球图片
IMAGE imgZombies[22]; //表示僵尸图片
IMAGE imgBulletNormal;  //表示豌豆子弹图片
IMAGE imgBulletBlast[4];   //碰撞帧数组
IMAGE imgZombieDead[20];    //僵尸死亡的图片
IMAGE imgZombieEat[21];    //僵尸吃植物图片
IMAGE imgChomperEat[9];      //大嘴花啃食动画
IMAGE imgChomperDigest[6];   //大嘴花消化动画
IMAGE imgPotatoDown;         //土豆地雷出土前（地下）
IMAGE imgPotatoBlast;        //土豆地雷爆炸

int curX, curY;//当前选中的植物，在移动过程中位置
int curZhiWu; //0:表示没有选中植物，1表示选中了第一个植物，2表示选中了第二个植物
int sunshine;
int plantCost[ZHI_WU_COUNT] = { 100, 50, 150, 25, 50 };  //豌豆/向日葵/大嘴花/土豆雷/坚果



Zombie* zombies[20] = {};   // ← 关键：指针数组，不存对象



Plant* zhiwuMap[3][9] = {};


bool fileExist(const char* name) {
	return filesystem::exists(name);
}

void gameInit() {

	//加载背景图片
	//把字符集修改为“多字节字符集”，否则会报错
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imaBar, "res/bar5.png");

	//初始化植物卡片图片
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//生成植物卡片的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0;j < 20;j++) {
			//生成植物的文件名
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
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
	sunshine = 50;

	for (int i = 0; i < 3; i++) for (int j = 0; j < 9; j++) zhiwuMap[i][j] = nullptr;
	for (int i = 0;i < 29;i++) {
		Sunflower::resetPool();
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//配置随机种子
	srand(time(NULL));


	//创建游戏图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	//设置当前字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;        //抗锯齿效果
	settextstyle(&f);
	setbkmode(TRANSPARENT);  //设置背景透明
	setcolor(BLUE);

	//初始化僵尸数据
	for (int i = 0; i < 20; i++) zombies[i] = nullptr;
	for (int i = 0;i < 22;i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		ConeheadZombie::loadRes();
		BucketheadZombie::loadRes();
		loadimage(&imgZombies[i], name);
	}

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	Peashooter::resetPool();
	//子弹爆炸帧加载
	for (int i = 0;i < 4;i++)
	{
		sprintf_s(name, sizeof(name), "res/bullets/bullet_blast%d.png", i);
		loadimage(&imgBulletBlast[i], name);
	}

	for (int i = 0; i < 9; i++) {
		sprintf_s(name, sizeof(name), "res/zhiwu/2_eat/%d.png", i + 1);   //大嘴花啃食动画
		loadimage(&imgChomperEat[i], name);
	}

	for (int i = 0; i < 6; i++) {
		sprintf_s(name, sizeof(name), "res/zhiwu/2_digest/%d.png", i + 1);    //大嘴花消化动画
		loadimage(&imgChomperDigest[i], name);
	}

	loadimage(&imgPotatoDown, "res/zhiwu/3_down.png");    //土豆地雷出土前（地下）
	loadimage(&imgPotatoBlast, "res/zhiwu/3_blast.png");  //土豆地雷爆炸

	for (int i = 0;i < 20;i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/Image%d.png", i + 1);
		loadimage(&imgZombieDead[i], name);
	}

	for (int i = 0;i < 21;i++) {
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);
			loadimage(&imgZombieEat[i], name);
	}

};

void drawZombie() {
	int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
	for (int i = 0; i < zombieCount; i++) {
		if (zombies[i]) zombies[i]->draw();
	}
}



void drawPlant() {
	for (int i = 0;i < 3;i++) {
		for (int j = 0;j < 9;j++) {
			if (zhiwuMap[i][j]) zhiwuMap[i][j]->draw();
		}
	}
	//渲染 拖动过程中的植物
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}
}

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
	drawPlant();
	Sunflower::drawSunshines();
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 67, scoreText);     //输出阳光数

	drawZombie();
	Peashooter::drawBullets();

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
			else {
				Sunflower::collectSunshine(&msg);
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
				if (zhiwuMap[row][col] == nullptr && curZhiWu >= 1 && curZhiWu <= ZHI_WU_COUNT) {
					int cost = plantCost[curZhiWu - 1];
					if (sunshine >= cost) {
						switch (curZhiWu) {
						case WAN_DOU + 1:       zhiwuMap[row][col] = new Peashooter(row, col); break;
						case XIANG_RI_KUI + 1:  zhiwuMap[row][col] = new Sunflower(row, col); break;
						case DA_ZUI_HUA + 1:    zhiwuMap[row][col] = new Chomper(row, col); break;
						case TU_DOU_DI_LEI + 1: zhiwuMap[row][col] = new PotatoMine(row, col); break;
						case JIAN_GUO_QIANG + 1: zhiwuMap[row][col] = new WallNut(row, col); break;
						}
						sunshine -= cost;
					}
				}
			}

			curZhiWu = 0;
			status = 0;

		}
	}
};

void createZombie() {
	static int zombieFre = 200;
	static int count = 0;
	if (++count <= zombieFre) return;
	count = 0;
	zombieFre = rand() % 300 + 100;

	int ZombieMax = sizeof(zombies) / sizeof(zombies[0]);
	int i;
	for (i = 0; i < ZombieMax && zombies[i] != nullptr; i++);
	if (i >= ZombieMax) return;

	int r = rand() % 10;
	int row = rand() % 3;
	if (r < 5)       zombies[i] = new NormalZombie(row);       // 0~4  共5/10 = 50%
	else if (r > 4 && r < 8)  zombies[i] = new ConeheadZombie(row);     // 5~7  共3/10 = 30%
	else             zombies[i] = new BucketheadZombie(row);   // 8~9  共2/10 = 20%
}

void updateZombie() {
	int ZombieMax = sizeof(zombies) / sizeof(zombies[0]);
	static int count = 0;
	count++;
	if (count > 8) {
		count = 0;
	}
	//更新僵尸的位置
	static int moveCount = 0;
	moveCount++;
	if (moveCount > 2) {        // 每 3 帧才移一次 → 速度降到原来 1/3
		moveCount = 0;
		for (int i = 0;i < ZombieMax;i++) {
			if (zombies[i]) zombies[i]->move();
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 > 4) {
		count2 = 0;
		for (int i = 0;i < ZombieMax;i++) {
			if (!zombies[i]) continue;
			zombies[i]->nextFrame();
			if (zombies[i]->isDeadEnd()) {     //死亡动画放完 → 回收
				delete zombies[i];
				zombies[i] = nullptr;
			}
		}
	}
}



void checkZombie() {
	int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
	for (int i = 0;i < zombieCount;i++) {
		Zombie* z = zombies[i];
		if (!z||z->isDead())continue;
		int row = z->getRow();
		int col = -1;
		for (int k = 0;k < 9;k++) {
			Plant* p = zhiwuMap[row][k];
			if (!p||!p->biteable()) continue;    //地下的土豆雷不啃
			int plantX = 256 + k * 81;
			int x1 = plantX + 10;   //植物左侧的坐标
			int x2 = plantX + 60;   //植物右侧的坐标
			int x3 = z->getX() + 80;   //僵尸左侧的坐标
			int x4 = z->getX() + 200;   //僵尸右侧坐标
			if (x3 > plantX + 10 && x3 < plantX + 60) { col = k; break; }   //找到嘴边的植物
		}
		if (col >= 0) {
			Plant* p = zhiwuMap[row][col];
			if (!z->isEating()) z->startEat();          //这只僵尸还没停 → 停下开啃
			if (z->biteTick()) p->hurt(25);
			if (p->isDead()) {
				delete p;
				zhiwuMap[row][col] = nullptr;
				z->finishEat();
			}
		}
		else if (z->isEating()) {
			z->finishEat();                             //嘴边的植物没了(被吞/被炸/被别人吃光) → 恢复行走
		}
	}
}


void collisionCheck() {
	Peashooter::checkBullet();   //子弹对僵尸的碰撞检测
	checkZombie();               //僵尸对植物的啃咬检测
}

void updatePlant() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (zhiwuMap[i][j]) zhiwuMap[i][j]->behaviour();   // 每帧：开枪/产阳光
		}
	}
	static int count = 0;
	if (++count >= 5) {
		count = 0;
		for (int i = 0;i < 3;i++) {
			for (int j = 0;j < 9;j++) {
				if (zhiwuMap[i][j]) zhiwuMap[i][j]->nextFrame();
			}
		}
	}
	for (int i = 0;i < 3;i++)
		for (int j = 0;j < 9;j++)
			if (zhiwuMap[i][j] && zhiwuMap[i][j]->isDead()) {
				delete zhiwuMap[i][j];
				zhiwuMap[i][j] = nullptr;
			}
	Sunflower::createSunshineFromSky();     // 原 createSunshine 前半段
}

void updateGame() {

	
	updatePlant();
	Sunflower::updateSunshine();
	createZombie(); //创建僵尸
	updateZombie();  //更新僵尸状态
	Peashooter::updateBullets();
	collisionCheck();  //实现豌豆子弹和僵尸的碰撞检测
}

void startUI() {
	IMAGE imgMenu, imgMenu1, imgMenu2;
	loadimage(&imgMenu, "res/Screen/MainMenu.png");
	loadimage(&imgMenu1, "res/Screen/Adventure_1.png");
	loadimage(&imgMenu2, "res/Screen/Adventure_0.png");
	int flag = 0;
	while (1) {
		BeginBatchDraw();
		putimage(0, 0, &imgMenu);
		putimagePNG(475, 75, flag ? &imgMenu2 : &imgMenu1);
		

		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN && msg.x > 474 && msg.x < 474 + 300 && msg.y>75 && msg.y < 75 + 140) {
				flag = 1;
				//EndBatchDraw();
			}
			else if (msg.message == WM_LBUTTONUP&& flag) {
				return;
			}
		}
		EndBatchDraw();
	}

};


int main(void) {
	gameInit();
	
	startUI();

	int timer = 0;
	bool flag = true;
	while (1) {
		userClick();
		timer += getDelay();
		if (timer > 15) {
			flag = true;
			timer = 0;
		}

		if (flag) {
			flag = false;
			//更新窗口内容
			updataWindow();
			updateGame();
		}
		

	}

	system("pause");
	return 0;
}