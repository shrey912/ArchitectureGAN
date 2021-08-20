#pragma once
#include <iostream>
#include <QVector>
#include <QPoint>
#include <QRect>
#include <QImage>
using namespace std;

#define HALFDOORWIDTH 6
#define HALFMIDWINDOWWIDTH 9
#define HALFSMALLWINDOWWIDTH 5
#define HALFLARGEWINDOWWIDTH 12

struct LABEL
{
	int index;
	QString name;
	int type; //room type : 0(private) / 1(public)
	QImage floorTexture;
	LABEL()
	{
	}
	LABEL(int index_, QString name_, int type_, QString texture_)
	{
		index = index_;
		name = name_;
		type = type_;
		floorTexture = QImage("texture/" + texture_ + ".jpg");
	}
};

struct DIRECTEDPOINT
{
	int dir; //search direction : 0(right) / 1(down) / 2(left) / 3(up)
	QPoint point;
	DIRECTEDPOINT()
	{
		dir = -1;
	}
	DIRECTEDPOINT(int w_, int h_, int dir_)
	{
		dir = dir_;
		point = QPoint(w_, h_);
	}
};

struct DIRECTEDEDGE
{
	int dir; //search direction : 0(horizontal) / 1(vertical)
	int level;
	int minLevel;
	int maxLevel;
	DIRECTEDEDGE()
	{
		dir = -1;
	}
	DIRECTEDEDGE(QPoint point1_, QPoint point2_)
	{
		if (point1_.rx() == point2_.rx())
		{
			dir = 1;
			level = point1_.rx();
			minLevel = min(point1_.ry(), point2_.ry());
			maxLevel = max(point1_.ry(), point2_.ry());
		}
		else
		{
			dir = 0;
			level = point1_.ry();
			minLevel = min(point1_.rx(), point2_.rx());
			maxLevel = max(point1_.rx(), point2_.rx());
		}
	}
};

struct DIRECTEDWALL
{
	int dir; //orientation : 0(right) / 1(down) / 2(left) / 3(up)
	QRect rect;
	DIRECTEDWALL()
	{
		dir = -1;
		rect = QRect(0, 0, 0, 0);
	}
};

struct ENTRY
{
	int type; //door type : 0(door) / 1(open wall)
	DIRECTEDWALL entry;
	ENTRY()
	{
		type = -1;
	}
};

struct ROOM
{
	LABEL label;
	ENTRY entry;
	QPoint centroid;
	QVector<QVector<int>> map;
	QVector<DIRECTEDPOINT> boundary;
	QVector<DIRECTEDWALL> windows;
};

struct DATA
{
	int width;
	int height;
	int minW;
	int maxW;
	int minH;
	int maxH;
	QVector<QVector<int>> boundary_map;
	QVector<QVector<int>> interior_wall_map;
	QVector<QVector<int>> label_map;
	QVector<QVector<int>> inside_map;
};

struct MYHOUSE
{
	ENTRY front_door;
	QVector<DIRECTEDPOINT> exterior_wall;
	QVector<ROOM> rooms;
	QVector<QVector<DIRECTEDPOINT>> interior_wall;
};

struct DOOR
{
	QRectF rect1;
	QRectF rect2;
	QRectF rect3;
	QRectF rect4;
	QRectF rect5;
	QRectF rect6;
};

struct OPENWALL
{
	QRectF rect1;
	QVector<QLineF> lines1;
	QVector<QLineF> lines2;
};

struct WINDOW
{
	QRectF rect1;
	QRectF rect2;
	QRectF rect3;
	QRectF rect4;
	QRectF rect5;
	QVector<QLineF> lines;
};

struct ARROW
{
	QRectF rect;
	QVector<QPointF> triangle;
};

struct DIR
{
	int dir_int;
	QString dir_string;
	DIR()
	{
	}
	DIR(int dir_int_, QString dir_string_)
	{
		dir_int = dir_int_;
		dir_string = dir_string_;
	}
};

const DIR ORIENTED[] = { DIR(0, "Right"), DIR(1, "down"), DIR(2, "Left"), DIR(3, "Up") };

const LABEL ROOMLABEL[] = { LABEL(0, "LivingRoom", 1, "PublicArea"), LABEL(1, "MasterRoom", 0, "Bedroom"), 
                            LABEL(2, "Kitchen", 1, "FunctionArea"), LABEL(3, "Bathroom", 0, "FunctionArea"),
							LABEL(4, "DiningRoom", 1, "FunctionArea"), LABEL(5, "ChildRoom", 0, "Bedroom"),
							LABEL(6, "StudyRoom", 0, "Bedroom"), LABEL(7, "SecondRoom", 0, "Bedroom"), 
							LABEL(8, "GuestRoom", 0, "Bedroom"), LABEL(9, "Balcony", 1, "PublicArea"),
							LABEL(10, "Entrance", 1, "PublicArea"), LABEL(11, "Storage", 0, "PublicArea"), 
							LABEL(12, "Wall-in", 0, "PublicArea") };

extern QString English2Chinese(const QString &english);