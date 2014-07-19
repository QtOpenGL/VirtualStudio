// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// GarmentStudioDoc.cpp : CGarmentStudioDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "GarmentStudio.h"
#endif

#include "GarmentStudioDoc.h"

#include <propkey.h>

#include "DesignView.h"
#include "SimulationView.h"
//#include "GLUtility.h"
#include "GLEnabledView.h"
#include "StyleSleveeless.h"
#include "StyleTop.h"
#include "StyleSkirt.h"
#include "string"
#include "TextureProduce.h"
#include "global\Global.h"
#include "ele\ClothData.h"
#include "preWork\dxfele\MyDxfFilter.h"
#include "preWork\dxfele\DXFClothData.h"
#include "preWork\clothData\ClothLoop.h"
#include "preWork\clothData\ClothPolyline.h"
#include "preWork\clothData\ClothStraightLine.h"
#include "preWork\clothData\ClothPiece.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGarmentStudioDoc

IMPLEMENT_DYNCREATE(CGarmentStudioDoc, CDocument)

BEGIN_MESSAGE_MAP(CGarmentStudioDoc, CDocument)
	ON_COMMAND(ID_FILE_SAVE, &CGarmentStudioDoc::OnFileSave)
	//ON_COMMAND(ID_FILE_OPEN, &CGarmentStudioDoc::OnFileOpen)
	ON_COMMAND(ID_STYLE_SLEVEELESS_CHANGE, &CGarmentStudioDoc::OnStyleSleveelessChange)
	ON_COMMAND(ID_STYLE_TOP, &CGarmentStudioDoc::OnStyleTop)
	//ON_COMMAND(ID_STYLE_SKIRT, &CGarmentStudioDoc::OnStyleSkirt)
	ON_COMMAND(ID_BUTTON9, &CGarmentStudioDoc::OnButton9)
	ON_COMMAND(ID_RESETMOVE, &CGarmentStudioDoc::OnResetmove)
	ON_COMMAND(ID_REDPOINT1, &CGarmentStudioDoc::OnRedpoint)
	ON_COMMAND(ID_BROWNCHECK1, &CGarmentStudioDoc::OnBrowncheck)
	ON_COMMAND(ID_OLIVECHECK1, &CGarmentStudioDoc::OnOlivecheck)
	ON_COMMAND(ID_REDBLUE1, &CGarmentStudioDoc::OnRedblue)
	ON_COMMAND(ID_REDGHECK1, &CGarmentStudioDoc::OnRedgheck)
	ON_COMMAND(ID_BLUEGINGHAM1, &CGarmentStudioDoc::OnBluegingham)
	ON_COMMAND(ID_PINKPLAIT1, &CGarmentStudioDoc::OnPinkplait)
	ON_COMMAND(ID_FLORALBLACK1, &CGarmentStudioDoc::OnFloralblack)
	ON_COMMAND(ID_FLORALCOLOR1, &CGarmentStudioDoc::OnFloralcolor)
	//ON_COMMAND(ID_BLUEPLAIT, &CGarmentStudioDoc::OnBlueplait)
	ON_COMMAND(ID_HART1, &CGarmentStudioDoc::OnHart)
	ON_COMMAND(ID_RETCLOTH, &CGarmentStudioDoc::OnRetcloth)
	ON_COMMAND(ID_INITCLOTH, &CGarmentStudioDoc::OnInitcloth)
	ON_COMMAND(ID_ISSHOWEDGE, &CGarmentStudioDoc::OnIsshowedge)
	ON_COMMAND(ID_FILEINTOXDF, &CGarmentStudioDoc::OnFileintoxdf)
	ON_COMMAND(ID_MODELTOSTL, &CGarmentStudioDoc::OnModeltostl)
	ON_COMMAND(ID_MODEL_LESSEN, &CGarmentStudioDoc::OnModelLessen)
	//ON_COMMAND(ID_MODEL_ZOOM, &CGarmentStudioDoc::OnModelZoom)
	ON_COMMAND(ID_MODELZOOM, &CGarmentStudioDoc::OnModelzoom)
	ON_COMMAND(ID_BLUEPLAIT1, &CGarmentStudioDoc::OnBlueplait)
	
	ON_COMMAND(ID_DESIGN_STYLYSLEEVELESS, &CGarmentStudioDoc::OndesignStylysleeveless)
	//ON_COMMAND(ID_STYLETOP, &CGarmentStudioDoc::OnStyletop)
	ON_COMMAND(ID_DESIGN_STYLESHIRT, &CGarmentStudioDoc::OndesignStyleshirt)
	ON_COMMAND(ID_BUTTON10, &CGarmentStudioDoc::OnButton10)
	ON_COMMAND(ID_MODEL_MAN, &CGarmentStudioDoc::OnModelMan)
	ON_COMMAND(ID_MODEL_WOMAN, &CGarmentStudioDoc::OnModelWoman)
	ON_COMMAND(ID_MODEL_ANIMAL, &CGarmentStudioDoc::OnModelAnimal)
	ON_COMMAND(ID_BUTTON7, &CGarmentStudioDoc::OnButton7)
	//ON_COMMAND(ID_SKIRT_STYLE, &CGarmentStudioDoc::OnSkirtStyle)
	ON_COMMAND(ID_SHIRT_BUTTON, &CGarmentStudioDoc::OnShirtButton)
	ON_COMMAND(ID_MODEL_READ1, &CGarmentStudioDoc::OnModelRead)
	ON_COMMAND(ID_READ_TEXTURE, &CGarmentStudioDoc::OnReadTexture)
	
	ON_COMMAND(ID_MERGETEXTURE, &CGarmentStudioDoc::OnMergetexture)
	ON_COMMAND(ID_PREOP_READ_CLOTH_OBJ, &CGarmentStudioDoc::OnPreopReadClothObj)
	ON_COMMAND(ID_PREOP_DRAW_MODE, &CGarmentStudioDoc::OnPreopDrawMode)
	ON_COMMAND(ID_PREOP_RELATE_MODE, &CGarmentStudioDoc::OnPreopRelateMode)
	ON_COMMAND(ID_PREOP_SEWMODE, &CGarmentStudioDoc::OnPreopSewmode)
	ON_COMMAND(ID_PREOP_DRAW_CONTOUR, &CGarmentStudioDoc::OnPreopDrawContour)
	ON_COMMAND(ID_PREOP_DRAW_ANCHOR, &CGarmentStudioDoc::OnPreopDrawAnchor)
	ON_COMMAND(ID_PREOP_MOVE, &CGarmentStudioDoc::OnPreopMove)
	ON_COMMAND(ID_PREOP_SCALE, &CGarmentStudioDoc::OnPreopScale)
	ON_COMMAND(ID_PREOP_DELETE, &CGarmentStudioDoc::OnPreopDelete)
	ON_COMMAND(ID_PREOP_DIV, &CGarmentStudioDoc::OnPreopDiv)
	ON_COMMAND(ID_PREOP_FINISH, &CGarmentStudioDoc::OnPreopFinish)
	ON_COMMAND(ID_PREOP_ROTATE_X, &CGarmentStudioDoc::OnPreopRotateX)
	ON_COMMAND(ID_PREOP_ROTATE_Y, &CGarmentStudioDoc::OnPreopRotateY)
	ON_COMMAND(ID_PREOP_ROTATE_Z, &CGarmentStudioDoc::OnPreopRotateZ)
	ON_UPDATE_COMMAND_UI(ID_PREWORK_MOVEX, &CGarmentStudioDoc::OnUpdatePreworkMovex)
	ON_UPDATE_COMMAND_UI(ID_PREWORK_MOVEY, &CGarmentStudioDoc::OnUpdatePreworkMovey)
	ON_UPDATE_COMMAND_UI(ID_PERWORK_MOVEZ, &CGarmentStudioDoc::OnUpdatePerworkMovez)
	ON_COMMAND(ID_PREWORK_MOVEX, &CGarmentStudioDoc::OnPreworkMovex)
	ON_COMMAND(ID_PREWORK_MOVEY, &CGarmentStudioDoc::OnPreworkMovey)
	ON_COMMAND(ID_PERWORK_MOVEZ, &CGarmentStudioDoc::OnPerworkMovez)
	ON_COMMAND(ID_READ_CLOTH_DXF, &CGarmentStudioDoc::OnReadClothDxf)
