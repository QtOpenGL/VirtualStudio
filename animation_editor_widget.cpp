#include "animation_editor_widget.h"

#include <QtGui>
#include <QGraphicsSceneDragDropEvent>
#include <QApplication>
#include <QtWidgets/QtWidgets>

#include <qcustomplot.h>
#include <fstream>
#include <cassert>

/************************************************************************/
/* 动画轨道场景                                                          */
/************************************************************************/
AnimationTrackScene::AnimationTrackScene( QObject *parent /*= 0*/ )
	: QGraphicsScene(parent), start_frame_(0), current_frame_(0), end_frame_(0), name_animation_(nullptr)
{
	setItemIndexMethod(QGraphicsScene::NoIndex);
	current_frame_pen_ = new QPen(QColor("orchid"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	highlight_pen_ = new QPen(QColor("lightgray"), 2, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
	end_frame_pen_ = new QPen(QColor("gold"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

void AnimationTrackScene::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
	if (event->mimeData()->hasFormat("application/x-animation")) {
		highlight_pen_->setColor(Qt::red);
		update();
		event->accept();
	}
	else {
		event->ignore();
	}
}

void AnimationTrackScene::dragLeaveEvent( QGraphicsSceneDragDropEvent *event )
{
	highlight_pen_->setColor(QColor("lightgray"));
	update();
	event->accept();
}

void AnimationTrackScene::dragMoveEvent( QGraphicsSceneDragDropEvent *event )
{
	if (event->mimeData()->hasFormat("application/x-animation") && event->scenePos().x() >= 0 && event->scenePos().y() >= 0 ) {
		// 更新动画片段拖放位置提示线
		drop_pos_ = event->scenePos();
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
	else {
		event->ignore();
	}
	update();
}

void AnimationTrackScene::dropEvent( QGraphicsSceneDragDropEvent *event )
{
	if (event->mimeData()->hasFormat("application/x-animation")) {
		QByteArray clipData = event->mimeData()->data("application/x-animation");
		QDataStream stream(&clipData, QIODevice::ReadOnly);
		QString text;
		double duration;
		stream >> text >> duration;
		// 根据动画片段名称查询片断对应的原始动画
		Animation* anim = findAnimation(text);
		Q_ASSERT(anim);	
		int y = qMin(static_cast<int>(drop_pos_.y() / TRACK_HEIGHT), tracks_.size() - 1) * TRACK_HEIGHT  + TRACK_HEIGHT / 2; 
		AnimationTrack* track = findTrackByYPos(y); // 根据y坐标放置到不同的轨道
		// 添加新的动画片段
		AnimationClip* clip = new AnimationClip(this, anim, track);

		int x = drop_pos_.x() + clip->width() / 2;
		clip->setPos(x, y);
		// 用空闲区间分配算法调整重叠区间
		arrangeClips(/*clip, track*/);
		connect(clip, SIGNAL(clipMoved(AnimationClip*, AnimationTrack*)), this, SIGNAL(clipUpdated(AnimationClip*, AnimationTrack*)));	// OK

		qreal end = x + clip->width() / 2;
		int frame = /*qCeil(end / 3)*/clip->width() * 10.f / RemixerWidget::getSampleInterval();			  // 每一帧3像素
		end_frame_ = qMax(end_frame_, frame); // 更新end_frame和场景大小
		adjustSceneWidth(end);

		emit clipUpdated(clip, track);	// 通知其他控件更新 并更新合成动画

		highlight_pen_->setColor(QColor("lightgray"));
		update();

		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
	else {
		event->ignore();
	}
}

void AnimationTrackScene::drawBackground( QPainter *painter, const QRectF &rect )
{
	Q_UNUSED(rect);
	// 绘制拖放点
	painter->setPen(*highlight_pen_);
	int y1 = qMin(static_cast<int>(drop_pos_.y() / TRACK_HEIGHT), tracks_.size() - 1)  * TRACK_HEIGHT; 
	painter->drawLine(drop_pos_.x(), y1, drop_pos_.x(), y1 + TRACK_HEIGHT);
	
	// 绘制时间线
	painter->setPen(*current_frame_pen_);
	painter->setBrush(current_frame_pen_->color());
	painter->drawRect(current_frame_*3, 0, 3, TRACK_HEIGHT * (tracks_.size() + 1)); // SECOND_WIDTH = 100 FRAME_TIME = 33 一秒钟对应100像素 一帧长达33毫秒 因此一帧对应3像素

	// 结束时间线
	painter->setPen(*end_frame_pen_);
	painter->setBrush(end_frame_pen_->color());
	painter->drawRect(end_frame_*3, 0, 3, TRACK_HEIGHT * (tracks_.size() + 1));

	// 绘制轨道线
	painter->setPen(palette().foreground().color());
	for (int j = 1; j <= tracks_.size(); ++j) {
		painter->drawLine(0, TRACK_HEIGHT * j, width(), TRACK_HEIGHT * j);
	}
}

void AnimationTrackScene::adjustSceneHeight()
{
	setSceneRect(0, 0, width(), qMax(static_cast<int>(INITIAL_HEIGHT), tracks_.size() * TRACK_HEIGHT) + 30);
}

void AnimationTrackScene::adjustSceneWidth( qreal end )
{
	int padding = 30;
	if (end > width()) {
		setSceneRect(0, 0, end + padding, height());
	}	
}

Animation* AnimationTrackScene::findAnimation(const QString& name)
{
	auto it = name_animation_->find(name);
	Q_ASSERT(it != name_animation_->end());
	return it->second;
}

AnimationTrack* AnimationTrackScene::findTrackByYPos( qreal y )
{
	int i = y / TRACK_HEIGHT;
	Q_ASSERT(i >= 0 && i < tracks_.size());
	return &tracks_[i];
}

// TODO
void AnimationTrackScene::arrangeClips(/*AnimationClip* clip, AnimationTrack* track*/)
{
// #ifdef _DEBUG
// 	qDebug() << "normalize"; // success!
// #endif
}

void AnimationTrackScene::updateSyntheticAnim(Animation*& syn_anim)
{
	delete syn_anim;
	foreach(AnimationTrack track, tracks_) {
		if (!track.isEmpty()) {

			Animation* anim = track.clips_[0]->animation();
			syn_anim = new Animation(anim/*, track.clips_[0]->startTime()*/, track.clips_[0]->weight());
			// 将轨道上其余的clip添加到合成动画中
			for (int clip_index = 1; clip_index < track.clips_.size(); ++clip_index) {
				AnimationClip* clip = track.clips_[clip_index];
				syn_anim->addKeyframes(clip->channels(), clip->startTime(), clip->length(), clip->weight()); // 注意更新 duration
			}
		}
	}
}

/************************************************************************/
/* 动画轨道视图                                                          */
/************************************************************************/
AnimationTrackView::AnimationTrackView( AnimationTrackScene* scene, QWidget *parent /*= 0*/ )
	: scene_(scene), QGraphicsView(parent), zoom_factor_(1.0)
{
	setScene(scene);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setAcceptDrops(true);
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	setStyleSheet("background: lightgray;");
}

void AnimationTrackView::wheelEvent( QWheelEvent *event )
{
	// TODO: 调整缩放比例
	if ( QApplication::keyboardModifiers() & Qt::ControlModifier)
	{
		double numDegrees = -event->delta() / 8.0;
		double numSteps = numDegrees / 15.0;
		zoom_factor_ = qMin(std::pow(1.125, numSteps), 32.0);
		zoom_factor_ = qMax(zoom_factor_, 0.5);
		//scale(zoom_factor_, 1.0);
		//setTransform(QTransform(zoom_factor_, 0, 0, 0, 1.0, 0, 0, 0, 1.0));
	}
}

void AnimationTrackView::setCurrentFrame( int frame )
{
	scene_->setCurrentFrame(frame);
	update();
}

/************************************************************************/
/* 动画轨道表格模型                                                      */
/************************************************************************/
AnimationTrackTableModel::AnimationTrackTableModel( AnimationTrackList* tracks, QObject *parent /*= 0*/ )
	: tracks_(tracks), QAbstractTableModel(parent)
{
	titles_ << tr("Locked") << tr("Visible");
}

void AnimationTrackTableModel::clear()
{
	tracks_ = nullptr;
	removeRows(0, rowCount(QModelIndex()), QModelIndex());
	//reset();
}

bool AnimationTrackTableModel::isEmpty() const
{
	if (tracks_)
		return tracks_->isEmpty();
	else
		return false;
}

int AnimationTrackTableModel::rowCount( const QModelIndex& parent ) const
{
	Q_UNUSED(parent);
	if (tracks_)
		return static_cast<int>(tracks_->size());
	else
		return 0;
}

int AnimationTrackTableModel::columnCount( const QModelIndex& parent ) const
{
	Q_UNUSED(parent);
	return COLUMN_COUNT;
}

QVariant AnimationTrackTableModel::data( const QModelIndex &index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignCenter);
	} 
	else if (role == Qt::DisplayRole) {
		if (tracks_) {
			if (index.column() == 0) {
				return (*tracks_).at(index.row()).locked_;
			}
			else if (index.column() == 1) {
				return (*tracks_).at(index.row()).visible_;
			}
		}
	}

	return QVariant();
}

bool AnimationTrackTableModel::setData( const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/ )
{
	if (index.isValid() && role == Qt::EditRole) {
		int row = index.row();
		int col = index.column();
		if (col == 0) {
			(*tracks_)[row].locked_ = value.toBool();
		}
		else if (col == 1) {
			(*tracks_)[row].visible_ = value.toBool();
		}
		emit dataChanged(index, index);
		return true;
	}
	return false;
}

QVariant AnimationTrackTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal)
			return titles_[section];
		else
			return section+1;
	}
	return QVariant();
}

Qt::ItemFlags AnimationTrackTableModel::flags( const QModelIndex &index ) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return (QAbstractTableModel::flags(index) |= Qt::ItemIsEditable);
}

bool AnimationTrackTableModel::insertRows( int position, int rows, const QModelIndex &index /*= QModelIndex()*/ )
{
	Q_UNUSED(index);
	beginInsertRows(QModelIndex(), position, position+rows-1);

	for (int row = 0; row < rows; row++) {
		tracks_->insert(position, AnimationTrack());
	}

	endInsertRows();
	return true;
}

bool AnimationTrackTableModel::removeRows( int position, int rows, const QModelIndex &index /*= QModelIndex()*/ )
{
	Q_UNUSED(index);
	beginRemoveRows(QModelIndex(), position, position+rows-1);

	for (int row=0; row < rows; row++) {
		tracks_->removeAt(position);
	}

	endRemoveRows();
	return true;
}

/************************************************************************/
/* 动画表格视图                                                          */
/************************************************************************/

AnimationTableView::AnimationTableView( AnimationTableModel* model /*= 0*/, QWidget *parent /*= 0*/ )
{
	Q_UNUSED(model);
	Q_UNUSED(parent);
	setAlternatingRowColors(true);
	setSelectionBehavior(QTableView::SelectRows);
	setSelectionMode(QTableView::SingleSelection);
	resizeColumnsToContents();
	horizontalHeader()->setStretchLastSection(true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void AnimationTableView::mousePressEvent( QMouseEvent *event )
{
	if (event->button() == Qt::LeftButton)
		start_pos_ = event->pos();
	QTableView::mousePressEvent(event);
}

void AnimationTableView::mouseMoveEvent( QMouseEvent *event )
{
	if (event->buttons() & Qt::LeftButton) {
		int distance = (event->pos() - start_pos_).manhattanLength();
		if (distance >= QApplication::startDragDistance()) {
			QModelIndex index = indexAt(start_pos_);
			if (index.isValid()) {
				QModelIndexList list;
				list << index;
				QDrag *drag = new QDrag(this);
				drag->setMimeData(model()->mimeData(list));
				drag->exec();
			}
		}
	}
	QTableView::mouseMoveEvent(event);
}

/************************************************************************/
/* 动画轨道表格视图                                                      */
/************************************************************************/
AnimationTrackTableView::AnimationTrackTableView(AnimationTrackTableModel* model , QWidget* parent)
{
	Q_UNUSED(model);
	Q_UNUSED(parent);
	setSelectionBehavior(QTableView::SelectRows);
	setSelectionMode(QTableView::SingleSelection);
	setAlternatingRowColors(true);
	resizeColumnsToContents();
	horizontalHeader()->setStretchLastSection(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

/************************************************************************/
/* 部件代理                                                              */
/************************************************************************/
CheckBoxDelegate::CheckBoxDelegate( QObject* parent /*= 0*/ )
	: QItemDelegate(parent)
{
}

QWidget * CheckBoxDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	BooleanWidget *editor = new BooleanWidget(parent);
	//editor->setIcon(QIcon(":/images/lock.png"));
	return editor;
}

void CheckBoxDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
	bool value = index.model()->data(index, Qt::EditRole).toBool();

	BooleanWidget* checkBox = static_cast<BooleanWidget*>(editor);
	checkBox->setChecked(value);
}

void CheckBoxDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
	BooleanWidget* checkBox = static_cast<BooleanWidget*>(editor);
	bool value = checkBox->isChecked();
	model->setData(index, value, Qt::EditRole);
}

void CheckBoxDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option/*, const QModelIndex &index*/ ) const
{
	editor->setGeometry(option.rect);
}

void CheckBoxDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	drawBackground(painter, option, index);
	drawCheck(painter, option, option.rect, index.data().toBool() ? Qt::Checked : Qt::Unchecked);
	//drawDecoration(painter, option, option.rect, QPixmap(":/images/lock.png"));
	drawFocus(painter, option, option.rect);
}

