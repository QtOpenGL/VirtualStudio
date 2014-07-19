#ifndef ANIMATION_EDITOR_WIDGET_H
#define ANIMATION_EDITOR_WIDGET_H

#include <QTabWidget>
#include <QTableView>
#include <QItemDelegate>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStateMachine>

#include "animation.h"

class QHBoxLayout;
class QPushButton;
class QLCDNumber;
class QTreeView;
class QCheckBox;
class QComboBox;
class QTimeLine;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

// Qtͼ��/��ͼ��

class RemixerWidget;
/************************************************************************/
/* �����������                                                          */
/************************************************************************/
class AnimationTrackScene : public QGraphicsScene
{
	friend RemixerWidget;

	Q_OBJECT

public:
	AnimationTrackScene(QObject *parent = 0);

	enum { INITIAL_LENGTH = 10 };	// ��ʼĬ�϶���ʱ��10s ע��AnimationClipItem::SECOND_WIDTH = 100 Ҳ����˵��ʼ���1000 
	enum { INITIAL_HEIGHT = 90};	// ��ʼĬ�Ͽ�����3����� ����߶�TRACK_HEIGHT = 30���� ��� ��Ϊ300����
	enum { TRACK_HEIGHT	= 30 };		// ����߶�30����
	enum { FRAME_LENGTH = 16 };		// ֡��16ms

	int endFrame() const { return end_frame_; }

	void setStartFrame(int frame) {	start_frame_ = frame; }
	void setCurrentFrame(int frame) { current_frame_ = frame; }
	void setEndFrame(int frame) { end_frame_ = frame; }
	void adjustSceneHeight();			// ���ݹ���������������߶�
	void adjustSceneWidth(qreal end);	// ����end_frame�����������

	void setNameAnimationMap(std::map<QString, Animation*>* name_anim) { name_animation_ = name_anim; }

	void updateSyntheticAnim(Animation*& syn_anim);	// ���ºϳɶ��� ʮ����Ҫ

private slots:
	void arrangeClips(/*AnimationClip* item, AnimationTrack* track*/);	// ����clip���������ص� ʮ����Ҫ

signals:
	void clipUpdated(AnimationClip*, AnimationTrack*); // ��ӡ�ɾ�����ƶ�Ƭ��ʱ֪ͨ�����ؼ����� �����ºϳɶ���

protected:
	void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
	void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
	void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
	void dropEvent(QGraphicsSceneDragDropEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);	// ���ƹ�� ʱ����

private:
	void addClip(const QString&, const QPointF&);	// ��Ӷ���Ƭ��
	void deleteClip(const QString&, const QPointF&);// ɾ������Ƭ��
	void moveClip(const QPointF&);					// �ƶ�����Ƭ�� 
	Animation* findAnimation(const QString& name);
	AnimationTrack* findTrackByYPos(qreal y);
	

	QPen*	current_frame_pen_;	// ��ǰ֡����
	QPen*	highlight_pen_;		// ����Ƭ���Ϸ�λ����ʾ�߻���
	QPen*	end_frame_pen_;		// ��ֹ֡����
	QPointF	drop_pos_;			// ����Ƭ���Ϸ�λ��
	int		start_frame_;		// ��ʼ֡
	int		current_frame_;		// ��ǰ֡
	int		end_frame_;			// ��ֹ֡

	QList<AnimationTrack>			tracks_;			// ��������� ��֤������һ���������
	std::map<QString, Animation*>*	name_animation_;	// name animationӳ�� �ɷ��泡���ڵ�������ģ��ʱ��ʼ�� 
};

/************************************************************************/
/* ���������ͼ                                                          */
/************************************************************************/
class AnimationTrackView : public QGraphicsView
{
	Q_OBJECT

public:
	AnimationTrackView(AnimationTrackScene* scene, QWidget *parent = 0);

public slots:
	void setCurrentFrame(int frame);

protected:
	void wheelEvent(QWheelEvent *event);

private:
	double zoom_factor_;
	AnimationTrackScene* scene_;
};

/************************************************************************/
/* ����������ģ��                                                      */
/************************************************************************/
class AnimationTrackTableModel : public QAbstractTableModel
{
public: 
	AnimationTrackTableModel(AnimationTrackList* tracks, QObject *parent = 0);

	void clear();
	bool isEmpty() const;

	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
	bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

private:
	enum {COLUMN_COUNT = 2};

	QStringList			titles_;
	AnimationTrackList* tracks_;
};

/************************************************************************/
/* ���������ͼ                                                          */
/************************************************************************/
class AnimationTableView : public QTableView
{
public:
	AnimationTableView(AnimationTableModel* model = 0, QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	QSize sizeHint() const { return QSize(300, 100); }

private:
	QPoint start_pos_;
};

/************************************************************************/
/* ������������ͼ                                                      */
/************************************************************************/
class AnimationTrackTableView : public QTableView
{
public:
	AnimationTrackTableView(AnimationTrackTableModel* model = 0, QWidget* parent = 0);
	QSize sizeHint() const { return QSize(150, 100); }
};

/************************************************************************/
/* ��������                                                              */
/************************************************************************/
// ���е�CheckBox
class BooleanWidget : public QWidget
{
public:
	BooleanWidget(QWidget* parent = 0);
	
