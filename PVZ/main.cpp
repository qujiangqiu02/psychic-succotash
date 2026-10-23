#include <iostream>
#include<graphics.h>
#include<filesystem>
#include<time.h>
#include"tools.h"
#include<mmsystem.h>
#include<math.h>
#pragma comment(lib,"winmm.lib")


using namespace std;
#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum{ WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT };

IMAGE imgBg;  //表示背景图片
IMAGE imaBar;
IMAGE imgCards[ZHI_WU_COUNT];  //表示卡片图片
IMAGE *imgZhiWu[ZHI_WU_COUNT][20];  //表示植物图片
IMAGE imgSunshineBall[29]; //表示阳光球图片
IMAGE imgZombies[22]; //表示僵尸图片
IMAGE imgBulletNormal;  //表示豌豆子弹图片
IMAGE imgBulletBlast[4];   //碰撞帧数组
IMAGE imgZombieDead[20];

int curX, curY;//当前选中的植物，在移动过程中位置
int curZhiWu; //0:表示没有选中植物，1表示选中了第一个植物，2表示选中了第二个植物
int sunshine;

struct zhiwu
{
	int type;        //0:没有植物，1：第一种植物
	int frameIndex;  //序列帧的序号
	int shootCd;     //植物攻击冷却计时器
};

struct zhiwu zhiwuMap[3][9]; //表示植物的二维数组

struct sunshineBall {
	int x, y;   //阳光球在飘落过程中的坐标（x不变）
	int frameIndex;  //当前显示的图片帧的序号
	int destY;  //阳光球的目标位置
	bool used;   //是否在使用
	int timer;   //定时器
	double xoff;  //阳光飞跃时每一帧的x偏移量
	double yoff;  //阳光飞跃时每一帧的y偏移量
};

//池,10个阳光球
struct sunshineBall sunballs[20];

struct Zombies {
	int x, y, frameIndex, speed, row, blood;
	bool used;
	bool dead;
};

struct Zombies zombies[20];
//子弹数据类型
struct bullet {
	int x, y, row, speed;
	int frameIndex;      //帧序号
	bool used;
	bool blast;   //是否发生碰撞
};
struct bullet bullets[100];


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
	sunshine = 50;

	memset(sunballs, 0, sizeof(sunballs));
	for (int i = 0;i < 29;i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//配置随机种子
	srand(time(NULL));


	//创建游戏图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT,1);
	
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
	memset(zombies, 0, sizeof(zombies));
	for (int i = 0;i < 22;i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZombies[i], name);
	}

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));
	//子弹爆炸帧加载
	for (int i = 0;i < 4;i++)
	{
		sprintf_s(name, sizeof(name), "res/bullets/bullet_blast%d.png", i);
		loadimage(&imgBulletBlast[i], name);
	}
	
	for (int i = 0;i < 20;i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZombieDead[i], name);
	}

};

void drawZombie() {
	int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
	for (int i = 0;i < zombieCount;i++) {
		if (zombies[i].used) {
			IMAGE* img = &imgZombies[zombies[i].frameIndex];
			putimagePNG(zombies[i].x, zombies[i].y - img->getheight(), img);
		}
	}
}

