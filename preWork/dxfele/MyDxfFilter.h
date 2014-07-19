#pragma once

#include "..\dxf\dl_creationadapter.h"
#include "..\global\Global.h"
#include "DXFClothData.h"

class MyDxfFilter: public DL_CreationAdapter
{
public:
	int insertCounter;
	DXFClothPiece *currPiece;
	ClothDataPolyLine *currPolyLine;
	ClothDataLine *currLine;
	MyDxfFilter(void);

	virtual void addBlock(const DL_BlockData& data) {
		//TRACE("add block");
		DXFClothPiece* clothPiece = new DXFClothPiece;
		currPiece = clothPiece;
		Global::Instance()->dxfClothData->clothPieces.push_back(clothPiece);
	}
	virtual void endBlock() {
		//TRACE("end Block");
	}

	virtual void addLayer(const DL_LayerData& data){
		//TRACE("1\n");
		//TRACE("\n");
	};

	virtual void addEllipse(const DL_EllipseData& data) {
		//TRACE("A\n");
		//TRACE("\n");
	};

	virtual void addSpline(const DL_SplineData& data) {
		//TRACE("B\n");
		//TRACE("degree: %d,  flags: %d,  control:%d,  knots:%d", data.degree, data.flags, data.nControl, data.nKnots);
		//TRACE("\n");
	};

	virtual void addControlPoint(const DL_ControlPointData& data) {
		//TRACE("C");
		//TRACE("x:%f,  y:%f,  z:%f", data.x, data.y, data.z);
		//TRACE("\n");
	};

	virtual void addKnot(const DL_KnotData& data) {
		//TRACE("D");
		//TRACE("k:%f", data.k);
		//TRACE("\n");
	};
	
	virtual void addInsert(const DL_InsertData& data) {
		//TRACE("E");
		//TRACE("yi dui shu ju ya");
		//TRACE("\n");
		Global::Instance()->dxfClothData->clothPieces.at(insertCounter)->insertX = data.ipx;
		Global::Instance()->dxfClothData->clothPieces.at(insertCounter)->insertY = data.ipy;
		insertCounter++;
	};
	
	virtual void addMText(const DL_MTextData& data) {
		//TRACE("F");
		//TRACE("hai shi yi dui shu ju");
		//TRACE("\n");
	};

	virtual void addMTextChunk(const char* ch) {
		//TRACE("G");
		//TRACE("ch: %s", ch);
		//TRACE("\n");
	};

	virtual void addText(const DL_TextData& data) {
		//TRACE("H");
		//TRACE("ni zai wan wo ma        %s", data.text.c_str());
		//TRACE("\n");
		// 两种 
		if(data.text.at(0) == '#') {
			// add point逻辑
			// 一种是注释第一个点
//			TRACE("MEMEMEMEMEMEMEM......\n");
		} else {
			// 一种是Piece前的介绍式
			int pos = data.text.find_first_of(':');
			string preStr = data.text.substr(0, pos);
//			string posStr = data.text.substr(pos + 2, data.text.length() - pos - 2);
//			TRACE("I M HERE        %s", preStr.c_str());
//			TRACE("I M HERE TOO        %s", posStr.c_str());
//			TRACE("\n");
			if(preStr.compare("PIECE NAME") == 0) {
			}
			ClothNote *cnote = new ClothNote;
			cnote->x = data.apx;
			cnote->y = data.apy;
			cnote->text = data.text;
			currPiece->clothNote.push_back(cnote);
		}

	};
	
	virtual void addDimAlign(const DL_DimensionData& data1, const DL_DimAlignedData& data2) {
		//TRACE("I");
		//TRACE("\n");
	};

	virtual void addDimLinear(const DL_DimensionData& data1, const DL_DimLinearData& data2) {
		//TRACE("J");
	};

	virtual void addDimRadial(const DL_DimensionData& data1, const DL_DimRadialData& data2) {
		//TRACE("K");
	};

