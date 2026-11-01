#pragma once
#include "game.h"
#include "zombie.h"

//  植物基类
class Plant {
protected:
	int type;         // WAN_DOU / XIANG_RI_KUI，索引 imgZhiWu
	int row, col;     // 所在行列
	int x, y;         // 绘制坐标
	int frameIndex;   // 序列帧序号
	int blood;        // 血量，被啃累计扣，归零死亡
	bool catched;     // 是否正被僵尸啃咬
public:
	Plant(int _type, int _row, int _col, int _blood = 100)
		: type(_type), row(_row), col(_col), blood(_blood),
		frameIndex(0), catched(false) {
		x = 256 + col * 81;
		y = 179 + row * 102 + 14;
	}
	virtual ~Plant() {}

	virtual void draw() {
		putimagePNG(x, y, imgZhiWu[type][frameIndex]);
	}
	virtual void nextFrame() {
		frameIndex++;
		if (imgZhiWu[type][frameIndex] == NULL) frameIndex = 0;
	}
	void hurt(int d) {          // 僵尸啃咬伤害累加（每口25）
		blood -= d;
		if (blood < 0) blood = 0;
	}
	bool isDead()    const { return blood <= 0; }
	bool isCatched() const { return catched; }
	void setCatch(bool b) { catched = b; }
	int  getX()   const { return x; }
	int  getY()   const { return y; }
	int  getRow() const { return row; }
	int  getType() const { return type; }

	virtual bool biteable() const { return true; }   // 能否被僵尸啃，土豆雷地下期返回 fals
	// 统一虚函数入口：真身住在各子类的原名成员函数里
	virtual void behaviour() {}
};

// ---------------- 豌豆射手 ----------------
class Peashooter : public Plant {
	class Bullet {              // 嵌套类：外部不可见，子弹不属于植物，所以封装在射手内部
	public:
		int x, y, row, speed;
		int frameIndex;
		bool used;
		bool blast;
		Bullet() : x(0), y(0), row(0), speed(0), frameIndex(0), used(false), blast(false) {}
		Bullet(int _x, int _y, int _row)
			: x(_x), y(_y), row(_row), speed(20), frameIndex(0), used(true), blast(false) {
		}
	};
	inline static Bullet bullets[100];   // C++20：声明即定义，全类共享弹药池
	int shootCd;   //发射冷却时间
public:
	Peashooter(int r, int c) : Plant(WAN_DOU, r, c), shootCd(0) {}

	//static表示这个函数属于"类"本身，不属于某个具体的对象
	static void resetPool() {
		int max = sizeof(bullets) / sizeof(bullets[0]);
		for (int i = 0; i < max; i++) bullets[i] = Bullet();
	}
	static void drawBullets() {
		int max = sizeof(bullets) / sizeof(bullets[0]);
		for (int i = 0; i < max; i++) {
			if (bullets[i].used) {
				if (bullets[i].blast)
					putimagePNG(bullets[i].x, bullets[i].y, &imgBulletBlast[bullets[i].frameIndex]);
				else
					putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
	static void updateBullets() {
		static int count = 0;
		if (++count < 3) return;
		count = 0;
		int max = sizeof(bullets) / sizeof(bullets[0]);
		for (int i = 0; i < max; i++) {
			if (!bullets[i].used) continue;
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
				continue;
			}
			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) bullets[i].used = false;
			}
		}
	}
	static void checkBullet() {  //检测子弹是否击中僵尸
		int max = sizeof(bullets) / sizeof(bullets[0]);
		int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
		int bulletWidth = imgBulletNormal.getwidth();
		for (int i = 0; i < max; i++) {
			if (!bullets[i].used || bullets[i].blast) continue;
			int bulletRight = bullets[i].x + bulletWidth;
			for (int k = 0; k < zombieCount; k++) {
				if (!zombies[k]) continue;
				int zx_left = zombies[k]->getX() + 80;
				int zx_right = zombies[k]->getX() + 110;
				if (!zombies[k]->isDead() && bullets[i].row == zombies[k]->getRow()
					&& bulletRight > zx_left && bullets[i].x < zx_right) {
					zombies[k]->hurt(10);
					bullets[i].blast = true;
					bullets[i].speed = 0;
					break;
				}
			}
		}
	}

	void shoot() {
		if (++shootCd < 50) return;    // 原每3帧+1阈值35 → 每帧+1阈值105
		int dangerX = WIN_WIDTH - imgZombies[0].getwidth() / 2;
		bool hasZombie = false;   //检测本行是否有僵尸
		int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
		for (int i = 0; i < zombieCount; i++) {
			if (zombies[i] && !zombies[i]->isDead() && zombies[i]->getX() < dangerX && zombies[i]->getRow() == row) {
				hasZombie = true;
				break;
			}
		}
		if (!hasZombie) return;
		shootCd = 0;
		int max = sizeof(bullets) / sizeof(bullets[0]);
		for (int k = 0; k < max; k++) {
			if (!bullets[k].used) {
				bullets[k] = Bullet(x + imgZhiWu[type][0]->getwidth() - 10, y + 5, row);
				break;
			}
		}
	}
	void behaviour() override { shoot(); }   //检查标签，声明这个函数是要覆写基类的虚函数
};

