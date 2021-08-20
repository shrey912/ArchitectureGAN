#include "deeplayout.h"

Deeplayout::Deeplayout(QWidget *parent): QMainWindow(parent)
{
	ui.setupUi(this);

	actionOpenData = new QAction(tr("&Open"), this);
	actionOpenData->setStatusTip(tr("Open an existing file"));
	connect(actionOpenData, SIGNAL(triggered()), this, SLOT(Open()));

	actionRenderData = new QAction(tr("&Render"), this);
	actionRenderData->setStatusTip(tr("Render the result "));
	connect(actionRenderData, SIGNAL(triggered()), this, SLOT(Render()));

	actionSaveData = new QAction(tr("&SaveData"), this);
	actionSaveData->setStatusTip(tr("Save the data to disk"));
	connect(actionSaveData, SIGNAL(triggered()), this, SLOT(SaveData()));

	actionSaveImage = new QAction(tr("&SaveImage"), this);
	actionSaveImage->setStatusTip(tr("Save the image to disk"));
	connect(actionSaveImage, SIGNAL(triggered()), this, SLOT(SaveImage()));


	ui.mainToolBar->addAction(actionOpenData);
	ui.mainToolBar->addAction(actionRenderData);
	ui.mainToolBar->addAction(actionSaveData);
	ui.mainToolBar->addAction(actionSaveImage);

    renderState = false;

	/*********************************** BatchSave ***********************************/
	/*QDir dataDir("synth_normalization");
	QStringList nameFilters;
	nameFilters << "*.png";
	QStringList dataFiles = dataDir.entryList(nameFilters);
	for (int i = 0; i < dataFiles.size(); i++)
	{
		cout << dataFiles[i].toLatin1().data() << endl;
		QString fileName = dataFiles[i];
		QString inputFileName = "synth_normalization/" + fileName;
		fileName.truncate(fileName.lastIndexOf("."));

		ReadImageData(QImage(inputFileName));
		HouseAbstract();
		SaveTextData("render_normalization/" + fileName + ".txt");
		SaveImageData("render_normalization/" + fileName + ".png", false);
		SaveImageData("render_normalization/" + fileName + "_label" + ".png", true);
	}
	cout << "BatchSave Finished !!!" << endl;*/
	/*********************************************************************************/
}

Deeplayout::~Deeplayout()
{

}

void Deeplayout::Open()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "./data", tr("Files(*.png)"));
	if (!fileName.isEmpty())
	{
		layoutImage.load(fileName);
		ReadImageData(layoutImage);
		renderState = false;
	}
	update();
}

void Deeplayout::Render()
{
	if (layoutImage.isNull())
	{
		return;
	}
	HouseAbstract();
	layoutImage = QImage(1024, 1024, QImage::Format_RGBA8888);
	//layoutImage.fill(Qt::white);
	layoutImage.fill(Qt::transparent);
	renderState = true;
	update();
}

void Deeplayout::SaveTextData(const QString & filename)
{
	QFile file(filename);
	file.open(QIODevice::Text | QIODevice::WriteOnly);
	QTextStream stream(&file);

	stream << "FrontDoor" << endl;
	stream << "{" << endl;
	int dir = house.front_door.entry.dir;
	QRect rect = house.front_door.entry.rect;
	stream << rect.x() << " " << rect.y() << " " << rect.width() << " " << rect.height() << " " << ORIENTED[dir].dir_string << endl;
	stream << "}" << endl;
	stream << endl;

	stream << "ExteriorWall" << endl;
	stream << "{" << endl;
	for (int i = 0; i < house.exterior_wall.size(); i++)
	{
		DIRECTEDPOINT point = house.exterior_wall[i];
		stream << point.point.rx() << " " << point.point.ry() << endl;
	}
	stream << "}" << endl;
	stream << endl;

	stream << "InteriorWall" << endl;
	stream << "{" << endl;
	for (int i = 0; i < house.interior_wall.size(); i++)
	{
		stream << endl;
		stream << "{" << endl;
		QVector<DIRECTEDPOINT> points = house.interior_wall[i];
		for (int j = 0; j < points.size(); j++)
		{
			DIRECTEDPOINT point = points[j];
			stream << point.point.rx() << " " << point.point.ry() << endl;
		}
		stream << "}" << endl;
	}
	stream << "}" << endl;
	stream << endl;

	stream << "Room" << endl;
	stream << "{" << endl;
	for (int i = 0; i < house.rooms.size(); i++)
	{
		stream << endl;
		ROOM room = house.rooms[i];
		stream << room.label.name << endl;
		stream << "{" << endl;

		stream << "Boundary" << endl;
		stream << "{" << endl;
		for (int j = 0; j < room.boundary.size(); j++)
		{
			DIRECTEDPOINT point = room.boundary[j];
			stream << point.point.rx() << " " << point.point.ry() << endl;
		}
		stream << "}" << endl;

		if (room.label.name != "LivingRoom")
		{
			int type = room.entry.type;
			int dir = room.entry.entry.dir;
			QRect rect = room.entry.entry.rect;
			if (type == 0)
			{
				stream << "Door" << endl;
				stream << "{" << endl;
				stream << rect.x() << " " << rect.y() << " " << rect.width() << " " << rect.height() << " " << ORIENTED[dir].dir_string << endl;
				stream << "}" << endl;
			}
			else
			{
				if (type == 1)
				{
					stream << "OpenWall" << endl;
					stream << "{" << endl;
					stream << rect.x() << " " << rect.y() << " " << rect.width() << " " << rect.height() << " " << ORIENTED[dir].dir_string << endl;
					stream << "}" << endl;
				}
			}
		}

		stream << "Window" << endl;
		stream << "{" << endl;
		for (int j = 0; j < room.windows.size(); j++)
		{
			DIRECTEDWALL window = room.windows[j];
			int dir = window.dir;
			QRect rect = window.rect;
			stream << rect.x() << " " << rect.y() << " " << rect.width() << " " << rect.height() << " " << ORIENTED[dir].dir_string << endl;
		}
		stream << "}" << endl;
	}
	stream << "}" << endl;
	stream << endl;
	file.close();
}

void Deeplayout::SaveImageData(const QString &filename, bool textState)
{
	layoutImage = QImage(1024, 1024, QImage::Format_RGBA8888);
	//layoutImage.fill(Qt::white);
	layoutImage.fill(Qt::transparent);
	QPainter *painter = new QPainter();
	painter->begin(&layoutImage);
	DrawInteriorWall(painter, house.interior_wall);
	DrawExteriorWall(painter, house.exterior_wall);
	for (int i = 0; i < house.rooms.size(); i++)
	{
		DrawRoom(painter, house.rooms[i]);
	}
	DrawEntry(painter, house.front_door);

	for (int i = 0; i < house.rooms.size(); i++)
	{
		DrawEntry(painter, house.rooms[i].entry);
		QVector<DIRECTEDWALL> windows = house.rooms[i].windows;
		for (int j = 0; j < windows.size(); j++)
		{
			DrawWindow(painter, windows[j]);
		}
		if (textState)
		{
			DrawText(painter, house.rooms[i]);
		}
	}
	painter->end();
	layoutImage.save(filename);
}

void Deeplayout::SaveData()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "./data", tr("Files(*.txt)"));
	if (!fileName.isNull())
	{
		SaveTextData(fileName);
	}
	update();
}

void Deeplayout::SaveImage()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "./data", tr("Images(*.png)"));
	if (!fileName.isNull())
	{
		layoutImage.save(fileName);
	}
	update();
}

