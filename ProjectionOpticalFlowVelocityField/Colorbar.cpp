#ifndef COLORBAR_H
#define COLORBAR_H
#include "Colorbar.h"
#endif


Colorbar::Colorbar()
{
	length = 1000;
	unit = "(m/s)";
	index = { "0","1","2","3","4","5","6","7","8","9","10" };
}

Colorbar::Colorbar(int length_a, string unit_a, vector<string> index_a)
{
	length = length_a;
	unit = unit_a;
	index = index_a;
}

vector<Scalar> Colorbar::makeColorbarColorIndex(void)
{
	vector<Scalar> maincolor;

	maincolor.push_back(Scalar(127.5, 0, 0));      //深紅色
	maincolor.push_back(Scalar(255, 0, 0));		   //紅色
	maincolor.push_back(Scalar(255, 127.5, 0));	   //紅色至黃色
	maincolor.push_back(Scalar(255, 255, 0));	   //黃色
	maincolor.push_back(Scalar(127.5, 255, 0));	   //黃色至綠色
	maincolor.push_back(Scalar(0, 255, 0));		   //綠色
	maincolor.push_back(Scalar(0, 255, 127.5));	   //綠色至青色
	maincolor.push_back(Scalar(0, 255, 255));	   //青色
	maincolor.push_back(Scalar(0, 127.5, 255));	   //青色至藍色
	maincolor.push_back(Scalar(0, 0, 255));		   //藍色
	maincolor.push_back(Scalar(0, 0, 127.5));      //深藍色

	int layer = 15;		//各漸層漸變階層數

	vector<Scalar> makeColorbarIndex;
	for (int i = 0; i < maincolor.size() - 1; i++)
	{
		for (int j = 0; j < layer; j++)
		{
			double r = maincolor[i][0] + (maincolor[i + 1][0] - maincolor[i][0]) / layer*j;
			double g = maincolor[i][1] + (maincolor[i + 1][1] - maincolor[i][1]) / layer*j;
			double b = maincolor[i][2] + (maincolor[i + 1][2] - maincolor[i][2]) / layer*j;
			makeColorbarIndex.push_back(Scalar(r, g, b));
		}
	}

	return makeColorbarIndex;
}

Mat Colorbar::makeColorbarColor(int length, int flag)
{
	int width = length / 20;

	Mat colorbarImg;
	if (flag == 0)
		colorbarImg.create(length, width, CV_8UC3);
	else
		colorbarImg.create(width, length, CV_8UC3);

	const vector<Scalar> colorbarColorIndex = Colorbar::makeColorbarColorIndex();

	for (int i = 0; i < length; ++i)
	{
		double level = (double)i / length * (colorbarColorIndex.size() - 1);					//計算高度對應之漸層色的實際索引位置
		int level0 = (int)level;																//計算高度對應之漸層色的索引位置下界
		int level1 = level0 + 1;																//計算高度對應之漸層色的索引位置上界
		if (level1 == colorbarColorIndex.size()) level1 = colorbarColorIndex.size() - 1;		//限制漸層色索引位置之上界不能超過總漸層索引的上界(if levelDown=59; levelUp =59)
		double levelSpacing = level - level0;													//計算實際索引位置至索引位置下界的距離
																								//levelSpacing = 0; // uncomment to see original color wheel  

		for (int b = 0; b < 3; b++)
		{
			double color0 = colorbarColorIndex[level0][b] / 255.0;
			double color1 = colorbarColorIndex[level1][b] / 255.0;
			double color = (1 - levelSpacing) * color0 + levelSpacing * color1;		//漸層色內插使顏色漸變更平滑

			for (int j = 0; j < width; ++j)
				if (flag == 0)
					colorbarImg.at<Vec3b>(i, j)[2 - b] = (int)(255.0 * color);
				else
					colorbarImg.at<Vec3b>(j, i)[2 - b] = (int)(255.0 * color);
		}
	}
	return colorbarImg;
}