END_MESSAGE_MAP()


// CGarmentStudioDoc 构造/析构

string modelname("models/man_15k.ply");
int iprob=-1;
CStyleSleveeless dlg1;
CStyleTop dlg2;
CStyleSkirt dlg3;
extern int ivalue;

CGarmentStudioDoc::CGarmentStudioDoc()
	: pAnalysis(0), is_tex_mouse_pos(false), m_texName(0),
	islider_active(-1), is_active_slider(false)
{
	// TODO: 在此添加一次性构造代码
	m_pViewDesign = NULL;
	m_pViewSimulation = NULL;
	isCoordXChecked = false;
	isCoordYChecked = false;
	isCoordZChecked = false;
}

CGarmentStudioDoc::~CGarmentStudioDoc()
{
	if (pAnalysis)
	{
		delete pAnalysis;
		pAnalysis = NULL;
	}
}

BOOL CGarmentStudioDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	iprob=0;
	ivalue=4;
	InitDocument();
	return TRUE;
}




// CGarmentStudioDoc 序列化

void CGarmentStudioDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
		/*gui_listner.Serialize( Com::CSerializer(m_projName.GetBuffer(),false ) );
		gui_listner.Solve_fromCad_InitValue();*/  
	}
	else
	{
		// TODO: 在此添加加载代码
		/*gui_listner.Serialize( Com::CSerializer(m_projName.GetBuffer(),true ) );*/
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CGarmentStudioDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CGarmentStudioDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:  strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CGarmentStudioDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CGarmentStudioDoc 诊断

#ifdef _DEBUG
void CGarmentStudioDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGarmentStudioDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CGarmentStudioDoc 命令


//BOOL CGarmentStudioDoc::OnOpenDocument(LPCTSTR lpszPathName)
//{
//	if (!CDocument::OnOpenDocument(lpszPathName))
//		return FALSE;
//
//	// TODO:  Add your specialized creation code here
//	//InitDocument();
//	//m_projName = lpszPathName;
//
//	return TRUE;
//}


//BOOL CGarmentStudioDoc::OnSaveDocument(LPCTSTR lpszPathName)
//{
//	// TODO: Add your specialized code here and/or call the base class
//	//m_projName = lpszPathName;
//	return CDocument::OnSaveDocument(lpszPathName);
//}



void CGarmentStudioDoc::InitDocument()
{
	m_pViewDesign = (CDesignView*)GetView(RUNTIME_CLASS(CDesignView));
	m_pViewSimulation = (CSimulationView*)GetView(RUNTIME_CLASS(CSimulationView));
	SetNewProblem();
}


void CGarmentStudioDoc::SetNewProblem()
{
	const unsigned int indprob[5] = {6,9,11,10, 0};
	unsigned int nprob = 6;  
	if( pAnalysis != 0 ){ 
		// 		::glutSetWindow(iwin_des);
		// 		::glutDetachMenu(GLUT_RIGHT_BUTTON);  
		// 		::glutDestroyMenu(imenu_right_click);
		delete pAnalysis; 
		pAnalysis = 0; 
	}
	
	pAnalysis = new CAnalysis2D_Cloth_Static();  
	gui_listner.SetAnalysisInitialize(pAnalysis,indprob[iprob]); 

	if( is_tex_mouse_pos ){ 
		pAnalysis->SetColor_FaceFEM(0.8,0.8,0.8); 
		gui_listner.SetColor_CadFace(0.8, 0.8, 0.8);        
		pAnalysis->SetTextureScale_FaceFEM(1);    
		gui_listner.SetTextureScale_CadFace(1);    
	}
	else{
		pAnalysis->SetColor_FaceFEM(1.0,1.0,1.0); 
		gui_listner.SetColor_CadFace(1, 1, 1);    
		//    pAnalysis->SetTextureScale_FaceFEM(1);
		//    gui_listner.SetTextureScale_CadFace(1);
		pAnalysis->SetTextureScale_FaceFEM(5);
		gui_listner.SetTextureScale_CadFace(5);    
	} 

	m_pViewDesign->imode_ope = 0; m_pViewDesign->SetNewProblem();
	m_pViewSimulation->imode_ope = 0; m_pViewSimulation->SetNewProblem();

	pAnalysis->SetIsLighting(m_pViewSimulation->is_lighting);
	pAnalysis->PerformStaticSolver();
	////////////////
	//iprob++;
	if( iprob == nprob-1 ){ iprob = 0; }
}

CView* CGarmentStudioDoc::GetView(CRuntimeClass* pClass)
{
	CView* pView;
	POSITION pos = GetFirstViewPosition();

	while (pos != NULL)
	{
		pView = GetNextView(pos);
		if (pView->IsKindOf(pClass))
			return pView;
	}

	return NULL;
}

void CGarmentStudioDoc::myGlutIdle()
{
	if( m_pViewSimulation->is_animation ){
		gui_listner.FollowMshToCad_ifNeeded();
		gui_listner.Solve_ifNeeded();
		if( pAnalysis->IsBlowUp() ){
			std::cout << "BlowUp" << std::endl;
			gui_listner.LoadTimeStamp();
		}
	}
}





void CGarmentStudioDoc::OnFileSave()
{
	// TODO: Add your command handler code here
	CFileDialog ldFile(FALSE,"*.gsp", "",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"*.gsp||", this->m_pViewDesign);
	ldFile.m_ofn.lpstrTitle="保存";
	ldFile.m_ofn.lpstrDefExt="gsp";//设置缺省扩展名
	CString filename;
	if(ldFile.DoModal()   ==   IDOK)
	{
		filename=ldFile.GetPathName();
		std::string str(filename.GetBuffer()); 
		filename.ReleaseBuffer();
		Com::CSerializer arch(Com::CSerializer(str,false));
		arch.ivalue=ivalue;
		arch.iprob=iprob;
		strcpy(arch.modelname,modelname.c_str());
		gui_listner.Serialize( arch );
		//gui_listner.Solve_fromCad_InitValue(); 

	}
}


//void CGarmentStudioDoc::OnFileOpen()
//{
//	// TODO: Add your command handler code here
//	CFileDialog fileDlg(TRUE,"*.gsp", "",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
//		"*.gsp||", this->m_pViewDesign);
//
//	fileDlg.m_ofn.lpstrTitle="打开";
//	fileDlg.m_ofn.lpstrDefExt="gsp";//设置缺省扩展名
//	CString filename;
//	if(fileDlg.DoModal()==IDOK)
//	{
//
//		//if (g_pMainFrame->m_pViewDesign != NULL && g_pMainFrame->m_pViewSimulation != NULL)
//		//{
//		//	g_pMainFrame->SetNewProblem();
//		//	// timer, make myGlutIdle run all the time
//		//	SetTimer(ID_IDLE_TIMER, 10, NULL);		
//		//}
//		/*	if(iprob==-1)
//		{
//		OnNewDocument();
//		}*/
//		filename=fileDlg.GetPathName();
//		std::string str(filename.GetBuffer());
//		filename.ReleaseBuffer();
//		//ofstream out(filename);
//		//out<<iprob<<endl;
//		Com::CSerializer arch(Com::CSerializer(str,true));
//		gui_listner.Serialize(arch);
//		if(iprob==-1)
//		{
//			iprob=arch.iprob;
//			InitDocument();
//		}
//		if(iprob!=arch.iprob)
//		{
//		iprob=arch.iprob;
//		//strcpy(modelname,arch.modelname);
//		modelname.clear();
//		modelname=arch.modelname;
//		modelname.erase(modelname.end()-1,modelname.end());
//		SetNewProblem();
//		}
//		Com::CSerializer arch1(Com::CSerializer(str,true));
//		gui_listner.Serialize(arch1);
//		gui_listner.Solve_fromCad_InitValue();
//		gui_listner.Solve_fromCad_InterpValue();
//	}
//	/*gui_listner.Serialize( Com::CSerializer("clothtest.gsp",true ) );
//	gui_listner.Solve_fromCad_InitValue();  */
//}


void CGarmentStudioDoc::OnStyleSleveelessChange()
{
	// TODO: Add your command handler code here
	iprob=0;
	Global::Instance()->isPreWorkFinished = true;
	SetNewProblem();
	
}


void CGarmentStudioDoc::OnStyleTop()
{
	// TODO: Add your command handler code here
	iprob=1;
	Global::Instance()->isPreWorkFinished = true;
	SetNewProblem();
}

//
//void CGarmentStudioDoc::OnStyleSkirt()
//{
//	// TODO: Add your command handler code here
//	iprob=2;
//	SetNewProblem();
//}


void CGarmentStudioDoc::OnButton9()
{
	// TODO: Add your command handler code here
	//SetNewProblem();
}


void CGarmentStudioDoc::OnResetmove()
{
	// TODO: Add your command handler code here
	SetNewProblem();
}




void CGarmentStudioDoc::OnRedpoint()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(0);
	m_pViewDesign->Initialize_OpenGL_Texture(0);
	ivalue=0;
}