void drawBullets() {
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0;i < bulletMax;i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
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



	for (int i = 0;i < 3;i++) {
		for (int j = 0;j < 9;j++) {
			if (zhiwuMap[i][j].type > 0) {
				int x = 256 + j * 81;
				int y = 179 + i * 102 + 14;
				int zhiwuType = zhiwuMap[i][j].type - 1;
				int index = zhiwuMap[i][j].frameIndex;
				putimagePNG(x, y, imgZhiWu[zhiwuType][index]);
			}
		}

	}
	//渲染 拖动过程中的植物
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	int sunballMax = sizeof(sunballs) / sizeof(sunballs[0]);
	for (int i = 0;i < sunballMax;i++) {
		if (sunballs[i].used||sunballs[i].xoff) {
			IMAGE* img = &imgSunshineBall[sunballs[i].frameIndex];
			putimagePNG(sunballs[i].x, sunballs[i].y, img);

		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 67, scoreText);     //输出阳光数

	drawZombie();
	drawBullets();

	EndBatchDraw();//结束双缓冲
};

void collectSunshine(ExMessage* msg) {
	int count = sizeof(sunballs) / sizeof(sunballs[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for (int i = 0;i < count;i++) {
		if (sunballs[i].used) {
			//判断鼠标点击的坐标是否在阳光球的范围内
			IMAGE* img = &imgSunshineBall[sunballs[i].frameIndex];
			int x = sunballs[i].x;
			int y = sunballs[i].y;
			if (msg->x > x &&msg->x <x + w && msg->y > y && msg->y < y + h) {
				sunballs[i].used = false;
				//sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);
				//设置阳光球点击时的位置
				double destY = 0;
				double destX = 262;
				double angle = atan((y - destY) / (x - destX));
				sunballs[i].xoff = 90 * cos(angle);
				sunballs[i].yoff = 90 * sin(angle);
			}
		}
	}
}

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
				collectSunshine(&msg);
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

void createSunshine() {
	static int count = 0;
	static int fre = 50;
	count++;

	if (count >= fre) {
		fre = 100 + rand() % 400;  //随机生成阳光球的时间间隔
		count = 0;
		//从阳光池中取一个可以使用的
		int sunshineBallMax = sizeof(sunballs) / sizeof(sunballs[0]);
		int i;
		for (i = 0;i < sunshineBallMax && sunballs[i].used;i++);
		if (i >= sunshineBallMax) return; //没有可以使用的阳光球
		sunballs[i].used = true;
		sunballs[i].frameIndex = 0;
		sunballs[i].x = 260 + rand() % (850 - 260);
		sunballs[i].y = 60;
		sunballs[i].destY = rand() % 4 * 90 + 200;   //
		sunballs[i].timer = 0;
		sunballs[i].xoff = 0;
		sunballs[i].yoff = 0;

	}
	
}

void updateSunshine() {
	int sunballMax = sizeof(sunballs) / sizeof(sunballs[0]);
	for (int i = 0;i < sunballMax;i++) {
		if (sunballs[i].used) {
			sunballs[i].frameIndex = (sunballs[i].frameIndex + 1) % 29;
			if (sunballs[i].timer == 0) {
				sunballs[i].y += 2;
			}
			if (sunballs[i].y >= sunballs[i].destY) {
				
				sunballs[i].timer++;
				if (sunballs[i].timer > 100) {
					sunballs[i].used = false;
				}
			}
		}
		else if (sunballs[i].xoff) {
			//设置阳光球点击时的位置
			double destY = 0;
			double destX = 262;
			double angle = atan((sunballs[i].y - destY) / (sunballs[i].x - destX));
			sunballs[i].xoff = 90 * cos(angle);
			sunballs[i].yoff = 90 * sin(angle);
			sunballs[i].x -= sunballs[i].xoff;
			sunballs[i].y -= sunballs[i].yoff;
			if (sunballs[i].y < 0 || sunballs[i].x < 262) {
				sunballs[i].xoff = 0;
				sunballs[i].yoff = 0;
				sunshine += 25;
			}
		}
	}
}

void createZombie() {
	int ZombieMax = sizeof(zombies) / sizeof(zombies[0]);
	static int zombieFre = 200;
	static int count = 0;
	count++;
	if (count > zombieFre) {
		count = 0;
		zombieFre = rand() % 300 + 100;
		int i;
		for (i = 0;i < ZombieMax && zombies[i].used;i++);
		if (i < ZombieMax) {
			zombies[i].used = true;
			zombies[i].x = WIN_WIDTH;
			zombies[i].row = rand() % 3;    
			zombies[i].y = 172 + (1 + zombies[i].row) * 100;
			zombies[i].speed = 1;
			zombies[i].blood = 100;
		}
	}
}

void updataZombie() {
	int ZombieMax = sizeof(zombies) / sizeof(zombies[0]);
	static int count = 0;
	count++;
	if (count > 2) {
		count = 0;
	}
	//更新僵尸的位置
	for (int i = 0;i < ZombieMax;i++) {
		if (zombies[i].used) {
			zombies[i].x -= zombies[i].speed;
			if (zombies[i].x < 170) {
				MessageBox(NULL, "over", "over", 0);  //待优化
				exit(0);
			}
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 > 4) {
		for (int i = 0;i < ZombieMax;i++) {
			if (zombies[i].used) {
				zombies[i].frameIndex = (zombies[i].frameIndex + 1) % 22;

			}
		}
	}
}

void shoot() {
	int lines[3] = { 0 };
	int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - imgZombies[0].getwidth();
	for (int i = 0;i < zombieCount;i++) {
		if (zombies[i].used && zombies[i].x<dangerX) {
			lines[zombies[i].row] = 1;
		}
	}
	for (int i = 0;i < 3;i++) {    //遍历每株植物
		for (int j = 0;j < 9;j++) {
			struct zhiwu* p = &zhiwuMap[i][j];
			if (p->type == WAN_DOU + 1) {
				p->shootCd++;   //每个豌豆自己计时
				if (lines[i] && p->shootCd >= 35) {   //同行有僵尸，且冷却走完
					p->shootCd = 0;    //重置自己的冷却
					int k;
					for (k = 0;k < bulletMax && bullets[k].used;k++);
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 20;
						bullets[k].blast = false;
						bullets[k].frameIndex = 0;
						int plantX = 256 + j * 81;
						int plantY = 179 + i * 102 + 14;
						bullets[k].x = plantX + imgZhiWu[zhiwuMap[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = plantY + 5;
					}
				}
				
			}
		}
	}
}

void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0;i < countMax;i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}
			
			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;
				}
			}
		}
	}
}