BooleanWidget::BooleanWidget( QWidget* parent /*= 0*/ )
	: QWidget(parent)
{
	checkbox_ = new QCheckBox(this);
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(checkbox_, 0, Qt::AlignCenter);
}

bool BooleanWidget::isChecked()
{
	return checkbox_->isChecked();
}

void BooleanWidget::setChecked( bool value )
{
	checkbox_->setChecked(value);
}
/************************************************************************/
/* 动画混合部件                                                          */
/************************************************************************/

int RemixerWidget::frame_interval_ = NORMAL;
int RemixerWidget::sample_interval_;
int RemixerWidget::sim_interval_;

RemixerWidget::RemixerWidget( QWidget *parent /*= 0*/ )
	: QWidget(parent), paused_(true), loop_(true)
{
	createWidgets();
	createSceneView();
	createLayout();
	createStates();
	// 添加最初的动画轨道 要保证至少有一个动画轨道
	addTrack();

	setAcceptDrops(true);

	state_machine_.setInitialState(play_state_);
	QTimer::singleShot(0, &state_machine_, SLOT(start()));

	// 动画时间线
	timer_ = new QTimer(this);
	timer_->start(frame_interval_);
	createConnections();

	std::ifstream ifs("patameters/sample_parameter.txt");
	assert(ifs.is_open());

	ifs >> sample_interval_;
	ifs >> sim_interval_;
	ifs.close();
}