void Deeplayout::paintEvent(QPaintEvent *)
{
	if (layoutImage.isNull())
	{
		return;
	}

	if (renderState)
	{
		QPainter *painter = new QPainter();
		painter->begin(&layoutImage);

		//DrawWall(painter);
		DrawInteriorWall(painter, house.interior_wall);
		DrawExteriorWall(painter, house.exterior_wall);
		for (int i = 0; i < house.rooms.size(); i++)
		{
			DrawRoom(painter, house.rooms[i]);
		}
		for (int i = 0; i < house.rooms.size(); i++)
		{
			DrawRoomEntry(painter, house.rooms[i]);
			QVector<DIRECTEDWALL> windows = house.rooms[i].windows;
			for (int j = 0; j < windows.size(); j++)
			{
				DrawWindow(painter, windows[j]);
			}
			//DrawText(painter, house.rooms[i]);
		}
		/*for (int i = 0; i < house.rooms.size(); i++)
		{
			DrawLocation(painter, house.rooms[i]);
		}*/
		DrawEntry(painter, house.front_door);
		painter->end();
	}
	QPainter *globelPainter = new QPainter();
	globelPainter->begin(this);
	globelPainter->drawImage(width() / 2 - layoutImage.width() / 2, height() / 2 + 30 - layoutImage.height() / 2, layoutImage);
	globelPainter->end();
}

void Deeplayout::DrawWall(QPainter *painter)
{
	painter->setPen(QPen(Qt::blue));
	painter->setBrush(QBrush(Qt::blue));
	for (int w = data.minW; w < data.maxW; w++)
	{
		for (int h = data.minH; h < data.maxH; h++)
		{
			if (data.interior_wall_map[w][h] > 0)
			{
				painter->drawRect(Rect2RectF(QRect(w, h, 1, 1)));
			}
		}
	}
}

void Deeplayout::DrawLocation(QPainter *painter, ROOM &room)
{
	painter->setPen(QPen(Qt::red));
	painter->setBrush(QBrush(Qt::red));
	painter->drawEllipse(Point2PointF(room.centroid), 7, 7);

	/*QFont font = painter->font();
	int fontSize = 12;
	font.setBold(true);
	font.setPointSize(fontSize);
	painter->setFont(font);
	int w = room.centroid.rx();
	int h = room.centroid.ry();
	int nameSize = room.label.name.size();
	QRectF rect(w - fontSize * nameSize / 2, h, fontSize * nameSize, fontSize);
	painter->drawText(RectF2RectF(rect), Qt::AlignCenter, QString("%1").arg(room.label.name));*/
}

void Deeplayout::DrawRoom(QPainter *painter, ROOM &room)
{
	painter->setBrush(QBrush(room.label.floorTexture));
	QVector<DIRECTEDPOINT> boundary = room.boundary;
	QVector<QPointF> boundaryPoints;
	for (int i = 0; i < boundary.size(); i++)
	{
		boundaryPoints.push_back(Point2PointF(boundary[i].point));
	}
	painter->drawPolygon(boundaryPoints);
}

void Deeplayout::DrawText(QPainter *painter, ROOM &room)
{
	QFont font = painter->font();
	int fontSize = 12;
	font.setBold(true);
	font.setPointSize(fontSize);
	painter->setFont(font);

	int minW = 1e8;
	int maxW = 0;
	int minH = 1e8;
	int maxH = 0;
	for (int i = 0; i < room.boundary.size(); i++)
	{
		QPoint point = room.boundary[i].point;
		if (minW > point.rx())
		{
			minW = point.rx();
		}
		if (maxW < point.rx())
		{
			maxW = point.rx();
		}
		if (minH > point.ry())
		{
			minH = point.ry();
		}
		if (maxH < point.ry())
		{
			maxH = point.ry();
		}
	}

	int nameSize = room.label.name.size();
	QRectF rect(minW, minH, maxW - minW, maxH - minH);
	painter->drawText(RectF2RectF(rect), Qt::AlignCenter, QString("%1").arg(room.label.name));
}

DOOR Deeplayout::GetDoor(QRectF &rect)
{
	QRectF rect1(rect);
	QRectF rect2(rect.x(), rect.y(), rect.height() / 4, rect.height());
	QRectF rect3(rect.x() + rect.width() - rect.height() / 4, rect.y(), rect.height() / 4, rect.height());
	QRectF rect4(rect.x() + rect.height() / 4, rect.y() + 3 * rect.height() / 8, rect.width() - rect.height() / 2, rect.height() / 4);
	QRectF rect5(rect.x() + rect.width() - rect.height() / 4 - rect.height() / 4, rect.y() - (rect.width() - rect.height() / 2), rect.height() / 4, rect.width() - rect.height() / 2);
	QRectF rect6(rect.x() + rect.height() / 4, rect.y() - (rect.width() - rect.height() / 2), 2 * (rect.width() - rect.height() / 2), 2 * (rect.width() - rect.height() / 2));

	DOOR doorShape;
	doorShape.rect1 = rect1;
	doorShape.rect2 = rect2;
	doorShape.rect3 = rect3;
	doorShape.rect4 = rect4;
	doorShape.rect5 = rect5;
	doorShape.rect6 = rect6;

	return(doorShape);
}

DOOR Deeplayout::GetAntiDoor(QRectF &rect)
{
	QRectF rect1(rect);
	QRectF rect2(rect.x(), rect.y(), rect.height() / 4, rect.height());
	QRectF rect3(rect.x() + rect.width() - rect.height() / 4, rect.y(), rect.height() / 4, rect.height());
	QRectF rect4(rect.x() + rect.height() / 4, rect.y() + 3 * rect.height() / 8, rect.width() - rect.height() / 2, rect.height() / 4);
	QRectF rect5(rect.x() + rect.height() / 4, rect.y() - (rect.width() - rect.height() / 2), rect.height() / 4, rect.width() - rect.height() / 2);
	QRectF rect6(rect.x() + rect.height() / 4 - (rect.width() - rect.height() / 2), rect.y() - (rect.width() - rect.height() / 2), 2 * (rect.width() - rect.height() / 2), 2 * (rect.width() - rect.height() / 2));

	DOOR doorShape;
	doorShape.rect1 = rect1;
	doorShape.rect2 = rect2;
	doorShape.rect3 = rect3;
	doorShape.rect4 = rect4;
	doorShape.rect5 = rect5;
	doorShape.rect6 = rect6;

	return(doorShape);
}

ARROW Deeplayout::GetArrow(QRectF &rect)
{
	QRectF arrowRect(rect.x() + rect.width() / 2 - rect.width() / 8, rect.y() + 3 * rect.height(), rect.width() / 4, rect.width() / 2);
	QVector<QPointF> arrowTriangle;
	arrowTriangle.push_back(QPointF(rect.x() + rect.width() / 2, rect.y() + 2 * rect.height()));
	arrowTriangle.push_back(QPointF(rect.x() + rect.width() / 2 + rect.width() / 4, rect.y() + 3 * rect.height()));
	arrowTriangle.push_back(QPointF(rect.x() + rect.width() / 2 - rect.width() / 4, rect.y() + 3 * rect.height()));
	ARROW arrow;
	arrow.rect = arrowRect;
	arrow.triangle = arrowTriangle;

	return(arrow);
}

OPENWALL Deeplayout::GetOpenWall(QRectF &rect)
{
	QRectF rect1(rect);
	QVector<QLineF> lines1;
	QVector<QLineF> lines2;
	int step = 10;
	int numStep = rect.width() / step;
	if (numStep % 2 == 0)
	{
		numStep --;
	}
	float delta = (rect.width() - numStep*step) / 2;

	float x = rect.x() + delta;
	for (int i = 0; i < (numStep + 1) / 2; i++)
	{
		lines1.push_back(QLineF(x, rect.y(), x + step, rect.y()));
		lines2.push_back(QLineF(x, rect.y() + rect.height(), x + step, rect.y() + rect.height()));
		x = x + 2 * step;
	}

	OPENWALL openWall;
	openWall.rect1 = rect1;
	openWall.lines1 = lines1;
	openWall.lines2 = lines2;

	return(openWall);
}

