#ifndef BALL_H
#define BALL_H

#include"Phybal.h"

#ifdef EXTL_GLOBAL //헤더에 전역변수를 선언하기 위한 트릭
#define EXTL
#else
#define EXTL extern
#endif

class CBall{
public:
	double x;
	double y;
	double vx;
	double vy;

	double r_x;
	double r_y;
	double r_vx;
	double r_vy;
	CBall(double _x=0.0, double _y=0.0, double _vx=0.0, double _vy=0.0){
		r_x  =  x = _x;
		r_y  =  y = _y;
		r_vx = vx = _vx;
		r_vy = vy = _vy;
	}

	void reset();
};

extern CBall* Balls[1000];
extern int B_Count;

#endif