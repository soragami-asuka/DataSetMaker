//=====================================
// �摜�t�@�C�������ɕϊ����鏈��
//=====================================

#include<boost/filesystem.hpp>

#include<opencv/highgui.h>

/** �摜�t�@�C�������ɕϊ����� */
cv::Mat ConvertImage2LineImage(cv::Mat& image);

/** �摜�t�@�C����ǂݍ���Ő���ɕϊ����� */
cv::Mat ConvertImage2LineImageFromFile(const boost::filesystem::path& filePath);