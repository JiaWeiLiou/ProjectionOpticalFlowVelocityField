// optical_flow.cpp : 定義主控台應用程式的進入點。
//

#include <string>
#include <time.h>
#include <fstream>
#include <cmath>
#include "Colorbar.h"

using namespace std;
using namespace cv;

vector<Scalar> makeColorbarColorIndex(void);

Mat drawJetColorSystem(const Mat& flow, double realSize, double upperbound, double lowerbound, double fps);

static vector<Scalar> colorbarIndex = makeColorbarColorIndex();

int main()
{
	string infile;
	cout << "Please enter video path : ";
	cin >> infile;
	//infile = "C:\\Users\\Jimmy\\Desktop\\流速分析\\testvideo.avi";   //test

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
	//default = 1; //test

	double lowerbound = 0, upperbound = 10;
	int indexNum = 10;
	string unit = "(m/s)";

	if (!default)
	{
		/*設定Jet Colorbar 的上下界*/
		std::cout << "Please enter output of Jet Colorbar's lowerbound and Upperbound : " << endl;
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
	//Point2f beforept[4] = { Point2f(433,248),Point2f(652,248),Point2f(321,474),cv::Point2f(132,351) };
	//Point2f afterpt[4] = { Point2f(0,0),Point2f(200,0),Point2f(200,400),cv::Point2f(0,400) }; //像素座標系
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
	int levels = 5;
	int winsize = 9;
	int iterations = 10;
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
	std::cout << endl << "Frame No. " << frameNum << "\t";


	/*創建Jet Colorbar*/
	Colorbar colorbar;
	colorbar.index = colorbar.indexString(upperbound, lowerbound, indexNum);		//刻度上界、下界、刻度數量
	colorbar.unit = unit;
	colorbar.length = newWarpFrame.rows;
	Mat colorbarImg = colorbar.makeColorbar(0);

	Mat ImageCombine(newWarpFrame.rows, newWarpFrame.cols + colorbarImg.cols + colorbarImg.cols / 10, newWarpFrame.type(), Scalar(255, 255, 255));
	Mat LeftImage = ImageCombine(Rect(0, 0, newWarpFrame.cols, newWarpFrame.rows));
	Mat RightImage = ImageCombine(Rect(newWarpFrame.cols + colorbarImg.cols / 10, 0, colorbarImg.cols, colorbarImg.rows));
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

		frameNum++;
		cout << frameNum << "\t";

		/*Farneback光流法計算*/
		calcOpticalFlowFarneback(prevWarpGray, newWarpGray, flow, pyr_scale, levels, winsize, iterations, poly_n, poly_sigma, flags);

		/*繪製速度場*/
		Mat velocityImg = drawJetColorSystem(flow, 0.6, upperbound, lowerbound, fps);
		velocityImg.copyTo(LeftImage);

		writer.write(ImageCombine);

		prevWarpGray = newWarpGray.clone();

	}

	timeEnd = clock();
	std::cout << endl << "total time = " << (timeEnd - timeStart) / CLOCKS_PER_SEC << " s" << endl;

	return 0;
}

vector<Scalar> makeColorbarColorIndex(void)
{
	vector<Scalar> maincolor;

	maincolor.push_back(Scalar(0, 0, 127.5));      //深藍色
	maincolor.push_back(Scalar(0, 0, 255));		   //藍色
	maincolor.push_back(Scalar(0, 127.5, 255));	   //青色至藍色
	maincolor.push_back(Scalar(0, 255, 255));	   //青色
	maincolor.push_back(Scalar(0, 255, 127.5));	   //綠色至青色
	maincolor.push_back(Scalar(0, 255, 0));		   //綠色
	maincolor.push_back(Scalar(127.5, 255, 0));	   //黃色至綠色
	maincolor.push_back(Scalar(255, 255, 0));	   //黃色
	maincolor.push_back(Scalar(255, 127.5, 0));	   //紅色至黃色
	maincolor.push_back(Scalar(255, 0, 0));		   //紅色
	maincolor.push_back(Scalar(127.5, 0, 0));      //深紅色

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

Mat drawJetColorSystem(const Mat& flow, double realSize, double upperbound, double lowerbound, double fps)
{
	if (colorbarIndex.empty())	colorbarIndex = makeColorbarColorIndex();

	Mat velocityFieldImg(flow.rows, flow.cols, CV_8UC3, Scalar(0, 0, 0));
	double pixelSize = flow.rows;
	for (int y = 0; y < flow.rows; ++y)
	{
		for (int x = 0; x < flow.cols; ++x)
		{
			uchar *data = velocityFieldImg.data + velocityFieldImg.step[0] * y + velocityFieldImg.step[1] * x;
			const Point2f &fxy = flow.at<Point2f>(y, x);
			double velocity = (double)sqrt(pow(fxy.x, 2) + pow(fxy.y, 2))*fps*realSize / pixelSize;

			if (velocity <= lowerbound)
			{
				data[0] = 127;
				data[1] = 0;
				data[2] = 0;
				continue;
			}
			else if (velocity >= upperbound)
			{
				data[0] = 0;
				data[1] = 0;
				data[2] = 127;
				continue;
			}

			double fk = (velocity - lowerbound) / (upperbound - lowerbound)* (colorbarIndex.size() - 1);  //計算速度對應之索引位置;
			int k0 = (int)fk;
			int k1 = k0 + 1;
			double f = fk - k0;

			for (int b = 0; b < 3; b++)
			{
				double col0 = colorbarIndex[k0][b] / 255.0;
				double col1 = colorbarIndex[k1][b] / 255.0;
				double col = (1 - f) * col0 + f * col1;
				data[2 - b] = (int)(255.0 * col);
			}
		}
	}
	return velocityFieldImg;
}