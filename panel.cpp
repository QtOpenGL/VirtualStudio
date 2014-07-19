#include <QtGui>
#include <QtWidgets/QtWidgets>

#include "panel.h"

/************************************************************************/
/* ·ìºÏÏß                                                               */
/************************************************************************/
SeamLine::SeamLine( Panel *startItem, Panel *endItem, QMenu *contextMenu, QGraphicsItem *parent /*= 0*/, QGraphicsScene *scene /*= 0*/ )
	: start_item_(startItem), end_item_(endItem), context_menu_(contextMenu), color_(Qt::green)
{
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setPen(QPen(color_, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

void SeamLine::updatePosition()
{
	QLineF line(mapFromItem(start_item_, 0, 0), mapFromItem(end_item_, 0, 0));
	setLine(line);
}

void SeamLine::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
	scene()->clearSelection();
	setSelected(true);
	context_menu_->exec(event->screenPos());
}

void SeamLine::setColor( const QColor &color )
{
	color_ = color;
	setPen(QPen(color_, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

Panel::Panel( QMenu *contextMenu, QGraphicsItem *parent /*= 0*/, QGraphicsScene *scene /*= 0*/ )
	: context_menu_(contextMenu)
{
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

/************************************************************************/
/* ÒÂÆ¬                                                                 */
/************************************************************************/
void Panel::removeSeamLine( SeamLine *seamline )
{
	int index = seam_lines_.indexOf(seamline);

	if (index != -1)
		seam_lines_.removeAt(index);
}

void Panel::removeSeamLines()
{
	foreach (SeamLine *seamline, seam_lines_)
	{
		seamline->startItem()->removeSeamLine(seamline);
		seamline->endItem()->removeSeamLine(seamline);
		scene()->removeItem(seamline);
		delete seamline;
	}
}

void Panel::addSeamLine( SeamLine *seamline )
{
	seam_lines_.append(seamline);
}

void Panel::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
	scene()->clearSelection();
	setSelected(true);
	context_menu_->exec(event->screenPos());
}

QVariant Panel::itemChange( GraphicsItemChange change, const QVariant &value )
{
	if (change == QGraphicsItem::ItemPositionChange)
	{
		foreach (SeamLine *seamline, seam_lines_)
		{
			seamline->updatePosition();
		}
	}

	return value;
}
