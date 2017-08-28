#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace cv;

class Colorbar {
public:
	Colorbar();
	Colorbar(int length, string unit, vector<string> index);
	
	int length = 1000;			//輸出的色條圖的總長度
	string unit = "(m/s)";		//刻度單位(可為空)
	vector<string> index{ "0","1","2","3","4","5","6","7","8","9","10" };	//刻度值

	/*色彩索引*/
	vector<Scalar> makeColorbarColorIndex(void);

	/*由上下界創建刻度*/
	vector<string> indexString(double upperbound, double lowerbound, int indexnum);		//刻度上界、下界、刻度數量

	/*創建含有刻度的色條圖*/
	Mat makeColorbar(int length, vector<string> index, string unit, int flag = COLORBAR_VERTICAL);
	Mat makeColorbar(int flag = COLORBAR_VERTICAL);

private:
	/*指定色條圖為水平或垂直*/
	enum COLORBARFLAG { COLORBAR_VERTICAL = 0, COLORBAR_HORIZONTAL = 1 };

	/*創建色條*/
	Mat makeColorbarColor(int length, int flag = COLORBAR_VERTICAL);
};

