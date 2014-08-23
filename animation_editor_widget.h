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

// Qt图形/视图类
class RemixerWidget;
/************************************************************************/
/* 动画轨道场景                                                          */
/************************************************************************/
class AnimationTrackScene : public QGraphicsScene
{
	friend RemixerWidget;
	Q_OBJECT

public:
    enum 
    { 
        INITIAL_WIDTH = 1000, 
        INITIAL_HEIGHT = 90
    };

	AnimationTrackScene( RemixerWidget* remixer, QObject *parent = 0 );
    ~AnimationTrackScene();

    int endTime() const;
	int endFrame() const;
    bool isEmpty() const;
    Animation* syntheticAnimation();

    void setStartFrame(int frame);
    void setCurrentFrame(int frame);
    void setEndFrame(int frame);

    void setNameAnimationMap(NameToAnimMap* name_anim);
    void updateSyntheticAnim(float play_speed = 1.0f);    // 更新合成动画
    void reset();

// 旧数据和接口
    enum { INITIAL_LENGTH = 10 };	// 初始默认动画时长10s 注意AnimationClipItem::SECOND_WIDTH = 100 也就是说初始宽度1000 
    enum { INITIAL_HEIGHT = 90};	// 初始默认可容纳3个轨道 轨道高度TRACK_HEIGHT = 30像素 因此 设为300像素
    enum { TRACK_HEIGHT	= 30 };		// 轨道高度30像素
    enum { FRAME_LENGTH = 16 };		// 帧长16ms
    void adjustSceneHeight();			// 根据轨道数量调整场景高度
    void adjustSceneWidth(qreal end);	// 根据end_frame调整场景宽度
private slots:
    void arrangeClips(/*AnimationClip* item, AnimationTrack* track*/);	// 调整clip消除区间重叠 十分重要
signals:
    void clipUpdated(AnimationClip*, AnimationTrack*); // 添加、删除、移动片段时通知其他控件更新 并更新合成动画

signals:
    void syntheticAnimationUpdated(); // 通知合成动画更新
protected:
	void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
	void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
	void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
	void dropEvent(QGraphicsSceneDragDropEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);

private slots:
    void onSelectionChanged();
    void onClipMoved();
    void deleteClip();        // 删除片段

private:
    void createContextMenu();

    AnimationTrack* addTrack(); // 添加轨道
    void delTrack();            // 删除轨道
    void moveUpTrack();         // 提高轨道
    void moveDownTrack();       // 降低轨道

	AnimationClip* addClip(const QString&, const QPointF&);	// 添加片段

	Animation* findAnimation(const QString& name);  // 根据名称查找动画
	AnimationTrack* findTrackByYPos(qreal y);       // 根据片段所处位置查找所属轨道

    void updateEndFrame();  // 更新终止帧位置
    void updateCurTrack();  // 更新当前轨道
    void adjustSize();      // 调整编辑区大小

private:
    RemixerWidget* remixer_;    // 所属动画混合部件

	QPen*	cur_frame_pen_;	    // 当前帧画笔
	QPen*	drop_pos_hint_pen_;	// 动画片段拖放位置提示线画笔
	QPen*	end_frame_pen_;		// 终止帧画笔
    QBrush* cur_track_brush_;   // 当前轨道画刷
	QPointF	drop_pos_;			// 动画片段拖放位置
	int		start_frame_;		// 起始帧
	int		cur_frame_;		    // 当前帧
	int		end_frame_;			// 终止帧
    int     cur_track_;         // 当前轨道

	QList<AnimationTrack*>  tracks_;			    // 动画轨道
	NameToAnimMap*          name_animation_;	    // 名称-动画映射表
    Animation*              synthetic_animation_;	// 合成动画

    QAction* delete_clip_action_;
    QMenu*   context_menu_;
};

/************************************************************************/
/* 动画轨道视图                                                         */
// 旧数据和方法
private:
	void addClip(const QString&, const QPointF&);	// 添加动画片断
	void deleteClip(const QString&, const QPointF&);// 删除动画片段
	void moveClip(const QPointF&);					// 移动动画片段 
	
	QPen*	highlight_pen_;		// 动画片段拖放位置提示线画笔
	QPen*	end_frame_pen_;		// 终止帧画笔
};

/************************************************************************/
/* 动画轨道视图                                                          */
>>>>>>> dev
/************************************************************************/
class AnimationTrackView : public QGraphicsView
{
	Q_OBJECT

public:
	AnimationTrackView(AnimationTrackScene* scene, QWidget *parent = 0);

public slots:
	void setCurrentFrame(int frame);

private:
	AnimationTrackScene* scene_;
// 旧方法
protected:
	void wheelEvent(QWheelEvent *event);
private:
	double zoom_factor_;
};

/************************************************************************/
/* 动画轨道表格模型                                                      */
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
/* 动画表格视图                                                         */
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

class MocapImportDialog;
class AnimationEditorWidget;
/************************************************************************/
/* 动画轨道表格视图                                                      */
/************************************************************************/
class AnimationTrackTableView : public QTableView
{
public:
	AnimationTrackTableView(AnimationTrackTableModel* model = 0, QWidget* parent = 0);
	QSize sizeHint() const { return QSize(150, 100); }
};

/************************************************************************/
/* 部件代理                                                              */
/************************************************************************/
// 居中的CheckBox
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

/************************************************************************/
/* 动画混合部件                                                          */
/************************************************************************/
// 负责关键帧混合 插值
class RemixerWidget : public QWidget
{
    friend AnimationEditorWidget;
	Q_OBJECT

public:
	explicit RemixerWidget(QWidget *parent = 0);
	~RemixerWidget();