RemixerWidget::~RemixerWidget()
{
	delete animation_track_table_model_;
	delete timer_;
}

QPushButton * RemixerWidget::createToolButton( const QString &toolTip, const QIcon &icon, const char *member )
{
	QPushButton* button = new QPushButton(this);
	button->setToolTip(toolTip);
	button->setIcon(icon);
	button->setIconSize(QSize(16, 16));
	connect(button, SIGNAL(clicked()), this, member);

	return button;
}

void RemixerWidget::createWidgets()
{
	play_control_layout_ = new QHBoxLayout;
	loop_button_ = createToolButton(tr("Loop"), QIcon(":/images/control_repeat_blue.png"), SLOT(loop()));
	loop_button_->setCheckable(true);
	loop_button_->setChecked(loop_);
	start_button_ = createToolButton(tr("Start"), QIcon(":/images/control_start_blue.png"), SLOT(start()));
	play_pause_button_ = createToolButton(tr("Play/Pause"), QIcon(":/images/control_play_blue.png"), SLOT(playpause()));
	end_button_ = createToolButton(tr("End"), QIcon(":/images/control_end_blue.png"), SLOT(end()));
	rewind_button_ = createToolButton(tr("Rewind"), QIcon(":/images/control_rewind_blue.png"), SLOT(rewind()));
	play_control_layout_->addWidget(loop_button_);
	play_control_layout_->addWidget(start_button_);
	play_control_layout_->addWidget(play_pause_button_);
	play_control_layout_->addWidget(end_button_);
	play_control_layout_->addWidget(rewind_button_);
	current_frame_lcd_ = new QLCDNumber(this);		
	current_frame_lcd_->setToolTip(tr("Current Frame"));	
	play_control_layout_->addWidget(current_frame_lcd_);
	fast_forward_button_ = createToolButton(tr("Fast Forward"), QIcon(":/images/control_fastforward_blue.png"), SLOT(ffw()));
	play_control_layout_->addWidget(fast_forward_button_);

	add_track_button_ = createToolButton(tr("Add Track"), QIcon(":/images/add.png"), SLOT(addTrack()));
	del_track_button_ = createToolButton(tr("Delete Track"), QIcon(":/images/cross.png"), SLOT(delTrack()));
	move_track_up_button_ = createToolButton(tr("Move Track Up"), QIcon(":images/arrow_up.png"), SLOT(moveTrackUp()));
	move_track_down_button_ = createToolButton(tr("Move Track Down"), QIcon(":images/arrow_down.png"), SLOT(moveTrackDown()));

	frame_slider_ = new QSlider(Qt::Horizontal);
	frame_slider_->setRange(0, AnimationTrackScene::INITIAL_LENGTH * 30);
	frame_slider_->setTickInterval(5);
	frame_slider_->setTickPosition(QSlider::TicksBelow);
	frame_slider_->setMinimumSize(400, 55);
	frame_slider_->setEnabled(paused_);

	end_frame_lcd_ = new QLCDNumber(this);	
	end_frame_lcd_->setToolTip(tr("Total Frame Number"));
	end_frame_lcd_->setMaximumHeight(25);

	speed_combox_ = new QComboBox(this);
	QStringList speed;
	speed << "0.5x" << "1x" << "2x";
	speed_combox_->addItems(speed);
	speed_combox_->setCurrentIndex(1);

	bindpose_button_ = createToolButton(tr("Restore to Bindpose"), QIcon(":/images/bindpose.png"), SIGNAL(bindposeRestored()));
}

