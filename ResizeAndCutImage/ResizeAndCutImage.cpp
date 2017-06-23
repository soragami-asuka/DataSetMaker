// ResizeAndCutImage.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
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

	if(argc < 7)
	{
		printf("引数が少なすぎます\n");
		printf("ConvertImage2GravisbellDataSet.exe 入力ディレクトリパス 出力先ディレクトリパス リサイズ後幅 リサイズ後高さ カット後幅 カット後高さ\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath importDirPath = argv[1];	importDirPath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();
	unsigned int resizeWidth  = _wtoi(argv[3]);
	unsigned int resizeHeight = _wtoi(argv[4]);
	unsigned int cutWidth     = _wtoi(argv[5]);
	unsigned int cutHeight    = _wtoi(argv[6]);

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
					if(p.extension().string() == ".jpg" || p.extension().string() == ".png" || p.extension().string() == ".bmp")
					{
						lpImageFilePath.insert(p);
					}
				}
			}
		}

		// カレントパスを元に戻す
		boost::filesystem::current_path(workDirPath);
	}


	// 画像 -> Gravisbellデータセット変換
	for(auto importFilePath : lpImageFilePath)
	{
		printf("Convert : %s\n",
			importFilePath.generic_string().c_str());

		// 親ディレクトリが存在しない限り作り続ける
		{
			boost::filesystem::wpath dirPath = exportDirPath / importFilePath.parent_path();
			while(!dirPath.empty())
			{
				if(boost::filesystem::exists(dirPath) && boost::filesystem::is_directory(dirPath))
					break;	// ディレクトリが存在するので終了

				// ディレクトリを作成
				boost::filesystem::create_directory(dirPath);

				dirPath = dirPath.parent_path();
			}
		}

		// 画像読み込み
		cv::Mat image = cv::imread((importDirPath / importFilePath).string(), CV_LOAD_IMAGE_COLOR);
		if(image.data == NULL)
			continue;

		// サイズ変更
		cv::Mat resizeImage;
		cv::resize(image, resizeImage, cv::Size(resizeWidth, resizeHeight), cv::INTER_CUBIC);

		unsigned int yCount = (resizeHeight + (cutHeight-1)) / cutHeight;
		unsigned int xCount = (resizeWidth  + (cutWidth -1)) / cutWidth;

		IplImage img = resizeImage;

		cv::Rect rect((resizeWidth - cutWidth)/2, (resizeHeight-cutHeight)/2, cutWidth, cutHeight);

		// 注目範囲を設定
		cvSetImageROI(&img, rect);

		// 保存
		char szFileName[256];
		sprintf(szFileName, "%s.jpg", importFilePath.stem().string().c_str());
		boost::filesystem::wpath exportFilePath = exportDirPath / importFilePath.parent_path() / szFileName;
		cvSaveImage(exportFilePath.string().c_str(), &img);

		// 注目範囲解除
		cvResetImageROI(&img);
	}

	return 0;
}