void CGarmentStudioDoc::OnBrowncheck()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(1);
	m_pViewDesign->Initialize_OpenGL_Texture(1);
	ivalue=1;
}


void CGarmentStudioDoc::OnOlivecheck()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(2);
	m_pViewDesign->Initialize_OpenGL_Texture(2);
	ivalue=2;
}


void CGarmentStudioDoc::OnRedblue()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(3);
	m_pViewDesign->Initialize_OpenGL_Texture(3);
	ivalue=3;
}


void CGarmentStudioDoc::OnRedgheck()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(4);
	m_pViewDesign->Initialize_OpenGL_Texture(4);
	ivalue=4;
}


void CGarmentStudioDoc::OnBluegingham()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(5);
	m_pViewDesign->Initialize_OpenGL_Texture(5);
	ivalue=5;
}


void CGarmentStudioDoc::OnPinkplait()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(6);
	m_pViewDesign->Initialize_OpenGL_Texture(6);
	ivalue=6;
}


void CGarmentStudioDoc::OnFloralblack()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(7);
	m_pViewDesign->Initialize_OpenGL_Texture(7);
	ivalue=7;
}


void CGarmentStudioDoc::OnFloralcolor()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(8);
	m_pViewDesign->Initialize_OpenGL_Texture(8);
	ivalue=8;
}




