#pragma once
#include "structure.h"

class Polygon
{
public:
	Polygon();
	Polygon(const QVector<QPoint> &polygon_);
	~Polygon();
	bool PointInThePolygon(const QPoint &point);

private:
	QVector<QPoint> polygon;
	int SignOfDet2x2(double x1, double y1, double x2, double y2);
};