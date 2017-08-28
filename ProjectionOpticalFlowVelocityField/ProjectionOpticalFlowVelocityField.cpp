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

Colorbar colorbar;
vector<Scalar> colorbarIndex = colorbar.makeColorbarColorIndex();

Mat drawJetColorSystem(const Mat& flow, double realSize, double upperbound, double lowerbound, double fps);

int main()
{
	string infile;
	cout << "Please enter video path : ";
	cin >> infile;

	/*確認檔案是否存在*/
	VideoCapture video(infile); // open the default camera
	if (!video.isOpened())  // check if we succeeded
	{
		cout << "Error opening Video !" << endl;
		system("pause");
		return -1;
	}

	std::cout << "Whether to use the default output ((0) N (1) Y ) : ";
	bool default;
	cin >> default;

	double lowerbound = 0, upperbound = 5;
	int indexNum = 10;
	string unit = "m/s";

	if (!default)
	{
		/*設定Jet Colorbar 的上下界*/
		std::cout << "Please enter output of Jet Colorbar's lowerbound and Upperbound : ";
		std::cout << "lowerbound : ";
		cin >> lowerbound;
		std::cout << "upperbound : ";
		cin >> upperbound;
		if (lowerbound < 0 || upperbound < 0 || lowerbound > 100 || upperbound >100)
		{
			std::cout << "Error Input !" << endl;
			std::cout << "Default output of Jet Colorbar's lowerbound = 0 and Upperbound = 10";
			lowerbound = 0;
			upperbound = 10;
		}

		std::cout << "Please enter output of Jet Colorbar's index number (<12) : ";
		cin >> indexNum;
		if (indexNum < 0 || indexNum > 11)
		{
			std::cout << "Error Input !" << endl;
			std::cout << "Default output of index = 10";
			indexNum = 10;
		}

		std::cout << "Please enter output of Jet Colorbar's index unit (ex.(m/s)) : ";
		unit.clear();
		cin >> unit;
	}

	/*計算程式執行時間*/
	double timeStart, timeEnd;
	timeStart = clock();

	/*轉換前後座標*/
	Point2f beforept[4] = { Point2f(767,267),Point2f(1463,307),Point2f(1595,977),cv::Point2f(569,900) };
	Point2f afterpt[4] = { Point2f(0,0),Point2f(660,0),Point2f(660,660),cv::Point2f(0,660) };
	Size aftersize = Size(afterpt[2].x, afterpt[2].y);

	/*透視投影轉換*/
	Mat perspective_matrix = getPerspectiveTransform(beforept, afterpt);

	/*設定輸出文件名*/
	string outfile;
	int pos1 = infile.find_last_of('/\\');
	int pos2 = infile.find_last_of('.');
	string filepath(infile.substr(0, pos1));
	string infile_name(infile.substr(pos1 + 1, pos2 - pos1 - 1));
	outfile = filepath + "\\" + infile_name + "_VelocityField.avi";

	/*獲取第一幀影像*/
	Mat newFrame, newWarpFrame;
	video >> newFrame;
	cv::warpPerspective(newFrame, newWarpFrame, perspective_matrix, aftersize);

	/*將彩色影像轉換為灰階並設定為第一幀*/
	Mat newWarpGray, prevWarpGray;
	cv::cvtColor(newWarpFrame, newWarpGray, CV_BGR2GRAY);
	prevWarpGray = newWarpGray.clone();

	/*設定Farneback光流法參數*/
	double pyr_scale = 0.5;
	int levels = 3;
	int winsize = 9;
	int iterations = 7;
	int poly_n = 5;
	double poly_sigma = 1.1;
	int flags = OPTFLOW_USE_INITIAL_FLOW;
	double fps = video.get(CV_CAP_PROP_FPS);   //獲取影片幀率

	/*輸出影片基本資訊*/
	std::cout << "Video's frame width  : " << video.get(CV_CAP_PROP_FRAME_WIDTH) << " pixel" << endl;
	std::cout << "Video's frame height : " << video.get(CV_CAP_PROP_FRAME_HEIGHT) << " pixel" << endl;
	std::cout << "Video's total frames : " << video.get(CV_CAP_PROP_FRAME_COUNT) << " frames" << endl;
	std::cout << "Video's frame rate   : " << fps << " FPS" << endl;

	std::cout << endl << "Start calculate ..." << endl;

	int frameNum = 1;
	std::cout << endl << "No. " << frameNum << "\t";


	/*創建Jet Colorbar*/
	vector<string> index = colorbar.indexString(upperbound, lowerbound, indexNum);		//刻度上界、下界、刻度數量
	Mat colorbarImg =colorbar.makeColorbar(newWarpFrame.rows, index, unit, 0);

	Mat ImageCombine(newWarpFrame.rows, newWarpFrame.cols + colorbarImg.cols + colorbarImg.cols/100, newWarpFrame.type());
	Mat LeftImage = ImageCombine(Rect(0, 0, newWarpFrame.cols, newWarpFrame.rows));
	Mat RightImage = ImageCombine(Rect(newWarpFrame.cols + colorbarImg.cols / 100, 0, colorbarImg.cols, colorbarImg.rows));
	colorbarImg.copyTo(RightImage);

	/*創建輸出影片物件*/
	VideoWriter writer = VideoWriter(outfile, CV_FOURCC('D', 'I', 'V', 'X'), fps, Size(ImageCombine.size()));

	/*光流場*/
	Mat flow = Mat(newWarpFrame.size(), CV_32FC2);

	while (1)
	{
		video >> newFrame;
		if (newFrame.empty()) break;
		cv::warpPerspective(newFrame, newWarpFrame, perspective_matrix, aftersize);
		cv::cvtColor(newWarpFrame, newWarpGray, CV_BGR2GRAY);
		prevWarpGray = newWarpGray.clone();

		frameNum++;
		cout << frameNum << "\t";

		/*Farneback光流法計算*/
		calcOpticalFlowFarneback(prevWarpGray, newWarpGray, flow, pyr_scale, levels, winsize, iterations, poly_n, poly_sigma, flags);
		
		/*繪製速度場*/
		Mat velocityImg = drawJetColorSystem(flow, 16, upperbound, lowerbound, fps);
		velocityImg.copyTo(LeftImage);

		writer.write(ImageCombine);

		prevWarpGray = newWarpGray.clone();

	}

	timeEnd = clock();
	std::cout << endl << "total time = " << (timeEnd - timeStart) / CLOCKS_PER_SEC << " s" << endl;

	return 0;
}

