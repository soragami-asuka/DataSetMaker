//=====================================
// 線画に画像を変換するコンバータ
//=====================================

#include "stdafx.h"

#include<Windows.h>
#include<string>
#include<set>

#include<boost/filesystem.hpp>
#include<boost/foreach.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>



int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	// カレントディレクトリを取得
	boost::filesystem::path workDirPath = boost::filesystem::current_path();

	if(argc < 3)
	{
		printf("引数が少なすぎます\n");
		printf("ConvertLineImage.exe 入力ディレクトリパス 出力先ディレクトリパス\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath importDirPath = argv[1];	importDirPath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();

	// ファイルの一覧を作成する
	std::set<boost::filesystem::wpath> lpImageFilePath;
	{
		// カレントパスを変更
		if(importDirPath.is_absolute())
			boost::filesystem::current_path(importDirPath);
		else
			boost::filesystem::current_path(workDirPath / importDirPath);

		{
			BOOST_FOREACH(const boost::filesystem::wpath& p,
				std::make_pair(boost::filesystem::recursive_directory_iterator("./"), boost::filesystem::recursive_directory_iterator()))
			{
				if (!boost::filesystem::is_directory(p))
				{
					if(p.extension().string() == ".jpg" || p.extension().string() == ".png")
					{
						lpImageFilePath.insert(p);
					}
				}
			}
		}

		// カレントパスを元に戻す
		boost::filesystem::current_path(workDirPath);
	}


	// 線画変換
	for(auto imporFilePath : lpImageFilePath)
	{
		printf("Convert : %s\n",
			imporFilePath.generic_string().c_str());

		boost::filesystem::wpath exportFilePath = exportDirPath / imporFilePath.parent_path() / (imporFilePath.stem().string() + ".jpg");

		// 親ディレクトリが存在しない限り作り続ける
		{
			boost::filesystem::wpath dirPath = exportFilePath.parent_path();
			while(!dirPath.empty())
			{
				if(boost::filesystem::exists(dirPath) && boost::filesystem::is_directory(dirPath))
					break;	// ディレクトリが存在するので終了

				// ディレクトリを作成
				boost::filesystem::create_directory(dirPath);

				dirPath = dirPath.parent_path();
			}
		}

		// 画像ファイルを読み込む
		cv::Mat image_gray = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_GRAYSCALE);
		if(image_gray.data == NULL)
			continue;


		// 白い部分を膨張させる
		cv::Mat element(5, 5, CV_8S, cv::Scalar(255));
		cv::Mat image_gray_dilate;
		cv::dilate(image_gray, image_gray_dilate, element);

		// 差を取る
		cv::Mat image_diff;
		cv::absdiff(image_gray, image_gray_dilate, image_diff);

		// 白黒反転
		cv::Mat image_diff_rev = 255-image_diff;

		// シグモイド関数を通して濃淡の極端化
		IplImage image = image_diff_rev;
		double threshold = 210;
		for(unsigned int y=0; y<image.height; y++)
		{
			for(unsigned int x=0; x<image.width; x++)
			{
				double preValue = (unsigned char)*(image.imageData + y*image.widthStep + x*image.nChannels);

				double tmpValue = (preValue - threshold);
				if(tmpValue < 0)
					tmpValue = tmpValue / threshold;
				else
					tmpValue = tmpValue / (0xFF - threshold);
				double result = 1.0 / (1.0 + exp(-5.0*tmpValue));

				*(image.imageData + y*image.widthStep + x*image.nChannels) = (unsigned char)(result * 0xFF);
			}
		}

		// 保存
		cv::imwrite(exportFilePath.string(), image_diff_rev);
	}



	return 0;
}

