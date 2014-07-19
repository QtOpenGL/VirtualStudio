#ifndef __CLOTH_STRAIGHT_LINE_H__
#define __CLOTH_STRAIGHT_LINE_H__

#include "ClothLine.h"

class ClothVertex;

class ClothStraightLine: public ClothLine
{
public:
	ClothStraightLine(void);
	~ClothStraightLine(void);
	ClothVertex* startPoint;
	ClothVertex* endPoint;
};

#endif