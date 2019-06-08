#pragma once
/**
	@file
	@brief link mpir/mpirxx of mpir
	@author MITSUNARI Shigeo(@herumi)
*/
#if defined(_WIN32) && defined(_MT)
    #pragma comment(lib, "mpir.lib")
    #pragma comment(lib, "mpirxx.lib")
#endif