void CGarmentStudioDoc::OnHart()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(10);
	m_pViewDesign->Initialize_OpenGL_Texture(10);
	ivalue=10;
}


void CGarmentStudioDoc::OnRetcloth()
{
	// TODO: Add your command handler code here
	gui_listner.Solve_fromCad_InitValue();
}



void CGarmentStudioDoc::OnInitcloth()
{
	// TODO: Add your command handler code here

	if( pAnalysis->GetMode()==CLOTH_INITIAL_LOCATION ){ 
		gui_listner.Solve_fromCad_InitValue();
		pAnalysis->PerformStaticSolver();
		//::glutSetWindow(iwin_des);  ::glutSetCursor(GLUT_CURSOR_WAIT);
		//::glutSetWindow(iwin_sim);  ::glutSetCursor(GLUT_CURSOR_WAIT);
	}
	else{ 
		pAnalysis->SetClothPiecePlacingMode();
	}
}




void CGarmentStudioDoc::OnIsshowedge()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->is_display_rotation = !m_pViewSimulation->is_display_rotation;
	if( m_pViewSimulation->is_display_rotation ){
		m_pViewSimulation->display_rotation_theta = -10;
		pAnalysis->SetIsDrawPatternBoundary(false);        
		{
			m_pViewSimulation->camera_r.SetRotationMode(Com::View::ROT_2DH);
			double rot[9];	
			m_pViewSimulation->camera_r.RotMatrix33(rot);
			Com::CBoundingBox3D bb = pAnalysis->GetBoundingBox(rot);
			m_pViewSimulation->camera_r.SetObjectBoundingBox(bb);
			m_pViewSimulation->camera_r.Fit();	
			m_pViewSimulation->camera_r.SetScale(1.8);
			////
			//::glutSetWindow(iwin_sim);
			::glMatrixMode(GL_PROJECTION);
			::glLoadIdentity();
			Com::View::SetProjectionTransform(m_pViewSimulation->camera_r);
			m_pViewSimulation->InvalidateRect(NULL, FALSE);
			//::glutPostRedisplay();
			//		drawer_coord.SetTrans(camera_r);
		}
		gui_listner.Enable_SolveCadChange(false);
	}
	else{
		pAnalysis->SetIsDrawPatternBoundary(true);        
		gui_listner.Enable_SolveCadChange(true);                
	}
}


void CGarmentStudioDoc::OnFileintoxdf()
{
	// TODO: Add your command handler code here
	gui_listner.File_WriteDXF("cloth_pattern.dxf", 4);
}




void CGarmentStudioDoc::OnModeltostl()
{
	// TODO: Add your command handler code here
	if( pAnalysis != 0 ){
		pAnalysis->WriteObjMeshSTL("object.stl", 100);
	}  
}


void CGarmentStudioDoc::OnModelLessen()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Lessen();
}




void CGarmentStudioDoc::OnModelzoom()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Zoom();
}


void CGarmentStudioDoc::OnBlueplait()
{
	// TODO: Add your command handler code here
	m_pViewSimulation->Initialize_OpenGL_Texture(9);
	m_pViewDesign->Initialize_OpenGL_Texture(9);
	ivalue=9;
}



void CGarmentStudioDoc::OndesignStylysleeveless()
{
	// TODO: Add your command handler code here
	if(IDOK==dlg1.DoModal())
	{
		//AfxMessageBox("do");
		iprob=0;
		SetNewProblem();
	}
}




void CGarmentStudioDoc::OndesignStyleshirt()
{
	// TODO: Add your command handler code here
	if(IDOK==dlg3.DoModal())
	{
		//afxmessagebox("do");
		iprob=2;
		Global::Instance()->isPreWorkFinished = true;
		SetNewProblem();
	}
}




void CGarmentStudioDoc::OnButton10()
{
	// TODO: Add your command handler code here
	if(IDOK==dlg2.DoModal())
	{
		//AfxMessageBox("do");
		iprob=1;
		SetNewProblem();
	}
}


void CGarmentStudioDoc::OnModelMan()
{
	// TODO: Add your command handler code here

	//char* tempname("models/man_15k.ply");
	//strcpy(modelname,tempname);
	Global::Instance()->isPreWorkFinished = true;
	string temp("models/man_15k.ply");
	modelname=temp;
	SetNewProblem();
	
}


void CGarmentStudioDoc::OnModelWoman()
{
	// TODO: Add your command handler code here
	string tempname("models/wm2_15k.ply");
	Global::Instance()->isPreWorkFinished = true;
	modelname=tempname;
	//strcpy(modelname,tempname);
	SetNewProblem();
	
}


void CGarmentStudioDoc::OnModelAnimal()
{
	// TODO: Add your command handler code here
	string tempname("models/wm2_15k.ply");
	Global::Instance()->isPreWorkFinished = true;
	modelname=tempname;
	
	//strcpy(modelname,tempname);
	SetNewProblem();
	
}


void CGarmentStudioDoc::OnButton7()
{
	// TODO: Add your command handler code here
	Global::Instance()->isPreWorkFinished = true;
	string tempname("models/wm2_15k.ply");

	modelname=tempname;
	//strcpy(modelname,tempname);
	SetNewProblem();
}


//void CGarmentStudioDoc::OnSkirtStyle()
//{
//	// TODO: Add your command handler code here
//	iprob=2;
//	SetNewProblem();
//}


void CGarmentStudioDoc::OnShirtButton()
{
	// TODO: Add your command handler code here
	iprob=2;
	SetNewProblem();
}


void CGarmentStudioDoc::OnModelRead()
{
	// TODO: Add your command handler code here

	CFileDialog dlg_read(TRUE,"ply","*.*");
	CString filename;
	if(dlg_read.DoModal() == IDOK)
	{
		filename=dlg_read.GetFileName();
		std::string str(filename.GetBuffer());
		modelname.clear();
		modelname+="models/";
		modelname+=str;
		//modelname.erase(modelname.end()-1,modelname.end());
		Global::Instance()->isPreWorkFinished = true;
		SetNewProblem();
	}


}