vector<string> Colorbar::indexString(double upperbound, double lowerbound, int num)
{
	vector<string> index;
	if (num < 2 || num > 11)
	{
		cout << "Number of Index must greater than 2 and less than 12" << endl;
		cout << "Using default number of index equal to 3." << endl;
		num = 3;
	}

	double spacing = (upperbound - lowerbound) / (num - 1);


	std::ostringstream streamObj;
	for (int i = 0; i < num; ++i)
	{
		double value = upperbound - spacing*i;
		streamObj << std::fixed << std::setprecision(2) << value;
		index.push_back(streamObj.str());
		streamObj.str("");
		streamObj.clear();
	}
	return index;
}

Mat Colorbar::makeColorbar(int length, vector<string> index, string unit, int flag)
{
	int n = index.size();		//索引字個數
	int L = 0.8*length;			//色條長度
	int W = 0.04*length;		//色條寬度

								/*查詢刻度字體長寬*/
	int font_face = cv::FONT_HERSHEY_COMPLEX;
	double font_scale = length / 1400.0;			//待修改
	int thickness = ceil(length / 1200.0);
	int baseline;					//基線高度

	Size index_size = getTextSize(index[0], font_face, font_scale, thickness, &baseline);
	int indexx = index_size.width;
	int indexy = index_size.height;

	/*查詢單位字體長寬*/
	Size unit_size = getTextSize(unit, font_face, font_scale, thickness, &baseline);
	int unitx = unit_size.width;
	int unity = unit_size.height;

	/*創建色條*/
	Mat colorbarColorColor = makeColorbarColor(L, flag);

	/*標示線長度*/
	int linelength = ceil(9 * length / 1200.0);

	/*加入色條及刻度*/
	Mat colorbarImg;
	if (flag == 0)
	{
		/*指定色條長寬*/
		colorbarImg.create(length, W + 1.2*indexx, CV_8UC3);
		/*將色條複製至指定區域*/
		Mat colorbarlocation = colorbarImg(Rect(0, (length - L) / 2, colorbarColorColor.cols, colorbarColorColor.rows));
		colorbarColorColor.copyTo(colorbarlocation);

		/*加入標示線及刻度*/
		if (index.empty() != 1)
			for (int i = 0; i < index.size(); i++)
			{
				line(colorbarImg, Point(0, (length - L) / 2 + L / (n - 1)*i), Point(linelength, (length - L) / 2 + L / (n - 1)*i), Scalar(0, 0, 0), thickness);								//左線
				line(colorbarImg, Point(W - linelength - 1, (length - L) / 2 + L / (n - 1)*i), Point(W - 1, (length - L) / 2 + L / (n - 1)*i), Scalar(0, 0, 0), thickness);					//右線
				putText(colorbarImg, index[i], Point(W + 0.1*indexx, (length - L + 0.99*indexy) / 2 + L / (n - 1)*i), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);							//刻度
			}

		/*加入單位標示*/
		if (unit.empty() != 1)
			putText(colorbarImg, unit, Point((colorbarImg.cols - unitx) / 2, (length - L) / 4), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);		//單位
	}
	else
	{
		/*指定色條長寬*/
		colorbarImg.create(W + 2 * indexy, length, CV_8UC3);
		/*將色條複製至指定區域*/
		Mat colorbarlocation = colorbarImg(Rect((length - L) / 2, 0, colorbarColorColor.cols, colorbarColorColor.rows));
		colorbarColorColor.copyTo(colorbarlocation);

		/*加入標示線及刻度*/
		if (index.empty() != 1)
			for (int i = 0; i < index.size(); i++)
			{
				line(colorbarImg, Point((length - L) / 2 + L / (n - 1)*i, 0), Point((length - L) / 2 + L / (n - 1)*i, linelength), Scalar(0, 0, 0), thickness);								//左線
				line(colorbarImg, Point((length - L) / 2 + L / (n - 1)*i, W - linelength - 1), Point((length - L) / 2 + L / (n - 1)*i, W - 1), Scalar(0, 0, 0), thickness);					//右線
				putText(colorbarImg, index[i], Point((length - L - indexx) / 2 + L / (n - 1)*i, W + 1.5*indexy), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);				//刻度
			}
		/*加入單位標示*/
		if (unit.empty() != 1)
			putText(colorbarImg, unit, Point((length - L) / 4 - unitx / 2, (colorbarImg.rows - unity) / 2), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);								//單位
	}




	return colorbarImg;
}

