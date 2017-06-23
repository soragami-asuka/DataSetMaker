// ResizeAndCutImage.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
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

	if(argc < 7)
	{
		printf("���������Ȃ����܂�\n");
		printf("ConvertImage2GravisbellDataSet.exe ���̓f�B���N�g���p�X �o�͐�f�B���N�g���p�X ���T�C�Y�㕝 ���T�C�Y�㍂�� �J�b�g�㕝 �J�b�g�㍂��\n");
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
		cv::Mat image = cv::imread((importDirPath / importFilePath).string(), CV_LOAD_IMAGE_COLOR);
		if(image.data == NULL)
			continue;

		// �T�C�Y�ύX
		cv::Mat resizeImage;
		cv::resize(image, resizeImage, cv::Size(resizeWidth, resizeHeight), cv::INTER_CUBIC);

		unsigned int yCount = (resizeHeight + (cutHeight-1)) / cutHeight;
		unsigned int xCount = (resizeWidth  + (cutWidth -1)) / cutWidth;

		IplImage img = resizeImage;

		cv::Rect rect((resizeWidth - cutWidth)/2, (resizeHeight-cutHeight)/2, cutWidth, cutHeight);

		// ���ڔ͈͂�ݒ�
		cvSetImageROI(&img, rect);

		// �ۑ�
		char szFileName[256];
		sprintf(szFileName, "%s.jpg", importFilePath.stem().string().c_str());
		boost::filesystem::wpath exportFilePath = exportDirPath / importFilePath.parent_path() / szFileName;
		cvSaveImage(exportFilePath.string().c_str(), &img);

		// ���ڔ͈͉���
		cvResetImageROI(&img);
	}

	return 0;
}