BOOL CGarmentStudioDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here

	string str(lpszPathName);
	m_pViewDesign = (CDesignView*)GetView(RUNTIME_CLASS(CDesignView));
	m_pViewSimulation = (CSimulationView*)GetView(RUNTIME_CLASS(CSimulationView));
	Com::CSerializer arch(Com::CSerializer(str,true));
	gui_listner.Serialize(arch);
	std::string logname=arch.m_file_name+"log";
	Com::CSerializer logarch(logname.c_str(),true);
	

	//if(iprob==-1 )
	//{
	//	iprob = arch.iprob;
	//	ivalue=arch.ivalue;
	//	modelname.clear();
	//	modelname=arch.modelname;
	//	modelname.erase(modelname.end()-1,modelname.end());
	//	m_pViewDesign = (CDesignView*)GetView(RUNTIME_CLASS(CDesignView));
	//	m_pViewSimulation = (CSimulationView*)GetView(RUNTIME_CLASS(CSimulationView));
	//	m_pViewDesign->Initialize_OpenGL_Texture(ivalue);
	//	m_pViewSimulation->Initialize_OpenGL_Texture(ivalue);
	//	SetNewProblem();
	//	
	//	}
	//	else if(iprob != arch.iprob)
	//	{
		iprob = arch.iprob;
		ivalue=arch.ivalue;
		modelname.clear();
		modelname=arch.modelname;
		modelname.erase(modelname.end()-1,modelname.end());
		
		m_pViewDesign->Initialize_OpenGL_Texture(ivalue);
		m_pViewSimulation->Initialize_OpenGL_Texture(ivalue);
		SetNewProblem();
		//}
		   //ivalue=arch.ivalue;
			Com::CSerializer arch1(Com::CSerializer(str,true));
			gui_listner.Serialize(arch1);
			gui_listner.Solve_fromCad_InitValue();
			gui_listner.Solve_fromCad_InterpValue();
			//gui_listner.Solve_fromCad_InterpValue(); 	
			//gui_listner.pAnalysis->InitDrawer();     
			
				//pAnalysis->PerformStaticSolver();
			//gui_listner.pAnalysis->InitDrawer();
			//double xs,ys,xe,ye;
			//while()

	return TRUE;
}


//void CGarmentStudioDoc::OnFileOpen()
	//{
	//	// TODO: Add your command handler code here
	//	CFileDialog fileDlg(TRUE,"*.gsp", "",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
	//		"*.gsp||", this->m_pViewDesign);
	//
	//	fileDlg.m_ofn.lpstrTitle="打开";
	//	fileDlg.m_ofn.lpstrDefExt="gsp";//设置缺省扩展名
	//	CString filename;
	//	if(fileDlg.DoModal()==IDOK)
	//	{
	//
	//		//if (g_pMainFrame->m_pViewDesign != NULL && g_pMainFrame->m_pViewSimulation != NULL)
	//		//{
	//		//	g_pMainFrame->SetNewProblem();
	//		//	// timer, make myGlutIdle run all the time
	//		//	SetTimer(ID_IDLE_TIMER, 10, NULL);		
	//		//}
	//		/*	if(iprob==-1)
	//		{
	//		OnNewDocument();
	//		}*/
	//		filename=fileDlg.GetPathName();
	//		std::string str(filename.GetBuffer());
	//		filename.ReleaseBuffer();
	//		//ofstream out(filename);
	//		//out<<iprob<<endl;
	//		Com::CSerializer arch(Com::CSerializer(str,true));
	//		gui_listner.Serialize(arch);
	//		if(iprob==-1)
	//		{
	//			iprob=arch.iprob;
	//			InitDocument();
	//		}
	//		if(iprob!=arch.iprob)
	//		{
	//		iprob=arch.iprob;
	//		//strcpy(modelname,arch.modelname);
	//		modelname.clear();
	//		modelname=arch.modelname;
	//		modelname.erase(modelname.end()-1,modelname.end());
	//		SetNewProblem();
	//		}
	//		Com::CSerializer arch1(Com::CSerializer(str,true));
	//		gui_listner.Serialize(arch1);
	//		gui_listner.Solve_fromCad_InitValue();
	//		gui_listner.Solve_fromCad_InterpValue();
	//	}
	//	/*gui_listner.Serialize( Com::CSerializer("clothtest.gsp",true ) );
	//	gui_listner.Solve_fromCad_InitValue();  */
	//}


void CGarmentStudioDoc::OnReadTexture()
{
	// TODO: Add your command handler code here
	CFileDialog lpszOpenFile(TRUE,"","",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"*.ppm||",this->m_pViewDesign);
	string texture_name = "";
	CString filename;
	lpszOpenFile.m_ofn.lpstrTitle="打开";
	lpszOpenFile.m_ofn.lpstrDefExt="ppm";//设置缺省扩展名
	if(lpszOpenFile.DoModal()==IDOK)
	{
		filename = lpszOpenFile.GetFileName();
		texture_name = filename.GetBuffer(0);
		this->m_pViewDesign->Initialize_OpenGL_Texture(-1,texture_name);
		this->m_pViewSimulation->Initialize_OpenGL_Texture(-1,texture_name);
	}
}





void CGarmentStudioDoc::OnMergetexture()
{
	// TODO: Add your command handler code here
	TextureProduce texturedlg;
	if(texturedlg.DoModal()==IDOK)
	{
		
		this->m_pViewDesign->Initialize_OpenGL_Texture(-1,string("tmp.ppm"));
		this->m_pViewSimulation->Initialize_OpenGL_Texture(-1,"tmp.ppm");
	}
}


void CGarmentStudioDoc::OnPreopReadClothObj()
{
	// TODO: Add your command handler code here
	CFileDialog lpszOpenFile(TRUE,"","",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"*.*||",this->m_pViewDesign);
	string obj_name = "";
	string fileType;
	CString filename;
	lpszOpenFile.m_ofn.lpstrTitle="打开";
//	lpszOpenFile.m_ofn.lpstrDefExt="ppm";//设置缺省扩展名
	if(lpszOpenFile.DoModal()==IDOK)
	{
		basepath = lpszOpenFile.GetFolderPath() + "\\";
		string fileFullPath = basepath + "\\";
		string fileName = lpszOpenFile.GetFileName();
		fileType = fileName.substr(fileName.find_last_of('.') + 1, 3);
		fileFullPath += lpszOpenFile.GetFileName();
		loadasset(fileFullPath.c_str());
		isOpen = true;

		loadGLTextures(scene);
		ClothData *clothData = new ClothData();
		int typeIndex = 0;
		if(fileType.compare("obj") == 0) {
			typeIndex = OBJ_TYPE;
			Global::Instance()->clothData->readClothData(scene, scene, scene->mRootNode, typeIndex);
		}
		Global::Instance()->hasFinishLoadObj = true;
		Global::Instance()->isPreWorkFinished = false;
		m_pViewDesign->Invalidate();
	}
}


