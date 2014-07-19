#include <QtGui>

//#include "dl_dxf.h"

#include "pattern.h"
#include "panel.h"
#include "preWork\dxfele\DXFClothData.h"
#include "preWork\dxfele\MyDxfFilter.h"
#include "preWork\dxf\dl_dxf.h"
#include "preWork\global\Global.h"

#include <iostream>
#include <cstdio>
#include <QMessageBox>
#include <cfloat>
/************************************************************************/
/* DXF文件导入器                                                         */
/************************************************************************/
//void DXFImpoter::addLayer( const DL_LayerData& data )
//{
//	printf("LAYER: %s flags: %d\n", data.name.c_str(), data.flags);
//	printAttributes();
//}
//
//void DXFImpoter::addPoint( const DL_PointData& data )
//{
//	printf("POINT    (%6.3f, %6.3f, %6.3f)\n", data.x, data.y, data.z);
//	printAttributes();
//}
//
//void DXFImpoter::addLine( const DL_LineData& data )
//{
//	printf("LINE     (%6.3f, %6.3f, %6.3f) (%6.3f, %6.3f, %6.3f)\n",
//		data.x1, data.y1, data.z1, data.x2, data.y2, data.z2);
//	printAttributes();
//}
//
//void DXFImpoter::addArc( const DL_ArcData& data )
//{
//	printf("ARC      (%6.3f, %6.3f, %6.3f) %6.3f, %6.3f, %6.3f\n",
//		data.cx, data.cy, data.cz,
//		data.radius, data.angle1, data.angle2);
//	printAttributes();
//}
//
//void DXFImpoter::addCircle( const DL_CircleData& data )
//{
//	printf("CIRCLE   (%6.3f, %6.3f, %6.3f) %6.3f\n",
//		data.cx, data.cy, data.cz,
//		data.radius);
//	printAttributes();
//}
//
//void DXFImpoter::addPolyline( const DL_PolylineData& data )
//{
//	printf("POLYLINE \n");
//	printf("flags: %d\n", (int)data.flags);
//	printAttributes();
//}
//
//void DXFImpoter::addVertex( const DL_VertexData& data )
//{
//	printf("VERTEX   (%6.3f, %6.3f, %6.3f) %6.3f\n",
//		data.x, data.y, data.z,
//		data.bulge);
//	printAttributes();
//}
//
//void DXFImpoter::add3dFace( const DL_3dFaceData& data )
//{
//	printf("3DFACE\n");
//	for (int i=0; i<4; i++) {
//		printf("   corner %d: %6.3f %6.3f %6.3f\n", 
//			i, data.x[i], data.y[i], data.z[i]);
//	}
//	printAttributes();
//}
//
//
//void DXFImpoter::printAttributes()
//{
//	printf("  Attributes: Layer: %s, ", attributes.getLayer().c_str());
//	printf(" Color: ");
//	if (attributes.getColor()==256)	{
//		printf("BYLAYER");
//	} else if (attributes.getColor()==0) {
//		printf("BYBLOCK");
//	} else {
//		printf("%d", attributes.getColor());
//	}
//	printf(" Width: ");
//	if (attributes.getWidth()==-1) {
//		printf("BYLAYER");
//	} else if (attributes.getWidth()==-2) {
//		printf("BYBLOCK");
//	} else if (attributes.getWidth()==-3) {
//		printf("DEFAULT");
//	} else {
//		printf("%d", attributes.getWidth());
//	}
//	printf(" Type: %s\n", attributes.getLineType().c_str());
//}
/************************************************************************/
/* 服装打板场景                                                          */
/************************************************************************/
PatternScene::PatternScene(/* QMenu *panelMenu, QMenu *seamlineMenu, */QObject *parent /*= 0*/ )
	: //panel_menu_(panelMenu), 
	//seamline_menu_(seamlineMenu), 
	mode_(MOVE_PATTERN),
	panel_color_(Qt::darkRed),
	seamline_color_(Qt::green),
	grid_visible_(true),
	//dxf_importer_(new DXFImpoter()),
	dxf_file_(nullptr)
{
}

PatternScene::~PatternScene()
{
	//delete dxf_importer_;
	delete dxf_file_;
}

void PatternScene::setPanelColor( const QColor &color )
{
	panel_color_ = color;;
	if (isItemChange(Panel::Type))
	{
		Panel *item = qgraphicsitem_cast<Panel *>(selectedItems().first());
		item->setBrush(panel_color_);
	}
}

void PatternScene::setSeamlineColor( const QColor &color )
{
	seamline_color_ = color;;
	if (isItemChange(SeamLine::Type))
	{
		SeamLine *item = qgraphicsitem_cast<SeamLine *>(selectedItems().first());
		item->setColor(seamline_color_);
		update();
	}
}