WINDOW Deeplayout::GetWindow(QRectF &rect)
{
	QRectF rect1(rect);
	QRectF rect2(rect.x(), rect.y(), rect.height() / 4, rect.height());
	QRectF rect3(rect.x() + rect.width() - rect.height() / 4, rect.y(), rect.height() / 4, rect.height());
	QRectF rect4(rect.x() + rect.height() / 4, rect.y(), rect.width() - rect.height() / 2, rect.height() / 4);
	QRectF rect5(rect.x() + rect.height() / 4, rect.y() + rect.height() / 4, rect.width() - rect.height() / 2, rect.height() / 4);
	QVector<QLineF> Lines;
	QPointF p1(rect.x() - rect.height() / 3, rect.y() + rect.height());
	QPointF p2(rect.x() - rect.height() / 3, rect.y() + rect.height() + rect.height() / 3);
	QPointF p3(rect.x() + rect.width() + rect.height() / 3, rect.y() + rect.height() + rect.height() / 3);
	QPointF p4(rect.x() + rect.width() + rect.height() / 3, rect.y() + rect.height());
	Lines.push_back(QLineF(p1, p2));
	Lines.push_back(QLineF(p2, p3));
	Lines.push_back(QLineF(p3, p4));

	WINDOW windowShape;
	windowShape.rect1 = rect1;
	windowShape.rect2 = rect2;
	windowShape.rect3 = rect3;
	windowShape.rect4 = rect4;
	windowShape.rect5 = rect5;
	windowShape.lines = Lines;

	return(windowShape);
}


int Deeplayout::GetRotatAngle(int dir)
{
	switch (dir)
	{
		case 0:
		{
			return(270);
		}
		case 1:
		{
			return(180);
		}	
		case 2:
		{
			return(90);
		}	
		case 3:
		{
			return(0);
		}
		default:
		{
			return(0);
		}
	}
}

QRectF Deeplayout::RotateRect(QRectF &rect, int angle)
{
	switch (angle)
	{
		case 0:
		{
			return(rect);
		}
		case 90:
		{
			return(QRectF(rect.x() - rect.height(), rect.y(), rect.height(), rect.width()));
		
		}	
		case 180:
		{
			return(QRectF(rect.x() - rect.width(), rect.y() - rect.height(), rect.width(), rect.height()));
		}
		case 270:
		{
			return(QRectF(rect.x(), rect.y() - rect.width(), rect.height(), rect.width()));
		}
		default:
		{
		    return(rect);
		}
	}
}


void Deeplayout::DrawArrow(QPainter *painter, DIRECTEDWALL &frontDoor)
{
	painter->setBrush(Qt::red);
	painter->setPen(QPen(Qt::red));
	int dir = frontDoor.dir;
	int rotateAngle = GetRotatAngle(dir);
	QRectF rect = RectF2RectF(frontDoor.rect);
	QRectF rectRotated = RotateRect(rect, rotateAngle);
	ARROW arrow = GetArrow(rectRotated);

	painter->translate(rect.topLeft());
	painter->rotate(-rotateAngle);
	painter->translate(-rect.topLeft());

	painter->drawRect(arrow.rect);
	painter->drawPolygon(arrow.triangle);

	painter->translate(rect.topLeft());
	painter->rotate(rotateAngle);
	painter->translate(-rect.topLeft());
}

void Deeplayout::DrawEntry(QPainter *painter, ENTRY &entry)
{
	painter->setBrush(Qt::white);
	painter->setPen(QPen(Qt::white));
	if (entry.type == 0)
	{
		DIRECTEDWALL door = entry.entry;
		int dir = door.dir;
		int rotateAngle = GetRotatAngle(dir);
		QRectF rect = RectF2RectF(door.rect);
		QRectF rectRotated = RotateRect(rect, rotateAngle);
		DOOR doorShape = GetDoor(rectRotated);

		painter->translate(rect.topLeft());
		painter->rotate(-rotateAngle);
		painter->translate(-rect.topLeft());

		painter->drawRect(doorShape.rect1);
		painter->setPen(QPen(Qt::black));
		painter->drawRect(doorShape.rect2);
		painter->drawRect(doorShape.rect3);
		painter->drawRect(doorShape.rect4);
		painter->drawRect(doorShape.rect5);
		painter->drawArc((doorShape.rect6), 90 * 16, 90 * 16);

		painter->translate(rect.topLeft());
		painter->rotate(rotateAngle);
		painter->translate(-rect.topLeft());
	}
	if (entry.type == 1)
	{
		DIRECTEDWALL door = entry.entry;
		int dir = door.dir;
		int rotateAngle = GetRotatAngle(dir);
		QRectF rect = RectF2RectF(door.rect);
		QRectF rectRotated = RotateRect(rect, rotateAngle);
		OPENWALL openWallShape = GetOpenWall(rectRotated);

		painter->translate(rect.topLeft());
		painter->rotate(-rotateAngle);
		painter->translate(-rect.topLeft());

		painter->drawRect(openWallShape.rect1);
		painter->setPen(QPen(Qt::black));
		painter->drawLines(openWallShape.lines1);
		painter->drawLines(openWallShape.lines2);

		painter->translate(rect.topLeft());
		painter->rotate(rotateAngle);
		painter->translate(-rect.topLeft());
	}
}

void Deeplayout::DrawWindow(QPainter *painter, DIRECTEDWALL &window)
{
	painter->setBrush(Qt::white);
	painter->setPen(QPen(Qt::white));
	int dir = window.dir;
	int rotateAngle = GetRotatAngle(dir);
	QRectF rect = RectF2RectF(window.rect);
	QRectF rectRotated = RotateRect(rect, rotateAngle);
	WINDOW windowShape = GetWindow(rectRotated);

	painter->translate(rect.topLeft());
	painter->rotate(-rotateAngle);
	painter->translate(-rect.topLeft());

	painter->drawRect(windowShape.rect1);
	painter->setPen(QPen(Qt::black));
	painter->drawRect(windowShape.rect2);
	painter->drawRect(windowShape.rect3);
	painter->drawRect(windowShape.rect4);
	painter->drawRect(windowShape.rect5);
	painter->drawLines(windowShape.lines);

	painter->translate(rect.topLeft());
	painter->rotate(rotateAngle);
	painter->translate(-rect.topLeft());
}

void Deeplayout::DrawExteriorWall(QPainter *painter, QVector<DIRECTEDPOINT> &wall)
{
	painter->setPen(QPen(Qt::black));
	painter->setBrush(Qt::black);
	QVector<QPointF> wallPoints;
	for (int i = 0; i < wall.size(); i++)
	{
		wallPoints.push_back(Point2PointF(wall[i].point));
	}
	painter->drawPolygon(wallPoints);
}

void Deeplayout::DrawInteriorWall(QPainter *painter, QVector<QVector<DIRECTEDPOINT>> &wall)
{
	painter->setPen(QPen(Qt::gray));
	painter->setBrush(Qt::gray);
	for (int i = 0; i < wall.size(); i++)
	{
		QVector<QPointF> wallPoints;
		for (int j = 0; j < wall[i].size(); j++)
		{
			wallPoints.push_back(Point2PointF(wall[i][j].point));
		}
		painter->drawPolygon(wallPoints);
	}
}

void Deeplayout::ReadImageData(const QImage &image)
{
	QImage imageData(image);
	QVector<QVector<int>> boundary_map;
	QVector<QVector<int>> interior_wall_map;
	QVector<QVector<int>> label_map;
	QVector<QVector<int>> inside_map;
	for (int w = 0; w < imageData.width(); w++)
	{
		QVector<int> boundary_map_w;
		QVector<int> interior_wall_map_w;
		QVector<int> label_map_w;
		QVector<int> inside_map_w;
		for (int h = 0; h < imageData.height(); h++)
		{
			QRgb rgb = imageData.pixel(w, h);
			boundary_map_w.push_back(qRed(rgb));
			interior_wall_map_w.push_back(qGreen(rgb));
			label_map_w.push_back(qBlue(rgb));
			inside_map_w.push_back(qAlpha(rgb));
		}
		boundary_map.push_back(boundary_map_w);
		interior_wall_map.push_back(interior_wall_map_w);
		label_map.push_back(label_map_w);
		inside_map.push_back(inside_map_w);
	}

	data.width = imageData.width();
	data.height = imageData.height();
	data.boundary_map = boundary_map;
	data.interior_wall_map = interior_wall_map;
	data.label_map = label_map;
	data.inside_map = inside_map;

	data.minW = data.width;
	data.maxW = 0;
	data.minH = data.height;
	data.maxH = 0;
	for (int w = 0; w < data.width; w++)
	{
		for (int h = 0; h < data.height; h++)
		{
			if (boundary_map[w][h] > 0)
			{
				data.minW = w < data.minW ? w : data.minW;
				data.maxW = w > data.maxW ? w : data.maxW;
				data.minH = h < data.minH ? h : data.minH;
				data.maxH = h > data.maxH ? h : data.maxH;
			}
		}
	}
	data.minW -= 10;
	data.maxW += 10;
	data.minH -= 10;
	data.maxH += 10;
	house.rooms.clear();
}

