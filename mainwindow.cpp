#include "mainwindow.h"

#include <QtGui>
#include <QWidget>
#include <QPrinter>
#include <QSplitter>
#include <QWindow>
#include <fstream>

// simulation
#include "scene.h"

// GUI
#include "simulation_window.h"
#include "pattern.h"
#include "object_browser_widget.h"
#include "animation_editor_widget.h"

MainWindow::MainWindow( QWidget *parent )
	: QMainWindow(parent), scene_(new Scene())
{
	printer_ = new QPrinter(QPrinter::HighResolution);

	createViews();
	createActions();
	createMenusAndToolBars();
	createDockWidgets();
	createConnections();

	setWindowIcon(QIcon(":images/clothes.png"));
} 

void MainWindow::createViews() 
{
	simulation_view_ = new SimulationWindow(scene_);
	//simulation_view_->setScene(scene_);
	
	pattern_scene_ = new PatternScene(this);
	QSize page_size = printer_->paperSize(QPrinter::Point).toSize();
	pattern_scene_->setSceneRect(0, 0, page_size.width(), page_size.height());
	design_view_ = new PatternView(this);
	design_view_->setScene(pattern_scene_);

	splitter_ = new QSplitter(Qt::Horizontal);
	QWidget* container = createWindowContainer(simulation_view_, this);
	container->setMinimumSize(simulation_view_->size());
	container->resize(500, 300);
	
	splitter_->addWidget(container);
	splitter_->addWidget(design_view_);
	splitter_->setStretchFactor(1, 1);
	setCentralWidget(splitter_);
}

