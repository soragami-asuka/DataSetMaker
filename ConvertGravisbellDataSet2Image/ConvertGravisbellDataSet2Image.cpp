// ConvertGravisbellDataSet2Image.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include<Windows.h>
#include<string>
#include<set>

#include<boost/filesystem.hpp>
#include<boost/foreach.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

#include<Gravisbell/Common/IODataStruct.h>


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
		printf("ConvertGravisbellDataSet2Image.exe 入力ディレクトリパス 出力先ディレクトリパス\n");
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
					if(p.extension().string() == ".bin")
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
		{
			// ファイルオープン
			FILE* fp = fopen((importDirPath / imporFilePath).string().c_str(), "rb");
			if(fp == NULL)
				continue;

			// データ構造を読み込み
			Gravisbell::IODataStruct dataStruct;
			fread(&dataStruct, sizeof(dataStruct), 1, fp);

			// バッファ確保
			std::vector<Gravisbell::U08> lpBuffer(dataStruct.GetDataCount());

			// 読み込み
			fread(&lpBuffer[0], sizeof(Gravisbell::U08), lpBuffer.size(), fp);

			// ファイルクローズ
			fclose(fp);

			// 画像を作成
			CvSize size;
			size.width  = dataStruct.x;
			size.height = dataStruct.y;
			IplImage* pImageBuf = cvCreateImage(size, IPL_DEPTH_8U, dataStruct.ch);

			// 画像へ書き込み
			for(int ch=0; ch<pImageBuf->nChannels; ch++)
			{
				for(int y=0; y<pImageBuf->height; y++)
				{
					for(int x=0; x<pImageBuf->width; x++)
					{
						Gravisbell::F32 value = ((Gravisbell::F32)lpBuffer[ch*dataStruct.x*dataStruct.y + y*dataStruct.x + x] / 0xFF);

						pImageBuf->imageData[y*pImageBuf->widthStep + x*pImageBuf->nChannels + ch] = (char)(value * 0xFF);
					}
				}
			}

			// 画像を保存
			cvSaveImage(exportFilePath.generic_string().c_str(), pImageBuf);

			// 画像開放
			cvReleaseImage(&pImageBuf);
		}
	}



	return 0;
}