void Deeplayout::DrawRoomEntry(QPainter *painter, ROOM &room)
{
	painter->setBrush(Qt::white);
	painter->setPen(QPen(Qt::white));
	ENTRY entry = room.entry;
	if (entry.type == 0)
	{
		DIRECTEDWALL door = entry.entry;
		int dir = door.dir;
		int rotateAngle = GetRotatAngle(dir);
		QRectF rect = RectF2RectF(door.rect);
		QRectF rectRotated = RotateRect(rect, rotateAngle);
		DOOR doorShape;
		if (room.label.name == "Bathroom" || room.label.name == "MastrRoom" || room.label.name == "StuRoom" || room.label.name == "SeondRoom")
		{
			doorShape = GetAntiDoor(rectRotated);
			painter->translate(rect.topLeft());
			painter->rotate(-rotateAngle);
			painter->translate(-rect.topLeft());

			painter->drawRect(doorShape.rect1);
			painter->setPen(QPen(Qt::black));
			painter->drawRect(doorShape.rect2);
			painter->drawRect(doorShape.rect3);
			painter->drawRect(doorShape.rect4);
			painter->drawRect(doorShape.rect5);
			painter->drawArc((doorShape.rect6), 0 * 16, 90 * 16);
		}
		else
		{
			doorShape = GetDoor(rectRotated);
			painter->translate(rect.topLeft());
			painter->rotate(-rotateAngle);
			painter->translate(-rect.topLeft());

			painter->drawRect(doorShape.rect1);
			painter->setPen(QPen(Qt::black));
			painter->drawRect(doorShape.rect2);
			painter->drawRect(doorShape.rect3);
			painter->drawRect(doorShape.rect4);
			painter->drawRect(doorShape.rect5);
			painter->drawArc((doorShape.rect6), 90 * 16, 90 * 16);
		}
		painter->translate(rect.topLeft());
		painter->rotate(rotateAngle);
		painter->translate(-rect.topLeft());
	}
	if (entry.type == 1)
	{
		DIRECTEDWALL door = entry.entry;
		int dir = door.dir;
		int rotateAngle = GetRotatAngle(dir);
		QRectF rect = RectF2RectF(door.rect);
		QRectF rectRotated = RotateRect(rect, rotateAngle);
		OPENWALL openWallShape = GetOpenWall(rectRotated);

		painter->translate(rect.topLeft());
		painter->rotate(-rotateAngle);
		painter->translate(-rect.topLeft());

		painter->drawRect(openWallShape.rect1);
		painter->setPen(QPen(Qt::black));
		painter->drawLines(openWallShape.lines1);
		painter->drawLines(openWallShape.lines2);

		painter->translate(rect.topLeft());
		painter->rotate(rotateAngle);
		painter->translate(-rect.topLeft());
	}
}

void Deeplayout::AddInteriorDoor(void)
{
	QVector<ROOM> rooms = house.rooms;
	QPoint frontDoorCentroid = house.front_door.entry.rect.center();
	for (int i = 0; i < rooms.size() - 1; i++)
	{
		if (rooms[i].label.name == "Balcony")
		{
			int contactLength = 0;
			DIRECTEDWALL openWall;
			for (int j = 0; j < rooms.size(); j++)
			{
				if (i != j)
				{
					QVector<DIRECTEDWALL> contactWalls = ContactWallForTwoBoundary(rooms[i].boundary, rooms[j].boundary);
					for (int j = 0; j < contactWalls.size(); j++)
					{
						int maxLength = max(contactWalls[j].rect.width(), contactWalls[j].rect.height());
						if (maxLength > contactLength)
						{
							contactLength = maxLength;
							openWall = contactWalls[j];
						}
					}
				}
			}

			if (contactLength != 0)
			{
				int width = openWall.rect.width();
				int height = openWall.rect.height();
				if (width > height)
				{
					openWall.rect.setX(openWall.rect.x() + width / 8);
					openWall.rect.setWidth(3 * width / 4);
				}
				else
				{
					openWall.rect.setY(openWall.rect.y() + height / 8);
					openWall.rect.setHeight(3 * height / 4);
				}

				ENTRY entry;
				entry.type = 1;
				entry.entry = openWall;
				rooms[i].entry = entry;
			}
		}
		else
		{
			if (rooms[i].label.type == 1)
			{
				int contactLength = 0;
				DIRECTEDWALL openWall;
				QVector<DIRECTEDWALL> contactWalls = ContactWallForTwoBoundary(rooms[i].boundary, rooms.back().boundary);
				for (int j = 0; j < contactWalls.size(); j++)
				{
					int maxLength = max(contactWalls[j].rect.width(), contactWalls[j].rect.height());
					if (maxLength > contactLength)
					{
						contactLength = maxLength;
						openWall = contactWalls[j];
					}
				}

				if (contactLength != 0)
				{
					int width = openWall.rect.width();
					int height = openWall.rect.height();
					if (width > height)
					{
						openWall.rect.setX(openWall.rect.x() + width / 8);
						openWall.rect.setWidth(3 * width / 4);
					}
					else
					{
						openWall.rect.setY(openWall.rect.y() + height / 8);
						openWall.rect.setHeight(3 * height / 4);
					}

					ENTRY entry;
					entry.type = 1;
					entry.entry = openWall;
					rooms[i].entry = entry;
				}
			}
			else
			{
				QVector<DIRECTEDWALL> contactWalls = ContactWallForTwoBoundary(rooms[i].boundary, rooms.back().boundary);
				QVector<DIRECTEDWALL> candidateDoor;
				for (int j = 0; j < contactWalls.size(); j++)
				{
					DIRECTEDWALL contactWall = contactWalls[j];
					int width = contactWall.rect.width();
					int height = contactWall.rect.height();
					if (width > height && width > 2 * HALFDOORWIDTH)
					{
						if (rooms[i].label.name == "Bathrom" || rooms[i].label.name == "SecndRoom" || rooms[i].label.name == "MasteRoom")
						{
							contactWall.rect.setLeft(contactWall.rect.right() - 2* HALFDOORWIDTH + 1);
							candidateDoor.push_back(contactWall);
						}
						else
						{
							contactWall.rect.setWidth(2 * HALFDOORWIDTH);
							candidateDoor.push_back(contactWall);
						}
					}
					if (height > width && height > 2 * HALFDOORWIDTH)
					{
						if (rooms[i].label.name == "Stoage" || rooms[i].label.name == "SecodRoom" || rooms[i].label.name == "MaterRoom")
						{
							contactWall.rect.setTop(contactWall.rect.bottom() - 2 * HALFDOORWIDTH + 1);
							candidateDoor.push_back(contactWall);
						}
						else
						{
							contactWall.rect.setHeight(2 * HALFDOORWIDTH);
							candidateDoor.push_back(contactWall);
						}
					}
				}

				if (candidateDoor.isEmpty())
				{
					int doorLength = 0;
					DIRECTEDWALL door;
					for (int j = 0; j < contactWalls.size(); j++)
					{
						int maxLength = max(contactWalls[j].rect.width(), contactWalls[j].rect.height());
						if (maxLength > doorLength)
						{
							doorLength = maxLength;
							door = contactWalls[j];
						}
					}
					if (doorLength != 0)
					{
						ENTRY entry;
						entry.type = 0;
						entry.entry = door;
						rooms[i].entry = entry;
					}
				}
				else
				{
					int dis = 1e8;
					DIRECTEDWALL door;
					for (int j = 0; j < candidateDoor.size(); j++)
					{
						QPoint candidateDoorCenter = candidateDoor[j].rect.center();
						//int candidateDis = (candidateDoorCenter.rx() - frontDoorCentroid.rx()) * (candidateDoorCenter.rx() - frontDoorCentroid.rx()) 
						//	+ (candidateDoorCenter.ry() - frontDoorCentroid.ry()) * (candidateDoorCenter.ry() - frontDoorCentroid.ry());
						int candidateDis = (candidateDoorCenter.rx() - frontDoorCentroid.rx())^2 
							+ (candidateDoorCenter.ry() - frontDoorCentroid.ry())^2;
						if (dis > candidateDis)
						{
							dis = candidateDis;
							door = candidateDoor[j];
						}
					}
					ENTRY entry;
					entry.type = 0;
					entry.entry = door;
					rooms[i].entry = entry;

					if (rooms[i].label.name == "Bathoom")
					{
						ENTRY entry;
						entry.type = 0;
						entry.entry = candidateDoor[1];
						rooms[i].entry = entry;
					}
				}
			}
		}
	}
	house.rooms = rooms;
}