void MainWindow::createActions()
{
	file_open_action_ = new QAction(QIcon(":images/open.png"), tr("Open..."), this);
	file_open_action_->setStatusTip(tr("Open project"));
	file_open_action_->setToolTip(tr("Open VirtualStudio project file"));
	file_open_action_->setShortcut(QKeySequence::Open);

	file_save_action_ = new QAction(QIcon(":images/save.png"), tr("Save"), this);
	file_save_action_->setStatusTip(tr("Save project"));
	file_save_action_->setToolTip(tr("Save VirtualStudio project file"));
	file_save_action_->setShortcut(QKeySequence::Save);

	file_save_as_action_ = new QAction(QIcon(":images/saveas.png"), tr("Save as..."), this);
	file_save_as_action_->setStatusTip(tr("Save project as ..."));
	file_save_as_action_->setToolTip(tr("Save VirtualStudio project file as ..."));
	file_save_as_action_->setShortcut(QKeySequence::SaveAs);

	file_import_avatar_action_ = new QAction(QIcon(":images/import_avatar.png"), tr("Import avatar..."), this);
	file_import_avatar_action_->setStatusTip(tr("Import avatar"));
	file_import_avatar_action_->setToolTip(tr("Import avatar into the simulation"));

	file_import_pattern_action_ = new QAction(QIcon(":images/import_pattern.png"), tr("Import pattern..."), this);
	file_import_pattern_action_->setStatusTip(tr("Import pattern"));
	file_import_pattern_action_->setToolTip(tr("Import pattern into the canvas"));

	// 读入OBJ衣服，wunf
	file_import_cloth_action_ = new QAction(QIcon(":images/clothwireframe.png"), tr("Import cloth..."), this);
	file_import_cloth_action_->setStatusTip(tr("Import obj cloth"));
	file_import_cloth_action_->setToolTip(tr("Import obj cloth into the simulation"));

	save_cm_file_ = new QAction(QIcon(":images/saveclothmotion.png"), tr("save cloth motion..."), this);
	save_cm_file_->setStatusTip(tr("Save cloth motion"));
	save_cm_file_->setToolTip(tr("save cloth motion to file"));

	open_cm_file_ = new QAction(QIcon(":images/clothmotion.png"), tr("open cloth motion..."), this);
	open_cm_file_->setStatusTip(tr("Open cloth motion"));
	open_cm_file_->setToolTip(tr("Open cloth motion file"));

	file_exit_action_ = new QAction(QIcon(":images/exit.png"), tr("Exit"), this);
	file_exit_action_->setStatusTip(tr("Quit"));
	file_exit_action_->setToolTip(tr("Quit the application"));
	file_exit_action_->setShortcut(QKeySequence::Quit);

	simulation_rotate_action_ = new QAction(QIcon(":images/rotate.png"), tr("Rotate"), this);
	simulation_rotate_action_->setStatusTip(tr("Drag to rotate the view"));
	simulation_rotate_action_->setToolTip(tr("Rotate"));
	simulation_rotate_action_->setCheckable(true);
	simulation_rotate_action_->setChecked(scene_->interactionMode() == Scene::ROTATE);

	simulation_pan_action_ = new QAction(QIcon(":images/pan.png"), tr("Pan"), this);
	simulation_pan_action_->setStatusTip(tr("Drag to pan the view"));
	simulation_pan_action_->setToolTip(tr("Pan"));
	simulation_pan_action_->setCheckable(true);
	simulation_pan_action_->setChecked(scene_->interactionMode() == Scene::PAN);

	simulation_zoom_action_ = new QAction(QIcon(":images/zoom.png"), tr("Zoom"), this);
	simulation_zoom_action_->setStatusTip(tr("Drag to zoom in/out the view"));
	simulation_zoom_action_->setToolTip(tr("Zoom in/out"));
	simulation_zoom_action_->setCheckable(true);
	simulation_zoom_action_->setChecked(scene_->interactionMode() == Scene::PAN);

	simulation_cloth_rotate_action_ = new QAction(QIcon(":images/clothrotate.png"), tr("Rotate Cloth"), this);
	simulation_cloth_rotate_action_->setStatusTip(tr("Drag to rotate the cloth"));
	simulation_cloth_rotate_action_->setToolTip(tr("Rotate Cloth"));
	simulation_cloth_rotate_action_->setCheckable(true);
	simulation_cloth_rotate_action_->setChecked(scene_->interactionMode() == Scene::CLOTH_ROTATE);

	simulation_cloth_move_action_ = new QAction(QIcon(":images/clothpan.png"), tr("Move Cloth"), this);
	simulation_cloth_move_action_->setStatusTip(tr("Drag to pan the cloth"));
	simulation_cloth_move_action_->setToolTip(tr("Move Cloth"));
	simulation_cloth_move_action_->setCheckable(true);
	simulation_cloth_move_action_->setChecked(scene_->interactionMode() == Scene::CLOTH_MOVE);

	simulation_cloth_scale_action_ = new QAction(QIcon(":images/clothzoom.png"), tr("Scale Cloth"), this);
	simulation_cloth_scale_action_->setStatusTip(tr("Drag to scale the cloth"));
	simulation_cloth_scale_action_->setToolTip(tr("Scale Cloth"));
	simulation_cloth_scale_action_->setCheckable(true);
	simulation_cloth_scale_action_->setChecked(scene_->interactionMode() == Scene::CLOTH_SCALE);

	interaction_group_ = new QActionGroup(this);
	interaction_group_->addAction(simulation_rotate_action_);
	interaction_group_->addAction(simulation_pan_action_);
	interaction_group_->addAction(simulation_zoom_action_);
	interaction_group_->addAction(simulation_cloth_rotate_action_);
	interaction_group_->addAction(simulation_cloth_move_action_);
	interaction_group_->addAction(simulation_cloth_scale_action_);

	simulation_shading_action_ = new QAction(QIcon(":images/shading.png"), tr("Shading"), this);
	simulation_shading_action_->setStatusTip(tr("View the avatar"));
	simulation_shading_action_->setToolTip(tr("Show avatar in shading"));

	simulation_wireframe_action_ = new QAction(QIcon(":images/skeleton.png"), tr("Skeleton"), this);
	simulation_wireframe_action_->setStatusTip(tr("View the skeleton"));
	simulation_wireframe_action_->setToolTip(tr("Show avatar's skeleton only"));

	simulation_shading_wireframe_action_ = new QAction(QIcon(":images/x_ray.png"), tr("X-Ray"), this);
	simulation_shading_wireframe_action_->setStatusTip(tr("View the avatar with skeleton"));
	simulation_shading_wireframe_action_->setToolTip(tr("Show avatar in x-ray style (see-through skeleton)"));

	start_simulate_ = new QAction(QIcon(":images/simulate.png"), tr("Simulate"), this);
	start_simulate_->setStatusTip(tr("Start simulation"));
	start_simulate_->setToolTip(tr("Start simulation"));

	shading_group_ = new QActionGroup(this);
	shading_group_->addAction(simulation_shading_action_);
	shading_group_->addAction(simulation_shading_wireframe_action_);
	shading_group_->addAction(simulation_wireframe_action_);

	design_showgrid_action_ = new QAction(QIcon(":images/showgrid.png"), tr("Show/Hide grid"), this);
	design_showgrid_action_->setCheckable(true);
	design_showgrid_action_->setChecked(pattern_scene_->gridVisible());
	design_showgrid_action_->setStatusTip(tr("Show/Hide the background grid"));
	design_showgrid_action_->setToolTip(tr("Show/Hide the background grid"));

	design_change_color_action_ = new QAction(QIcon(":images/graphics.png"), tr("Change cloth color"), this);
	design_change_color_action_->setStatusTip(tr("Change cloth color"));
	design_change_color_action_->setToolTip(tr("Change cloth color"));

	design_change_texture_action_ = new QAction(QIcon("./images/picture.png"), tr("Change cloth texture"), this);
	design_change_texture_action_->setStatusTip(tr("Change cloth texture"));
	design_change_texture_action_->setToolTip(tr("Change cloth texture"));

	record_action_ = new QAction(QIcon("./images/record.png"), tr("Record"), this);
	record_action_->setStatusTip(tr("Record animation to AVI file."));
	record_action_->setToolTip(tr("Record animation to AVI file."));
}

