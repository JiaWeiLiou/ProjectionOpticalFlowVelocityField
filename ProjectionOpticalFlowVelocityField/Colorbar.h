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
	
	int length = 1000;			//��X������Ϫ��`����
	string unit = "(m/s)";		//��׳��(�i����)
	vector<string> index{ "0","1","2","3","4","5","6","7","8","9","10" };	//��׭�

	/*��m����*/
	vector<Scalar> makeColorbarColorIndex(void);

	/*�ѤW�U�ɳЫب��*/
	vector<string> indexString(double upperbound, double lowerbound, int indexnum);		//��פW�ɡB�U�ɡB��׼ƶq

	/*�Ыاt����ת������*/
	Mat makeColorbar(int length, vector<string> index, string unit, int flag = COLORBAR_VERTICAL);
	Mat makeColorbar(int flag = COLORBAR_VERTICAL);

private:
	/*���w����Ϭ������Ϋ���*/
	enum COLORBARFLAG { COLORBAR_VERTICAL = 0, COLORBAR_HORIZONTAL = 1 };

	/*�Ыئ��*/
	Mat makeColorbarColor(int length, int flag = COLORBAR_VERTICAL);
};

