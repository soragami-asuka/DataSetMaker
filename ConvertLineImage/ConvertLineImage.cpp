//=====================================
// ����ɉ摜��ϊ�����R���o�[�^
//=====================================

#include "stdafx.h"

#include<Windows.h>
#include<string>
#include<set>

#include<boost/filesystem.hpp>
#include<boost/foreach.hpp>

#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

#include"ConvertImage2LineImage.h"



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
		printf("ConvertLineImage.exe ���̓f�B���N�g���p�X �o�͐�f�B���N�g���p�X\n");
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


	// ����ϊ�
	for(auto imporFilePath : lpImageFilePath)
	{
		printf("Convert : %s\n",
			imporFilePath.generic_string().c_str());

		boost::filesystem::wpath exportFilePath = exportDirPath / imporFilePath.parent_path() / (imporFilePath.stem().string() + ".png");

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

		// �摜�t�@�C�������ɕϊ����ēǂݍ���
		cv::Mat lineImage = ::ConvertImage2LineImageFromFile(importDirPath / imporFilePath);

		// �ۑ�
		cv::imwrite(exportFilePath.string(), lineImage);
	}



	return 0;
}

