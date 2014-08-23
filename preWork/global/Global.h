#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <vector>
#include <map>
//#include "preEdit\drawEdit\DrawModeInterface.h"
//#include "preEdit\relateEdit\RelateModeInterface.h"
//#include "preEdit\sewEdit\SewModeInterface.h"
//#include "GLEnabledView.h"
using namespace std;

class ClothData;
class ClothPatch;
class LeftMenuBar;
class DXFClothData;
class ClothPiece;

enum {
	DrawMode,
	RelateMode,
	SewMode
};

enum {
	DrawContour, 
	DrawCenterPoint, 
	DrawMove,
	DrawScale, 
	DrawRotate, 
	DrawDelete, 
	DrawDiv
};

class Global
{
public:
	static Global* Instance();
	void initGlobal();
	void removeThePatch(ClothPatch* patch);
protected:
	static Global *_global;
public:
	LeftMenuBar *leftMenuBar;
	ClothPatch *currSelectedPatch;
	ClothPiece *currSelectedPiece;

	ClothData* clothData;
	DXFClothData* dxfClothData;
	/*DrawModeInterface *drawModeInstance;
	RelateModeInterface *relateModeInstance;
	SewModeInterface *sewModeInstance;
	CGLEnabledView *designView;
	CGLEnabledView *previewView;*/
	vector<ClothPatch*> clothPatchs;
	vector<ClothPiece*> clothPieces;
	float scale;
	float globalCenterX;
	float globalCenterY;

	/*aiVector3D *boundingBoxMin;
	aiVector3D *boundingBoxMax;*/

	int designMode;
	int drawDesignOp;
	bool isPreWorkFinished;
	bool hasFinishLoadObj;

	//CMFCRibbonBar *ribbonBar;
};

#endif