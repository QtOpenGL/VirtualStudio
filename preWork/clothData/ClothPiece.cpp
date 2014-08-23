//#include "StdAfx.h"
#include "ClothPiece.h"
#include "ClothLoop.h"
#include "ClothPolyline.h"
#include "ClothLoop.h"
#include "ClothPiece.h"
#include "..\dxfele\DXFClothData.h"
#include "..\global/Global.h"

ClothPiece::ClothPiece(void)
	: worldPosX(-0.3f)
	, worldPosY(0.7f)
	, worldPosZ(0)
	, worldRotX(0)
	, worldRotY(0)
	, worldRotZ(0)
	, pieceIndex(-1)
	, posType(0)
	, moveDeltaX(0)
	, moveDeltaY(0)
	, rotateDelta(0)
	, scaleDelta(1)
	, radius(0)
{
	outLoop = new ClothLoop;
	centerX = 0;
	centerY = 0;
}


ClothPiece::~ClothPiece(void)
{
}

bool ClothPiece::insideThePos(float x, float y)
{
	int leftEdgeCount = 0;
	int rightEdgeCount = 0;
	for(int i = 0; i<outLoop->lines.size(); i++)
	{
		ClothPolyline* cPolyline = (ClothPolyline*)outLoop->lines.at(i);
		ClothVertex *cv1 = NULL;
		ClothVertex *cv2 = NULL;
		cv1 = cPolyline->points.at(0);
		float x1 = (cv1->x - Global::Instance()->globalCenterX) / Global::Instance()->scale;
		float y1 = (cv1->y - Global::Instance()->globalCenterY) / Global::Instance()->scale;
		float x2, y2;
		for(int j = 1; j<cPolyline->points.size(); j++)
		{
			cv2 = cPolyline->points.at(j);
			x2 = (cv2->x - Global::Instance()->globalCenterX) / Global::Instance()->scale;
			y2 = (cv2->y - Global::Instance()->globalCenterY) / Global::Instance()->scale;
			if((y1 - y) * (y2 - y) < 0)
			{
				float theX;
				if(y1 != y2) {
					theX= (y - y1) / (y2 - y1) * (x2 - x1) + x1;
					if(theX < x) {
						leftEdgeCount ++;
					} else if (theX > x) {
						rightEdgeCount++;
					}
				}
			}
			x1 = x2;
			y1=  y2;
		}
	}
	if((leftEdgeCount % 2) == 0 || (rightEdgeCount % 2) == 0) {
		return false;
	} else if((leftEdgeCount % 2) == 1 && (rightEdgeCount % 2) == 1) {
		return true;
	} else {
		return false;
	}
}

void ClothPiece::flipX()
{
	ClothLoop *newLoop = new ClothLoop;

	vector<ClothLine*>::reverse_iterator lineIt;
	for(lineIt = outLoop->lines.rbegin(); lineIt != outLoop->lines.rend(); lineIt++) {
		ClothPolyline* polyline = (ClothPolyline*) (*lineIt);
		ClothPolyline* newPolyline = new ClothPolyline;
		newPolyline->startVertex = polyline->endVertex;
		newPolyline->endVertex = polyline->startVertex;
		vector<ClothVertex*>::reverse_iterator it;
		for(it = polyline->points.rbegin(); it != polyline->points.rend(); it++) {
			if((*it) != newPolyline->endVertex) {
				(*it)->x = centerX * 2 - (*it)->x;	
			}
			newPolyline->points.push_back(*it);
		}
		newPolyline->lineIndex = newLoop->lines.size();
		newPolyline->fPiece = polyline->fPiece;
		newPolyline->isAdded = polyline->isAdded;
		newPolyline->sewLine = polyline->sewLine;
		newPolyline->isPolyline = polyline->isPolyline;
		//newPolyline->color = polyline->color;
		newLoop->lines.push_back(newPolyline);
	}
	delete outLoop;
	outLoop = newLoop;
}