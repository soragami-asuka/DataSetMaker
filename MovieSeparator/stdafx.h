// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: プログラムに必要な追加ヘッダーをここで参照してください。

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif


/** newで確保されたメモリを安全に開放する */
#define SAFE_DELETE(p){if(p) delete p; p=NULL;}
/** new[]で確保された配列を安全に開放する */
#define SAFE_DELETE_ARRAY(p){if(p) delete[] p; p=NULL;}