void Deeplayout::AddWindow(void)
{
	QVector<ROOM> rooms = house.rooms;
	QVector<DIRECTEDPOINT> exterior_wall = house.exterior_wall;
	for (int i = 0; i < rooms.size(); i++)
	{
		if (rooms[i].label.name == "Balcony")
		{
			QVector<DIRECTEDWALL> contactWalls = ContactWallForTwoBoundary(rooms[i].boundary, exterior_wall);
			for (int j = 0; j < contactWalls.size(); j++)
			{
				DIRECTEDWALL contactWall = contactWalls[j];
				int maxLength = max(contactWall.rect.width(), contactWall.rect.height());
				if (maxLength > 2 * HALFSMALLWINDOWWIDTH)
				{
					DIRECTEDWALL window = contactWalls[j];

					int width = window.rect.width();
					int height = window.rect.height();
					if (width > height)
					{
						contactWall.rect.setX(contactWall.rect.x() + width / 10);
						contactWall.rect.setWidth(4 * width / 5);
						rooms[i].windows.push_back(contactWall);
					}
					else
					{
						contactWall.rect.setY(contactWall.rect.y() + height / 8);
						contactWall.rect.setHeight(3 * height / 4);
						rooms[i].windows.push_back(contactWall);
					}
				}
			}
		}
		else
		{
			if (rooms[i].label.name == "LivingRoom")
			{
				QVector<DIRECTEDWALL> contactWalls = ContactWallForTwoBoundary(rooms[i].boundary, exterior_wall);
				for (int j = 0; j < contactWalls.size(); j++)
				{
					DIRECTEDWALL contactWall = contactWalls[j];
					int maxLength = max(contactWall.rect.width(), contactWall.rect.height());
					if (maxLength < 3 * HALFLARGEWINDOWWIDTH && maxLength > 3 * HALFMIDWINDOWWIDTH)
					{
						DIRECTEDWALL window = contactWalls[j];

						int width = window.rect.width();
						int height = window.rect.height();
						if (width > height)
						{
							int central = window.rect.x() + window.rect.width() / 2;
							window.rect = QRect(central - HALFMIDWINDOWWIDTH, window.rect.y(), 2 * HALFMIDWINDOWWIDTH, window.rect.height());
							rooms[i].windows.push_back(window);
						}
						else
						{
							int central = window.rect.y() + window.rect.height() / 2;
							window.rect = QRect(window.rect.x(), central - HALFMIDWINDOWWIDTH, window.rect.width(), 2 * HALFMIDWINDOWWIDTH);
							rooms[i].windows.push_back(window);
						}
					}
					else
					{
						if (maxLength > 3 * HALFLARGEWINDOWWIDTH)
						{
							DIRECTEDWALL window = contactWalls[j];

							int width = window.rect.width();
							int height = window.rect.height();
							if (width > height)
							{
								int central = window.rect.x() + window.rect.width() / 2;
								window.rect = QRect(central - HALFLARGEWINDOWWIDTH, window.rect.y(), 2 * HALFLARGEWINDOWWIDTH, window.rect.height());
								rooms[i].windows.push_back(window);
							}
							else
							{
								int central = window.rect.y() + window.rect.height() / 2;
								window.rect = QRect(window.rect.x(), central - HALFLARGEWINDOWWIDTH, window.rect.width(), 2 * HALFLARGEWINDOWWIDTH);
								rooms[i].windows.push_back(window);
							}
						}
					}
				}
			}
			else
			{
				if (rooms[i].label.name == "Bathroom")
				{
					int contactLength = 1e8;
					DIRECTEDWALL window;
					QVector<DIRECTEDWALL> contactWalls = ContactWallForTwoBoundary(rooms[i].boundary, exterior_wall);
					for (int j = 0; j < contactWalls.size(); j++)
					{
						int maxLength = max(contactWalls[j].rect.width(), contactWalls[j].rect.height());
						if (maxLength < contactLength && maxLength > 3 * HALFSMALLWINDOWWIDTH)
						{
							contactLength = maxLength;
							window = contactWalls[j];
						}
					}

					if (contactLength != 1e8)
					{
						int width = window.rect.width();
						int height = window.rect.height();
						if (width > height)
						{
							int central = window.rect.x() + window.rect.width() / 2;
							window.rect = QRect(central - HALFSMALLWINDOWWIDTH, window.rect.y(), 2 * HALFSMALLWINDOWWIDTH, window.rect.height());
							rooms[i].windows.push_back(window);
						}
						else
						{
							int central = window.rect.y() + window.rect.height() / 2;
							window.rect = QRect(window.rect.x(), central - HALFSMALLWINDOWWIDTH, window.rect.width(), 2 * HALFSMALLWINDOWWIDTH);
							rooms[i].windows.push_back(window);
						}
					}
				}
				else
				{
					int contactLength = 0;
					DIRECTEDWALL window;
					QVector<DIRECTEDWALL> contactWalls = ContactWallForTwoBoundary(rooms[i].boundary, exterior_wall);
					for (int j = 0; j < contactWalls.size(); j++)
					{
						int maxLength = max(contactWalls[j].rect.width(), contactWalls[j].rect.height());
						if (maxLength > contactLength && maxLength > 3 * HALFMIDWINDOWWIDTH)
						{
							contactLength = maxLength;
							window = contactWalls[j];
						}
					}

					if (contactLength != 0)
					{
						int width = window.rect.width();
						int height = window.rect.height();
						if (width > height)
						{
							int central = window.rect.x() + window.rect.width() / 2;
							window.rect = QRect(central - HALFMIDWINDOWWIDTH, window.rect.y(), 2 * HALFMIDWINDOWWIDTH, window.rect.height());
							rooms[i].windows.push_back(window);
						}
						else
						{
							int central = window.rect.y() + window.rect.height() / 2;
							window.rect = QRect(window.rect.x(), central - HALFMIDWINDOWWIDTH, window.rect.width(), 2 * HALFMIDWINDOWWIDTH);
							rooms[i].windows.push_back(window);
						}
					}
				}
			}
		}
	}
	house.rooms = rooms;
}

