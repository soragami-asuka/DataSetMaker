//=====================================
// ����̃m�C�Y����
//=====================================

#include "stdafx.h"

#include<Windows.h>
#include<string>
#include<set>
#include<algorithm>

#include<boost/filesystem.hpp>
#include<boost/foreach.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

/** �m�C�Y�������� */
void CleanNoisze(IplImage& inputBuf, IplImage& outputBuf, unsigned int filterSize);
/** �������� */
void EmphansisLine(IplImage& inputBuf, IplImage& outputBuf, unsigned int filterSize);


/** �t�B���^�[�͈͂��v�Z���� */
void CalculateFilterArea(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeY, int& areaMinX, int& areaMinY, int& areaMaxX, int& areaMaxY);
void CalculateFilterArea(const IplImage& imageBuf, int x, int y, int filterSize, int& areaMinX, int& areaMinY, int& areaMaxX, int& areaMaxY);
/** �t�B���^�[���̕��ϒl�����߂� */
float AverageByFilter(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeY);
float AverageByFilter(const IplImage& imageBuf, int x, int y, int filterSize);
/** �t�B���^�[���̕W���΍������߂� */
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeXY);
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSize);
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeY, float average);
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSize, float average);
/** �t�B���^�[���̕΍��l�����߂� */
float StandardScoreByFilter(const IplImage& imageBuf, int x, int y, int ch, int filterSizeX, int filterSizeY);
float StandardScoreByFilter(const IplImage& imageBuf, int x, int y, int ch, int filterSize);
float StandardScoreByFilter(const IplImage& imageBuf, int x, int y, int ch, float average, float deviation);

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


	// �t�@�C���̈ꗗ���쐬����
	std::set<boost::filesystem::wpath> lpImageFilePath;
	{
		// �J�����g�p�X��ύX
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

		// �J�����g�p�X�����ɖ߂�
		boost::filesystem::current_path(workDirPath);
	}


	// �摜 -> Gravisbell�f�[�^�Z�b�g�ϊ�
	for(auto importFilePath : lpImageFilePath)
	{
		printf("Convert : %s\n",
			importFilePath.generic_string().c_str());

		// �e�f�B���N�g�������݂��Ȃ������葱����
		{
			boost::filesystem::wpath dirPath = exportDirPath / importFilePath.parent_path();
			while(!dirPath.empty())
			{
				if(boost::filesystem::exists(dirPath) && boost::filesystem::is_directory(dirPath))
					break;	// �f�B���N�g�������݂���̂ŏI��

				// �f�B���N�g�����쐬
				boost::filesystem::create_directory(dirPath);

				dirPath = dirPath.parent_path();
			}
		}

		// �摜�ǂݍ���
		cv::Mat image = cv::imread((importDirPath / importFilePath).string(), CV_LOAD_IMAGE_GRAYSCALE);
		if(image.data == NULL)
			continue;
		cv::Mat imageOutput1 = image.clone();
		cv::Mat imageOutput2 = image.clone();

		IplImage inputBuf = image;
		IplImage outputBuf1 = imageOutput1;
		IplImage outputBuf2 = imageOutput2;
		// �m�C�Y����1���
		CleanNoisze(inputBuf, outputBuf1, 16);
		// �m�C�Y����2���
		CleanNoisze(outputBuf1, outputBuf2, 32);
		// �m�C�Y����3���
		CleanNoisze(outputBuf2, outputBuf1, 64);

		// ����
		EmphansisLine(outputBuf1, outputBuf2, 128);

		IplImage outputBuf = outputBuf2;

		// �摜��ۑ�
		boost::filesystem::wpath exportFilePath = exportDirPath / importFilePath.parent_path() / (importFilePath.stem().string() + ".jpg");
		cvSaveImage(exportFilePath.string().c_str(), &outputBuf);
	}


	return 0;
}


