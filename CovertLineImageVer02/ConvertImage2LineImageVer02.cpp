//=====================================
// �摜�t�@�C�������ɕϊ����鏈��
//=====================================
#include"stdafx.h"

#include <opencv2/imgproc/imgproc.hpp>

#include"ConvertImage2LineImageVer02.h"


/** �摜�t�@�C�������ɕϊ����� */
cv::Mat ConvertImage2LineImageVer02(cv::Mat& image)
{
	cv::Mat_<double> mat_gray;
	if(image.channels() > 1)
	{
		// �O���[�X�P�[���ɕϊ�����
		cvtColor(image, mat_gray,CV_RGB2GRAY);
	}
	else
	{
		mat_gray = image;
	}
	mat_gray /= 0xFF;

	// ����������c��������
	cv::Mat_<double> element(3, 3, 1.0);
	cv::Mat_<double> mat_gray_dilate;
	cv::dilate(mat_gray, mat_gray_dilate, element);

	// �����Ă��J���[
	cv::Mat mat_line_base = cv::min(1.0f, mat_gray.mul(1.0f/mat_gray_dilate));

	// ���ϒl���Z�o
	cv::Mat_<double> mat_ave_rows;	// �s���Ƃ̕��ϒl
	cv::reduce(mat_line_base, mat_ave_rows, 1, CV_REDUCE_AVG);
	cv::Mat_<double> mat_ave_cols;	// �񂲂Ƃ̕��ϒl
	cv::reduce(mat_ave_rows, mat_ave_cols, 0, CV_REDUCE_AVG);

	// �e�v�f�ƕ��ϒl�̍����Ƃ�
	double ave = mat_ave_cols.at<double>(0,0);
	cv::Mat_<double> mat_dist_line_average = mat_line_base - ave;	// �e�v�f�ƕ��ϒl�̍�
	cv::Mat_<double> mat_dist_line_average_pow2 = mat_dist_line_average.mul(mat_dist_line_average);	// �e�v�f�ƕ��ϒl�̍���2��

	cv::Mat_<double> mat_dist_line_average_rows;	// �s���Ƃ̕��ϒl
	cv::reduce(mat_dist_line_average_pow2, mat_dist_line_average_rows, 1, CV_REDUCE_AVG);
	cv::Mat_<double> mat_dist_line_average_cols;	// �񂲂Ƃ̕��ϒl
	cv::reduce(mat_dist_line_average_rows, mat_dist_line_average_cols, 0, CV_REDUCE_AVG);

	double standard_diviation = sqrt(mat_dist_line_average_cols.at<double>(0,0));

	double bias = std::min(0.1, standard_diviation*2) * 3;


		// �摜�ɕϊ�
	cv::Mat_<double> mat_result = (mat_line_base - bias) / (ave - bias);



	return mat_result * 0xFF;
}

/** �摜�t�@�C����ǂݍ���Ő���ɕϊ����� */
cv::Mat ConvertImage2LineImageVer02FromFile(const boost::filesystem::path& filePath)
{
	// �摜�t�@�C����ǂݍ���
	cv::Mat image_gray = cv::imread(filePath.string(), CV_LOAD_IMAGE_GRAYSCALE);
	if(image_gray.data == NULL)
			return cv::Mat();

	return ConvertImage2LineImageVer02(image_gray);
}