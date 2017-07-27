// ChooseUseImage.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
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

	// �J�����g�f�B���N�g�����擾
	boost::filesystem::path workDirPath = boost::filesystem::current_path();

	if(argc < 3)
	{
		printf("���������Ȃ����܂�\n");
		printf("ConvertImage2GravisbellDataSet.exe ���̓f�B���N�g���p�X �o�͐�f�B���N�g���p�X\n");
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
		printf("���̓f�B���N�g����������܂���\n");
		printf("%s\n", importDirPath.generic_string().c_str());
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
	}
	if(!boost::filesystem::is_directory(exportDirPath))
	{
		printf("�o�̓f�B���N�g����������܂���\n");
		printf("%s\n", exportDirPath.generic_string().c_str());
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
	}

	// �J�����g�p�X��ύX
	if(importDirPath.is_absolute())
		boost::filesystem::current_path(importDirPath);
	else
		boost::filesystem::current_path(workDirPath / importDirPath);

	// ���l���t�@�C���ɏ����o��
#ifdef _DEBUG
	FILE* fp_log = fopen((exportDirPath / "log.csv").string().c_str(), "w");
	fprintf(fp_log, "�t�@�C����,�O���C�덷,�ʓx����,�ʓx��敽��,�ʓx���U,�P�x����,�P�x��敽��,�P�x���U,�F������,�F�����U\n");
#endif

	// �t�@�C�����������Ȃ���R�s�[����
	BOOST_FOREACH(const boost::filesystem::path& dirPath,
		std::make_pair(boost::filesystem::recursive_directory_iterator("./"), boost::filesystem::recursive_directory_iterator()))
	{
		if (boost::filesystem::is_directory(dirPath))
		{
			printf("%s\n", dirPath.string().c_str());

				// �摜�t�@�C���̈ꗗ���쐬����
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

			// ��[�ƏI�[�̉摜�t�@�C�����g�p����
			std::set<boost::filesystem::path> lpUseImageFilePath;
			lpUseImageFilePath.insert(*lpImageFilePath.begin());
			lpUseImageFilePath.insert(*lpImageFilePath.rbegin());
//			lpUseImageFilePath = lpImageFilePath;
			for(boost::filesystem::path importFilePath : lpUseImageFilePath)
			{
				// �e�f�B���N�g�����𗘗p���ďo�̓t�@�C���������߂�
				std::string exportFileName = importFilePath.filename().string();
				boost::filesystem::path parentPath = importFilePath.parent_path();
				while(!parentPath.empty() && parentPath.string() != ".")
				{
					exportFileName = parentPath.stem().string() + "_" + exportFileName;

					parentPath = parentPath.parent_path();
				}

				// �摜�t�@�C�����g�p�\���ǂ������m�F����

				// �摜�ǂݍ���
				cv::Mat image = cv::imread(importFilePath.string(), CV_LOAD_IMAGE_COLOR);
				if(image.data == NULL)
					continue;
				cv::blur(image, image, cv::Size(5,5));

				// �T�C�Y�ύX
				cv::Mat resizeImage;
				cv::resize(image, resizeImage, cv::Size(256, 256), cv::INTER_CUBIC);

				// RGB > HSV�ϊ�
				cv::Mat hsvImage;
				cv::cvtColor(resizeImage, hsvImage, CV_RGB2HSV);

				// HSV�𕪗�
				cv::Mat lpMatChannels[3];
				cv::split(hsvImage, lpMatChannels);

				// ���ς��Ƃ�
				double lpChannels[3];
				for(int ch=0; ch<3; ch++)
				{
					cv::Mat_<double> mat_ave_rows;	// �s���Ƃ̕��ϒl
					cv::reduce(lpMatChannels[ch], mat_ave_rows, 1, CV_REDUCE_AVG);
					cv::Mat_<double> mat_ave_cols;	// �񂲂Ƃ̕��ϒl
					cv::reduce(mat_ave_rows, mat_ave_cols, 0, CV_REDUCE_AVG);

					lpChannels[ch] = mat_ave_cols.at<double>(0,0);
				}
				double aveSaturation = lpChannels[1];
				double aveLightness  = lpChannels[2];

				// 2�敽�ρA���U���Ƃ�
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

				// �F���̕��ς����߂�
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
						cv::Mat_<double> mat_ave_rows;	// �s���Ƃ̕��ϒl
						cv::reduce(lpHuePos[ch], mat_ave_rows, 1, CV_REDUCE_AVG);
						cv::Mat_<double> mat_ave_cols;	// �񂲂Ƃ̕��ϒl
						cv::reduce(mat_ave_rows, mat_ave_cols, 0, CV_REDUCE_AVG);

						hueVec[ch] = mat_ave_cols.at<double>(0,0);
					}

					hueAveRad   = atan2(hueVec[1], hueVec[0]);
					hueAveDeg   = 180 * hueAveRad / 3.1415;
					hueAveValue = 0xFF * (hueAveRad + 3.1415) / 3.1415 / 2;
				}

				// �F���̕��U�����߂�
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

				// �e�F�ƃO���C�Ƃ̍������
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

					// �t�@�C�����R�s�[����
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