void MainWindow::createMenusAndToolBars()
{
	file_menu_ = menuBar()->addMenu(tr("&File"));
	file_menu_->addAction(file_open_action_);
	file_menu_->addAction(file_save_action_);
	file_menu_->addAction(file_save_as_action_);
	file_menu_->addSeparator();
	file_menu_->addAction(file_import_avatar_action_);
	file_menu_->addAction(file_import_pattern_action_);
	//读入OBJ衣服，wunf
	file_menu_->addAction(file_import_cloth_action_);
	//file_menu_->addSeparator();
	// abandonded
	//file_menu_->addAction(open_cm_file_);
	//file_menu_->addAction(save_cm_file_);
	file_menu_->addSeparator();
	file_menu_->addAction(file_exit_action_);

	window_menu_ = menuBar()->addMenu(tr("&Window"));

	simulation_menu_ = menuBar()->addMenu(tr("&Simulation"));
	simulation_menu_->addAction(simulation_rotate_action_);
	simulation_menu_->addAction(simulation_pan_action_);
	simulation_menu_->addAction(simulation_zoom_action_);
	simulation_menu_->addAction(simulation_cloth_rotate_action_);
	simulation_menu_->addAction(simulation_cloth_move_action_);
	simulation_menu_->addAction(simulation_cloth_scale_action_);
	simulation_menu_->addSeparator();
	QMenu* display_mode_menu = new QMenu(tr("Display mode"));
	display_mode_menu->addAction(simulation_shading_action_);
	display_mode_menu->addAction(simulation_wireframe_action_);
	display_mode_menu->addAction(simulation_shading_wireframe_action_);
	simulation_menu_->addSeparator();
	simulation_menu_->addAction(start_simulate_);
	simulation_menu_->addAction(record_action_);
	simulation_menu_->addSeparator();
	simulation_menu_->addMenu(display_mode_menu);

	design_menu_ = menuBar()->addMenu(tr("&Design"));
	design_menu_->addAction(design_showgrid_action_);
	design_menu_->addAction(design_change_color_action_);
	design_menu_->addAction(design_change_texture_action_);

	file_tool_bar_ = addToolBar(tr("&File"));
	file_tool_bar_->addAction(file_open_action_);
	file_tool_bar_->addAction(file_save_action_);
	file_tool_bar_->addSeparator();
	file_tool_bar_->addAction(file_import_avatar_action_);
	file_tool_bar_->addAction(file_import_pattern_action_);
	file_tool_bar_->addAction(file_import_cloth_action_);
	//file_tool_bar_->addSeparator();
	// abandonded
	//file_tool_bar_->addAction(open_cm_file_);
	//file_tool_bar_->addAction(save_cm_file_);

	simulation_tool_bar_ = addToolBar(tr("&Simulation"));
	simulation_tool_bar_->addAction(simulation_rotate_action_);
	simulation_tool_bar_->addAction(simulation_pan_action_);
	simulation_tool_bar_->addAction(simulation_zoom_action_);
	simulation_tool_bar_->addSeparator();
	simulation_tool_bar_->addAction(simulation_cloth_rotate_action_);
	simulation_tool_bar_->addAction(simulation_cloth_move_action_);
	simulation_tool_bar_->addAction(simulation_cloth_scale_action_);
	simulation_tool_bar_->addSeparator();
	rendering_mode_combo_ = new QComboBox;
	rendering_mode_combo_->addItem(simulation_shading_action_->icon(), simulation_shading_action_->text());
	rendering_mode_combo_->addItem(simulation_wireframe_action_->icon(), simulation_wireframe_action_->text());
	rendering_mode_combo_->addItem(simulation_shading_wireframe_action_->icon(), simulation_shading_wireframe_action_->text());
	simulation_tool_bar_->addWidget(rendering_mode_combo_);
	simulation_tool_bar_->addSeparator();
	simulation_tool_bar_->addAction(start_simulate_);
	simulation_tool_bar_->addAction(record_action_);

	design_tool_bar_ = addToolBar(tr("&Design"));
	design_tool_bar_->addAction(design_showgrid_action_);
	design_tool_bar_->addAction(design_change_color_action_);
	design_tool_bar_->addAction(design_change_texture_action_);

}		

