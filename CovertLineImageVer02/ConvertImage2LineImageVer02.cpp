//=====================================
// 画像ファイルを線画に変換する処理
//=====================================
#include"stdafx.h"

#include <opencv2/imgproc/imgproc.hpp>

#include"ConvertImage2LineImageVer02.h"


/** 画像ファイルを線画に変換する */
cv::Mat ConvertImage2LineImageVer02(cv::Mat& image)
{
	cv::Mat_<double> mat_gray;
	if(image.channels() > 1)
	{
		// グレースケールに変換する
		cvtColor(image, mat_gray,CV_RGB2GRAY);
	}
	else
	{
		mat_gray = image;
	}
	mat_gray /= 0xFF;

	// 白い部分を膨張させる
	cv::Mat_<double> element(3, 3, 1.0);
	cv::Mat_<double> mat_gray_dilate;
	cv::dilate(mat_gray, mat_gray_dilate, element);

	// 覆い焼きカラー
	cv::Mat mat_line_base = cv::min(1.0f, mat_gray.mul(1.0f/mat_gray_dilate));

	// 平均値を算出
	cv::Mat_<double> mat_ave_rows;	// 行ごとの平均値
	cv::reduce(mat_line_base, mat_ave_rows, 1, CV_REDUCE_AVG);
	cv::Mat_<double> mat_ave_cols;	// 列ごとの平均値
	cv::reduce(mat_ave_rows, mat_ave_cols, 0, CV_REDUCE_AVG);

	// 各要素と平均値の差をとる
	double ave = mat_ave_cols.at<double>(0,0);
	cv::Mat_<double> mat_dist_line_average = mat_line_base - ave;	// 各要素と平均値の差
	cv::Mat_<double> mat_dist_line_average_pow2 = mat_dist_line_average.mul(mat_dist_line_average);	// 各要素と平均値の差の2乗

	cv::Mat_<double> mat_dist_line_average_rows;	// 行ごとの平均値
	cv::reduce(mat_dist_line_average_pow2, mat_dist_line_average_rows, 1, CV_REDUCE_AVG);
	cv::Mat_<double> mat_dist_line_average_cols;	// 列ごとの平均値
	cv::reduce(mat_dist_line_average_rows, mat_dist_line_average_cols, 0, CV_REDUCE_AVG);

	double standard_diviation = sqrt(mat_dist_line_average_cols.at<double>(0,0));

	double bias = std::min(0.1, standard_diviation*2) * 3;


		// 画像に変換
	cv::Mat_<double> mat_result = (mat_line_base - bias) / (ave - bias);



	return mat_result * 0xFF;
}

/** 画像ファイルを読み込んで線画に変換する */
cv::Mat ConvertImage2LineImageVer02FromFile(const boost::filesystem::path& filePath)
{
	// 画像ファイルを読み込む
	cv::Mat image_gray = cv::imread(filePath.string(), CV_LOAD_IMAGE_GRAYSCALE);
	if(image_gray.data == NULL)
			return cv::Mat();

	return ConvertImage2LineImageVer02(image_gray);
}