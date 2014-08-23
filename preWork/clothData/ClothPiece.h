#ifndef __CLOTH_PIECE_H__
#define __CLOTH_PIECE_H__

#include <vector>
using namespace std;
class ClothLoop;

enum {
	POS_NONE, 
	POS_X,
	POS_Y,
	POS_Z
};

class ClothPiece
{
public:
	ClothLoop *outLoop;
	vector<ClothLoop*> interLoops;
	bool insideThePos(float x, float y);
	void flipX();
	ClothPiece(void);
	~ClothPiece(void);
	int pieceIndex;
	float left;
	float right;
	float top;
	float bottom;
	float originalLeft;
	float originalRight;
	float originalTop;
	float originalBottom;
	float scale;
	float centerX;
	float centerY;
	float worldPosX;
	float worldPosY;
	float worldPosZ;
	float worldRotX;
	float worldRotY;
	float worldRotZ;
	float moveDeltaX;
	float moveDeltaY;
	float rotateDelta;
	float scaleDelta;
	float radius;
	int posType;
};
#endif