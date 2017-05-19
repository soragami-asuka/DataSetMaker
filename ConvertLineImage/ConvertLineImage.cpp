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
		cv::Mat image_gray = cv::imread((importDirPath / imporFilePath).string(), CV_LOAD_IMAGE_GRAYSCALE);
		if(image_gray.data == NULL)
			continue;


		// ����������c��������
		cv::Mat element(5, 5, CV_8S, cv::Scalar(255));
		cv::Mat image_gray_dilate;
		cv::dilate(image_gray, image_gray_dilate, element);

		// �������
		cv::Mat image_diff;
		cv::absdiff(image_gray, image_gray_dilate, image_diff);

		// �������]
		cv::Mat image_diff_rev = 255-image_diff;

		// �V�O���C�h�֐���ʂ��ĔZ�W�̋ɒ[��
		IplImage image = image_diff_rev;
		double threshold = 210;
		for(unsigned int y=0; y<image.height; y++)
		{
			for(unsigned int x=0; x<image.width; x++)
			{
				double preValue = (unsigned char)*(image.imageData + y*image.widthStep + x*image.nChannels);

				double tmpValue = (preValue - threshold);
				if(tmpValue < 0)
					tmpValue = tmpValue / threshold;
				else
					tmpValue = tmpValue / (0xFF - threshold);
				double result = 1.0 / (1.0 + exp(-5.0*tmpValue));

				*(image.imageData + y*image.widthStep + x*image.nChannels) = (unsigned char)(result * 0xFF);
			}
		}

		// �ۑ�
		cv::imwrite(exportFilePath.string(), image_diff_rev);
	}



	return 0;
}