void RemixerWidget::createSceneView()
{
	animation_table_view_ = new AnimationTableView(nullptr, this);

	animation_track_scene_ = new AnimationTrackScene;
	animation_track_scene_->setSceneRect(0, 0, AnimationTrackScene::INITIAL_LENGTH * AnimationClip::SECOND_WIDTH, AnimationTrackScene::INITIAL_HEIGHT);	// 根据end_frame_设置场景大小
	animation_track_view_ = new AnimationTrackView(animation_track_scene_, this);

	animation_track_table_model_ = new AnimationTrackTableModel(&(animation_track_scene_->tracks_), this);
	animation_track_table_view_ = new AnimationTrackTableView(animation_track_table_model_, this);
	animation_track_table_view_->setItemDelegate(new CheckBoxDelegate);
}

void RemixerWidget::createLayout()
{
	QGridLayout* main_layout = new QGridLayout;
	main_layout->addLayout(play_control_layout_, 0, 0, 1, 1);
	main_layout->addWidget(animation_table_view_, 1, 0, 1, 1);

	QWidget* track_editor = new QWidget;
	QGridLayout* track_editor_layout = new QGridLayout;
	track_editor_layout->addWidget(add_track_button_, 0, 0, 1, 1);
	track_editor_layout->addWidget(del_track_button_, 0, 1, 1, 1);
	track_editor_layout->addWidget(move_track_up_button_, 0, 2, 1, 1);
	track_editor_layout->addWidget(move_track_down_button_, 0, 3, 1, 1);
	track_editor_layout->addWidget(animation_track_table_view_, 1, 0, 1, 4);
	track_editor->setLayout(track_editor_layout);

	QGridLayout* right_layout = new QGridLayout;
	right_layout->addWidget(track_editor, 0, 0, 2, 1);
	right_layout->addWidget(frame_slider_, 0, 1, 1, 1);
	right_layout->addWidget(end_frame_lcd_, 0, 2, 1, 1);
	right_layout->addWidget(speed_combox_, 0, 3, 1, 1);
	right_layout->addWidget(bindpose_button_, 0, 4, 1, 1);
	right_layout->addWidget(animation_track_view_,1, 1, 1, 4);
	main_layout->addLayout(right_layout, 0, 1, 2, 1);
	setLayout(main_layout);

	// 整体布局确定之后 表格方可正确显示
	animation_track_table_view_->setModel(animation_track_table_model_);
}

