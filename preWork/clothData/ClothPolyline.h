#ifndef __CLOTH_POLYLINE_H__
#define __CLOTH_POLYLINE_H__

#include <vector>
#include "ClothLine.h"
using namespace std;

class ClothVertex;

class ClothPolyline: public ClothLine
{
public:
	ClothPolyline(void);
	~ClothPolyline(void);
	vector<ClothVertex*> points;
	ClothVertex *startVertex;
	ClothVertex *endVertex;
};

#endif