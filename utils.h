/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/utils.h $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#pragma once

// C++ bool type
#define UTILS_CONV_BOOLEAN              0x0001 // true | false
// signed regular numbers
#define UTILS_CONV_SIGNED_NUMBER        0x0010 // 1234 | -1234
// unsigned regular numbers
#define UTILS_CONV_UNSIGNED_NUMBER      0x0020 // 1234
// miscellaneous
#define UTILS_CONV_TIME_T               0x0040 // 1234567890

template<typename T>
void CreateProtoService(const char *module,const char *service,
	int (__cdecl T::*serviceProc)(WPARAM,LPARAM),T *self)
{
	char temp[MAX_PATH*2];

	mir_snprintf(temp,sizeof(temp),"%s%s",module,service);
	CreateServiceFunctionObj(temp,( MIRANDASERVICEOBJ )*(void**)&serviceProc, self );
}

template<typename T>
void HookProtoEvent(const char* evt, int (__cdecl T::*eventProc)(WPARAM,LPARAM), T *self)
{
	::HookEventObj(evt,(MIRANDAHOOKOBJ)*(void**)&eventProc,self);
}

template<typename T>
HANDLE ForkThreadEx(void (__cdecl T::*thread)(void*),T *self,void *data = 0)
{
	return reinterpret_cast<HANDLE>( mir_forkthreadowner(
		(pThreadFuncOwner)*(void**)&thread,self,data,0));
}

template<typename T>
void ForkThread(void (__cdecl T::*thread)(void*),T *self,void *data = 0)
{
	CloseHandle(ForkThreadEx(thread,self,data));
}

namespace utils
{
	namespace url {
		std::string encode(const std::string &s);
	};

	namespace time {
		std::string unix_timestamp( );
		std::string mili_timestamp( );
	};

	namespace number {
		int random( );
	};

	namespace text {
		void replace_first( std::string* data, std::string from, std::string to );
		void replace_all( std::string* data, std::string from, std::string to );
		unsigned int count_all( std::string* data, std::string term );
		std::string special_expressions_decode( std::string data );
		std::string remove_html( std::string data );
		std::string slashu_to_utf8( std::string data );
		std::string trim( std::string data );
		std::string source_get_value( std::string* data, unsigned int argument_count, ... );
	};

	namespace conversion {
		std::string to_string( void*, WORD type );
	};

	namespace debug {
		void info( const char* info, HWND parent = NULL );
		void test( FacebookProto* fbp );
		int log(std::string file_name, std::string text);
	};

	namespace mem {
		void __fastcall detract(char** str );
		void __fastcall detract(void** p);
		void __fastcall detract(void* p);
		void* __fastcall allocate(size_t size);
	};
};

class ScopedLock
{
public:
	ScopedLock(HANDLE h) : handle_(h)
	{
		WaitForSingleObject(handle_,INFINITE);
	}
	~ScopedLock()
	{
		if(handle_)
			ReleaseMutex(handle_);
	}

	void Unlock()
	{
		ReleaseMutex(handle_);
		handle_ = 0;
	}
private:
	HANDLE handle_;
};

static const struct
{
	char *ext;
	int fmt;
} formats[] = {
	{ ".png",  PA_FORMAT_PNG  },
	{ ".jpg",  PA_FORMAT_JPEG },
	{ ".jpeg", PA_FORMAT_JPEG },
	{ ".ico",  PA_FORMAT_ICON },
	{ ".bmp",  PA_FORMAT_BMP  },
	{ ".gif",  PA_FORMAT_GIF  },
};

int ext_to_format(const std::string &ext);

void MB( const char* m );
void MBI( int a );