void MainWindow::createDockWidgets()
{
	setDockOptions(QMainWindow::AnimatedDocks);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	QDockWidget::DockWidgetFeatures features = QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable;

	object_browser_ = new ObjectBrowserWidget(this);
	object_browser_dock_widget_ = new QDockWidget(tr("Object Browser"), this);
	object_browser_dock_widget_->setFeatures(features);
	object_browser_dock_widget_->setWidget(object_browser_);
	addDockWidget(Qt::RightDockWidgetArea, object_browser_dock_widget_);

	animation_editor_ = new AnimationEditorWidget(this);
	animation_editor_dock_widget_ = new QDockWidget(tr("Animation Editor"), this);
	animation_editor_dock_widget_->setFeatures(features);
	animation_editor_dock_widget_->setWidget(animation_editor_);
	addDockWidget(Qt::BottomDockWidgetArea, animation_editor_dock_widget_);

	window_menu_->addAction(object_browser_dock_widget_->toggleViewAction());
	window_menu_->addAction(animation_editor_dock_widget_->toggleViewAction());
}

void MainWindow::createConnections()
{
	connect(file_open_action_, SIGNAL(triggered()), this, SLOT(fileOpen()));
	connect(file_save_action_, SIGNAL(triggered()), this, SLOT(fileSave()));
	connect(file_save_as_action_, SIGNAL(triggered()), this, SLOT(fileSaveAs()));

	connect(file_import_avatar_action_, SIGNAL(triggered()), this, SLOT(fileImportAvatar()));
	connect(file_import_pattern_action_, SIGNAL(triggered()), this, SLOT(fileImportPattern()));
	// OBJ服装，wunf
	connect(file_import_cloth_action_, SIGNAL(triggered()), this, SLOT(fileImportCloth()));
	connect(save_cm_file_, SIGNAL(triggered()), this, SLOT(save_cm_file()));
	connect(open_cm_file_, SIGNAL(triggered()), this, SLOT(open_cm_file()));
	// interaction mode
	connect(rendering_mode_combo_, SIGNAL(currentIndexChanged(int)), this, SLOT(renderingModeChanged(int)));

	connect(simulation_rotate_action_, SIGNAL(triggered()), this, SLOT(switchToRotateMode()));
	connect(simulation_pan_action_, SIGNAL(triggered()), this, SLOT(switchToPanMode()));
	connect(simulation_zoom_action_, SIGNAL(triggered()), this, SLOT(switchToZoomMode()));
	connect(simulation_cloth_rotate_action_, SIGNAL(triggered()), this, SLOT(switchToClothRotateMode()));
	connect(simulation_cloth_move_action_, SIGNAL(triggered()), this, SLOT(switchToClothMoveMode()));
	connect(simulation_cloth_scale_action_, SIGNAL(triggered()), this, SLOT(switchToClothScaleMode()));

	connect(start_simulate_, SIGNAL(triggered()), this, SLOT(startSimulate()));

	connect(animation_editor_, SIGNAL(frameChanged(int)), simulation_view_, SLOT(updateAnimation(int)));
	connect(animation_editor_, SIGNAL(clipUpdated(AnimationClip*, AnimationTrack*)), this, SLOT(updateSynthesizedAnimation(AnimationClip*, AnimationTrack*)));
	connect(animation_editor_, SIGNAL(bindposeRestored()), simulation_view_, SLOT(restoreToBindpose()));

	connect(design_showgrid_action_, SIGNAL(triggered()), this, SLOT(toggleGridVisible()));
	connect(design_change_color_action_, SIGNAL(triggered()), this, SLOT(changeClothColor()));
	connect(design_change_texture_action_, SIGNAL(triggered()), this, SLOT(changeClothTexture()));

	connect(record_action_, SIGNAL(triggered()), this, SLOT(toggleRecord()));
}