void RemixerWidget::createConnections()
{
	connect(frame_slider_, SIGNAL(valueChanged(int)), this, SLOT(setFrame(int)));

	connect(this, SIGNAL(frameChanged(int)), frame_slider_, SLOT(setValue(int)));
	connect(this, SIGNAL(frameChanged(int)), current_frame_lcd_, SLOT(display(int)));
	connect(this, SIGNAL(frameChanged(int)), animation_track_view_, SLOT(setCurrentFrame(int)));

	connect(animation_track_scene_, SIGNAL(clipUpdated(AnimationClip*, AnimationTrack*)), this, SLOT(updateUI(AnimationClip*, AnimationTrack*)));

	connect(timer_, SIGNAL(timeout()), this, SLOT(updateAnimation()));
	connect(speed_combox_, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSpeed(int)));
}

void RemixerWidget::createStates()
{
	play_state_ = new QState(&state_machine_);
	play_state_->assignProperty(play_pause_button_, "icon", QIcon(":/images/control_play_blue.png"));
	paused_state_ = new QState(&state_machine_);
	paused_state_->assignProperty(play_pause_button_, "icon", QIcon(":/images/control_pause_blue.png"));
	play_state_->addTransition(play_pause_button_, SIGNAL(clicked()), paused_state_);
	paused_state_->addTransition(play_pause_button_, SIGNAL(clicked()), play_state_);
}

