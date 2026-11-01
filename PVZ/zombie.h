#pragma once
#include "game.h"

class Zombie {
protected:
	int x, y, row;     //图片基准坐标，所在行 0~2
	int frameIndex;
	int speed;         // 当前速度（啃植物时为 0）
	int walkSpeed;      // 正常行走速度，啃完要恢复它
	int blood;
	int eatTimer;
	bool dead, eating;
public:
	Zombie(int _row, int _x, int _y, int _blood, int _speed)
		: row(_row), x(_x), y(_y), blood(_blood),
		speed(_speed), walkSpeed(_speed),
		frameIndex(0), dead(false), eating(false), eatTimer(0) {
	}
	virtual ~Zombie() {}

	void move();                 // 位移
	void nextFrame();            // 动画帧推进
	void draw();                 // 画自己
	void hurt(int d);            // 被子弹打
	void startEat();             // 开始啃植物
	void finishEat();            // 植物没了，恢复行走
	int  getX()   const { return x; }
	int  getRow() const { return row; }
	bool isDead() const { return dead; }
	bool isDeadEnd() const { return dead && frameIndex >= 20; }
	bool isEating() const { return eating; }

	bool biteTick() {
		if (++eatTimer >= 30) {
			eatTimer = 0;
			return true;
		}
		return false;
	}

	virtual int walkFrameCount() const { return 22; }  // 虚函数，子类若换图集可覆盖
	virtual int eatFrameCount() const { return 21; }        // 啃咬图集帧数
	virtual IMAGE* walkImage() const { return &imgZombies[frameIndex]; }
	virtual IMAGE* eatImage()  const { return &imgZombieEat[frameIndex]; }
};
class NormalZombie : public Zombie {
public:
	NormalZombie(int r)
		: Zombie(r, WIN_WIDTH, 172 + (1 + r) * 100, 100, 1) {
	}
};

// ---------------- 路障僵尸：200血，掉到100脱盔变普通动画 ----------------
class ConeheadZombie : public Zombie {
	inline static IMAGE imgWalk[21];      // res/zm_cone
	inline static IMAGE imgEat[11];       // res/zm_cone_eat
	bool lostArmor() const { return blood <= 100; }
public:
	ConeheadZombie(int r) : Zombie(r, WIN_WIDTH, 172 + (1 + r) * 100, 200, 1) {}

	static void loadRes() {
		char name[64];
		for (int i = 0; i < 21; i++) {
			sprintf_s(name, sizeof(name), "res/zm_cone/%d.png", i + 1);
			loadimage(&imgWalk[i], name);
		}
		for (int i = 0; i < 11; i++) {
			sprintf_s(name, sizeof(name), "res/zm_cone_eat/%d.png", i + 1);
			loadimage(&imgEat[i], name);
		}
	}
	int walkFrameCount() const override { return lostArmor() ? 22 : 21; }
	int eatFrameCount()  const override { return lostArmor() ? 21 : 11; }
	IMAGE* walkImage() const override { return lostArmor() ? Zombie::walkImage() : &imgWalk[frameIndex]; }
	IMAGE* eatImage()  const override { return lostArmor() ? Zombie::eatImage() : &imgEat[frameIndex]; }
};

// ---------------- 铁桶僵尸：300血，掉到100掉桶变普通动画 ----------------
class BucketheadZombie : public Zombie {
	inline static IMAGE imgWalk[15];      // res/zm_bucket
	inline static IMAGE imgEat[11];       // res/zm_bucket_eat
	bool lostArmor() const { return blood <= 100; }
public:
	BucketheadZombie(int r) : Zombie(r, WIN_WIDTH, 172 + (1 + r) * 100, 300, 1) {}

	static void loadRes() {
		char name[64];
		for (int i = 0; i < 15; i++) {
			sprintf_s(name, sizeof(name), "res/zm_bucket/%d.png", i + 1);
			loadimage(&imgWalk[i], name);
		}
		for (int i = 0; i < 11; i++) {
			sprintf_s(name, sizeof(name), "res/zm_bucket_eat/%d.png", i + 1);
			loadimage(&imgEat[i], name);
		}
	}
	int walkFrameCount() const override { return lostArmor() ? 22 : 15; }
	int eatFrameCount()  const override { return lostArmor() ? 21 : 11; }
	IMAGE* walkImage() const override { return lostArmor() ? Zombie::walkImage() : &imgWalk[frameIndex]; }
	IMAGE* eatImage()  const override { return lostArmor() ? Zombie::eatImage() : &imgEat[frameIndex]; }
};

void Zombie::move() {
	x -= speed;
}

void Zombie::nextFrame() {
	if (dead) {
		frameIndex++;
	}
	else if (eating) {
		frameIndex = (frameIndex + 1) % eatFrameCount();
	}
	else {
		frameIndex = (frameIndex + 1) % walkFrameCount();
	}
}

void Zombie::draw() {
	IMAGE* img;
	if (dead)        img = &imgZombieDead[frameIndex];
	else if (eating) img = eatImage();
	else             img = walkImage();
	putimagePNG(x, y - img->getheight(), img);
}

void Zombie::hurt(int d) {
	blood -= d;
	if (blood <= 0) {
		dead = true;
		eating = false;
		speed = 0;
		frameIndex = 0;
	}
}


void Zombie::startEat() {
	eating = true;
	speed = 0;
	frameIndex = 0;
	eatTimer = 0;
}

void Zombie::finishEat() {
	eating = false;
	speed = walkSpeed;
	frameIndex = 0;
}

extern Zombie* zombies[20];
