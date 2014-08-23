#ifndef __DXF_CLOTH_DATA_H__
#define __DXF_CLOTH_DATA_H__

#include <vector>
#include <string>
using namespace std;

class DXFClothPiece;

class DXFClothData
{
public:
	vector<DXFClothPiece*> clothPieces;
};

class ClothDataPolyLine;
class ClothDataLine;
class ClothVertex;
class ClothPoint;
class ClothNote;

class DXFClothPiece
{
public:
	vector<ClothNote*> clothNote;
	vector<ClothDataPolyLine*> polyLines;
	vector<ClothDataLine*> lines;
//	vector<ClothVertex*> vertexs;
	vector<ClothPoint*> points;
	float insertX;
	float insertY;
	float top;
	float bottom;
	float left;
	float right;
};

class ClothNote {
public :
	string text;
	float x;
	float y;
};

class ClothDataLine
{
public:
	ClothPoint *startPoint;
	ClothPoint *endPoint;
	float x1;
	float y1;
	float x2;
	float y2;
};

class ClothVertex
{
public:
	int index;
	float x;
	float y;
	float z;
	ClothVertex() {
		index = -1;
	}
	ClothVertex(float x, float y) {
		index = -1;
		this->x = x;
		this->y = y;
	}
};

class ClothPoint
{
public:
	float x;
	float y;
	float z;
	int index;
};

class ClothDataPolyLine
{
public:
	vector<ClothVertex*> vectexs;
	bool containsThePos(float x, float y) {
		for(int i = 0; i<vectexs.size(); i++) {
			if(((vectexs.at(i)->x - x) * (vectexs.at(i)->x - x) + (vectexs.at(i)->y - y) * (vectexs.at(i)->y - y)) < 5) {
				return true;
			}
		}
		return false;
	};
	int flag;
};

#endif