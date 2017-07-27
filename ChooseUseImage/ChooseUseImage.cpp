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

	if(argc < 3)
	{
		printf("引数が少なすぎます\n");
		printf("ConvertImage2GravisbellDataSet.exe 入力ディレクトリパス 出力先ディレクトリパス\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath importDirPath = argv[1];	importDirPath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();

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

	// 数値をファイルに書き出す
#ifdef _DEBUG
	FILE* fp_log = fopen((exportDirPath / "log.csv").string().c_str(), "w");
	fprintf(fp_log, "ファイル名,グレイ誤差,彩度平均,彩度二乗平均,彩度分散,輝度平均,輝度二乗平均,輝度分散,色相平均,色相分散\n");
#endif

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
//			lpUseImageFilePath = lpImageFilePath;
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

				// RGB > HSV変換
				cv::Mat hsvImage;
				cv::cvtColor(resizeImage, hsvImage, CV_RGB2HSV);

				// HSVを分離
				cv::Mat lpMatChannels[3];
				cv::split(hsvImage, lpMatChannels);

				// 平均をとる
				double lpChannels[3];
				for(int ch=0; ch<3; ch++)
				{
					cv::Mat_<double> mat_ave_rows;	// 行ごとの平均値
					cv::reduce(lpMatChannels[ch], mat_ave_rows, 1, CV_REDUCE_AVG);
					cv::Mat_<double> mat_ave_cols;	// 列ごとの平均値
					cv::reduce(mat_ave_rows, mat_ave_cols, 0, CV_REDUCE_AVG);

					lpChannels[ch] = mat_ave_cols.at<double>(0,0);
				}
				double aveSaturation = lpChannels[1];
				double aveLightness  = lpChannels[2];

				// 2乗平均、分散をとる
				double lpChannelsAve2[3]     = {0.0, 0.0, 0.0};
				double lpChannelsVariance[3] = {0.0, 0.0, 0.0};
				for(int ch=0; ch<3; ch++)
				{
					for(int y=0; y<lpMatChannels[ch].rows; y++)
					{
						for(int x=0; x<lpMatChannels[ch].cols; x++)
						{
							double value = lpMatChannels[ch].at<unsigned char>(x,y);
							double ave = lpChannels[ch];

							lpChannelsAve2[ch]     += value * value;
							lpChannelsVariance[ch] += (value - ave) * (value - ave);
						}
					}

					lpChannelsAve2[ch]     = sqrt(lpChannelsAve2[ch]     / (lpMatChannels[ch].rows * lpMatChannels[ch].cols));
					lpChannelsVariance[ch] = sqrt(lpChannelsVariance[ch] / (lpMatChannels[ch].rows * lpMatChannels[ch].cols));
				}

				// 色相の平均を求める
				double hueAveRad	= 0.0;
				double hueAveDeg	= 0.0;
				double hueAveValue	= 0.0;
				{
					cv::Mat_<double> lpHuePos[2];
					lpHuePos[0] = cv::Mat_<double>(lpMatChannels[0].rows, lpMatChannels[0].cols);
					lpHuePos[1] = cv::Mat_<double>(lpMatChannels[0].rows, lpMatChannels[0].cols);

					for(int y=0; y<lpHuePos[0].rows; y++)
					{
						for(int x=0; x<lpHuePos[0].cols; x++)
						{
							double rot = lpMatChannels[0].at<unsigned char>(x,y);
							double rad = 3.1415 * rot / 0xFF;

							lpHuePos[0].at<double>(x, y) = cos(rad);
							lpHuePos[1].at<double>(x, y) = sin(rad);
						}
					}


					double hueVec[2];
					for(int ch=0; ch<2; ch++)
					{
						cv::Mat_<double> mat_ave_rows;	// 行ごとの平均値
						cv::reduce(lpHuePos[ch], mat_ave_rows, 1, CV_REDUCE_AVG);
						cv::Mat_<double> mat_ave_cols;	// 列ごとの平均値
						cv::reduce(mat_ave_rows, mat_ave_cols, 0, CV_REDUCE_AVG);

						hueVec[ch] = mat_ave_cols.at<double>(0,0);
					}

					hueAveRad   = atan2(hueVec[1], hueVec[0]);
					hueAveDeg   = 180 * hueAveRad / 3.1415;
					hueAveValue = 0xFF * (hueAveRad + 3.1415) / 3.1415 / 2;
				}

				// 色相の分散を求める
				double hueVariance = 0.0;
				{
					for(int y=0; y<lpMatChannels[0].rows; y++)
					{
						for(int x=0; x<lpMatChannels[0].cols; x++)
						{
							double value = lpMatChannels[0].at<unsigned char>(x,y);

							double dist = abs(value - hueAveValue);
							if(dist > 0x80)
								dist = 0xff - dist;

							hueVariance += dist * dist;
						}
					}
					hueVariance = sqrt(hueVariance / (lpMatChannels[0].rows * lpMatChannels[0].cols));
				}

				// 各色とグレイとの差を取る
				IplImage imageBuf = resizeImage;
				double grayDist = 0;
				for(int y=0; y<imageBuf.height; y++)
				{
					for(int x=0; x<imageBuf.width; x++)
					{
						unsigned char r = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + 0];
						unsigned char g = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + 1];
						unsigned char b = imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + 2];

						unsigned char gray = (r + g + b) / 3;
						grayDist += ((r-gray)*(r-gray) + (g-gray)*(g-gray) + (b-gray)*(b-gray))/3;
					}
				}
				grayDist = sqrt(grayDist / (imageBuf.height * imageBuf.width));

#ifdef _DEBUG
				fprintf(fp_log, "%s,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", exportFileName.c_str(), grayDist, lpChannels[1], lpChannelsAve2[1], lpChannelsVariance[1], lpChannels[2], lpChannelsAve2[2], lpChannelsVariance[2], hueAveValue, hueVariance);
#endif

				if(grayDist >= 5 && aveSaturation>=20 && aveLightness>=45)
				{
					boost::filesystem::path copyFromPath = importDirPath / importFilePath;	copyFromPath.normalize();
					boost::filesystem::path copyToPath   = workDirPath / exportDirPath / exportFileName;	copyToPath.normalize();

					// ファイルをコピーする
					boost::filesystem::copy(copyFromPath, copyToPath);
				}
			}
		}
	}

#ifdef _DEBUG
	fclose(fp_log);
#endif


	return 0;
}

