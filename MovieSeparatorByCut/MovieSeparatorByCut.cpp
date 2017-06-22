//=====================================
// ����t�@�C�����J�b�g���Ƃɕ��ނ��ăt�H���_�������ăt�@�C���ɕۑ�
//=====================================

#include "stdafx.h"

#include<Windows.h>
#include<string>
#include<vector>
#include<algorithm>

#include<boost/filesystem.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

#define COMPARE_SIZE_W	(512)
#define COMPARE_SIZE_H	(512)
#define COMPARE_FRAME	(200)


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

	// �J�����g�f�B���N�g�����擾
	boost::filesystem::wpath currentDir = boost::filesystem::current_path();

	if(argc < 3)
	{
		printf("���������Ȃ����܂�\n");
		printf("LampConverter.exe ����t�@�C�� �o�͐�f�B���N�g���p�X �o�͉摜�� �o�͉摜����\n");
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

	// ����t�@�C�����J��
	cv::VideoCapture movie(movieFilePath.string());
	if(!movie.isOpened())
		return -1;

	unsigned int frameCount = (unsigned int)movie.get(CV_CAP_PROP_FRAME_COUNT);
	unsigned int FPS = (unsigned int)movie.get(CV_CAP_PROP_FPS);
	printf("frame count = %d\n", frameCount);
	printf("fps = %d\n", FPS);

	// �e�t���[���̑O�t���[���Ƃ̌덷���Z�o����
	printf("�e�t���[���̌덷�Z�o\n");
	std::vector<double> lpFrameDistance(frameCount, 0);
	{
		cv::Mat captureImage_last;
		{
			cv::Mat tmpImage;
			movie >> tmpImage;
			cv::resize(tmpImage, captureImage_last, cv::Size(COMPARE_SIZE_W, COMPARE_SIZE_H), cv::INTER_CUBIC);
		}
		cv::blur(captureImage_last, captureImage_last, cv::Size(5,5));


		for(unsigned int frame=1;frame<frameCount; frame++)
		{
			if((frame-1)%10 == 0)
			{
				printf("%6d frame", frame);
				printf("\b\b\b\b\b\b\b\b\b\b\b\b");
			}

			cv::Mat captureImage;
			{
				cv::Mat tmpImage;
				movie >> tmpImage;
				if(tmpImage.size().area() <= 0)
				{
					frameCount = frame;
					lpFrameDistance.resize(frameCount);
					break;
				}
				cv::resize(tmpImage, captureImage, cv::Size(COMPARE_SIZE_W, COMPARE_SIZE_H), cv::INTER_CUBIC);
			}
			cv::blur(captureImage, captureImage, cv::Size(5,5));

			IplImage image0 = captureImage_last;
			IplImage image1 = captureImage;

			for(unsigned int y=0; y<image0.height; y++)
			{
				for(unsigned int x=0; x<image0.width; x++)
				{
					RGB color0 = *(RGB*)(image0.imageData + y*image0.widthStep + x*image0.nChannels);
					RGB color1 = *(RGB*)(image1.imageData + y*image1.widthStep + x*image0.nChannels);

					double dist = (color1.R-color0.R)*(color1.R-color0.R)
							 + (color1.G-color0.G)*(color1.G-color0.G)
							 + (color1.B-color0.B)*(color1.B-color0.B);

					lpFrameDistance[frame] += dist;
				}
			}
			lpFrameDistance[frame] = sqrt(lpFrameDistance[frame] / (image0.width * image0.height));

			// ���݂̃t���[���𒼑O�̃t���[���ɐݒ�
			captureImage_last = captureImage;
		}
	}
	printf("\n");

	// �O�t���[���Ƃ̍������
	std::vector<double> lpFrameDistanceDist(frameCount, 0);
	std::vector<double> lpFrameDistanceDist2(frameCount, 0);
	for(unsigned int frame=2; frame<lpFrameDistance.size()-2; frame++)
	{
		lpFrameDistanceDist[frame]  = std::max(0.0, lpFrameDistance[frame]*2 - lpFrameDistance[frame-1] - lpFrameDistance[frame+1]);
		lpFrameDistanceDist2[frame] = sqrt(std::max(0.0, (lpFrameDistance[frame]*lpFrameDistance[frame]) - (lpFrameDistance[frame-1]*lpFrameDistance[frame-1]) - (lpFrameDistance[frame+1]*lpFrameDistance[frame+1])));
	}

#if 0
	// ���ϒl���Z�o
	printf("���ϒl���Z�o\n");
	double average = 0.0;
	double average2 = 0.0;
	for(unsigned int frame=2; frame<lpFrameDistanceDist.size()-1; frame++)
	{
		average  += lpFrameDistanceDist[frame];
		average2 += lpFrameDistanceDist2[frame];
	}
	average  /= (lpFrameDistanceDist.size() -3);
	average2 /= (lpFrameDistanceDist2.size() -3);

	// ���U�����߂�
	printf("���U�����߂�\n");
	double variance = 0.0f;
	double variance2 = 0.0f;
	for(unsigned int frame=2; frame<lpFrameDistanceDist.size()-1; frame++)
	{
		variance  += (lpFrameDistanceDist[frame]  - average)  * (lpFrameDistanceDist[frame]  - average);
		variance2 += (lpFrameDistanceDist2[frame] - average2) * (lpFrameDistanceDist2[frame] - average2);
	}
	variance  = sqrt(variance  / (lpFrameDistanceDist.size()  - 3));
	variance2 = sqrt(variance2 / (lpFrameDistanceDist2.size() - 3));

	// ���K��
	printf("���K��\n");
	std::vector<double> lpFrameDistanceNormalize(frameCount, 0);
	std::vector<double> lpFrameDistanceNormalize2(frameCount, 0);
	for(unsigned int frame=2; frame<lpFrameDistanceDist.size(); frame++)
	{
		lpFrameDistanceNormalize[frame]  = (lpFrameDistanceDist[frame]  - average) / variance;
		lpFrameDistanceNormalize2[frame] = (lpFrameDistanceDist2[frame] - average2) / variance2;
	}
#else
	printf("���K��\n");
	std::vector<double> lpFrameDistanceNormalize(frameCount, 0);
	std::vector<double> lpFrameDistanceNormalize2(frameCount, 0);
	for(int frame=0; frame<lpFrameDistanceDist.size(); frame++)
	{
		// ���ϒl���Z�o
		double average = 0.0;
		double average2 = 0.0;
		for(int addFrame=frame-COMPARE_FRAME; addFrame<frame+COMPARE_FRAME; addFrame++)
		{
			int tmpFrame = std::max(0, std::min((int)lpFrameDistanceDist.size()-1, addFrame));

			average  += lpFrameDistanceDist[tmpFrame];
			average2 += lpFrameDistanceDist2[tmpFrame];
		}
		average  /= (COMPARE_FRAME*2);
		average2 /= (COMPARE_FRAME*2);

		// ���U�����߂�
		double variance = 0.0f;
		double variance2 = 0.0f;
		for(int addFrame=frame-COMPARE_FRAME; addFrame<frame+COMPARE_FRAME; addFrame++)
		{
			int tmpFrame = std::max(0, std::min((int)lpFrameDistanceDist.size()-1, addFrame));

			variance  += (lpFrameDistanceDist[tmpFrame]   - average)   * (lpFrameDistanceDist[tmpFrame]  - average);
			variance2 += (lpFrameDistanceDist2[tmpFrame]  - average2)  * (lpFrameDistanceDist2[tmpFrame] - average2);
		}
		variance  = sqrt(variance  / (COMPARE_FRAME*2));
		variance2 = sqrt(variance2 / (COMPARE_FRAME*2));

		// ���K��
		lpFrameDistanceNormalize[frame]  = (lpFrameDistanceDist[frame]  - average)  / variance;
		lpFrameDistanceNormalize2[frame] = (lpFrameDistanceDist2[frame] - average2) / variance2;
	}
#endif

	// ���K���l���t�@�C���ɏ����o��
	{
		FILE* fp = fopen((exportDirPath / "log.csv").string().c_str(), "w");
		if(fp)
		{
			for(unsigned int frame=0;frame<frameCount; frame++)
			{
				fprintf(fp, "%d,%f,%f,%f,%f,%f\n", frame,
					lpFrameDistance[frame],
					lpFrameDistanceDist[frame],
					lpFrameDistanceDist2[frame],
					lpFrameDistanceNormalize[frame],
					lpFrameDistanceNormalize2[frame]);
			}
			fclose(fp);
		}
	}


	// �����擪�ɖ߂�
	printf("�����擪�ɖ߂�\n");
	movie.set(CV_CAP_PROP_POS_FRAMES, 0);

	// ������o�͂���
	printf("������o�͂���\n");
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

	unsigned int lastCutFrame = 0;
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
			if(tmpImage.size().area() <= 0)
				continue;
			cv::resize(tmpImage, captureImage, captureImage.size(), cv::INTER_CUBIC);
		}
		if(captureImage.empty())
			break;

//		if(lpFrameDistanceNormalize[frame]>3.5 || (frame<frameCount-1 && lpFrameDistanceNormalize[frame]>2.5 && lpFrameDistanceNormalize[frame+1]<-3.0))	// �J�b�g�ւ�
		if(lpFrameDistanceNormalize2[frame]>3.0 && lpFrameDistanceDist2[frame]>100)
		{
			if(frame > lastCutFrame+2)
			{
				// �V�K�̃J�b�g�t�H���_���쐬
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
			lastCutFrame = frame;
		}

		// �摜��ۑ�
		char szBuf[256];
		sprintf(szBuf, "%05d.jpg", frame);

		boost::filesystem::path exportFilePath = exportCutDirPath / szBuf;

		cv::imwrite(exportFilePath.string(), captureImage);
	}
	printf("\n");

	return 0;
}

