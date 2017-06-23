// ChooseUseImage.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

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

	if(argc < 4)
	{
		printf("引数が少なすぎます\n");
		printf("ConvertImage2GravisbellDataSet.exe 入力ディレクトリパス 出力先ディレクトリパス 閾値(0～255)\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath importDirPath = argv[1];	importDirPath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();
	unsigned int threshold_value = _wtoi(argv[3]);

	if(!boost::filesystem::is_directory(importDirPath))
	{
		printf("入力ディレクトリが見つかりません\n");
		printf("%s\n", importDirPath.generic_string().c_str());
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
	}
	if(!boost::filesystem::is_directory(exportDirPath))
	{
		printf("出力ディレクトリが見つかりません\n");
		printf("%s\n", exportDirPath.generic_string().c_str());
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
	}

	// カレントパスを変更
	if(importDirPath.is_absolute())
		boost::filesystem::current_path(importDirPath);
	else
		boost::filesystem::current_path(workDirPath / importDirPath);

	// ファイルを検索しながらコピーする
	BOOST_FOREACH(const boost::filesystem::path& dirPath,
		std::make_pair(boost::filesystem::recursive_directory_iterator("./"), boost::filesystem::recursive_directory_iterator()))
	{
		if (boost::filesystem::is_directory(dirPath))
		{
			printf("%s\n", dirPath.string().c_str());

				// 画像ファイルの一覧を作成する
			std::set<boost::filesystem::path> lpImageFilePath;
			BOOST_FOREACH(const boost::filesystem::path& p,
				std::make_pair(boost::filesystem::directory_iterator(dirPath), boost::filesystem::directory_iterator()))
			{
				if (!boost::filesystem::is_directory(p))
				{
					if(p.extension().string() == ".jpg" || p.extension().string() == ".png")
					{
						lpImageFilePath.insert(p);
					}
				}
			}
			if(lpImageFilePath.empty())
				continue;

			// 先端と終端の画像ファイルを使用する
			std::set<boost::filesystem::path> lpUseImageFilePath;
			lpUseImageFilePath.insert(*lpImageFilePath.begin());
			lpUseImageFilePath.insert(*lpImageFilePath.rbegin());
			for(boost::filesystem::path importFilePath : lpUseImageFilePath)
			{
				// 親ディレクトリ名を利用して出力ファイル名を決める
				std::string exportFileName = importFilePath.filename().string();
				boost::filesystem::path parentPath = importFilePath.parent_path();
				while(!parentPath.empty() && parentPath.string() != ".")
				{
					exportFileName = parentPath.stem().string() + "_" + exportFileName;

					parentPath = parentPath.parent_path();
				}

				// 画像ファイルが使用可能かどうかを確認する

				// 画像読み込み
				cv::Mat image = cv::imread(importFilePath.string(), CV_LOAD_IMAGE_COLOR);
				if(image.data == NULL)
					continue;
				cv::blur(image, image, cv::Size(5,5));

				// サイズ変更
				cv::Mat resizeImage;
				cv::resize(image, resizeImage, cv::Size(256, 256), cv::INTER_CUBIC);

				// 各色と平均値との差を取る
				IplImage imageBuf = resizeImage;
				double distValue = 0;
				for(int y=0; y<imageBuf.height; y++)
				{
					for(int x=0; x<imageBuf.width; x++)
					{
						unsigned char r = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + 0];
						unsigned char g = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + 1];
						unsigned char b = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + 2];

						unsigned char gray = (r + g + b) / 3;
						distValue += ((r-gray)*(r-gray) + (g-gray)*(g-gray) + (b-gray)*(b-gray))/3;
					}
				}
				distValue = sqrt(distValue / (imageBuf.height * imageBuf.width));

				if(distValue > threshold_value)
				{
					// ファイルをコピーする
					boost::filesystem::copy(importFilePath, exportDirPath / exportFileName);
				}
			}
		}
	}



	return 0;
}