bool MainWindow::okToContinue()
{
	if (isWindowModified()) {
		int r = QMessageBox::warning(this, tr("VirtualStudio"),
			tr("The document has been modified.\nDo you want to save your changes?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (r == QMessageBox::Yes) {
			return save();
		} else if (r == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
}

void MainWindow::fileImportAvatar()
{
	if (okToContinue()) {
		QString file_name = QFileDialog::getOpenFileName(this, tr("Import Avatar"),  ".", tr("Avatar files (*.dae)"));

		if (!file_name.isEmpty()) {
			avatar_file_ = file_name;
			scene_->importAvatar(file_name);
			simulation_view_->paintGL();
			animation_editor_->setAnimationModel(scene_->avatarAnimationModel());
			animation_editor_->setSkeletonModel(scene_->avatarSkeletonModel());
			animation_editor_->setNameAnimationMap(scene_->avatarNameAnimationMap());
		}
	}
}

void MainWindow::fileImportPattern()
{
	if (okToContinue()) {
		QString file_name = QFileDialog::getOpenFileName(this, tr("Import Pattern"),  ".", tr("Pattern files (*.dxf)"));

		if (!file_name.isEmpty()) {
			pattern_scene_->importPattern(file_name);
			pattern_scene_->update();
		}
	}
}

// wunf
void MainWindow::fileImportCloth()
{
	if (okToContinue()) {
		QString file_name = QFileDialog::getOpenFileName(this, tr("Import Cloth"),  ".", tr("Cloth files (*.obj)"));

		if (!file_name.isEmpty()) {
			obj_cloth_file_ = file_name;
			scene_->importCloth(file_name);
			simulation_view_->paintGL();
		}
	}
}

bool MainWindow::save()
{
	return true;
}

void MainWindow::updateSynthesizedAnimation( AnimationClip* clip, AnimationTrack* track)
{
	Q_UNUSED(clip);
	Q_UNUSED(track);

	if (animation_editor_ && scene_) {
		animation_editor_->updateSyntheticAnim(scene_->synthesizedAnimation());
		scene_->updateAvatarAnimation(0); // wunf 2014.3.31
		simulation_view_->paintGL();
	}
}

void MainWindow::toggleGridVisible()
{
	pattern_scene_->setGridVisible(design_showgrid_action_->isChecked());
	pattern_scene_->update();
}

void MainWindow::switchToRotateMode()
{
	scene_->setInteractionMode(Scene::ROTATE);
}

void MainWindow::switchToPanMode()
{
	scene_->setInteractionMode(Scene::PAN);
}

void MainWindow::switchToZoomMode()
{
	scene_->setInteractionMode(Scene::ZOOM);
}

void MainWindow::switchToClothRotateMode()
{
	scene_->setInteractionMode(Scene::CLOTH_ROTATE);
}

void MainWindow::switchToClothMoveMode()
{
	scene_->setInteractionMode(Scene::CLOTH_MOVE);
}

void MainWindow::switchToClothScaleMode()
{
	scene_->setInteractionMode(Scene::CLOTH_SCALE);
}

void MainWindow::switchToSelectMode()
{
	scene_->setInteractionMode(Scene::SELECT);
}

void MainWindow::renderingModeChanged( int index)
{
	switch (index)
	{
	case 0:
		scene_->setDisplayMode(Scene::SHADING);
		break;
	case 1:
		scene_->setDisplayMode(Scene::SKELETON);
		break;
	case 2:
		scene_->setDisplayMode(Scene::XRAY);
		break;
	case 3:
		break;
	default: 
		break;
	}
}

void MainWindow::startSimulate()
{
	simulation_view_->startSimulate();
}

void MainWindow::save_cm_file()
{
	
}

void MainWindow::open_cm_file()
{
	if (okToContinue()) {
		QString file_name = QFileDialog::getOpenFileName(this, tr("Open Cloth Motion File"),  ".", tr("Motion files (*.cmf)"));

		if (!file_name.isEmpty()) {
			cloth_motion_file_ = file_name;
			scene_->load_cm_file(file_name.toStdString().c_str());
		}
	}
}

void MainWindow::changeClothColor()
{
	if (okToContinue()) {
		QColor color = QColorDialog::getColor();
		QVector4D colorvec(color.redF(), color.greenF(), color.blueF(), color.alphaF());
		scene_->setClothColor(colorvec, 0);
		simulation_view_->paintGL();
	}
}

void MainWindow::changeClothTexture()
{
	if (okToContinue()) {
		QString file_name = QFileDialog::getOpenFileName(this, tr("Open Texture File"),  ".", tr("Picture files (*.jpg)"));

		if (!file_name.isEmpty()) {
			scene_->setClothTexture(file_name);
			simulation_view_->paintGL();
		}
	}
}

void MainWindow::toggleRecord()
{
	simulation_view_->record();
}

void MainWindow::fileOpen()
{
	if (okToContinue()) {
		QString file_name = QFileDialog::getOpenFileName(this, tr("Open Project File"),  ".", tr("Project files (*.prj)"));

		if (!file_name.isEmpty()) {
			project_file_ = file_name;
		}
	}

	std::ifstream prjfile(project_file_.toStdString());
	std::string tag, file;
	while(prjfile >> tag)
	{
		if(tag == "avatar")
		{
			prjfile >> file;
			avatar_file_ = QString::fromStdString(file);
			scene_->importAvatar(avatar_file_);
			animation_editor_->setAnimationModel(scene_->avatarAnimationModel());
			animation_editor_->setSkeletonModel(scene_->avatarSkeletonModel());
			animation_editor_->setNameAnimationMap(scene_->avatarNameAnimationMap());
		}
		else if(tag == "obj")
		{
			prjfile >> file;
			obj_cloth_file_ = QString::fromStdString(file);
			scene_->importCloth(obj_cloth_file_);
		}
		else if(tag == "cmf")
		{
			prjfile >> file;
			cloth_motion_file_ = QString::fromStdString(file);
			scene_->load_cm_file(cloth_motion_file_.toStdString().c_str());
		}
	}
	prjfile.close();
	simulation_view_->paintGL();
}

void MainWindow::fileSave()
{
	if(project_file_.isEmpty())
	{
		if (okToContinue()) {
			QString file_name = QFileDialog::getSaveFileName(this, tr("Save Project File"),  ".", tr("Project files (*.prj)"));

			if (!file_name.isEmpty()) {
				project_file_ = file_name;
			}
		}
	}

	std::ofstream prjfile(project_file_.toStdString());
	prjfile << "avatar " << avatar_file_.toStdString() << std::endl;
	if(cloth_motion_file_.isEmpty())
		cloth_motion_file_ = simulation_view_->getClothMotionFile();
	if(cloth_motion_file_.isEmpty())
	{
		if(!obj_cloth_file_.isEmpty())
			prjfile << "obj " << obj_cloth_file_.toStdString() << std::endl;
	}
	else
		prjfile << "cmf " << cloth_motion_file_.toStdString() << std::endl;

	prjfile.close();
}

void MainWindow::fileSaveAs()
{
	if (okToContinue()) {
		QString file_name = QFileDialog::getSaveFileName(this, tr("Save Project File"),  ".", tr("Project files (*.prj)"));

		if (!file_name.isEmpty()) {
			project_file_ = file_name;
		}
	}

	std::ofstream prjfile(project_file_.toStdString());
	prjfile << "avatar " << avatar_file_.toStdString() << std::endl;
	if(cloth_motion_file_.isEmpty())
		cloth_motion_file_ = simulation_view_->getClothMotionFile();
	if(cloth_motion_file_.isEmpty())
	{
		if(!obj_cloth_file_.isEmpty())
			prjfile << "obj " << obj_cloth_file_.toStdString() << std::endl;
	}
	else
		prjfile << "cmf " << cloth_motion_file_.toStdString() << std::endl;
	prjfile.close();
}