void CGarmentStudioDoc::OnPreopDrawMode()
{
	// TODO: Add your command handler code here
	Global::Instance()->designMode = DrawMode;
	Global::Instance()->drawDesignOp = DrawContour;
}


void CGarmentStudioDoc::OnPreopRelateMode()
{
	// TODO: Add your command handler code here
	Global::Instance()->designMode = RelateMode;
}


void CGarmentStudioDoc::OnPreopSewmode()
{
	// TODO: Add your command handler code here
	Global::Instance()->designMode = SewMode;
}


void CGarmentStudioDoc::OnPreopDrawContour()
{
	// TODO: Add your command handler code here
	if(Global::Instance()->designMode == DrawMode) {
		Global::Instance()->drawDesignOp = DrawContour;
	}
}


void CGarmentStudioDoc::OnPreopDrawAnchor()
{
	// TODO: Add your command handler code here
	if(Global::Instance()->designMode == DrawMode) {
		Global::Instance()->drawDesignOp = DrawCenterPoint;
	}
}


void CGarmentStudioDoc::OnPreopMove()
{
	// TODO: Add your command handler code here
	if(Global::Instance()->designMode == DrawMode) {
		Global::Instance()->drawDesignOp = DrawMove;
	}
}


void CGarmentStudioDoc::OnPreopScale()
{
	// TODO: Add your command handler code here
	if(Global::Instance()->designMode == DrawMode) {
		Global::Instance()->drawDesignOp = DrawScale;
	}
}

int CGarmentStudioDoc::loadasset(const char *path) 
{
	scene = aiImportFile(path, aiProcessPreset_TargetRealtime_Quality);
	if(scene) {
		get_bounding_box(&scene_min, &scene_max);
	}
	return 1;
}

bool CGarmentStudioDoc::isOpen = false;

void CGarmentStudioDoc::get_bounding_box (aiVector3D* min, aiVector3D* max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode, min, max, &trafo);
}

void CGarmentStudioDoc::get_bounding_box_for_node (const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo)
{
	aiMatrix4x4 prev;
	unsigned int n = 0, t;
	prev = *trafo;
	aiMultiplyMatrix4(trafo, &nd->mTransformation);

	for(; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for(t = 0; t < mesh->mNumVertices; ++t) {
			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, trafo);
			min->x = min->x < tmp.x ? min->x : tmp.x;
			min->y = min->y < tmp.y ? min->y : tmp.y;
			min->z = min->z < tmp.z ? min->z : tmp.z;

			max->x = max->x > tmp.x ? max->x : tmp.x;
			max->y = max->y > tmp.y ? max->y : tmp.y;
			max->z = max->z > tmp.z ? max->z : tmp.z;

		}
	}
	for(n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n], min, max, trafo);
	}
	*trafo = prev;
}

void CGarmentStudioDoc::calBoundBox(DXFClothPiece *piece)
{
	float top = 0;
	float bottom = 0;
	float left = 0;
	float right = 0;
	for(int j = 0; j<piece->polyLines.size(); j++) {
		ClothDataPolyLine *polyline = piece->polyLines.at(j);
		if(polyline->flag == 0) {
			continue;
		}
		for(int k = 0; k<polyline->vectexs.size(); k++) {
			ClothVertex *vertex = polyline->vectexs.at(k);
			bottom = k == 0? vertex->y: bottom;
			top = k == 0? vertex->y: top;
			left = k == 0? vertex->x: left;
			right = k == 0? vertex->x: right;
			bottom = bottom > vertex->y? vertex->y: bottom;
			top = top < vertex->y? vertex->y: top;
			left = left > vertex->x? vertex->x: left;
			right = right < vertex->x? vertex->x: right;
		}
		break;
	}
	piece->bottom = bottom;
	piece->top = top;
	piece->left = left;
	piece->right = right;
}

void CGarmentStudioDoc::analysisClothPiece(DXFClothPiece *piece)
{
	ClothPiece* cPiece = new ClothPiece;
	for(int j = 0; j<piece->polyLines.size(); j++) {
		ClothDataPolyLine *polyline = piece->polyLines.at(j);
		if(polyline->flag == 0) {
			continue;
		}
		getClothContourLoop(polyline, &cPiece->outLoop, cPiece);
		cPiece->left = piece->left;
		cPiece->right = piece->right;
		cPiece->top = piece->top;
		cPiece->bottom = piece->bottom;
		cPiece->centerX = (cPiece->left + cPiece->right) / 2;
		cPiece->centerY = (cPiece->top + cPiece->bottom) / 2;
		break;
	}
	Global::Instance()->clothPieces.push_back(cPiece);
}

