#ifndef PANEL_H
#define PANEL_H

#include <QGraphicsPathItem>
#include <QList>
#include "cad2d.h"

class QGraphicsItem;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QStyleOptionGraphicsItem;

class Panel;
/************************************************************************/
/* ·ìºÏÏß                                                               */
/************************************************************************/
class SeamLine : public QGraphicsLineItem
{
public:
	enum { Type = UserType + 2 };

	SeamLine(Panel *startItem, Panel *endItem, QMenu *contextMenu, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

	QColor color() const { return color_; }
	void setColor(const QColor &color);
	Panel * startItem() const { return start_item_; }	
	Panel * endItem() const { return end_item_; }

	void updatePosition();

protected:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
	Panel*  start_item_;
	Panel*  end_item_;
	QMenu*  context_menu_;
	QColor  color_;
};

/************************************************************************/
/* ÒÂÆ¬                                                                 */
/************************************************************************/
class Panel : public QGraphicsPathItem
{
public:
	enum { Type = UserType + 1 };

	Panel(QMenu *contextMenu, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

	void removeSeamLine(SeamLine *seamline);
	void removeSeamLines();
	void addSeamLine(SeamLine *seamline);

	void setToolTip(const QString &toolTip) { tool_tip_ = toolTip; }
	void setColor(const QColor &color) { color_ = color; }

	QColor color() const { return color_; }
	QString toolTip() const { return tool_tip_; }

protected:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QColor  color_;
	QString tool_tip_;
	QMenu*  context_menu_;
	QList<SeamLine*> seam_lines_;
};
#endif // PANELITEM_H
