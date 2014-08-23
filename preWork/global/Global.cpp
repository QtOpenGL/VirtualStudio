//#include "StdAfx.h"
#include "Global.h"


Global* Global::_global = 0;

Global* Global::Instance()
{
	if(_global == 0)
	{
		_global = new Global;
		_global->initGlobal();
	}
	return _global;
}

void Global::initGlobal()
{
	clothData = NULL;
	dxfClothData = NULL;
	/*drawModeInstance = NULL;
	relateModeInstance = NULL;
	sewModeInstance = NULL;*/
	currSelectedPatch = NULL;
	currSelectedPiece = NULL;
	isPreWorkFinished = false;
	hasFinishLoadObj = false;
	designMode = DrawMode;
	drawDesignOp = DrawContour;
}

void Global::removeThePatch(ClothPatch* patch)
{
	vector<ClothPatch*>::iterator it;
	for(it = clothPatchs.begin(); it != clothPatchs.end(); it++)
	{
		if((*it) == patch)
		{
			clothPatchs.erase(it);
			return;
		}
	}
}
