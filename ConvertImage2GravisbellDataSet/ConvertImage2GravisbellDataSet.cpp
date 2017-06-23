//============================================
// 画像ファイルをGravisbell用のデータセットに変換する
//============================================

#include "stdafx.h"

#include<Windows.h>
#include<string>
#include<set>

#include<boost/filesystem.hpp>
#include<boost/foreach.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

#include<Gravisbell/Common/IODataStruct.h>

#include"../ConvertLineImage/ConvertImage2LineImage.h"

enum ExportType
{
	EXPORT_TYPE_MONO,
	EXPORT_TYPE_COLOR,
	EXPORT_TYPE_LINE,
	EXPORT_TYPE_LINE_HINT,

	EXPORT_TYPE_COUNT
};

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	// カレントディレクトリを取得
	boost::filesystem::path workDirPath = boost::filesystem::current_path();

	if(argc < 6)
	{
		printf("引数が少なすぎます\n");
		printf("ConvertImage2GravisbellDataSet.exe 入力ディレクトリパス 出力先ディレクトリパス 出力画像幅 出力画像高 出力データ種別(0=モノクロ,1=カラー,2=線画,3=線画+ヒント色)\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath importDirPath = argv[1];	importDirPath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();
	unsigned int image_width  = _wtoi(argv[3]);
	unsigned int image_height = _wtoi(argv[4]);
	ExportType export_type = (ExportType)_wtoi(argv[5]);

	if(export_type<0 || export_type>=EXPORT_TYPE_COUNT)
	{
		printf("出力データ種別が範囲外です\n");
		return -1;
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

	std::vector<BYTE> lpOutputBuffer;
	Gravisbell::IODataStruct outputDataStruct(1, image_width, image_height, 1);
	switch(export_type)
	{
	case EXPORT_TYPE_MONO:		lpOutputBuffer.resize(image_width * image_height * 1);	outputDataStruct.ch = 1;	break;
	case EXPORT_TYPE_COLOR:		lpOutputBuffer.resize(image_width * image_height * 3);	outputDataStruct.ch = 3;	break;
	case EXPORT_TYPE_LINE:		lpOutputBuffer.resize(image_width * image_height * 1);	outputDataStruct.ch = 1;	break;
	case EXPORT_TYPE_LINE_HINT:	lpOutputBuffer.resize(image_width * image_height * 4);	outputDataStruct.ch = 4;	break;
	}

	// 画像 -> Gravisbellデータセット変換
	for(auto imporFilePath : lpImageFilePath)
	{
		printf("Convert : %s\n",
			imporFilePath.generic_string().c_str());

		boost::filesystem::wpath exportFilePath = exportDirPath / imporFilePath.parent_path() / (imporFilePath.stem().string() + ".bin");

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
		switch(export_type)
		{
		case EXPORT_TYPE_MONO:	// モノクロ画像として取り扱う
			{
				// 画像読み込み
				cv::Mat image = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_GRAYSCALE);
				if(image.data == NULL)
					continue;

				// サイズ変更
				cv::Mat resizeImage;
				cv::resize(image, resizeImage, cv::Size(image_width, image_height), cv::INTER_CUBIC);

				// バッファへ書き込み
				IplImage imageBuf = resizeImage;
				for(int ch=0; ch<imageBuf.nChannels; ch++)
				{
					for(int y=0; y<imageBuf.height; y++)
					{
						for(int x=0; x<imageBuf.width; x++)
						{
							lpOutputBuffer[ch*image_width*image_height + y*image_width + x] = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + ch];
						}
					}
				}
			}
			break;
		case EXPORT_TYPE_COLOR:	// カラー画像として取り扱う
			{
				// 画像読み込み
				cv::Mat image = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_COLOR);
				if(image.data == NULL)
					continue;

				// サイズ変更
				cv::Mat resizeImage;
				cv::resize(image, resizeImage, cv::Size(image_width, image_height), cv::INTER_CUBIC);

				// バッファへ書き込み
				IplImage imageBuf = resizeImage;
				for(int ch=0; ch<imageBuf.nChannels; ch++)
				{
					for(int y=0; y<imageBuf.height; y++)
					{
						for(int x=0; x<imageBuf.width; x++)
						{
							lpOutputBuffer[ch*image_width*image_height + y*image_width + x] = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + ch];
						}
					}
				}
			}
			break;
		case EXPORT_TYPE_LINE:
			{
				// 画像読み込み
				cv::Mat image = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_COLOR);
				if(image.data == NULL)
					continue;

				// 線画に変換
				cv::Mat lineImage = ::ConvertImage2LineImage(image);
				if(lineImage.data == NULL)
					continue;

				// サイズ変更
				cv::Mat resizeImage;
				cv::resize(lineImage, resizeImage, cv::Size(image_width, image_height), cv::INTER_CUBIC);

				// バッファへ書き込み
				IplImage imageBuf = resizeImage;
				for(int ch=0; ch<imageBuf.nChannels; ch++)
				{
					for(int y=0; y<imageBuf.height; y++)
					{
						for(int x=0; x<imageBuf.width; x++)
						{
							lpOutputBuffer[ch*image_width*image_height + y*image_width + x] = ((unsigned char)imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + ch] > 192) ? 255 : (unsigned char)imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + ch];
						}
					}
				}
			}
			break;
		case EXPORT_TYPE_LINE_HINT:
			{
			}
			break;
		}

		// ファイルに出力
		FILE* fp = fopen(exportFilePath.string().c_str(), "wb");
		if(fp == NULL)
			continue;

		fwrite(&outputDataStruct, 1, sizeof(outputDataStruct), fp);
		fwrite(&lpOutputBuffer[0], 1, lpOutputBuffer.size(), fp);

		// ファイルクローズ
		fclose(fp);
	}

	return 0;
}

