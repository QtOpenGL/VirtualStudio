#ifndef __CLOTH_LINE_H__
#define __CLOTH_LINE_H__

//#include "resultData\ResultDataColor.h"

class ClothPiece;

class ClothLine
{
public:
	ClothLine(void);
	~ClothLine(void);
	ClothLine *sewLine;
	bool isPolyline;
	bool isAdded;
	ClothPiece *fPiece;
	int lineIndex;
	//ResultDataColor *color;
};

#endif