Mat Colorbar::makeColorbar(int flag)
{
	int n = index.size();		//索引字個數
	int L = 0.8*length;			//色條長度
	int W = 0.04*length;		//色條寬度

								/*查詢刻度字體長寬*/
	int font_face = cv::FONT_HERSHEY_COMPLEX;
	double font_scale = length / 1400.0;			//待修改
	int thickness = ceil(length / 1200.0);
	int baseline;					//基線高度

	Size index_size = getTextSize(index[0], font_face, font_scale, thickness, &baseline);
	int indexx = index_size.width;
	int indexy = index_size.height;

	/*查詢單位字體長寬*/
	Size unit_size = getTextSize(unit, font_face, font_scale, thickness, &baseline);
	int unitx = unit_size.width;
	int unity = unit_size.height;

	/*創建色條*/
	Mat colorbarColorColor = makeColorbarColor(L, flag);

	/*標示線長度*/
	int linelength = ceil(9 * length / 1200.0);

	/*加入色條及刻度*/
	Mat colorbarImg;
	if (flag == 0)
	{
		/*指定色條長寬*/
		colorbarImg.create(length, W + 1.2*indexx, CV_8UC3);
		/*將色條複製至指定區域*/
		Mat colorbarlocation = colorbarImg(Rect(0, (length - L) / 2, colorbarColorColor.cols, colorbarColorColor.rows));
		colorbarColorColor.copyTo(colorbarlocation);

		/*加入標示線及刻度*/
		if (index.empty() != 1)
			for (int i = 0; i < index.size(); i++)
			{
				line(colorbarImg, Point(0, (length - L) / 2 + L / (n - 1)*i), Point(linelength, (length - L) / 2 + L / (n - 1)*i), Scalar(0, 0, 0), thickness);								//左線
				line(colorbarImg, Point(W - linelength - 1, (length - L) / 2 + L / (n - 1)*i), Point(W - 1, (length - L) / 2 + L / (n - 1)*i), Scalar(0, 0, 0), thickness);					//右線
				putText(colorbarImg, index[i], Point(W + 0.1*indexx, (length - L + 0.99*indexy) / 2 + L / (n - 1)*i), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);							//刻度
			}

		/*加入單位標示*/
		if (unit.empty() != 1)
			putText(colorbarImg, unit, Point((colorbarImg.cols - unitx) / 2, (length - L) / 4), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);		//單位
	}
	else
	{
		/*指定色條長寬*/
		colorbarImg.create(W + 2 * indexy, length, CV_8UC3);
		/*將色條複製至指定區域*/
		Mat colorbarlocation = colorbarImg(Rect((length - L) / 2, 0, colorbarColorColor.cols, colorbarColorColor.rows));
		colorbarColorColor.copyTo(colorbarlocation);

		/*加入標示線及刻度*/
		if (index.empty() != 1)
			for (int i = 0; i < index.size(); i++)
			{
				line(colorbarImg, Point((length - L) / 2 + L / (n - 1)*i, 0), Point((length - L) / 2 + L / (n - 1)*i, linelength), Scalar(0, 0, 0), thickness);								//左線
				line(colorbarImg, Point((length - L) / 2 + L / (n - 1)*i, W - linelength - 1), Point((length - L) / 2 + L / (n - 1)*i, W - 1), Scalar(0, 0, 0), thickness);					//右線
				putText(colorbarImg, index[i], Point((length - L - indexx) / 2 + L / (n - 1)*i, W + 1.5*indexy), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);				//刻度
			}
		/*加入單位標示*/
		if (unit.empty() != 1)
			putText(colorbarImg, unit, Point((length - L) / 4 - unitx / 2, (colorbarImg.rows - unity) / 2), font_face, font_scale, Scalar(0, 0, 0), thickness, CV_AA);								//單位
	}




	return colorbarImg;
}