	virtual void addDimDiametric(const DL_DimensionData& data1, const DL_DimDiametricData& data2) {
		//TRACE("L");
	};

	virtual void addDimAngular(const DL_DimensionData& data1, const DL_DimAngularData& data2) {
		//TRACE("M");
	};

	virtual void addDimAngular3P(const DL_DimensionData& data1, const DL_DimAngular3PData& data2) {
		//TRACE("N");
	};

	virtual void addDimOrdinate(const DL_DimensionData& data1, const DL_DimOrdinateData& data2) {
		//TRACE("O");
	};

	virtual void addLeader(const DL_LeaderData& data) {
		//TRACE("P");
	};

	virtual void addLeaderVertex(const DL_LeaderVertexData& data) {
		//TRACE("Q");
	};
	
	virtual void addHatch(const DL_HatchData& data) {
		//TRACE("R");
	};

	virtual void addTrace(const DL_TraceData& data) {
		//TRACE("S");
	};
	virtual void addSolid(const DL_SolidData& data) {
		//TRACE("T");
	};
	
	virtual void addImage(const DL_ImageData& data) {
		//TRACE("U");
	};

	virtual void linkImage(const DL_ImageDefData& data) {
		//TRACE("V");
	};

	virtual void addHatchLoop(const DL_HatchLoopData& data) {
		//TRACE("W");
	};

	virtual void addHatchEdge(const DL_HatchEdgeData& data) {
		//TRACE("X");
	};

	virtual void addPoint(const DL_PointData& data){
		//TRACE("2\n");
		//TRACE("x: %f, y: %f  z: %f", data.x, data.y, data.z);
		//TRACE("\n");
	};
	virtual void addLine(const DL_LineData& data){
		//TRACE("3\n");
		//TRACE("x1: %f   x2: %f   y1: %f   y2: %f   z1: %f   z2: %f", data.x1, data.x2, data.y1, data.y2, data.z1, data.z2);
		//TRACE("\n");
		currPolyLine = NULL;
		currLine = NULL;
		currLine = new ClothDataLine;
		currLine->x1 =  data.x1;
		currLine->x2 =  data.x2;
		currLine->y1 =  data.y1;
		currLine->y2 =  data.y2;
		currPiece->lines.push_back(currLine);
	};
	virtual void addArc(const DL_ArcData& data){
		//TRACE("4\n");
		//TRACE("angele: %f", data.angle1);
		//TRACE("\n");
	};
	virtual void addCircle(const DL_CircleData& data){
		//TRACE("5\n");
		//TRACE("radius: %f", data.radius);
		//TRACE("\n");
	};
	virtual void addPolyline(const DL_PolylineData& data){
		insertCounter = 0;
		//TRACE("6\n");
		//TRACE("flags: %d,  m: %d, n: %d, number: %d", data.flags, data.m, data.n, data.number);
		//TRACE("\n");
		currPolyLine = NULL;
		currLine = NULL;
		currPolyLine = new ClothDataPolyLine;
		currPolyLine->flag = data.flags;
		currPiece->polyLines.push_back(currPolyLine);
	};
	virtual void addVertex(const DL_VertexData& data){
		//TRACE("7\n");
		//TRACE("bulge: %f, x:%f, y: %f, z: %f", data.bulge, data.x, data.y, data.z);
		//TRACE("\n");
		if(!currPolyLine->containsThePos(data.x, data.y)) {
			ClothVertex* vertex = new ClothVertex;
			vertex->x = (float) data.x;
			vertex->y = (float) data.y;
			vertex->z = (float) data.z;
			currPolyLine->vectexs.push_back(vertex);
		}
	};
	virtual void add3dFace(const DL_3dFaceData& data){
		//TRACE("8\n");
		//TRACE("thickness: %f, x: %f, y: %f, z: %f", data.thickness, data.x, data.y, data.z);
		//TRACE("\n");
	};
};

