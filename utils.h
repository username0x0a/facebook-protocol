/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka

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

#define C_UNSIGNED    0x1000
//
#define C_BOOLEAN 0x0001
#define C_CHAR    0x0002
#define C_SHORT   0x0004
#define C_INTEGER 0x0008
#define C_LONG    C_INTEGER
#define C_INTEGER64   0x0010
#define C_LONGLONG    C_INTEGER64
#define C_FLOAT   0x0020
#define C_DOUBLE  0x0040
#define C_LONGDOUBLE  0x0080
#define C_TIME_T  0x0100

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
		template<typename T> T from_string(const std::string& s, unsigned short flag);
        std::string to_string( const void* data, unsigned short flag );
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

template<typename T> T utils::conversion::from_string(const std::string& s, unsigned short flag)
{
	unsigned short type = flag & 0x0FFF;
	std::ostringstream out(s);

	switch ( type )
	{

	case C_BOOLEAN:
		if ( s == "true" || s == "1" || s == "TRUE" ) return true;
		return false;

	case C_INTEGER:
		if (FLAG_CONTAINS(flag,C_UNSIGNED)) return strtoul(s.c_str(),NULL,0);
		return strtol(s.c_str(),NULL,0);

	case C_TIME_T:
	case C_INTEGER64:
		if (FLAG_CONTAINS(flag,C_UNSIGNED)) return strtoull(s.c_str(),NULL,0);
		return strtoll(s.c_str(),NULL,0);

	case C_FLOAT:
		return strtof(s.c_str(),NULL);

	case C_DOUBLE:
		return strtod(s.c_str(),NULL);

	case C_LONGDOUBLE:
		return strtold(s.c_str(),NULL);

	default:
		return NULL;

	}
}

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