Mat drawJetColorSystem(const Mat& flow, double realSize, double upperbound, double lowerbound, double fps)
{
	Mat velocityFieldImg(flow.rows, flow.cols, CV_8UC3, Scalar(0, 0, 0));
	double pixelSize = flow.rows;
	for (int y = 0; y < flow.rows; ++y)
	{
		for (int x = 0; x < flow.cols; ++x)
		{
			uchar *data = velocityFieldImg.data + velocityFieldImg.step[0] * y + velocityFieldImg.step[1] * x;
			const Point2f& fxy = flow.at<Point2f>(y, x);
			double velocity = (double)sqrt(pow(fxy.x, 2) + pow(fxy.y, 2))*fps*realSize / pixelSize;
			double fk = (velocity - lowerbound) / (upperbound - lowerbound)* (colorbarIndex.size() - 1);  //計算角度對應之索引位置;
			int k0 = (int)fk;
			int k1 = k0 + 1;
			float f = fk - k0;

			for (int b = 0; b < 3; b++)
			{
				float col0 = colorbarIndex[k0][b] / 255.0;
				float col1 = colorbarIndex[k1][b] / 255.0;
				float col = (1 - f) * col0 + f * col1;
				if (velocity >= upperbound)
					col = 1;
				else if (velocity <= lowerbound)
					col = 0;
				else
					col = col;
				data[2 - b] = (int)(255.0 * col);
			}

		}
	}
	return velocityFieldImg;
}