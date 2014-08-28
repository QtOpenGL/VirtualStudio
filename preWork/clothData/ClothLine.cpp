#include "cmheader.h"
#include "ClothLine.h"


ClothLine::ClothLine(void)
{
	isAdded = false;
	fPiece = nullptr;
	lineIndex = -1;
	//color = new ResultDataColor();
	sewLine = nullptr;
}


ClothLine::~ClothLine(void)
{
}
