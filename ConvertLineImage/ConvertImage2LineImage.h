//=====================================
// 画像ファイルを線画に変換する処理
//=====================================

#include<boost/filesystem.hpp>

#include<opencv/highgui.h>

/** 画像ファイルを線画に変換する */
cv::Mat ConvertImage2LineImage(cv::Mat& image);

/** 画像ファイルを読み込んで線画に変換する */
cv::Mat ConvertImage2LineImageFromFile(const boost::filesystem::path& filePath);