void collisionCheck() {
	int bulletCount = sizeof(bullets) / sizeof(bullets)[0];
	int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
	int bulletWidth = imgBulletNormal.getwidth();
	for (int i = 0;i < bulletCount;i++) {
		if (bullets[i].used == false || bullets[i].blast)continue;
		int bulletRight = bullets[i].x + bulletWidth;
		for (int k = 0;k < zombieCount;k++) {
			if (zombies[k].used == false)continue;
			//僵尸受击范围
			int zx_left = zombies[k].x + 80;
			int zx_right = zombies[k].x + 110;
			if (bullets[i].row == zombies[k].row && bulletRight > zx_left && bullets[i].x < zx_right) {  //僵尸与植物同行，子弹从右侧接触僵尸身体才碰撞
				zombies[k].blood -= 10;
				bullets[i].blast = true;
				bullets[i].speed = 0;
				if (zombies[k].blood <= zombies[k].blood * 0.1) {
					zombies[k].dead = true;
					zombies[k].speed = 0;
					zombies[k].frameIndex = 0;

				}
				break;   //一个子弹只打一个僵尸
			}
		}
	}
}

void updataGame() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (zhiwuMap[i][j].type > 0) {
				zhiwuMap[i][j].frameIndex++;
				int zhiwuType = zhiwuMap[i][j].type - 1;
				int index = zhiwuMap[i][j].frameIndex;
				if (imgZhiWu[zhiwuType][index] == NULL) {
					zhiwuMap[i][j].frameIndex = 0;
				}
				//else {
				//	zhiwuMap[i][j].frameIndex = index;
				//}
			}
		}
	}

	createSunshine(); //创建阳光
	updateSunshine(); //更新阳光的状态
	createZombie(); //创建僵尸
	updataZombie();  //更新僵尸状态
	shoot();         //发射豌豆子弹
	updateBullets();  //更新豌豆子弹
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
		if (timer > 70) {
			flag = true;
			timer = 0;
		}

		if (flag) {
			flag = false;
			//更新窗口内容
			updataWindow();
			updataGame();
		}
		

	}

	system("pause");
	return 0;
}