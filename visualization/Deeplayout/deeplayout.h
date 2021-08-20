#pragma once

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <QTextStream>
#include <QPainter>
#include <QTime>  
#include "ui_deeplayout.h"
#include "polygon.h"
using namespace std;

class Deeplayout : public QMainWindow
{
	Q_OBJECT

public:
	Deeplayout(QWidget *parent = 0);
	~Deeplayout();

	void ReadImageData(const QImage &);
	void SaveTextData(const QString &);
	void SaveImageData(const QString &, bool textState=false);

	void HouseAbstract(void);
	void HouseAbstract_gt(void);
	void AddInteriorDoor(void);
	void AddWindow(void);
	ENTRY FrontDoorAbstract(const QVector<QVector<int>> &);
	QVector<DIRECTEDPOINT> BoundaryAbstract(const QVector<QVector<int>> &);
	void HoleAllocation(const QVector<QVector<int>> &, QVector<ROOM> &);
	QVector<QVector<DIRECTEDPOINT>> InteriorWallAbstract(const QVector<QVector<int>> &);
	QVector<QVector<int>> RoomMapAbstract(const QVector<QVector<int>> &, QPoint &, int direction = 0);
	QVector<QVector<int>> RoomMapAbstract_gt(const QVector<QVector<int>> &, QPoint &);
	QVector<QVector<int>> RoomMapExpand(const QVector<QVector<int>> &, const QVector<DIRECTEDPOINT> &, int expand);

	int ContactLength(const QPoint &, const QPoint &);
	QPoint IntersectLine(const QPoint &, const QPoint &);
	QPoint ContactLine(const QVector<QVector<int>> &, int location, QPoint inputLine, int direction = 0);
	QPoint IntersectLine(const QVector<QVector<int>> &, int location, QPoint inputLine, int direction = 0);
	QVector<QPoint> ScanLine(const QVector<QVector<int>> &, int location, int direction = 0, int scanValue = 255);
	QVector<DIRECTEDWALL> ContactWallForTwoBoundary(const QVector<DIRECTEDPOINT> &, const QVector<DIRECTEDPOINT> &);

private:
	Ui::DeeplayoutClass ui;
	QAction		*actionOpenData;
	QAction		*actionRenderData;
	QAction		*actionSaveData;
	QAction		*actionSaveImage;
	QAction		*actionBatchSave;
	QAction		*actionBatchChinese;
	QAction		*actionBatchCopy;

	DATA data; 
	MYHOUSE house;
	Polygon polygon;
	QImage layoutImage; 

	bool renderState;

	QPointF PointF2PointF(const QPointF &point)
	{
		return (QPointF(4 * point.x(), 4 * point.y()));
	}
	QLineF LineF2LineF(const QLineF &line)
	{
		return (QLineF(4 * line.x1(), 4 * line.y1(), 4 * line.x2(), 4 * line.y2()));
	}
	QRectF RectF2RectF(const QRectF &rect)
	{
		return (QRectF(4 * rect.x(), 4 * rect.y(), 4 * rect.width(), 4 * rect.height()));
	}

	QPointF Point2PointF(const QPoint &point)
	{
		return (QPointF(4 * point.x(), 4 * point.y()));
	}
	QLineF Line2LineF(const QLine &line)
	{
		return (QLineF(4 * line.x1(), 4 * line.y1(), 4 * line.x2(), 4 * line.y2()));
	}
	QRectF Rect2RectF(const QRect &rect)
	{
		return (QRectF(4 * rect.x(), 4 * rect.y(), 4 * rect.width(), 4 * rect.height()));
	}

public:
	void paintEvent(QPaintEvent *);
	void DrawExteriorWall(QPainter *, QVector<DIRECTEDPOINT> &);
	void DrawInteriorWall(QPainter *, QVector<QVector<DIRECTEDPOINT>> &);
	void DrawEntry(QPainter *, ENTRY &); 
	void DrawRoomEntry(QPainter *, ROOM &);
	void DrawRoom(QPainter *, ROOM &);
	void DrawText(QPainter *, ROOM &);
	void DrawWindow(QPainter *, DIRECTEDWALL &);
	void DrawArrow(QPainter *, DIRECTEDWALL &);

	void DrawLocation(QPainter *, ROOM &);
	void DrawWall(QPainter *);

	int GetRotatAngle(int dir);
	QRectF RotateRect(QRectF &, int angle = 0);
	DOOR GetDoor(QRectF &);
	DOOR GetAntiDoor(QRectF &);
	OPENWALL GetOpenWall(QRectF &);
	WINDOW GetWindow(QRectF &);
	ARROW GetArrow(QRectF &);

public slots:
	void Open();
	void Render();
	void SaveData();
	void SaveImage();
};