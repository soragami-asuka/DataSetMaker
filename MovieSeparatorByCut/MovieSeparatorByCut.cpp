//=====================================
// 動画ファイルをカットごとに分類してフォルダ分けしてファイルに保存
//=====================================

#include "stdafx.h"

#include<Windows.h>
#include<string>
#include<vector>

#include<boost/filesystem.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

struct RGB
{
	unsigned char B;
	unsigned char G;
	unsigned char R;
};

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
		printf("LampConverter.exe 動画ファイル 出力先ディレクトリパス 出力画像幅 出力画像高さ\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath movieFilePath = argv[1];	movieFilePath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();
	unsigned int image_width  = _wtoi(argv[3]);
	unsigned int image_heigth = _wtoi(argv[4]);

	// 動画ファイルを開く
	cv::VideoCapture movie(movieFilePath.string());
	if(!movie.isOpened())
		return -1;

	unsigned int frameCount = (unsigned int)movie.get(CV_CAP_PROP_FRAME_COUNT);
	unsigned int FPS = (unsigned int)movie.get(CV_CAP_PROP_FPS);
	printf("frame count = %d\n", frameCount);
	printf("fps = %d\n", FPS);

	// 各フレームの前フレームとの誤差を算出する
	printf("各フレームの誤差算出\n");
	std::vector<__int64> lpFrameDistance(frameCount, 0);
	{
		cv::Mat captureImage_last = cv::Mat::ones(image_heigth, image_width, CV_32F);
		{
			cv::Mat tmpImage;
			movie >> tmpImage;
			cv::resize(tmpImage, captureImage_last, captureImage_last.size(), cv::INTER_CUBIC);
		}
		cv::blur(captureImage_last, captureImage_last, cv::Size(5,5));


		for(unsigned int frame=1;frame<frameCount; frame++)
		{
			if((frame-1)%10 == 0)
			{
				printf("%6d frame", frame);
				printf("\b\b\b\b\b\b\b\b\b\b\b\b");
			}

			cv::Mat captureImage = cv::Mat::ones(image_heigth, image_width, CV_32F);
			{
				cv::Mat tmpImage;
				movie >> tmpImage;
				cv::resize(tmpImage, captureImage, captureImage.size(), cv::INTER_CUBIC);
			}
			cv::blur(captureImage, captureImage, cv::Size(5,5));

			IplImage image0 = captureImage_last;
			IplImage image1 = captureImage;

			for(unsigned int y=0; y<image_heigth; y++)
			{
				for(unsigned int x=0; x<image_width; x++)
				{
					RGB color0 = *(RGB*)(image0.imageData + y*image0.widthStep + x*image0.nChannels);
					RGB color1 = *(RGB*)(image1.imageData + y*image1.widthStep + x*image0.nChannels);

					int dist = (color1.R-color0.R)*(color1.R-color0.R)
							 + (color1.G-color0.G)*(color1.G-color0.G)
							 + (color1.B-color0.B)*(color1.B-color0.B);

					lpFrameDistance[frame] += dist;
				}
			}
			lpFrameDistance[frame] = (__int64)sqrt(lpFrameDistance[frame]);

			// 現在のフレームを直前のフレームに設定
			captureImage_last = captureImage;
		}
	}
	printf("\n");

	// 前フレームとの差を取る
	std::vector<__int64> lpFrameDistanceDist(frameCount, 0);
	for(unsigned int frame=2; frame<lpFrameDistance.size(); frame++)
	{
		lpFrameDistanceDist[frame] = lpFrameDistance[frame] - lpFrameDistance[frame-1];
	}

	// 平均値を算出
	printf("平均値を算出\n");
	double average = 0.0;
	for(unsigned int frame=2; frame<lpFrameDistanceDist.size(); frame++)
	{
		average += lpFrameDistanceDist[frame];
	}
	average /= (lpFrameDistanceDist.size() -2);


	// 分散を求める
	printf("分散を求める\n");
	double variance = 0.0f;
	for(unsigned int frame=2; frame<lpFrameDistanceDist.size(); frame++)
	{
		variance += (lpFrameDistanceDist[frame] - average) * (lpFrameDistanceDist[frame] - average);
	}
	variance = sqrt(variance / (lpFrameDistanceDist.size() - 2));

	// 正規化
	printf("正規化\n");
	std::vector<double> lpFrameDistanceNormalize(frameCount, 0);
	for(unsigned int frame=2; frame<lpFrameDistanceDist.size(); frame++)
	{
		lpFrameDistanceNormalize[frame] = (lpFrameDistanceDist[frame] - average) / variance;
	}


	// 動画を先頭に戻す
	printf("動画を先頭に戻す\n");
	movie.set(CV_CAP_PROP_POS_FRAMES, 0);

	// 動画を出力する
	printf("動画を出力する\n");
	unsigned int cutNo = 0;
	boost::filesystem::path exportCutDirPath;
	{
		char szBuf[256];
		sprintf(szBuf, "%04d", cutNo);
		exportCutDirPath = exportDirPath / szBuf;
		
		if(!boost::filesystem::is_directory(exportCutDirPath))
		{
			boost::filesystem::create_directory(exportCutDirPath);
		}
	}

	for(unsigned int frame=0;frame<frameCount; frame++)
	{
		if(frame%10 == 1)
		{
			printf("%5d frame", frame);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b");
		}

		cv::Mat captureImage = cv::Mat::ones(image_heigth, image_width, CV_32F);
		{
			cv::Mat tmpImage;
			movie >> tmpImage;
			cv::resize(tmpImage, captureImage, captureImage.size(), cv::INTER_CUBIC);
		}
		if(captureImage.empty())
			break;

		if(lpFrameDistanceNormalize[frame] > 2.0)	// カット替え
		{
			// 新規のカットフォルダを作成
			cutNo++;
			{
				char szBuf[256];
				sprintf(szBuf, "%04d", cutNo);
				exportCutDirPath = exportDirPath / szBuf;

				if(!boost::filesystem::is_directory(exportCutDirPath))
				{
					boost::filesystem::create_directory(exportCutDirPath);
				}
			}
		}

		// 画像を保存
		char szBuf[256];
		sprintf(szBuf, "%05d.jpg", frame);

		boost::filesystem::path exportFilePath = exportCutDirPath / szBuf;

		cv::imwrite(exportFilePath.string(), captureImage);
	}
	printf("\n");

	return 0;
}