void PatternScene::setMode( Mode mode )
{
	mode_ = mode;
}

void PatternScene::mousePressEvent( QGraphicsSceneMouseEvent *mouseEvent )
{
	QGraphicsScene::mousePressEvent(mouseEvent);
}

void PatternScene::mouseMoveEvent( QGraphicsSceneMouseEvent *mouseEvent )
{
	QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void PatternScene::mouseReleaseEvent( QGraphicsSceneMouseEvent *mouseEvent )
{
	QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void PatternScene::drawBackground( QPainter *painter, const QRectF &rect )
{
	// 绘制网格
	if (grid_visible_) {
		const int grid_size = 20;
		const int MaxX = static_cast<int>(qCeil(width()) / grid_size) * grid_size;
		const int MaxY = static_cast<int>(qCeil(height()) / grid_size) * grid_size;
		painter->setPen(palette().foreground().color());
		for (int x = 0; x <= MaxX; x += grid_size) {
			painter->drawLine(x, 0, x, height());
		}
		for (int y = 0; y <= MaxY; y += grid_size) {
			painter->drawLine(0, y, width(), y);
		}
	}
}

void PatternScene::drawForeground(QPainter *painter, const QRectF &rect)
{
	painter->setPen(palette().foreground().color());
	DXFClothData * dxfcd = Global::Instance()->dxfClothData;
	if(dxfcd)
	{
		float xmin = FLT_MAX, ymin = FLT_MAX, xmax = FLT_MIN, ymax = FLT_MIN;
		for(auto dxfcp : dxfcd->clothPieces)
		{
			for(auto dxfcdpl : dxfcp->polyLines)
			{
				for(auto cv : dxfcdpl->vectexs)
				{
					xmin = min(cv->x, xmin);
					ymin = min(cv->y, ymin);
					xmax = max(cv->x, xmax);
					ymax = max(cv->y, ymax);
				}
			}
		}
		float xf = width() / (xmax - xmin), yf = height() / (ymax - ymin);

		for(auto dxfcp : dxfcd->clothPieces)
		{
			for(auto dxfcdl : dxfcp->lines)
			{
				painter->drawLine(
					(dxfcdl->x1 - xmin) * xf, 
					(dxfcdl->y1 - ymin) * yf, 
					(dxfcdl->x2 - xmin) * xf, 
					(dxfcdl->y2 - ymin) * yf
					);
			}

			for(auto dxfcdpl : dxfcp->polyLines)
			{
				ClothVertex * pcv = dxfcdpl->vectexs[0];
				for(auto cv : dxfcdpl->vectexs)
				{
					painter->drawLine(
						(cv->x - xmin) * xf, 
						(cv->y - ymin) * yf, 
						(pcv->x - xmin) * xf, 
						(pcv->y - ymin) * yf
						);
					pcv = cv;
				}
				pcv = dxfcdpl->vectexs[0];
				ClothVertex * pcv1 = dxfcdpl->vectexs[dxfcdpl->vectexs.size() - 1];
				painter->drawLine(
						(pcv->x - xmin) * xf, 
						(pcv->y - ymin) * yf, 
						(pcv1->x - xmin) * xf, 
						(pcv1->y - ymin) * yf
						);
			}
		}
	}
}

bool PatternScene::isItemChange( int type )
{
	foreach (QGraphicsItem *item, selectedItems()) {
		if (item->type() == type)
			return true;
	}
	return false;
}

void PatternScene::importPattern( const QString& filename )
{
	delete dxf_file_;
	Global::Instance()->dxfClothData = new DXFClothData;
	dxf_file_ = new DL_Dxf();
	MyDxfFilter * myDxfFilter = new MyDxfFilter();
	if (!dxf_file_->in(filename.toStdString(), myDxfFilter)) { // if file open failed
		QMessageBox::critical(0, "error", filename + " could not be opened.");
		//std::cerr << filename.toStdString() << " could not be opened.\n";
		return;
	}
	/*Global::Instance()->dxfClothData = new DXFClothData;
	DL_Dxf *dxf = new DL_Dxf();
	MyDxfFilter *myDxfFilter = new MyDxfFilter();
	dxf->in("1.dxf", myDxfFilter);*/
}


/************************************************************************/
/* 服装打板视图                                                          */
/************************************************************************/

PatternView::PatternView( QWidget *parent /*= 0*/ ) : QGraphicsView(parent)
{
	setDragMode(RubberBandDrag);
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
}

void PatternView::wheelEvent( QWheelEvent *event )
{
	scaleBy(qPow(4.0 / 3.0, -event->delta() / 240.0));
}

void PatternView::scaleBy( double factor )
{
	scale(factor, factor);
}