void RemixerWidget::addTrack()		
{
	animation_track_table_model_->insertRows(animation_track_table_model_->rowCount(QModelIndex()), 1);
	animation_track_scene_->adjustSceneHeight();
	animation_track_view_->update();
}

void RemixerWidget::delTrack()		
{

}

void RemixerWidget::moveTrackUp()		
{

}

void RemixerWidget::moveTrackDown()	
{

}

void RemixerWidget::loop()		
{
	loop_ = !loop_;
	loop_button_->setChecked(loop_);
}

void RemixerWidget::start()			
{
	animation_track_scene_->current_frame_ = animation_track_scene_->start_frame_;
	emit frameChanged(animation_track_scene_->current_frame_);
}

void RemixerWidget::playpause()		
{
	paused_ = !paused_;
	animation_track_scene_->clearSelection();
	// 更新UI
	frame_slider_->setEnabled(paused_);
	start_button_->setEnabled(paused_);
	end_button_->setEnabled(paused_);
	rewind_button_->setEnabled(paused_);
	fast_forward_button_->setEnabled(paused_);

	add_track_button_->setEnabled(paused_);
	del_track_button_->setEnabled(paused_);
	move_track_up_button_->setEnabled(paused_);
	move_track_down_button_->setEnabled(paused_);
	animation_table_view_->setEnabled(paused_);
	animation_track_view_->setInteractive(paused_);
	animation_track_table_view_->setEnabled(paused_);
}

void RemixerWidget::end()				
{
	animation_track_scene_->current_frame_ = animation_track_scene_->end_frame_;
	emit frameChanged(animation_track_scene_->current_frame_);
}

void RemixerWidget::rewind()			
{
	animation_track_scene_->current_frame_ = qMax(animation_track_scene_->start_frame_, animation_track_scene_->current_frame_-1);
	emit frameChanged(animation_track_scene_->current_frame_);
}

void RemixerWidget::ffw()				
{
	animation_track_scene_->current_frame_ = qMin(animation_track_scene_->end_frame_, animation_track_scene_->current_frame_+1);
	emit frameChanged(animation_track_scene_->current_frame_);
}

void RemixerWidget::updateAnimation()
{
	if (!paused_) {
		if (loop_)
			animation_track_scene_->current_frame_ = (animation_track_scene_->current_frame_ + 1) % (animation_track_scene_->end_frame_ - animation_track_scene_->start_frame_ + 1);
		else
			animation_track_scene_->current_frame_ = qMin(animation_track_scene_->end_frame_, animation_track_scene_->current_frame_ + 1);
		emit frameChanged(animation_track_scene_->current_frame_);
	}
}

