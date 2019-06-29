#pragma once

/**
	@file
	@brief link libeay32.lib of openssl
	@author MITSUNARI Shigeo(@herumi)
*/
#if defined(_WIN32) && defined(_MT)
    #pragma comment(lib, "libeay32.lib")
    #pragma comment(lib, "advapi32.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "user32.lib")
#endif
