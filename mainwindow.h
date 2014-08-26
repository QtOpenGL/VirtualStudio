#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// Simulation classes
class Scene;
class AnimationClip;
class AnimationTrack;
// UI classes
class SimulationWindow;
class PatternView;
class PatternScene;
class ObjectBrowserWidget;
class AnimationEditorWidget;
class QDockWidget;
class QAction;
class QActionGroup;
class QMenu;
class QToolBar;
class QSplitter;
class QPrinter;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() {}

private:
    void createViews();
    void createActions();
    void createMenusAndToolBars();
    void createDockWidgets();
    void createConnections();
    bool okToContinue();

private slots:

    //void fileOpen();
    //void fileSave();
    //void fileSaveAs();

    void fileImportAvatar();	// 导入虚拟人物
    void fileImportPattern();	// 导入服装打板
    void fileImportCloth();
    void toggleGridVisible();	// 显示/隐藏参考网格

    void switchInteractionMode();
    //void switchToRotateMode();
    //void switchToPanMode();
    //void switchToZoomMode();
    //void switchToClothRotateMode();
    //void switchToClothMoveMode();
    //void switchToClothScaleMode();
    //void switchToSelectMode();

    void renderingModeChanged(int);

    void exportAsVideo();
    void addSeamline();
    void generateCloth();

    bool save();
    void updateAnimation(int);
    void importMocap(QString& , QString&);
    void startSimulate();
    bool save();
    //void updateAnimation(int);
    void updateSynthesizedAnimation(AnimationClip*, AnimationTrack*);

    void save_cm_file();
    void open_cm_file();

    void changeClothColor();
    void changeClothTexture();
    void toggleRecord();

private:
    // 仿真相关成员
    Scene* scene_;

    // UI相关成员
    QAction* file_open_action_;
    QAction* file_import_avatar_action_;
    QAction* file_import_pattern_action_;
    QAction* file_exit_action_;
    QAction* file_export_as_video_action_;
    QAction* simulation_select_action_;
    QActionGroup* shading_group_;
    QAction* simulation_shading_action_;
    QAction* simulation_xray_action_;
    QAction* simulation_skeleton_action_;
    QAction* design_showgrid_action_;
    QAction* design_add_seamline_action_;
    QAction* design_generate_cloth_action_;
    QAction* file_save_action_;
    QAction* file_save_as_action_;
    QAction* file_import_avatar_action_;
    QAction* file_import_pattern_action_;
	// 读入OBJ衣服动作，wunf
    QAction* file_import_cloth_action_;
    QAction* file_exit_action_;
    QActionGroup* interaction_group_;
    QAction* simulation_rotate_action_;
    QAction* simulation_pan_action_;
    QAction* simulation_zoom_action_;
    QAction* simulation_cloth_rotate_action_;
    QAction* simulation_cloth_move_action_;
    QAction* simulation_cloth_scale_action_;
    QActionGroup* shading_group_;
    QAction* simulation_shading_action_;
    QAction* simulation_shading_wireframe_action_;
    QAction* simulation_wireframe_action_;
    QAction* design_showgrid_action_;
    QAction* design_change_color_action_;
    QAction* design_change_texture_action_;
    QAction* start_simulate_;
    QAction* save_cm_file_;
    QAction* open_cm_file_;
    QAction* reset_;
    QAction* record_action_;

    QMenu* file_menu_;
    QMenu* window_menu_;
    QMenu* simulation_menu_;
    QMenu* design_menu_;

    QComboBox* rendering_mode_combo_;

    QToolBar* file_tool_bar_;
    QToolBar* simulation_tool_bar_;
    QToolBar* design_tool_bar_;

    SimulationWindow*		simulation_view_;
    PatternView*			design_view_;
    PatternScene*			pattern_scene_;
    QSplitter*				splitter_;
    ObjectBrowserWidget*	object_browser_;
    AnimationEditorWidget*	animation_editor_;
    QDockWidget*			object_browser_dock_widget_;
    QDockWidget*			animation_editor_dock_widget_;
    QPrinter*				printer_;

    QString avatar_file_;
    QString obj_cloth_file_;
    QString cloth_motion_file_;
    QString project_file_;
};

#endif // MAINWINDOW_H