	void createLayout();
	void createSceneView();
	void createWidgets();
	void createStates();
	void createConnections();

    double playSpeed() const;

public slots:
	void addTrack();		// 添加轨道

    void delTrack();		// 删除轨道
	void moveTrackUp();		// 提升轨道
	void moveTrackDown();	// 降低轨道 	

	void updateFrameRangeUI();
    void updateTrackEditUI();

	void loop();			// 循环播放
	typedef enum { SLOW = 33, NORMAL = 16, FAST = 8 } FrameTime; // 采样时间

	void setFrameTime(FrameTime time);

public slots:
	void updateUI(AnimationClip* item, AnimationTrack* track);

	void start();			// 到起始
	void playpause();		// 播放/暂停
	void end();				// 到终止
	void rewind();			// 前一帧
	void ffw();				// 下一帧
    void addKeyframe();     // 添加关键帧
    void showImportMocapDialog();
	void updateAnimation();
	void setFrame(int);
	void changeSpeed(int);

signals:
	void frameChanged(int frame);
	void bindposeRestored();                // 切换到bindpose
    void syntheticAnimationUpdated();       // 合成动画已更新
    void mocapSelected(QString& , QString&);  // 已选定mocap数据文件

private:
	QPushButton*    createToolButton(const QString &toolTip, const QIcon &icon, const char *member);

private:
		
	static int getSampleInterval(){return sample_interval_;}
	static int getSimInterval(){return sim_interval_;}
	static int getFrameInterval(){return frame_interval_;}

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
    QPushButton*    add_keyframe_button_;
    QPushButton*    import_mocap_button_;
	QPushButton*	move_track_down_button_;
	QSlider*		frame_slider_;
	QLCDNumber*		end_frame_lcd_;	
	QComboBox*		speed_combox_;
	QPushButton*	bindpose_button_;
	
	AnimationTableView*		anim_table_view_;
	AnimationTrackScene*	track_scene_;
	AnimationTrackView*		track_view_;
    MocapImportDialog*      mocap_import_dialog_;

	QTimer*			timer_;			// 动画计时器
	QStateMachine	state_machine_;	// 状态机 用于管理播放、暂停等状态
	QState*			play_state_;    // 播放状态
	QState*			paused_state_;  // 暂停状态
	bool			paused_;		// 是否处于暂停状态 
	bool			loop_;			// 是否循环播放
	double          play_speed_;	// 刷新时间 默认1x速度
	AnimationTrackTableView*	animation_track_table_view_;
	AnimationTrackTableModel*	animation_track_table_model_;	// 动画轨道表格模型	


	static int sample_interval_;	// 采样时间间隔
	static int sim_interval_;	// 模拟时间间隔
	static int frame_interval_;	// 刷新时间间隔
};

class QCustomPlot;
/************************************************************************/
/* 动画通道编辑部件                                                     */
/************************************************************************/
// 类似maya的Graph Editor
class PoserWidget : public QWidget
{
    friend AnimationEditorWidget;
	Q_OBJECT
	
public:
	explicit PoserWidget(QWidget *parent = 0);

    void reset();
    void updateGraphData();
    void setNameChannelIndexMap(NameToChIdMap* name_chid);
    
    void updateChannelData(const Animation* animation);

private slots:
	void selectionChanged();
	void mousePress();
	void mouseWheel();
	void addGraph(); 
    void setCurChannel(const QModelIndex& index);
	void addRandomGraph();

private:
	QTreeView*		skeleton_tree_view_;
	QCustomPlot*	channel_plotter_;
    NameToChIdMap*  name_channelindex_;     // 名称-通道索引映射表

    int cur_channel_; 
    // 采样的旋转数据 用于poser_
    QVector<QVector<QQuaternion> > sampled_rot_keys_;

    QVector<double> time_;
    QVector<double> x_rot_;
    QVector<double> y_rot_;
    QVector<double> z_rot_;
};

/************************************************************************/
/* 动画编辑器部件                                                       */
/************************************************************************/
class AnimationEditorWidget : public QTabWidget
{
	Q_OBJECT

public:
	AnimationEditorWidget(QWidget *parent = 0); 
	~AnimationEditorWidget();

    void reset();
	void setAnimationTableModel(AnimationTableModel* model);// 设置动画表格模型
	void setSkeletonTreeModel(SkeletonTreeModel* model);	// 设置骨架树形模型
	void setNameAnimationMap(NameToAnimMap* name_anim);     // 设置名称-动画映射表
    void setNameChannelIndexMap(NameToChIdMap* name_chid);  // 设置名称-通道索引映射表

    Animation* syntheticAnimation();

signals:
	void frameChanged(int);
	void bindposeRestored();	                       // 切换到bindpose
//	void clipUpdated(AnimationClip*, AnimationTrack*); // 编辑片段时通知其他控件更新 并更新合成动画
    void mocapSelected(QString& , QString&);  // 已选定mocap数据文件

private slots:
    void updateChannelData();

private:
	RemixerWidget*	remixer_;	// 非线性动画编辑器
	PoserWidget*	poser_;		// 通道编辑器

public:
	void updateSyntheticAnim(Animation*& syn_anim);	

signals:
	void clipUpdated(AnimationClip*, AnimationTrack*); // 添加、删除、移动片段时通知其他控件更新 并更新合成动画
;
#endif // ANIMATION_EDITOR_WIDGET_H