/** �m�C�Y�������� */
void CleanNoisze(IplImage& inputBuf, IplImage& outputBuf, unsigned int filterSize)
{
	// �S�̂̕��ς����߂�
	float averageAll = AverageByFilter(inputBuf, inputBuf.width/2, inputBuf.height/2, inputBuf.height)  / 0xFF;
	float deviationAll = StandardDeviationByFilter(inputBuf, inputBuf.width/2, inputBuf.height/2, inputBuf.height) / 0xFF;


	for(int y=0; y<inputBuf.height; y++)
	{
		for(int x=0; x<inputBuf.width; x++)
		{
			for(int ch=0; ch<inputBuf.nChannels; ch++)
			{
				int pos = y*inputBuf.widthStep + x*inputBuf.nChannels + ch;

				float average   = AverageByFilter(inputBuf, x, y, filterSize);
				float deviation = StandardDeviationByFilter(inputBuf, x, y, filterSize, average);
				float score = StandardScoreByFilter(inputBuf, x, y, ch, average, deviation);
//				float averageBig = AverageByFilter(inputBuf, x, y, filterSize*2) / 0xFF;
				float baseValue = (float)(unsigned char)inputBuf.imageData[pos] / 0xFF;

				average /= 0xFF;
				deviation /= 0xFF;

//				if(score > 0.0)
//					imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + ch] = (char)0xFF;
				float value = std::min(1.0f, std::max(0.0f, (score + 0.5f) ));

				float resultValue = 1.0f;
				if(baseValue < average)
				{
//					resultValue = exp(baseValue * (score + 0.5f)) - 1.0f;
//					resultValue = exp(value * (score + 0.5f)) - 1.0f;
					resultValue = baseValue;
				}
				else
				{
					resultValue = 1.0f;
				}
				

				outputBuf.imageData[pos] = (char)(unsigned char)((std::min(1.0f, std::max(0.0f, resultValue))) * 0xFF);
			}
		}
	}
}

/** �������� */
void EmphansisLine(IplImage& inputBuf, IplImage& outputBuf, unsigned int filterSize)
{
	float averageAll   = AverageByFilter(inputBuf, inputBuf.width/2, inputBuf.height/2, inputBuf.width, inputBuf.height);
	float deviationAll = StandardDeviationByFilter(inputBuf, inputBuf.width/2, inputBuf.height/2, inputBuf.width, inputBuf.height, averageAll);

	for(int y=0; y<inputBuf.height; y++)
	{
		for(int x=0; x<inputBuf.width; x++)
		{
			for(int ch=0; ch<inputBuf.nChannels; ch++)
			{
				int pos = y*inputBuf.widthStep + x*inputBuf.nChannels + ch;

				float score = StandardScoreByFilter(inputBuf, x, y, ch, averageAll, deviationAll);
				float baseValue = (float)(unsigned char)inputBuf.imageData[pos] / 0xFF;

				float average = averageAll / 0xFF;
				float deviation = deviationAll / 0xFF;

				float resultValue = 1.0f;
				if(baseValue < average)
				{
					resultValue = pow(baseValue, 1.0f + -score*10.0f);
//					resultValue = pow(baseValue, std::max(0.0f, 1.0f + -score*10.0f));
				}
				//else
				//{
				//	resultValue = baseValue;
				//}

				outputBuf.imageData[pos] = (char)(unsigned char)((std::min(1.0f, std::max(0.0f, resultValue))) * 0xFF);
			}
		}
	}
}


/** �t�B���^�[�͈͂��v�Z���� */
void CalculateFilterArea(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeY, int& areaMinX, int& areaMinY, int& areaMaxX, int& areaMaxY)
{
	areaMinX = x - filterSizeX/2;
	areaMinY = y - filterSizeY/2;
	areaMaxX = x + filterSizeX/2;
	areaMaxY = y + filterSizeY/2;

	if(areaMinX < 0)
	{
		areaMaxX = std::min(imageBuf.width, areaMaxX+(-areaMinX));
		areaMinX = 0;
	}
	if(areaMaxX > imageBuf.width)
	{
		areaMinX = std::max(0, areaMinX - (areaMaxX - imageBuf.width));
		areaMaxX = imageBuf.width;
	}

	if(areaMinY < 0)
	{
		areaMaxY = std::min(imageBuf.height, areaMaxY+(-areaMinY));
		areaMinY = 0;
	}
	if(areaMaxY > imageBuf.height)
	{
		areaMinY = std::max(0, areaMinY - (areaMaxY - imageBuf.height));
		areaMaxY = imageBuf.height;
	}
}
void CalculateFilterArea(const IplImage& imageBuf, int x, int y, int filterSize, int& areaMinX, int& areaMinY, int& areaMaxX, int& areaMaxY)
{
	CalculateFilterArea(imageBuf, x, y, filterSize, filterSize, areaMinX, areaMinY, areaMaxX, areaMaxY);
}

