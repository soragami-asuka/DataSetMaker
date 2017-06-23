//============================================
// �摜�t�@�C����Gravisbell�p�̃f�[�^�Z�b�g�ɕϊ�����
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

	// �J�����g�f�B���N�g�����擾
	boost::filesystem::path workDirPath = boost::filesystem::current_path();

	if(argc < 6)
	{
		printf("���������Ȃ����܂�\n");
		printf("ConvertImage2GravisbellDataSet.exe ���̓f�B���N�g���p�X �o�͐�f�B���N�g���p�X �o�͉摜�� �o�͉摜�� �o�̓f�[�^���(0=���m�N��,1=�J���[,2=����,3=����+�q���g�F)\n");
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
		printf("�o�̓f�[�^��ʂ��͈͊O�ł�\n");
		return -1;
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
					if(p.extension().string() == ".jpg" || p.extension().string() == ".png")
					{
						lpImageFilePath.insert(p);
					}
				}
			}
		}

		// �J�����g�p�X�����ɖ߂�
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

	// �摜 -> Gravisbell�f�[�^�Z�b�g�ϊ�
	for(auto imporFilePath : lpImageFilePath)
	{
		printf("Convert : %s\n",
			imporFilePath.generic_string().c_str());

		boost::filesystem::wpath exportFilePath = exportDirPath / imporFilePath.parent_path() / (imporFilePath.stem().string() + ".bin");

		// �e�f�B���N�g�������݂��Ȃ������葱����
		{
			boost::filesystem::wpath dirPath = exportFilePath.parent_path();
			while(!dirPath.empty())
			{
				if(boost::filesystem::exists(dirPath) && boost::filesystem::is_directory(dirPath))
					break;	// �f�B���N�g�������݂���̂ŏI��

				// �f�B���N�g�����쐬
				boost::filesystem::create_directory(dirPath);

				dirPath = dirPath.parent_path();
			}
		}

		// �摜�t�@�C����ǂݍ���
		switch(export_type)
		{
		case EXPORT_TYPE_MONO:	// ���m�N���摜�Ƃ��Ď�舵��
			{
				// �摜�ǂݍ���
				cv::Mat image = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_GRAYSCALE);
				if(image.data == NULL)
					continue;

				// �T�C�Y�ύX
				cv::Mat resizeImage;
				cv::resize(image, resizeImage, cv::Size(image_width, image_height), cv::INTER_CUBIC);

				// �o�b�t�@�֏�������
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
		case EXPORT_TYPE_COLOR:	// �J���[�摜�Ƃ��Ď�舵��
			{
				// �摜�ǂݍ���
				cv::Mat image = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_COLOR);
				if(image.data == NULL)
					continue;

				// �T�C�Y�ύX
				cv::Mat resizeImage;
				cv::resize(image, resizeImage, cv::Size(image_width, image_height), cv::INTER_CUBIC);

				// �o�b�t�@�֏�������
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
				// �摜�ǂݍ���
				cv::Mat image = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_COLOR);
				if(image.data == NULL)
					continue;

				// ����ɕϊ�
				cv::Mat lineImage = ::ConvertImage2LineImage(image);
				if(lineImage.data == NULL)
					continue;

				// �T�C�Y�ύX
				cv::Mat resizeImage;
				cv::resize(lineImage, resizeImage, cv::Size(image_width, image_height), cv::INTER_CUBIC);

				// �o�b�t�@�֏�������
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

		// �t�@�C���ɏo��
		FILE* fp = fopen(exportFilePath.string().c_str(), "wb");
		if(fp == NULL)
			continue;

		fwrite(&outputDataStruct, 1, sizeof(outputDataStruct), fp);
		fwrite(&lpOutputBuffer[0], 1, lpOutputBuffer.size(), fp);

		// �t�@�C���N���[�Y
		fclose(fp);
	}

	return 0;
}

