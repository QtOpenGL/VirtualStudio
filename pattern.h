#ifndef PATTERNSCENE_H
#define PATTERNSCENE_H
 
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWheelEvent>
 
// dxflib headers
//#include <dl_creationadapter.h>
 
 class QGraphicsSceneMouseEvent;
 class QMenu;
 class QPointF;
 class QGraphicsLineItem;
 class QColor;
 class Panel;
 class SeamLine;
 class DL_Dxf;
 /************************************************************************/
 /* DXF文件导入器                                                         */
 /************************************************************************/
 //class DXFImpoter : public DL_CreationAdapter
 //{
 //public:
	//DXFImpoter() {}
	// virtual void addLayer(const DL_LayerData& data);
	// virtual void addPoint(const DL_PointData& data);
	// virtual void addLine(const DL_LineData& data);
	// virtual void addArc(const DL_ArcData& data);
	// virtual void addCircle(const DL_CircleData& data);
	// virtual void addPolyline(const DL_PolylineData& data);
	// virtual void addVertex(const DL_VertexData& data);
	// virtual void add3dFace(const DL_3dFaceData& data);
 //
	// void printAttributes();
 //};
 
 
 /************************************************************************/
 /* 服装打板场景                                                          */
 /************************************************************************/
 class PatternScene : public QGraphicsScene
 {
	Q_OBJECT
 
 public:
	enum Mode { ADD_PATTERN, ADD_SEAMLINE, MOVE_PATTERN };
 
	PatternScene(/*QMenu *panelMenu, QMenu *seamlineMenu, */QObject *parent = 0);
	 ~PatternScene();
 
	void importPattern(const QString& filename);	// 导入服装打板
		
	bool gridVisible() const { return grid_visible_; }
	QColor panelColor() const { return panel_color_; }
	QColor seamlineColor() const { return seamline_color_; }
	void setPanelColor(const QColor &color);
	void setSeamlineColor(const QColor &color);
	void setGridVisible(bool val) { grid_visible_ = val; }
 
	void setMode(Mode mode);
 
 signals:
	void panelAdded(Panel *item);
 
 protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
	void drawBackground( QPainter *painter, const QRectF &rect );
	void drawForeground(QPainter *painter, const QRectF &rect);
 
 private:
	bool isItemChange(int type);
 
	QMenu*	panel_menu_;
	QMenu*	seamline_menu_;
	Mode	mode_;
	QColor	panel_color_;
	QColor	seamline_color_;
	bool	grid_visible_;
 
	QList<Panel*>	panels_;	    // 衣片
	 //DXFImpoter*     dxf_importer_;  // DXF导入器
	 DL_Dxf*         dxf_file_;      // DXF文件
 };
 
 /************************************************************************/
 /* 服装打板视图                                                          */
 /************************************************************************/
 class PatternView : public QGraphicsView
 {
 public:
	explicit PatternView(QWidget *parent = 0);
 
	void zoomIn() { scaleBy(1.1); }
	void zoomOut() { scaleBy(1.0/ 1.1); }
 
 protected:
	void wheelEvent(QWheelEvent *event);
 
 private:
	void scaleBy(double factor);
 };
 #endif // PATTERNSCENE_H