	bool isChecked();
	void setChecked(bool value);

private:
	QCheckBox* checkbox_;
};

class CheckBoxDelegate : public QItemDelegate
{
public:
	CheckBoxDelegate(QObject* parent = 0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QWidget *createEditor(QWidget *parent,	const QStyleOptionViewItem &option,	const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, 	QAbstractItemModel *model,	const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,	const QStyleOptionViewItem &option/*,	const QModelIndex &index*/) const;
};

class AnimationEditorWidget;
/************************************************************************/
/* ������ϲ���                                                          */
/************************************************************************/
// ����ؼ�֡��� ��ֵ
class RemixerWidget : public QWidget
{
	Q_OBJECT
	friend AnimationEditorWidget;

public:
	explicit RemixerWidget(QWidget *parent = 0);
	~RemixerWidget();

	void createLayout();
	void createSceneView();
	void createWidgets();
	void createStates();
	void createConnections();

	typedef enum { SLOW = 33, NORMAL = 16, FAST = 8 } FrameTime; // ����ʱ��

	void setFrameTime(FrameTime time);

public slots:
	void addTrack();		// ��Ӷ������
	void delTrack();		// ɾ���������
	void moveTrackUp();		// �����������
	void moveTrackDown();	// ���Ͷ������ 	

	void updateUI(AnimationClip* item, AnimationTrack* track);

	void loop();			// ѭ�����Ŷ���
	void start();			// ����ʼ
	void playpause();		// ����/��ͣ
	void end();				// ����ֹ
	void rewind();			// ǰһ֡
	void ffw();				// ��һ֡
	void updateAnimation();
	void setFrame(int);
	void changeSpeed(int);
		
	static int getSampleInterval(){return sample_interval_;}
	static int getSimInterval(){return sim_interval_;}
	static int getFrameInterval(){return frame_interval_;}

signals:
	void frameChanged(int frame);
	void bindposeRestored();// �л���bindpose

private:
	QPushButton *createToolButton(const QString &toolTip, const QIcon &icon, const char *member);

	QHBoxLayout*	play_control_layout_;
	QPushButton*	loop_button_;
	QPushButton*	start_button_;
	QPushButton*	play_pause_button_;
	QPushButton*	end_button_;
	QPushButton*	rewind_button_;
	QLCDNumber*		current_frame_lcd_;		
	QPushButton*	fast_forward_button_;
	QPushButton*	add_track_button_;
	QPushButton*	del_track_button_;
	QPushButton*	move_track_up_button_;
	QPushButton*	move_track_down_button_;
	QSlider*		frame_slider_;
	QLCDNumber*		end_frame_lcd_;	
	QComboBox*		speed_combox_;
	QPushButton*	bindpose_button_;
	
	AnimationTableView*			animation_table_view_;
	AnimationTrackScene*		animation_track_scene_;
	AnimationTrackView*			animation_track_view_;
	AnimationTrackTableView*	animation_track_table_view_;
	AnimationTrackTableModel*	animation_track_table_model_;	// ����������ģ��	

	QTimer*			timer_;			// ������ʱ��
	QStateMachine	state_machine_;	// ״̬�� ���ڹ����š���ͣ��״̬
	QState*			play_state_;
	QState*			paused_state_;
	bool			paused_;		// �Ƿ�����ͣ״̬ 
	bool			loop_;			// �Ƿ�ѭ������

	static int sample_interval_;	// ����ʱ����
	static int sim_interval_;	// ģ��ʱ����
	static int frame_interval_;	// ˢ��ʱ����
};

class QCustomPlot;
/************************************************************************/
/* ����ͨ���༭����                                                      */
/************************************************************************/
// ����maya��Graph Editor
class PoserWidget :  public QWidget
{
	Q_OBJECT

	friend AnimationEditorWidget;
public:
	explicit PoserWidget(QWidget *parent = 0);

private slots:
	void selectionChanged();
	void mousePress();
	void mouseWheel();
	void addRandomGraph();

private:
	QTreeView*		skeleton_tree_view_;
	QCustomPlot*	channel_plotter_;
};

/************************************************************************/
/* �����༭������                                                        */
/************************************************************************/
class AnimationEditorWidget : public QTabWidget
{
	Q_OBJECT

public:
	AnimationEditorWidget(QWidget *parent = 0);
	~AnimationEditorWidget();

	void setAnimationModel(AnimationTableModel* model);					// ���ö������ģ��
	void setSkeletonModel(SkeletonModel* model);						// ���ùǼ�����ģ��
	void setNameAnimationMap(std::map<QString, Animation*>* name_anim); // ��������-����ӳ���

	void updateSyntheticAnim(Animation*& syn_anim);	

signals:
	void frameChanged(int);
	void bindposeRestored();	// �л���bindpose
	void clipUpdated(AnimationClip*, AnimationTrack*); // ��ӡ�ɾ�����ƶ�Ƭ��ʱ֪ͨ�����ؼ����� �����ºϳɶ���

private:
	RemixerWidget*	remixer_;	// �����Զ����༭��
	PoserWidget*	poser_;		// ����ͨ���༭��
};
#endif // ANIMATION_EDITOR_WIDGET_H