// ---------------- 向日葵 ----------------
class Sunflower : public Plant {
	class SunshineBall {
	public:
		int x = 0, y = 0;         // 原代码遗留字段，暂未使用，先保留
		int frameIndex = 0;
		int destY = 0;
		bool used = false;
		int timer = 0;
		int status = 0;
		double xoff = 0, yoff = 0, t = 0, speed = 0;
		vector2 p1, p2, p3, p4, pCur;
	};
	inline static SunshineBall sunballs[40];   // 球池：天降+生产共用
	int timer;
public:
	Sunflower(int r, int c) : Plant(XIANG_RI_KUI, r, c), timer(0) {}

	static void resetPool() {
		int max = sizeof(sunballs) / sizeof(sunballs[0]);
		for (int i = 0; i < max; i++) sunballs[i] = SunshineBall();
	}
	static void drawSunshines() {
		int max = sizeof(sunballs) / sizeof(sunballs[0]);
		for (int i = 0; i < max; i++) {
			if (sunballs[i].used) {
				IMAGE* img = &imgSunshineBall[sunballs[i].frameIndex];
				putimagePNG((int)sunballs[i].pCur.x, (int)sunballs[i].pCur.y, img);
			}
		}
	}
	static void collectSunshine(ExMessage* msg) {
		int max = sizeof(sunballs) / sizeof(sunballs[0]);
		int w = imgSunshineBall[0].getwidth();
		int h = imgSunshineBall[0].getheight();
		for (int i = 0; i < max; i++) {
			if (!sunballs[i].used) continue;
			int x = (int)sunballs[i].pCur.x;
			int y = (int)sunballs[i].pCur.y;
			if (msg->x > x && msg->x < x + w && msg->y > y && msg->y < y + h) {
				sunballs[i].status = SUNSHINE_COLLET;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);
				sunballs[i].p1 = sunballs[i].pCur;
				sunballs[i].p4 = vector2(262, 0);
				sunballs[i].t = 0;
				double distance = dis(sunballs[i].p1 - sunballs[i].p4);
				sunballs[i].speed = 1.0 / (distance / 64);
				break;
			}
		}
	}
	static void updateSunshine() {
		int max = sizeof(sunballs) / sizeof(sunballs[0]);
		for (int i = 0; i < max; i++) {
			SunshineBall* sun = &sunballs[i];
			if (!sun->used) continue;
			sun->frameIndex = (sun->frameIndex + 1) % 29;
			if (sun->status == SUNSHINE_DOWN) {
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) { sun->status = SUNSHINE_GROUND; sun->timer = 0; }
			}
			else if (sun->status == SUNSHINE_GROUND) {
				sun->timer++;
				if (sun->timer > 100) { sun->used = false; sun->timer = 0; }
			}
			else if (sun->status == SUNSHINE_COLLET) {
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) { sun->used = false; sunshine += 25; }
			}
			else if (sun->status == SUNHSINE_PRODUCT) {
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) { sun->status = SUNSHINE_GROUND; sun->timer = 0; }
			}
		}
	}

	static void createSunshineFromSky() {   //天上掉下阳光球，随机位置
		static int count = 0;
		static int fre = 50;
		if (++count < fre) return;
		fre = 100 + rand() % 400;
		count = 0;
		int max = sizeof(sunballs) / sizeof(sunballs[0]);
		int i;
		for (i = 0; i < max && sunballs[i].used; i++);
		if (i >= max) return;
		sunballs[i].used = true;
		sunballs[i].frameIndex = 0;
		sunballs[i].timer = 0;
		sunballs[i].status = SUNSHINE_DOWN;
		sunballs[i].t = 0;
		sunballs[i].p1 = vector2(260 + rand() % (900 - 260), 60);
		sunballs[i].p4 = vector2(sunballs[i].p1.x, rand() % 4 * 90 + 200);
		double distance = sunballs[i].p4.y - sunballs[i].p1.y;
		sunballs[i].speed = 1.0 / (distance / 2);
	}

	void createSunshine() {   //向日葵生产阳光球
		if (++timer <= 200) return;
		timer = 0;
		int max = sizeof(sunballs) / sizeof(sunballs[0]);
		int k;
		for (k = 0; k < max && sunballs[k].used; k++);
		if (k >= max) return;
		sunballs[k].used = true;
		sunballs[k].p1 = vector2(x, y);
		int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);
		sunballs[k].p4 = vector2(x + w, y + imgZhiWu[XIANG_RI_KUI][0]->getheight()
			- imgSunshineBall[0].getheight());
		sunballs[k].p2 = vector2(sunballs[k].p1.x + w * 0.3, sunballs[k].p1.y - 100);
		sunballs[k].p3 = vector2(sunballs[k].p1.x + w * 0.7, sunballs[k].p1.y - 100);
		sunballs[k].status = SUNHSINE_PRODUCT;
		sunballs[k].speed = 0.05;
		sunballs[k].t = 0;
	}
	void behaviour() override { createSunshine(); }
	//以上的曲线采用贝塞尔曲线，p1为起点，p4为终点，p2、p3为控制点
};