void Deeplayout::HouseAbstract()
{
	QVector<QVector<int>> exterior_wall_map;
	QVector<QVector<int>> front_door_map;
	for (int w = 0; w < data.width; w++)
	{
		QVector<int> exterior_wall_map_w;
		QVector<int> front_door_map_w;
		for (int h = 0; h < data.height; h++)
		{
			if (data.boundary_map[w][h] == 127)
			{
				exterior_wall_map_w.push_back(255);
			}
			else
			{
				exterior_wall_map_w.push_back(0);
			}
			if (data.boundary_map[w][h] == 255)
			{
				front_door_map_w.push_back(255);
			}
			else
			{
				front_door_map_w.push_back(0);
			}
		}
		exterior_wall_map.push_back(exterior_wall_map_w);
		front_door_map.push_back(front_door_map_w);
	}
	house.exterior_wall = BoundaryAbstract(exterior_wall_map);
	house.front_door = FrontDoorAbstract(front_door_map);

	QVector<ROOM> rooms;
	QVector<QVector<int>> wall_map;

	for (int w = 0; w < data.width; w++)
	{
		QVector<int> wall_map_w;
		for (int h = 0; h < data.height; h++)
		{
			if (data.boundary_map[w][h] > 0 || data.interior_wall_map[w][h] > 0)
			{
				wall_map_w.push_back(255);
			}
			else
			{
				wall_map_w.push_back(0);
			}
		}
		wall_map.push_back(wall_map_w);
	}

	QPoint livingRoomCentroid;
	for (int w = data.minW; w < data.maxW; w++)
	{
		for (int h = data.minH; h < data.maxH; h++)
		{
			if (data.label_map[w][h] > 0 && data.inside_map[w][h] == 255)
			{
				int roomIndex = data.label_map[w][h] - 100;
				if (ROOMLABEL[roomIndex].name == "LivingRoom")
				{
					livingRoomCentroid = QPoint(w, h);
				}
				else
				{
					ROOM room;
					room.label = ROOMLABEL[roomIndex];
					room.centroid = QPoint(w, h);
					room.map = RoomMapAbstract(wall_map, room.centroid, 0);
					room.boundary = BoundaryAbstract(room.map);
					rooms.push_back(room);
				}
			}
		}
	}

	QVector<QVector<int>> holeMap = data.inside_map;
	for (int i = 0; i < rooms.size(); i++)
	{
		for (int w = data.minW; w < data.maxW; w++)
		{
			for (int h = data.minH; h < data.maxH; h++)
			{
				if (rooms[i].map[w][h] > 0)
				{
					holeMap[w][h] = 0;
				}
			}
		}
	}
	HoleAllocation(holeMap, rooms);

	ROOM livingRoom;
	QVector<QVector<int>> livingRoom_map = data.inside_map;
	QVector<QVector<int>> interior_wall_map = data.inside_map;
	for (int i = 0; i < rooms.size(); i++)
	{
		QVector<QVector<int>> room_map = rooms[i].map;
		QVector<QVector<int>> room_map_expand = RoomMapExpand(rooms[i].map, rooms[i].boundary, 3);
		for (int w = data.minW; w < data.maxW; w++)
		{
			for (int h = data.minH; h < data.maxH; h++)
			{
				if (room_map_expand[w][h] > 0)
				{
					livingRoom_map[w][h] = 0;
				}
				if (room_map[w][h] > 0)
				{
					interior_wall_map[w][h] = 0;
				}
			}
		}
	}
	livingRoom.label = ROOMLABEL[0];
	livingRoom.map = livingRoom_map;
	livingRoom.centroid = livingRoomCentroid;
	livingRoom.boundary = BoundaryAbstract(livingRoom_map);
	rooms.push_back(livingRoom);
	house.rooms = rooms;

	AddInteriorDoor();
	AddWindow();

	for (int w = data.minW; w < data.maxW; w++)
	{
		for (int h = data.minH; h < data.maxH; h++)
		{
			if (livingRoom_map[w][h] > 0)
			{
				interior_wall_map[w][h] = 0;
			}
		}
	}
	for (int k = 0; k < house.rooms.size(); k++)
	{
		QRect rect = house.rooms[k].entry.entry.rect;
		for (int w = rect.x(); w < rect.x() + rect.width(); w++)
		{
			for (int h = rect.y(); h < rect.y() + rect.height(); h++)
			{
				interior_wall_map[w][h] = 0;
			}
		}
	}
	house.interior_wall = InteriorWallAbstract(interior_wall_map);
}

ENTRY Deeplayout::FrontDoorAbstract(const QVector<QVector<int>> &front_door_map)
{
	int width = data.width;
	int height = data.height;
	int front_door_min_w = width;
	int	front_door_max_w = 0;
	int front_door_min_h = height;
	int front_door_max_h = 0;

	for (int w = 0; w < width; w++)
	{
		for (int h = 0; h < height; h++)
		{
			if (front_door_map[w][h] == 255)
			{
				front_door_min_w = front_door_min_w > w ? w : front_door_min_w;
				front_door_max_w = front_door_max_w < w ? w : front_door_max_w;
				front_door_min_h = front_door_min_h > h ? h : front_door_min_h;
				front_door_max_h = front_door_max_h < h ? h : front_door_max_h;
			}
		}
	}

	DIRECTEDWALL entry;
	entry.rect = QRect(front_door_min_w, front_door_min_h, front_door_max_w - front_door_min_w + 1, front_door_max_h - front_door_min_h + 1);
	if (entry.rect.width() > entry.rect.height())
	{
		if (data.inside_map[entry.rect.x() + entry.rect.width() / 2][entry.rect.y() - 1] == 255)
		{
			entry.dir = 3;
		}
		if (data.inside_map[entry.rect.x() + entry.rect.width() / 2][entry.rect.y() + entry.rect.height() + 1] == 255)
		{
			entry.dir = 1;
		}
	}
	else
	{
		if (data.inside_map[entry.rect.x() - 1][entry.rect.y() + entry.rect.height() / 2] == 255)
		{
			entry.dir = 2;
		}
		if (data.inside_map[entry.rect.x() + entry.rect.width() + 1][entry.rect.y() + entry.rect.height() / 2] == 255)
		{
			entry.dir = 0;
		}
	}
	ENTRY front_door;
	front_door.type = 0;
	front_door.entry = entry;
	return(front_door);
}

QVector<QVector<int>> Deeplayout::RoomMapExpand(const QVector<QVector<int>> &map, const QVector<DIRECTEDPOINT> &boundary, int expand)
{
	QVector<QVector<int>> roomMap = map;
	QVector<DIRECTEDPOINT> roomBoundary = boundary;
	roomBoundary.push_back(boundary[0]);

	for (int i = 0; i < roomBoundary.size() - 1; i++)
	{
		int dir = roomBoundary[i].dir;
		if (dir == 0)
		{
			for (int w = roomBoundary[i].point.rx() - expand; w < roomBoundary[i + 1].point.rx() + expand; w++)
			{
				for (int h = roomBoundary[i].point.ry() - expand; h < roomBoundary[i].point.ry(); h++)
				{
					if (data.inside_map[w][h] == 255)
					{
						roomMap[w][h] = 255;
					}
				}
			}
		}
		if (dir == 1)
		{
			for (int w = roomBoundary[i].point.rx(); w < roomBoundary[i].point.rx() + expand; w++)
			{
				for (int h = roomBoundary[i].point.ry() - expand; h < roomBoundary[i + 1].point.ry() + expand; h++)
				{
					if (data.inside_map[w][h] == 255)
					{
						roomMap[w][h] = 255;
					}
				}
			}
		}
		if (dir == 2)
		{
			for (int w = roomBoundary[i + 1].point.rx() - expand; w < roomBoundary[i].point.rx() + expand; w++)
			{
				for (int h = roomBoundary[i].point.ry(); h < roomBoundary[i].point.ry() + expand; h++)
				{
					if (data.inside_map[w][h] == 255)
					{
						roomMap[w][h] = 255;
					}
				}
			}
		}
		if (dir == 3)
		{
			for (int w = roomBoundary[i + 1].point.rx() - expand; w < roomBoundary[i + 1].point.rx(); w++)
			{
				for (int h = roomBoundary[i + 1].point.ry() - expand; h < roomBoundary[i].point.ry() + expand; h++)
				{
					if (data.inside_map[w][h] == 255)
					{
						roomMap[w][h] = 255;
					}
				}
			}
		}
	}

	return(roomMap);
}

