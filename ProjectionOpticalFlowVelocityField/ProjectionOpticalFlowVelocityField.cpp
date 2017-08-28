// optical_flow.cpp : 定義主控台應用程式的進入點。
//

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <time.h>
#include <fstream>
#include <cmath>
#include "Colorbar.cpp"

using namespace std;
using namespace cv;


void outputVelocity(const Mat& flow, double realSize, ofstream &ofile, double fps)
{
	double velocity;
	double pixelSize = flow.rows;
	int i, j;
	//for (int y = -1; y < flow.rows; y += step)
	//{

	//	for (int x = -1; x < flow.cols; x += step)
	//	{
	for (int y = 0; y < flow.rows; y ++)
	{

		for (int x = 0; x < flow.cols; x ++)
		{
			//i = x == -1 ? 0 : x;
			//j = y == -1 ? 0 : y;
			//const Point2f& fxy = flow.at<Point2f>(j, i);
			const Point2f& fxy = flow.at<Point2f>(y, x);
			velocity = sqrt(pow(fxy.x, 2) + pow(fxy.y, 2))*fps*realSize / pixelSize;
			ofile << velocity << "\t";
		}
		ofile << endl;
	}
	ofile << endl;
}

int main()
{
	string infile;
	string outfile, outfile2, outfile3;
	cout << "Please enter video path : ";
	cin >> infile;

	/*確認檔案是否存在*/
	VideoCapture cap(infile); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
	{
		cout << "Error opening Video !" << endl;
		system("pause");
		return -1;
	}

	/*計算程式執行時間*/
	double timeStart, timeEnd;
	timeStart = clock();

	/*轉換前後座標*/
	//Point2f beforept[4] = { Point2f(767,267),Point2f(1463,307),Point2f(1595,977),cv::Point2f(569,900) };
	//Point2f afterpt[4] = { Point2f(0,0),Point2f(660,0),Point2f(660,660),cv::Point2f(0,660) };
	//Size aftersize = Size(afterpt[2].x, afterpt[2].y);

	//修改文件名
	int pos1 = infile.find_last_of('/\\');
	int pos2 = infile.find_last_of('.');
	string filepath(infile.substr(0, pos1));
	string infile_name(infile.substr(pos1 + 1, pos2 - pos1 - 1));
	outfile = filepath + "\\" + infile_name + "_direction.avi";
	outfile2 = filepath + "\\" + infile_name + "_projectioncolor.avi";
	outfile3 = filepath + "\\" + infile_name + "_velocity.txt";

	Mat newFrame, newGray, newGray_temp, newGray_temp_temp, prevGray, warpFrame, thresholdFrame;

	Mat motion2color;

	cap >> newFrame; // get a new frame from camera
	

	/*透視投影轉換*/
	//Mat perspective_matrix = getPerspectiveTransform(beforept, afterpt);
	//warpPerspective(newFrame, warpFrame, perspective_matrix, aftersize);

	cvtColor(newFrame, newGray, CV_BGR2GRAY);

	prevGray = newGray.clone();

	double pyr_scale = 0.5;
	int levels = 3;
	int winsize = 9;
	int iterations = 5;
	int poly_n = 5;
	double poly_sigma = 1.1;
	int flags = OPTFLOW_USE_INITIAL_FLOW;

	double fps = cap.get(CV_CAP_PROP_FPS);
	fps = 5.9;

	//VideoWriter writer = VideoWriter(outfile, CV_FOURCC('D', 'I', 'V', 'X'), fps, newGray.size());
	VideoWriter colorWriter = VideoWriter(outfile, CV_FOURCC('D', 'I', 'V', 'X'), fps, Size(newFrame.cols*2, newFrame.rows));
	
	ofstream ofile(outfile3,ios::out);

	int test = 0;
	Mat flow = Mat(newGray.size(), CV_32FC2);
	Mat warpflow = Mat(newGray.size(), CV_32FC2);
	Mat oldFlow = Mat(newGray.size(), CV_32FC2);

	while (1)
	{
		++test;

		cap >> newFrame;

		if (newFrame.empty()) break;
		//warpPerspective(newFrame, warpFrame, perspective_matrix, aftersize);
		cvtColor(newFrame, newGray, CV_BGR2GRAY);

		//threshold(newGray, newGray_temp, 175, 255, THRESH_TOZERO);
		//warpPerspective(newGray_temp, thresholdFrame, perspective_matrix, aftersize);

		/*Farneback光流法計算*/
		calcOpticalFlowFarneback(prevGray, newGray, flow, pyr_scale, levels, winsize, iterations, poly_n, poly_sigma, flags);
		//warpPerspective(flow, warpflow, perspective_matrix, aftersize);
		
		outputVelocity(flow, 16, ofile, fps);
		
		if (test == 1)
			drawOptFlowMap(flow, newFrame, 20, CV_RGB(255, 0, 0));
		drawOptFlowMap(oldFlow, flow, newFrame, 20, CV_RGB(255, 0, 0));

		motionToColor(flow, motion2color);



		Mat colorCombine(motion2color.rows, motion2color.cols + newFrame.cols, motion2color.type());
		Mat imageROI,imageROI2;
		imageROI = colorCombine(Rect(0, 0, newFrame.cols, newFrame.rows));
		imageROI2 = colorCombine(Rect(newFrame.cols, 0, motion2color.cols, motion2color.rows));
		newFrame.copyTo(imageROI);
		motion2color.copyTo(imageROI2);

		/*儲存計算結果*/
		//writer.write(newFrame);
		colorWriter.write(colorCombine);

		//namedWindow("Output", WINDOW_NORMAL);
		//if (newGray.cols >= 1366 || newGray.rows >= 768)
		//	resizeWindow("Output", round(newGray.cols / 2), round(newGray.rows / 2));
		//imshow("Output", newGray);
		//waitKey(0);

		prevGray = newGray.clone();
		oldFlow = flow;

		////test
		//if (test == 100)
		//{
		//	system("pause");
		//}

	}

	//timeEnd = clock();
	//cout << "total time = " << (timeEnd - timeStart) / CLOCKS_PER_SEC << " s" << endl;
	//system("pause");
	return 0;
}