// ---------------- 大嘴花 ----------------
class Chomper : public Plant {
	enum { IDLE, EATING, DIGEST } state;   // 待机吞咬 / 吞咬动画 / 消化
	int digestCd;                          // 消化计时（按游戏帧）
public:
	Chomper(int r, int c) : Plant(DA_ZUI_HUA, r, c), state(IDLE), digestCd(0) {
		y -= 30;
	}

	// 僵尸身体左边缘落在"本格右侧再往前50像素"内 → 可吞
	bool inRange(Zombie* z) {
		if (!z || z->isDead() || z->getRow() != row) return false;
		int zb = z->getX() + 80;
		return zb > x && zb < x + 81 + 50;
	}

	void behaviour() override {
		if (state == IDLE) {
			int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
			for (int i = 0; i < zombieCount; i++) {
				if (inRange(zombies[i])) {
					state = EATING;
					frameIndex = 0;
					break;
				}
			}
		}
		else if (state == DIGEST) {
			if (++digestCd >= 1000) {
				state = IDLE;
				digestCd = 0;
			}   // 15秒消化（约60游戏帧/秒）
		}
	}
	void nextFrame() override {
		if (state == IDLE) {
			Plant::nextFrame();
			return;
		}   // 空闲
		if (state == EATING) {
			frameIndex++;
			if (frameIndex >= 9) {            // 啃食动画放完, 僵尸直接消失（不播死亡动画）
				int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
				for (int i = 0; i < zombieCount; i++) {
					if (inRange(zombies[i])) {
						delete zombies[i];
						zombies[i] = nullptr;
						break;
					}
				}
				state = DIGEST;
				frameIndex = 0;
				digestCd = 0;
			}
		}
		// DIGEST 的动画帧由 digestCd 换算，不用推 frameIndex
	}
	void draw() override {
		if (state == IDLE) Plant::draw();
		else {
			// 吞咬/消化图比空闲图大，底部对齐、水平居中贴过去；
			int w0 = imgZhiWu[type][0]->getwidth(), h0 = imgZhiWu[type][0]->getheight();
			IMAGE* src = (state == EATING) ? &imgChomperEat[frameIndex] : &imgChomperDigest[(digestCd / 10) % 6];
			putimagePNG(x + (w0 - src->getwidth()) / 2, y + (h0 - src->getheight()), src);
		}
	}
};

// ---------------- 土豆地雷 ----------------
class PotatoMine : public Plant {
	enum { DOWN, ARMED, BLAST } state;   // 地下5秒 / 出土警戒 / 爆炸展示
	int downCd, blastCd;
public:
	PotatoMine(int r, int c) : Plant(TU_DOU_DI_LEI, r, c), state(DOWN), downCd(0), blastCd(0) {}

	// 地下期间僵尸当它不存在：不啃、不炸
	bool biteable() const override { return state != DOWN; }

	void behaviour() override {
		if (state == DOWN) {
			if (++downCd >= 330) { state = ARMED; downCd = 0; }   // 5秒出土
		}
		else if (state == ARMED) {
			int zombieCount = sizeof(zombies) / sizeof(zombies[0]);
			for (int i = 0; i < zombieCount; i++) {
				Zombie* z = zombies[i];
				if (!z || z->isDead() || z->getRow() != row) continue;
				int zb = z->getX() + 80;               // 僵尸身体左边缘
				if (zb >= x && zb <= x + 81) {         // 踩到土豆地雷这一格
					state = BLAST;
					blastCd = 0;
					for (int j = 0; j < zombieCount; j++) {   // 本格活僵尸全部直接消失
						Zombie* t = zombies[j];
						if (!t || t->isDead() || t->getRow() != row) continue;
						int tb = t->getX() + 80;
						if (tb >= x && tb <= x + 81) {
							delete t;
							zombies[j] = nullptr;
						}
					}
					break;
				}
			}
		}
		else {   // BLAST：爆炸图展示约0.3秒后自己退场（置血0，由updatePlant统一回收）
			if (++blastCd >= 20) blood = 0;
		}
	}
	void nextFrame() override {
		if (state == ARMED) Plant::nextFrame();   // 警戒期
	}
	void draw() override {
		if (state == DOWN)       putimagePNG(x + 3, y, &imgPotatoDown);     // 75宽对中，微调
		else if (state == BLAST) putimagePNG(x - 28, y - 19, &imgPotatoBlast);
		else                     Plant::draw();                              // 警戒用基类帧
	}
};

// ---------------- 坚果墙 ----------------
class WallNut : public Plant {
public:
	WallNut(int r, int c) : Plant(JIAN_GUO_QIANG, r, c, 750) {}  // 25伤害×30口=750
};

extern Plant* zhiwuMap[3][9];