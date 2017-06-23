// ConvertGravisbellDataSet2Image.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
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

	// �J�����g�f�B���N�g�����擾
	boost::filesystem::path workDirPath = boost::filesystem::current_path();

	if(argc < 3)
	{
		printf("���������Ȃ����܂�\n");
		printf("ConvertGravisbellDataSet2Image.exe ���̓f�B���N�g���p�X �o�͐�f�B���N�g���p�X\n");
#ifdef _DEBUG
		printf("Press Any Key to Continue\n");
		getc(stdin);
#endif
		return -1;
	}
	boost::filesystem::wpath importDirPath = argv[1];	importDirPath.normalize();
	boost::filesystem::wpath exportDirPath = argv[2];	exportDirPath.normalize();


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
					if(p.extension().string() == ".bin")
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
	for(auto imporFilePath : lpImageFilePath)
	{
		printf("Convert : %s\n",
			imporFilePath.generic_string().c_str());

		boost::filesystem::wpath exportFilePath = exportDirPath / imporFilePath.parent_path() / (imporFilePath.stem().string() + ".jpg");

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
		{
			// �t�@�C���I�[�v��
			FILE* fp = fopen((importDirPath / imporFilePath).string().c_str(), "rb");
			if(fp == NULL)
				continue;

			// �f�[�^�\����ǂݍ���
			Gravisbell::IODataStruct dataStruct;
			fread(&dataStruct, sizeof(dataStruct), 1, fp);

			// �o�b�t�@�m��
			std::vector<Gravisbell::U08> lpBuffer(dataStruct.GetDataCount());

			// �ǂݍ���
			fread(&lpBuffer[0], sizeof(Gravisbell::U08), lpBuffer.size(), fp);

			// �t�@�C���N���[�Y
			fclose(fp);

			// �摜���쐬
			CvSize size;
			size.width  = dataStruct.x;
			size.height = dataStruct.y;
			IplImage* pImageBuf = cvCreateImage(size, IPL_DEPTH_8U, dataStruct.ch);

			// �摜�֏�������
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

			// �摜��ۑ�
			cvSaveImage(exportFilePath.generic_string().c_str(), pImageBuf);

			// �摜�J��
			cvReleaseImage(&pImageBuf);
		}
	}



	return 0;
}