void RemixerWidget::setFrame(int frame)
{
	if (animation_track_scene_->current_frame_ != frame) {
		animation_track_scene_->current_frame_ = frame;
		emit frameChanged(animation_track_scene_->current_frame_);
	}
}

void RemixerWidget::updateUI(AnimationClip* clip, AnimationTrack* track)
{
	Q_UNUSED(clip);
	Q_UNUSED(track);
	frame_slider_->setRange(animation_track_scene_->start_frame_, animation_track_scene_->end_frame_);
	end_frame_lcd_->display(animation_track_scene_->end_frame_);
}

void RemixerWidget::changeSpeed( int index )
{
	switch(index) {
	case 0: setFrameTime(SLOW); break;
	case 1: setFrameTime(NORMAL); break;
	case 2: setFrameTime(FAST); break;
	default: qDebug() << "Invalid index"; break;
	}
}

void RemixerWidget::setFrameTime( FrameTime time )
{
	frame_interval_ = time;
	if (timer_)
		timer_->setInterval(time);
}

/************************************************************************/
/* 动画通道编辑部件                                                      */
/************************************************************************/
PoserWidget::PoserWidget( QWidget *parent /*= 0*/ )
	: QWidget(parent)
{
	skeleton_tree_view_ = new QTreeView(this);
	skeleton_tree_view_->setSelectionMode(QTreeView::SingleSelection);
	skeleton_tree_view_->setSelectionBehavior(QTreeView::SelectRows);
	skeleton_tree_view_->setAnimated(true);
	skeleton_tree_view_->setMaximumWidth(400);

	channel_plotter_ = new QCustomPlot(this);
	channel_plotter_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
									QCP::iSelectLegend | QCP::iSelectPlottables);
	channel_plotter_->xAxis->setRange(-8, 8);
	channel_plotter_->yAxis->setRange(-5, 5);
	channel_plotter_->axisRect()->setupFullAxesBox();

	//channel_plotter_->plotLayout()->insertRow(0);
	//channel_plotter_->plotLayout()->addElement(0, 0, new QCPPlotTitle(channel_plotter_, "Animation Channel"));
	channel_plotter_->xAxis->setLabel("time");
	channel_plotter_->yAxis->setLabel("rotation");
	channel_plotter_->legend->setVisible(true);
	channel_plotter_->legend->setSelectableParts(QCPLegend::spItems);

	addRandomGraph();
	addRandomGraph();
	addRandomGraph();
	addRandomGraph();

	QHBoxLayout* main_layout = new QHBoxLayout;
	main_layout->addWidget(skeleton_tree_view_);
	main_layout->addWidget(channel_plotter_);

	setLayout(main_layout);

	// connect slot that ties some axis selections together (especially opposite axes):
	connect(channel_plotter_, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
	// connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
	connect(channel_plotter_, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
	connect(channel_plotter_, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

	// make bottom and left axes transfer their ranges to top and right axes:
	connect(channel_plotter_->xAxis, SIGNAL(rangeChanged(QCPRange)), channel_plotter_->xAxis2, SLOT(setRange(QCPRange)));
	connect(channel_plotter_->yAxis, SIGNAL(rangeChanged(QCPRange)), channel_plotter_->yAxis2, SLOT(setRange(QCPRange)));
}

void PoserWidget::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.
   
   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.
   
   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */
  
  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (channel_plotter_->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || channel_plotter_->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
	  channel_plotter_->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || channel_plotter_->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
	channel_plotter_->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
	channel_plotter_->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (channel_plotter_->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || channel_plotter_->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
	  channel_plotter_->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || channel_plotter_->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
	channel_plotter_->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
	channel_plotter_->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  
  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<channel_plotter_->graphCount(); ++i)
  {
	QCPGraph *graph = channel_plotter_->graph(i);
	QCPPlottableLegendItem *item = channel_plotter_->legend->itemWithPlottable(graph);
	if (item->selected() || graph->selected())
	{
	  item->setSelected(true);
	  graph->setSelected(true);
	}
  }
}

void PoserWidget::mousePress()
{
	// if an axis is selected, only allow the direction of that axis to be dragged
	// if no axis is selected, both directions may be dragged

	if (channel_plotter_->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		channel_plotter_->axisRect()->setRangeDrag(channel_plotter_->xAxis->orientation());
	else if (channel_plotter_->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		channel_plotter_->axisRect()->setRangeDrag(channel_plotter_->yAxis->orientation());
	else
		channel_plotter_->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void PoserWidget::mouseWheel()
{
	// if an axis is selected, only allow the direction of that axis to be zoomed
	// if no axis is selected, both directions may be zoomed

	if (channel_plotter_->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		channel_plotter_->axisRect()->setRangeZoom(channel_plotter_->xAxis->orientation());
	else if (channel_plotter_->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		channel_plotter_->axisRect()->setRangeZoom(channel_plotter_->yAxis->orientation());
	else
		channel_plotter_->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void PoserWidget::addRandomGraph()
{
	int n = 50; // number of points in graph
	double xScale = (rand()/(double)RAND_MAX + 0.5)*2;
	double yScale = (rand()/(double)RAND_MAX + 0.5)*2;
	double xOffset = (rand()/(double)RAND_MAX - 0.5)*4;
	double yOffset = (rand()/(double)RAND_MAX - 0.5)*5;
	double r1 = (rand()/(double)RAND_MAX - 0.5)*2;
	double r2 = (rand()/(double)RAND_MAX - 0.5)*2;
	double r3 = (rand()/(double)RAND_MAX - 0.5)*2;
	double r4 = (rand()/(double)RAND_MAX - 0.5)*2;
	QVector<double> x(n), y(n);
	for (int i=0; i<n; i++)
	{
		x[i] = (i/(double)n-0.5)*10.0*xScale + xOffset;
		y[i] = (sin(x[i]*r1*5)*sin(cos(x[i]*r2)*r4*3)+r3*cos(sin(x[i])*r4*2))*yScale + yOffset;
	}

	channel_plotter_->addGraph();
	channel_plotter_->graph()->setName(QString("New graph %1").arg(channel_plotter_->graphCount()-1));
	channel_plotter_->graph()->setData(x, y);
	channel_plotter_->graph()->setLineStyle((QCPGraph::LineStyle)(rand()%5+1));
	if (rand()%100 > 75)
		channel_plotter_->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(rand()%9+1)));
	QPen graphPen;
	graphPen.setColor(QColor(rand()%245+10, rand()%245+10, rand()%245+10));
	graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
	channel_plotter_->graph()->setPen(graphPen);
	channel_plotter_->replot();
}

/************************************************************************/
/* 动画编辑器部件                                                        */
/************************************************************************/
AnimationEditorWidget::AnimationEditorWidget( QWidget *parent /*= 0*/ )
	: QTabWidget(parent)
{
	remixer_ = new RemixerWidget(this);
	poser_ = new PoserWidget(this);

	addTab(remixer_, tr("Remixer"));
	addTab(poser_, tr("Poser"));

	setAcceptDrops(true);
	connect(remixer_, SIGNAL(frameChanged(int)), this, SIGNAL(frameChanged(int)));
	connect(remixer_, SIGNAL(bindposeRestored()), this, SIGNAL(bindposeRestored()));
	connect(remixer_->animation_track_scene_, SIGNAL(clipUpdated(AnimationClip*, AnimationTrack*)), this, SIGNAL(clipUpdated(AnimationClip*, AnimationTrack*)));
}

AnimationEditorWidget::~AnimationEditorWidget()
{
	delete remixer_;
	delete poser_;
}

void AnimationEditorWidget::setAnimationModel( AnimationTableModel* model )
{
	if (remixer_)
		remixer_->animation_table_view_->setModel(model);
}

void AnimationEditorWidget::setSkeletonModel( SkeletonModel* model )
{
	if (poser_)
		poser_->skeleton_tree_view_->setModel(model);
}

void AnimationEditorWidget::setNameAnimationMap( std::map<QString, Animation*>* name_anim )
{
	if (remixer_)
		remixer_->animation_track_scene_->setNameAnimationMap(name_anim);
}

void AnimationEditorWidget::updateSyntheticAnim(Animation*& syn_anim)
{
	if (remixer_) {
		remixer_->animation_track_scene_->updateSyntheticAnim(syn_anim);
	}
}