QVector<DIRECTEDWALL> Deeplayout::ContactWallForTwoBoundary(const QVector<DIRECTEDPOINT> &mainBoundary, const QVector<DIRECTEDPOINT> &secondBoundary)
{
	QVector<DIRECTEDWALL> contactWalls;
	QVector<DIRECTEDPOINT> boundary1 = mainBoundary;
	QVector<DIRECTEDPOINT> boundary2 = secondBoundary;
	boundary1.push_back(boundary1[0]);
	boundary2.push_back(boundary2[0]);
	for (int i = 0; i < boundary1.size() - 1; i++)
	{
		DIRECTEDEDGE line1(boundary1[i].point, boundary1[i + 1].point);
		for (int j = 0; j < boundary2.size() - 1; j++)
		{
			DIRECTEDEDGE line2(boundary2[j].point, boundary2[j + 1].point);
			int min1 = min(line1.minLevel, line2.minLevel);
			int max1 = max(line1.maxLevel, line2.maxLevel);
			int length1 = line1.maxLevel - line1.minLevel;
			int length2 = line2.maxLevel - line2.minLevel;
			int length = max1 - min1;
			if (line1.dir == line2.dir && line1.level != line2.level && abs(line1.level - line2.level) < 6 && length < length1 + length2)
			{
				DIRECTEDWALL contactWall;
				if (line1.dir == 0)
				{
					int minh = min(line1.level, line2.level);
					int maxh = max(line1.level, line2.level);
					int minw = max(line1.minLevel, line2.minLevel);
					int maxw = min(line1.maxLevel, line2.maxLevel);

					if (line1.level > line2.level)
					{
						contactWall.dir = 1;
					}
					else
					{
						contactWall.dir = 3;
					}
					contactWall.rect = QRect(minw, minh, maxw - minw, maxh - minh);
				}
				else
				{
					int minw = min(line1.level, line2.level);
					int maxw = max(line1.level, line2.level);
					int minh = max(line1.minLevel, line2.minLevel);
					int maxh = min(line1.maxLevel, line2.maxLevel);

					if (line1.level > line2.level)
					{
						contactWall.dir = 0;
					}
					else
					{
						contactWall.dir = 2;
					}
					contactWall.rect = QRect(minw, minh, maxw - minw, maxh - minh);
				}
				contactWalls.push_back(contactWall);
			}
		}
	}

	return(contactWalls);
}