void CGarmentStudioDoc::getClothContourLoop(ClothDataPolyLine* polyline,  ClothLoop **outLoop, ClothPiece *fPiece)
{
// 这里需要把轮廓线 分割成线段和曲线 
	ClothVertex *startVertex, *currVertex1, *currVertex2, *nextVertex;
	ClothPolyline *currPolyline = NULL;
	ClothPolyline *startPolyline = NULL;
	startVertex = polyline->vectexs.at(0);
	currVertex1 = polyline->vectexs.at(0);
	for(int i = 0; i<polyline->vectexs.size(); i++) {
		currVertex2 = polyline->vectexs.at(i);
		if(currVertex2->index < 0) {
			currVertex2->index = i;
		}
		nextVertex = polyline->vectexs.at((i + 1) % polyline->vectexs.size());
		if(currPolyline == NULL) {
			currPolyline = new ClothPolyline;
			currPolyline->lineIndex = (*outLoop)->lines.size();
			currPolyline->fPiece = fPiece;
			(*outLoop)->lines.push_back(currPolyline);
			currPolyline->isPolyline = true;
			startPolyline = currPolyline;
			currPolyline->points.push_back(currVertex2);
			currPolyline->points.push_back(nextVertex);
			currPolyline->startVertex = currVertex2;
			currPolyline->endVertex = nextVertex;
		} else {
			float k1;
			int angle1 = -1;
			if(currVertex2->x != currVertex1->x) {
				k1 = (currVertex2->y - currVertex1->y) / (currVertex2->x - currVertex1->x);
			} else {
				angle1 = 10000;
			}
			float k2;
			int angle2 = -1;
			if(nextVertex->x != currVertex2->x) {
				k2 = (nextVertex->y - currVertex2->y) / (nextVertex->x - currVertex2->x);
			} else {
				angle2 = 10000;
			}
			float angle;
			if(angle1 == -1 && angle2 == -1) {
				float tanAng = abs((k2 - k1) / (1 + k1 * k2));
				angle = atan(tanAng);
			} else if(angle1 == 10000 && angle2 == 10000){
				angle = 0;
			} else if(angle1 == 10000 && angle2 == -1) {
				angle = atan(abs(1/k2));
			} else {
				angle = atan(abs(1/k1));
			}
			if(abs(angle) < 0.2f) {
				// currVertex1 和 currVertex2     与      currVertex2 和 nextVertex角度 小于阈值
				currPolyline->points.push_back(nextVertex);
				currPolyline->endVertex = nextVertex;
			} else {
				currPolyline = new ClothPolyline;
				currPolyline->lineIndex = (*outLoop)->lines.size();
				currPolyline->fPiece = fPiece;
				(*outLoop)->lines.push_back(currPolyline);
				currPolyline->isPolyline = true;
				currPolyline->points.push_back(currVertex2);
				currPolyline->points.push_back(nextVertex);
				currPolyline->startVertex = currVertex2;
				currPolyline->endVertex = nextVertex;
			}
		}
		currVertex1 = currVertex2;
		currVertex2 = nextVertex;

/*		nextVertex = polyline->vectexs.at((i + 1) % polyline->vectexs.size());
		if((nextVertex->x - currVertex->x) * (nextVertex->x - currVertex->x) + 
			(nextVertex->y - currVertex->y) * (nextVertex->y - currVertex->y) < 400) {
			// 添加到polyline
			if(currPolyline == NULL) {
				currPolyline = new ClothPolyline;
				currPolyline->isPolyline = true;
				if(currVertex == startVertex) {
					startPolyline = currPolyline;
				}
				currPolyline->points.push_back(currVertex);
				currPolyline->points.push_back(nextVertex);
				(*outLoop)->lines.push_back(currPolyline);
			} else {
				currPolyline->points.push_back(nextVertex);
			}
			
			if(nextVertex == startVertex) {
				// merge currPolyline & startPolyline
				if(startPolyline != NULL) {
					currPolyline->points.insert(currPolyline->points.end(), 
							startPolyline->points.begin(), startPolyline->points.end());
					(*outLoop)->lines.erase((*outLoop)->lines.begin());
				}
			}
		} else {
			// 添加到直线
			ClothStraightLine *straightLine = new ClothStraightLine;
			currPolyline = NULL;
			straightLine->isPolyline = false;
			straightLine->startPoint = currVertex;
			straightLine->endPoint = nextVertex;
			(*outLoop)->lines.push_back(straightLine);
		}
		currVertex = nextVertex;*/
	}
}

void CGarmentStudioDoc::preDealWithClothPieces()
{
	float biggestBoxEdge = 0;
	float scale = 1;
	float leftest = 0;
	float rightest = 0;
	float topest = 0;
	float bottomest = 0;
	for(int i = 0; i<Global::Instance()->clothPieces.size(); i++) {
		ClothPiece* cp = Global::Instance()->clothPieces.at(i);
		float length1 = cp->right - cp->left;
		float length2 = cp->top - cp->bottom;
		float length = length1 > length2? length1: length2;
		if(length > biggestBoxEdge) {
			biggestBoxEdge = length;
			scale = biggestBoxEdge;
		}
		leftest = i == 0? cp->left: leftest;
		rightest = i == 0? cp->right: rightest;
		topest = i == 0? cp->top: topest;
		bottomest = i == 0? cp->bottom: bottomest;

		leftest = cp->left < leftest? cp->left: leftest;
		rightest = cp->right > rightest? cp->right: rightest;
		topest = cp->top > topest? cp->top: topest;
		bottomest = cp->bottom < bottomest? cp->bottom: bottomest;
	}
	float centerX = (leftest + rightest) / 2;
	float centerY = (topest + bottomest) / 2;
//	for(int i = 0; i<Global::Instance()->clothPieces.size(); i++) {
//		Global::Instance()->clothPieces.at(i)->scale = scale * 2.0f;
//		Global::Instance()->clothPieces.at(i)->centerX = centerX;
//		Global::Instance()->clothPieces.at(i)->centerY = centerY;
//	}
	Global::Instance()->scale = scale * 2.0f;
	Global::Instance()->globalCenterX = centerX;
	Global::Instance()->globalCenterY = centerY;
}

int CGarmentStudioDoc::loadGLTextures(const aiScene* scene)
{
	ILboolean success;
	if(ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		ILint test = ilGetInteger(IL_VERSION_NUM);
		string err_msg = "DevIL version is too old";
		char* cErr_msg = (char *) err_msg.c_str();
		return -1;
	}
	ilInit();
	if(scene->HasTextures())
		return 0;
	for(unsigned int m = 0; m<scene->mNumMaterials; m++)
	{
		int texIndex = 0;
		aiReturn texFound = AI_SUCCESS;
		aiString path;
		while(texFound == AI_SUCCESS)
		{
			texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
			textureIdMap[path.data] = NULL;
			texIndex++;
		}
	}
	int numTextures = textureIdMap.size();
	ILuint* imageIds = NULL;
	imageIds = new ILuint[numTextures];
	ilGenImages(numTextures, imageIds);
	textureIds = new GLuint[numTextures];
	glGenTextures(numTextures, textureIds);
	map<string, GLuint*>::iterator itr = textureIdMap.begin();
	for(int i = 0; i<numTextures; i++)
	{
		string filename = (*itr).first;
		if(filename.size() <= 0)
			continue;
		(*itr).second = &textureIds[i];
		itr++;
		ilBindImage(imageIds[i]);
		string fileloc = basepath + filename.substr(2, filename.size());

		success = ilLoadImage(fileloc.c_str());

		if(success)
		{
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
			if(!success)
			{
				return -1;
			}
			glBindTexture(GL_TEXTURE_2D, textureIds[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), 
				ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, 
				ilGetData());
		}
		else 
		{

		}
	}
	ilDeleteImages(numTextures, imageIds);
	delete[] imageIds;
	imageIds = NULL;

	return TRUE;
}