/** �t�B���^�[���̕��ϒl�����߂� */
float AverageByFilter(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeY)
{
	int areaMinX, areaMinY, areaMaxX, areaMaxY;
	CalculateFilterArea(imageBuf, x, y, filterSizeX, filterSizeY, areaMinX, areaMinY, areaMaxX, areaMaxY);

	float sumValue = 0.0;
	
	for(int ch=0; ch<imageBuf.nChannels; ch++)
	{
		for(int posY=areaMinY; posY<areaMaxY; posY++)
		{
			for(int posX=areaMinX; posX<areaMaxX; posX++)
			{
				sumValue += (unsigned char)imageBuf.imageData[posY*imageBuf.widthStep + posX*imageBuf.nChannels + ch];
			}
		}
	}

	return sumValue / (imageBuf.nChannels * filterSizeX * filterSizeY);
}
float AverageByFilter(const IplImage& imageBuf, int x, int y, int filterSize)
{
	return AverageByFilter(imageBuf, x, y, filterSize, filterSize);
}
/** �t�B���^�[���̕W���΍������߂� */
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeY)
{
	float average = AverageByFilter(imageBuf, x, y, filterSizeX, filterSizeY);

	return StandardDeviationByFilter(imageBuf, x, y, filterSizeX, filterSizeY, average);
}
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSize)
{
	return StandardDeviationByFilter(imageBuf, x, y, filterSize, filterSize);
}
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSizeX, int filterSizeY, float average)
{
	int areaMinX, areaMinY, areaMaxX, areaMaxY;
	CalculateFilterArea(imageBuf, x, y, filterSizeX, filterSizeY, areaMinX, areaMinY, areaMaxX, areaMaxY);

	float sumValue = 0.0;

	for(int ch=0; ch<imageBuf.nChannels; ch++)
	{
		for(int posY=areaMinY; posY<areaMaxY; posY++)
		{
			for(int posX=areaMinX; posX<areaMaxX; posX++)
			{
				float value = ((unsigned char)imageBuf.imageData[posY*imageBuf.widthStep + posX*imageBuf.nChannels + ch] - average);

				sumValue += value * value;
			}
		}
	}

	return sqrtf(sumValue / (imageBuf.nChannels * filterSizeX * filterSizeY));
}
float StandardDeviationByFilter(const IplImage& imageBuf, int x, int y, int filterSize, float average)
{
	return StandardDeviationByFilter(imageBuf, x, y, filterSize, filterSize);
}
/** �t�B���^�[���̕W���΍������߂� */
float StandardScoreByFilter(const IplImage& imageBuf, int x, int y, int ch, int filterSizeX, int filterSizeY)
{
	float average   = AverageByFilter(imageBuf, x, y, filterSizeX, filterSizeY);
	float deviation = StandardDeviationByFilter(imageBuf, x, y, filterSizeX, filterSizeY, average);

	return StandardScoreByFilter(imageBuf, x, y, ch, average, deviation);
}
float StandardScoreByFilter(const IplImage& imageBuf, int x, int y, int ch, int filterSize)
{
	return StandardScoreByFilter(imageBuf, x, y, ch, filterSize, filterSize);
}
float StandardScoreByFilter(const IplImage& imageBuf, int x, int y, int ch, float average, float deviation)
{
	return ((unsigned char)imageBuf.imageData[y*imageBuf.widthStep + x*imageBuf.nChannels + ch] - average) / deviation;
}