QVector<QVector<int>> Deeplayout::RoomMapAbstract(const QVector<QVector<int>> &wall_map, QPoint &centroid, int direction)
{
	int width = data.width;
	int height = data.height;
	QVector<QVector<int>> room_map;
	for (int w = 0; w < width; w++)
	{
		QVector<int> room_map_w;
		for (int h = 0; h < height; h++)
		{
			room_map_w.push_back(0);
		}
		room_map.append(room_map_w);
	}

	if (direction == 0)
	{
		int location = centroid.ry();
		QPoint startLine(-1, -2);
		QVector<QPoint> lines = ScanLine(wall_map, location, 0);
		for (int i = 0; i < lines.size(); i++)
		{
			if (centroid.rx() >= lines[i].rx() && centroid.rx() <= lines[i].ry())
			{
				startLine = lines[i];
				for (int k = lines[i].rx(); k < lines[i].ry() + 1; k++)
				{
					room_map[k][location] = 255;
				}
			}
		}

		QPoint line(startLine);
		int lineLength = line.ry() - line.rx() + 1;
		for (int h = location + 1; h < height; h++)
		{
			QPoint intersectLine = IntersectLine(wall_map, h, line, 0);
			QPoint contactLine = ContactLine(wall_map, h, line, 0);
			int intersectLineLength = intersectLine.ry() - intersectLine.rx() + 1;
			int contactLineLength = contactLine.ry() - contactLine.rx() + 1;
			if (intersectLineLength > lineLength / 3 && contactLineLength < 2 * lineLength)
			//if (intersectLineLength == lineLength)
			{
				line = contactLine;
				lineLength = contactLineLength;
				for (int k = line.rx(); k < line.ry() + 1; k++)
				{
					room_map[k][h] = 255;
				}
			}
			else
			{
				break;
			}
		}

		line = startLine;
		lineLength = line.ry() - line.rx() + 1;
		for (int h = location - 1; h >= 0; h--)
		{
			QPoint intersectLine = IntersectLine(wall_map, h, line, 0);
			QPoint contactLine = ContactLine(wall_map, h, line, 0);
			int intersectLineLength = intersectLine.ry() - intersectLine.rx() + 1;
			int contactLineLength = contactLine.ry() - contactLine.rx() + 1;
			if (intersectLineLength > lineLength / 3 && contactLineLength < 2 * lineLength)
			//if (intersectLineLength == lineLength)
			{
				line = contactLine;
				lineLength = contactLineLength;
				for (int k = line.rx(); k < line.ry() + 1; k++)
				{
					room_map[k][h] = 255;
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		int location = centroid.rx();
		QPoint startLine(-1, -2);
		QVector<QPoint> lines = ScanLine(wall_map, location, 1);
		for (int i = 0; i < lines.size(); i++)
		{
			if (centroid.ry() >= lines[i].rx() && centroid.ry() <= lines[i].ry())
			{
				startLine = lines[i];
				for (int k = lines[i].rx(); k < lines[i].ry() + 1; k++)
				{
					room_map[location][k] = 255;
				}
			}
		}

		QPoint line(startLine);
		int lineLength = line.ry() - line.rx() + 1;
		for (int w = location + 1; w < width; w++)
		{
			QPoint intersectLine = IntersectLine(wall_map, w, line, 1);
			int intersectLineLength = intersectLine.ry() - intersectLine.rx() + 1;
			if (intersectLineLength > lineLength / 3)
			{
				line = intersectLine;
				lineLength = intersectLineLength;
				for (int k = line.rx(); k < line.ry() + 1; k++)
				{
					room_map[w][k] = 255;
				}
			}
			else
			{
				break;
			}
		}

		line = startLine;
		lineLength = line.ry() - line.rx() + 1;
		for (int w = location - 1; w >= 0; w--)
		{
			QPoint intersectLine = IntersectLine(wall_map, w, line, 1);
			int intersectLineLength = intersectLine.ry() - intersectLine.rx() + 1;
			if (intersectLineLength > lineLength / 3)
			{
				line = intersectLine;
				lineLength = intersectLineLength;
				for (int k = line.rx(); k < line.ry() + 1; k++)
				{
					room_map[w][k] = 255;
				}
			}
			else
			{
				break;
			}
		}
	}

	return(room_map);
}


void Deeplayout::HoleAllocation(const QVector<QVector<int>> &holeMap, QVector<ROOM> &rooms)
{
	int width = data.width;
	int height = data.height;
	QVector<QVector<int>> map = holeMap;

	while (true)
	{
		QVector<DIRECTEDPOINT> candidate_boudnary = BoundaryAbstract(map);
		QVector<QPoint> candidate_point;
		QVector<QVector<int>> candidate_map;
		for (int w = 0; w < width; w++)
		{
			QVector<int> candidate_map_w;
			for (int h = 0; h < height; h++)
			{
				candidate_map_w.push_back(0);
			}
			candidate_map.push_back(candidate_map_w);
		}
		if (candidate_boudnary.isEmpty())
		{
			break;
		}
		else
		{
			for (int i = 0; i < candidate_boudnary.size(); i++)
			{
				candidate_point.push_back(candidate_boudnary[i].point);
			}
		}
		int area = 0;
		for (int w = 0; w < width; w++)
		{
			for (int h = 0; h < height; h++)
			{
				QPoint point = QPoint(w, h);
				polygon = Polygon(candidate_point);
				if (polygon.PointInThePolygon(point))
				{
					map[w][h] = 0;
					candidate_map[w][h] = 255;
					area++;
				}
				else
				{
					candidate_map[w][h] = 0;
				}
			}
		}
		if (area > 0 && area < 3000)
		{
			bool addHoleFlag = true;
			int nearest_room_index = -1;
			for (int i = 0; i < candidate_point.size(); i++)
			{
				QPoint point = candidate_point[i];
				for (int k = 0; k < rooms.size(); k++)
				{
					QVector<QVector<int>> roomMap = rooms[k].map;
					if (roomMap[point.rx() - 1][point.ry()] == 255 || roomMap[point.rx() + 1][point.ry()] == 255 ||
						roomMap[point.rx()][point.ry() - 1] == 255 || roomMap[point.rx()][point.ry() + 1] == 255)
					{
						if (nearest_room_index == -1)
						{
							nearest_room_index = k;
						}
						else
						{
							if (nearest_room_index != k)
							{
								addHoleFlag = false;
								break;
							}
						}
					}
 				}
				if (addHoleFlag == false)
				{
					break;
				}
			}

			if (addHoleFlag)
			{
				for (int w = 0; w < width; w++)
				{
					for (int h = 0; h < height; h++)
					{
						if (candidate_map[w][h] == 255)
						{
							rooms[nearest_room_index].map[w][h] = 255;
						}
					}
				}
				rooms[nearest_room_index].boundary = BoundaryAbstract(rooms[nearest_room_index].map);
			}
		}
	}
}

QVector<QVector<DIRECTEDPOINT>> Deeplayout::InteriorWallAbstract(const QVector<QVector<int>> &interior_wall_map)
{	
	int width = data.width;
	int height = data.height;
	QVector<QVector<DIRECTEDPOINT>> interior_walls;
	QVector<QVector<int>> map = interior_wall_map;

	while (true)
	{
		QVector<DIRECTEDPOINT> interior_wall = BoundaryAbstract(map);
		QVector<QPoint> interior_wall_point;
		if (interior_wall.isEmpty())
		{
			break;
		}
		else
		{
			interior_walls.push_back(interior_wall);
			for (int i = 0; i < interior_wall.size(); i++)
			{
				interior_wall_point.push_back(interior_wall[i].point);
			}
		}
		for (int w = 0; w < width; w++)
		{
			for (int h = 0; h < height; h++)
			{
				QPoint point = QPoint(w, h);
				polygon = Polygon(interior_wall_point);
				if (polygon.PointInThePolygon(point))
				{
					map[w][h] = 0;
				}
			}
		}
	}
	return interior_walls;
}

QVector<DIRECTEDPOINT> Deeplayout::BoundaryAbstract(const QVector<QVector<int>> &map)
{
	int width = data.width;
	int height = data.height;
	QVector<DIRECTEDPOINT> boundary;
	bool flag = false;

	for (int w = 0; w < width; w++)
	{
		for (int h = 0; h < height; h++)
		{
			if (map[w][h] == 255)
			{
				DIRECTEDPOINT point(w, h, 0);
				boundary.push_back(point);
				flag = true;
				break;
			}
		}
		if (flag)
		{
			break;
		}
	}

	while (flag)
	{
		DIRECTEDPOINT dPoint(-1, -1, -1);

		if (boundary.back().dir == 0)
		{
			int h = boundary.back().point.ry();
			for (int w = boundary.back().point.rx() + 1; w < width; w++)
			{
				int corner_sum = 0;
				if (map[w][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (corner_sum == 1)
				{
					dPoint = DIRECTEDPOINT(w, h, 1);
					break;
				}
				if (corner_sum == 3)
				{
					dPoint = DIRECTEDPOINT(w, h, 3);
					break;
				}
				if (map[w][h] == 0 && map[w - 1][h] == 255 && map[w][h - 1] == 255 && map[w - 1][h - 1] == 0)
				{
					dPoint = DIRECTEDPOINT(w, h, 1);
					break;
				}
			}
		}

		if (boundary.back().dir == 1)
		{
			int w = boundary.back().point.rx();
			for (int h = boundary.back().point.ry() + 1; h < height; h++)
			{
				int corner_sum = 0;
				if (map[w][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (corner_sum == 1)
				{
					dPoint = DIRECTEDPOINT(w, h, 2);
					break;
				}
				if (corner_sum == 3)
				{
					dPoint = DIRECTEDPOINT(w, h, 0);
					break;
				}
				if (map[w][h] == 255 && map[w - 1][h] == 0 && map[w][h - 1] == 0 && map[w - 1][h - 1] == 255)
				{
					dPoint = DIRECTEDPOINT(w, h, 2);
					break;
				}
			}
		}

		if (boundary.back().dir == 2)
		{
			int h = boundary.back().point.ry();
			for (int w = boundary.back().point.rx() - 1; w > 0; w--)
			{
				int corner_sum = 0;
				if (map[w][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (corner_sum == 1)
				{
					dPoint = DIRECTEDPOINT(w, h, 3);
					break;
				}
				if (corner_sum == 3)
				{
					dPoint = DIRECTEDPOINT(w, h, 1);
					break;
				}
				if (map[w][h] == 0 && map[w - 1][h] == 255 && map[w][h - 1] == 255 && map[w - 1][h - 1] == 0)
				{
					dPoint = DIRECTEDPOINT(w, h, 3);
					break;
				}
			}
		}
		if (boundary.back().dir == 3)
		{
			int w = boundary.back().point.rx();
			for (int h = boundary.back().point.ry() - 1; h > 0; h--)
			{
				int corner_sum = 0;
				if (map[w][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h] == 255)
				{
					corner_sum += 1;
				}
				if (map[w - 1][h - 1] == 255)
				{
					corner_sum += 1;
				}
				if (corner_sum == 1)
				{
					dPoint = DIRECTEDPOINT(w, h, 0);
					break;
				}
				if (corner_sum == 3)
				{
					dPoint = DIRECTEDPOINT(w, h, 2);
					break;
				}
				if (map[w][h] == 255 && map[w - 1][h] == 0 && map[w][h - 1] == 0 && map[w - 1][h - 1] == 255)
				{
					dPoint = DIRECTEDPOINT(w, h, 0);
					break;
				}
			}
		}

		if (dPoint.dir != -1)
		{
			if (dPoint.point != boundary[0].point)
			{
				boundary.push_back(dPoint);
			}
			else
			{
				flag = false;
			}
		}
	}

	return boundary;
}

int Deeplayout::ContactLength(const QPoint &line1, const QPoint &line2)
{
	int length1 = line1.y() - line1.x() + 1;
	int length2 = line2.y() - line2.x() + 1;
	int length = max(line1.y(), line2.y()) - min(line1.x(), line2.x()) + 1;
	return(length1 + length2 - length);
}

QPoint Deeplayout::IntersectLine(const QPoint &line1, const QPoint &line2)
{
	int x = max(line1.x(), line2.x());
	int y = min(line1.y(), line2.y());
	return(QPoint(x, y));
}

QPoint Deeplayout::ContactLine(const QVector<QVector<int>> &map, int location, QPoint inputLine, int direction)
{
	QVector<QPoint> candidataLines = ScanLine(map, location, direction);
	int contactLength = 0;
	QPoint maxContactLine(-1, -2);
	for (int i = 0; i < candidataLines.size(); i++)
	{
		int newContactLength = ContactLength(candidataLines[i], inputLine);
		if (newContactLength > contactLength)
		{
			maxContactLine = candidataLines[i];
			contactLength = newContactLength;
		}
	}
	return(maxContactLine);
}

QPoint Deeplayout::IntersectLine(const QVector<QVector<int>> &map, int location, QPoint inputLine, int direction)
{
	QVector<QPoint> candidataLines = ScanLine(map, location, direction);
	int contactLength = 0;
	QPoint intersectLine(-1, -2);
	for (int i = 0; i < candidataLines.size(); i++)
	{
		int newContactLength = ContactLength(candidataLines[i], inputLine); 
		QPoint newIntersectLine = IntersectLine(candidataLines[i], inputLine);
		if (newContactLength > contactLength)
		{
			intersectLine = newIntersectLine;
			contactLength = newContactLength;
		}
	}
	return(intersectLine);
}

QVector<QPoint> Deeplayout::ScanLine(const QVector<QVector<int>> &map, int location, int direction, int scanValue)
{
	int width = data.width;
	int height = data.height;
	QVector<QPoint> lines;

	if (direction == 0)
	{
		int wMax = -2;
		int wMin = -1;
		for (int w = 1; w < width - 1; w++)
		{
			if (map[w - 1][location] == scanValue && map[w][location] == 0)
			{
				wMin = w;
			}
			if (map[w][location] == 0 && map[w + 1][location] == scanValue)
			{
				wMax = w;
				lines.append(QPoint(wMin, wMax));
			}
		}
	}
	else
	{
		int hMax = -2;
		int hMin = -1;
		for (int h = 1; h < height - 1; h++)
		{
			if (map[location][h - 1] == scanValue && map[location][h] == 0)
			{
				hMin = h;
			}
			if (map[location][h] == 0 && map[location][h + 1] == scanValue)
			{
				hMax = h;
				lines.append(QPoint(hMin, hMax));
			}
		}
	}
	return(lines);
}