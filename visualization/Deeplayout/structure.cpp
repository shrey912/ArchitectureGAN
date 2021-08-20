#include <Structure.h>
#pragma execution_character_set("utf-8")

QString English2Chinese(const QString &english)
{
	QString chinese = " ";
	if (english == "LivingRoom")
	{
		chinese = "客厅";
	}
	if (english == "MasterRoom")
	{
		chinese = "主卧";
	}
	if (english == "Kitchen")
	{
		chinese = "厨房";
	}
	if (english == "Bathroom")
	{
		chinese = "卫生间";
	}
	if (english == "DiningRoom")
	{
		chinese = "餐 厅";
	}
	if (english == "ChildRoom")
	{
		chinese = "儿童房";
	}
	if (english == "StudyRoom")
	{
		chinese = "书房";
	}
	if (english == "SecondRoom")
	{
		chinese = "次卧";
	}
	if (english == "GuestRoom")
	{
		chinese = "客卧";
	}
	if (english == "Balcony")
	{
		chinese = "阳台";
	}
	if (english == "Entrance")
	{
		chinese = "门厅";
	}
	if (english == "Storage")
	{ 
		chinese = "储物间";
	}
	if (english == "Wall-in")
	{
		chinese = "壁橱";
	}

	return(chinese);
}