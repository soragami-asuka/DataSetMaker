//=====================================
// �摜�t�@�C�������ɕϊ����鏈��
//=====================================
#include"stdafx.h"

#include <opencv2/imgproc/imgproc.hpp>

#include"ConvertImage2LineImage.h"


/** �摜�t�@�C�������ɕϊ����� */
cv::Mat ConvertImage2LineImage(cv::Mat& image)
{
	cv::Mat image_gray;
	if(image.channels() > 1)
	{
		// �O���[�X�P�[���ɕϊ�����
		cvtColor(image, image_gray,CV_RGB2GRAY);
	}
	else
	{
		image_gray = image;
	}

	// ����������c��������
	cv::Mat element(5, 5, CV_8S, cv::Scalar(255));
	cv::Mat image_gray_dilate;
	cv::dilate(image_gray, image_gray_dilate, element);

	// �������
	cv::Mat image_diff;
	cv::absdiff(image_gray, image_gray_dilate, image_diff);

	// �������]
	cv::Mat image_diff_rev = 255-image_diff;

	//// �V�O���C�h�֐���ʂ��ĔZ�W�̋ɒ[��
	//IplImage image = image_diff_rev;
	//double threshold = 210;
	//for(unsigned int y=0; y<image.height; y++)
	//{
	//	for(unsigned int x=0; x<image.width; x++)
	//	{
	//		double preValue = (unsigned char)*(image.imageData + y*image.widthStep + x*image.nChannels);

	//		double tmpValue = (preValue - threshold);
	//		if(tmpValue < 0)
	//			tmpValue = tmpValue / threshold;
	//		else
	//			tmpValue = tmpValue / (0xFF - threshold);
	//		double result = 1.0 / (1.0 + exp(-5.0*tmpValue));

	//		*(image.imageData + y*image.widthStep + x*image.nChannels) = (unsigned char)(result * 0xFF);
	//	}
	//}

	return image_diff_rev;
}

/** �摜�t�@�C����ǂݍ���Ő���ɕϊ����� */
cv::Mat ConvertImage2LineImageFromFile(const boost::filesystem::path& filePath)
{
	// �摜�t�@�C����ǂݍ���
	cv::Mat image_gray = cv::imread(filePath.string(), CV_LOAD_IMAGE_GRAYSCALE);
	if(image_gray.data == NULL)
			return cv::Mat();

	return ConvertImage2LineImage(image_gray);
}