void CGarmentStudioDoc::OnPreopDelete()
{
	// TODO: Add your command handler code here
	if(Global::Instance()->designMode == DrawMode) {
		Global::Instance()->drawDesignOp = DrawDelete;
	}
}


void CGarmentStudioDoc::OnPreopDiv()
{
	// TODO: Add your command handler code here
	if(Global::Instance()->designMode == DrawMode) {
		Global::Instance()->drawDesignOp = DrawDiv;
	}
}


void CGarmentStudioDoc::OnPreopFinish()
{
	// TODO: Add your command handler code here
	Global::Instance()->isPreWorkFinished = true;
	// 所有数据结构需要刷新一下 更新index
	int pSize = Global::Instance()->clothPatchs.size();
	for(int i = 0; i<pSize; i++) {
		Global::Instance()->clothPatchs.at(i)->updateIndex();
	}
	iprob = 4;
	SetNewProblem();
}

double CGarmentStudioDoc::getRotateValue(int type)
{

	int ID = ID_PREOP_ROTATE_X;
	switch(type) {
	case 0:
		ID = ID_PREOP_ROTATE_X;
		break;
	case 1:
		ID = ID_PREOP_ROTATE_Y;
		break;
	case 2:
		ID = ID_PREOP_ROTATE_Z;
		break;
	}
	CMFCRibbonBar *pRibbon = ((CMDIFrameWndEx*) AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);
	CMFCRibbonSlider *pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pRibbon->FindByID(ID));
	int position = pSlider->GetPos();
	return (double)position / (double) pSlider->GetRangeMax();
}

int CGarmentStudioDoc::getMoveCoord()
{
	CMFCRibbonBar *pRibbon = ((CMDIFrameWndEx*) AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);
	CMFCRibbonCheckBox *checkBoxX = DYNAMIC_DOWNCAST(CMFCRibbonCheckBox, pRibbon->FindByID(ID_PREWORK_MOVEX));
	CMFCRibbonCheckBox *checkBoxY = DYNAMIC_DOWNCAST(CMFCRibbonCheckBox, pRibbon->FindByID(ID_PREWORK_MOVEY));
	CMFCRibbonCheckBox *checkBoxZ = DYNAMIC_DOWNCAST(CMFCRibbonCheckBox, pRibbon->FindByID(ID_PERWORK_MOVEZ));

	int result = 0;
	if(checkBoxX->IsChecked()){
		result += 1;
	}
	if(checkBoxY->IsChecked()){
		result += 2;
	}
	if(checkBoxZ->IsChecked()){
		result += 4;
	}
	return result;
}

void CGarmentStudioDoc::OnPreopRotateX()
{
	// TODO: Add your command handler code here
}


void CGarmentStudioDoc::OnPreopRotateY()
{
	// TODO: Add your command handler code here
}


void CGarmentStudioDoc::OnPreopRotateZ()
{
	// TODO: Add your command handler code here
}

void CGarmentStudioDoc::OnUpdatePreworkMovex(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(isCoordXChecked);
}


void CGarmentStudioDoc::OnUpdatePreworkMovey(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(isCoordYChecked);
}


void CGarmentStudioDoc::OnUpdatePerworkMovez(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(isCoordZChecked);
}


void CGarmentStudioDoc::OnPreworkMovex()
{
	// TODO: Add your command handler code here
	isCoordXChecked = !isCoordXChecked;
}


void CGarmentStudioDoc::OnPreworkMovey()
{
	// TODO: Add your command handler code here
	isCoordYChecked = !isCoordYChecked;
}


void CGarmentStudioDoc::OnPerworkMovez()
{
	// TODO: Add your command handler code here
	isCoordZChecked = !isCoordZChecked;
}


void CGarmentStudioDoc::OnReadClothDxf()
{
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here
	CFileDialog lpszOpenFile(TRUE,"","",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"*.*||",this->m_pViewDesign);
	string obj_name = "";
	string fileType;
	CString filename;
	lpszOpenFile.m_ofn.lpstrTitle="打开";
//	lpszOpenFile.m_ofn.lpstrDefExt="ppm";//设置缺省扩展名
	if(lpszOpenFile.DoModal()==IDOK)
	{
		basepath = lpszOpenFile.GetFolderPath() + "\\";
		string fileFullPath = basepath + "\\";
		string fileName = lpszOpenFile.GetFileName();
		fileType = fileName.substr(fileName.find_last_of('.') + 1, 3);
		fileFullPath += lpszOpenFile.GetFileName();
		loadasset(fileFullPath.c_str());
		isOpen = true;

		loadGLTextures(scene);
		ClothData *clothData = new ClothData();
		int typeIndex = 0;
		if(fileType.compare("dxf") == 0) {
			typeIndex = DXF_TYPE;
			Global::Instance()->dxfClothData = new DXFClothData;
			DL_Dxf *dxf = new DL_Dxf();
			MyDxfFilter *myDxfFilter = new MyDxfFilter();
			if(!dxf->in(fileFullPath, myDxfFilter)) {
				delete Global::Instance()->dxfClothData;
				Global::Instance()->dxfClothData = NULL;
			}
			Global::Instance()->clothPieces.clear();
			for(int i = 0; i<Global::Instance()->dxfClothData->clothPieces.size(); i++) {
				DXFClothPiece *piece = Global::Instance()->dxfClothData->clothPieces.at(i);
				calBoundBox(piece);
				analysisClothPiece(piece);
			}
			preDealWithClothPieces();
		}
		
		Global::Instance()->hasFinishLoadObj = true;
		Global::Instance()->isPreWorkFinished = false;
		m_pViewDesign->Invalidate();
	}
}
