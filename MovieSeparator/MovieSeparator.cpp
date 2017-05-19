//=====================================
// 動画ファイルを画像ファイルの連番に変換するコンバータ
//=====================================

#include "stdafx.h"

#include<Windows.h>
#include<string>

#include<boost/filesystem.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	// カレントディレクトリを取得
	boost::filesystem::wpath currentDir = boost::filesystem::current_path();

	if(argc < 3)
	{
		printf("引数が少なすぎます\n");
		printf("LampConverter.exe 動画ファイル 出力先ディレクトリパス\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath movieFilePath = argv[1];	movieFilePath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();


	// 動画ファイルを開く準備
	cv::VideoCapture movie(movieFilePath.string());
	if(!movie.isOpened())
		return -1;

	unsigned int frameCount = (unsigned int)movie.get(CV_CAP_PROP_FRAME_COUNT);
	double fps = movie.get(CV_CAP_PROP_FPS);

	// 連番に変換して保存
	for(unsigned int frame=0; frame<frameCount; frame++)
	{
		cv::Mat captureImage;
		movie >> captureImage;

		cv::Mat resizeImage = cv::Mat::ones(404, 720, CV_32F);
		cv::resize(captureImage, resizeImage, resizeImage.size(), cv::INTER_CUBIC);

		if(captureImage.empty())
			break;

		char szBuf[256];
		sprintf(szBuf, "%05d.png", frame);

		boost::filesystem::path exportFilePath = exportDirPath / szBuf;

		cv::imwrite(exportFilePath.string(), resizeImage);
	}

	return 0;
}

