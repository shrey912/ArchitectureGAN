#include"polygon.h"

Polygon::Polygon()
{

}

Polygon::Polygon(const QVector<QPoint> &polygon_)
{
	polygon = polygon_;
	polygon.push_back(polygon_[0]);
}

Polygon::~Polygon()
{

}

bool Polygon::PointInThePolygon(const QPoint &point)
{
	double xInt;
	int crossings = 0;
	float x1;
	float y1;
	float x2;
	float y2;

	for (int i = 0; i < polygon.size() - 1; i++)
	{
		QPoint p1 = polygon[i + 1];
		QPoint p2 = polygon[i];
		x1 = p1.rx() - point.x();
		y1 = p1.ry() - point.y();
		x2 = p2.rx() - point.x();
		y2 = p2.ry() - point.y();

		if (((y1 > 0) && (y2 <= 0)) || ((y2 > 0) && (y1 <= 0)))
		{
			xInt = SignOfDet2x2(x1, y1, x2, y2) / (y2 - y1);
			if (0.0 < xInt) crossings++;
		}
	}

	if ((crossings % 2) == 1) return true;
	return false;
}

int Polygon::SignOfDet2x2(double x1, double y1, double x2, double y2)
{
	int sign = 1;
	double swap;
	double k;
	long count = 0;

	if ((abs(x1) < 1e-5) || (abs(y2) < 1e-5))
	{
		if ((abs(y1) < 1e-5) || (abs(x2) < 1e-5))
		{
			return 0;
		}
		else if (y1 > 0)
		{
			if (x2 > 0)
			{
				return -sign;
			}
			else
			{
				return sign;
			}
		}
		else
		{
			if (x2 > 0)
			{
				return sign;
			}
			else
			{
				return -sign;
			}
		}
	}
	if ((abs(y1) < 1e-5) || (abs(x2) < 1e-5))
	{
		if (y2 > 0)
		{
			if (x1 > 0)
			{
				return sign;
			}
			else
			{
				return -sign;
			}
		}
		else
		{
			if (x1 > 0)
			{
				return -sign;
			}
			else
			{
				return sign;
			}
		}
	}

	if (0.0 < y1)
	{
		if (0.0 < y2)
		{
			if (y1 > y2)
			{
				sign = -sign;
				swap = x1;
				x1 = x2;
				x2 = swap;
				swap = y1;
				y1 = y2;
				y2 = swap;
			}
		}
		else
		{
			if (y1 < -y2)
			{
				sign = -sign;
				x2 = -x2;
				y2 = -y2;
			}
			else
			{
				swap = x1;
				x1 = -x2;
				x2 = swap;
				swap = y1;
				y1 = -y2;
				y2 = swap;
			}
		}
	}
	else
	{
		if (0.0 < y2)
		{
			if (-y1 < y2)
			{
				sign = -sign;
				x1 = -x1;
				y1 = -y1;
			}
			else
			{
				swap = -x1;
				x1 = x2;
				x2 = swap;
				swap = -y1;
				y1 = y2;
				y2 = swap;
			}
		}
		else
		{
			if (y1 > y2)
			{
				x1 = -x1;
				y1 = -y1;
				x2 = -x2;
				y2 = -y2;
			}
			else
			{
				sign = -sign;
				swap = -x1;
				x1 = -x2;
				x2 = swap;
				swap = -y1;
				y1 = -y2;
				y2 = swap;
			}
		}
	}

	if (0.0 < x1)
	{
		if (0.0 < x2)
		{
			if (x1 > x2)
			{
				return sign;
			}
		}
		else
		{
			return sign;
		}
	}
	else
	{
		if (0.0 < x2)
		{
			return -sign;
		}
		else
		{
			if (x1 > x2)
			{
				sign = -sign;
				x1 = -x1;
				x2 = -x2;
			}
			else
			{
				return -sign;
			}
		}
	}

	while (true)
	{
		count = count + 1;
		k = std::floor(x2 / x1);
		x2 = x2 - k *x1;
		y2 = y2 - k *y1;

		if (y2 < 0.0)
		{
			return -sign;
		}
		if (y2 > y1)
		{
			return sign;
		}

		if (x1 > x2 + x2)
		{
			if (y1 < y2 + y2)
			{
				return sign;
			}
		}
		else
		{
			if (y1 > y2 + y2)
			{
				return -sign;
			}
			else
			{
				x2 = x1 - x2;
				y2 = y1 - y2;
				sign = -sign;
			}
		}
		if (abs(y2) < 1e-5)
		{
			if (abs(x2) < 1e-5)
			{
				return 0;
			}
			else
			{
				return -sign;
			}
		}
		if (abs(x2) < 1e-5)
		{
			return sign;
		}

		k = std::floor(x1 / x2);
		x1 = x1 - k *x2;
		y1 = y1 - k *y2;

		if (y1 < 0.0)
		{
			return sign;
		}
		if (y1 > y2)
		{
			return -sign;
		}

		if (x2 > x1 + x1)
		{
			if (y2 < y1 + y1)
			{
				return -sign;
			}
		}
		else
		{
			if (y2 > y1 + y1)
			{
				return sign;
			}
			else
			{
				x1 = x2 - x1;
				y1 = y2 - y1;
				sign = -sign;
			}
		}
		if (abs(y1) < 1e-5)
		{
			if (abs(x1) < 1e-5)
			{
				return 0;
			}
			else
			{
				return sign;
			}
		}
		if (abs(x1) < 1e-5)
		{
			return -sign;
		}
	}
}