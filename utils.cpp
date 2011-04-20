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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/utils.cpp $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#include "common.h"

std::string utils::url::encode(const std::string &s)
{
	char *encoded = reinterpret_cast<char*>(CallService( MS_NETLIB_URLENCODE,
		0,reinterpret_cast<LPARAM>(s.c_str()) ));
	std::string ret = encoded;
	HeapFree(GetProcessHeap(),0,encoded);

	return ret;
}

std::string utils::time::unix_timestamp( )
{
	time_t in = ::time( NULL );
	return utils::conversion::to_string( ( void* )&in, C_TIME_T );
}

std::string utils::time::mili_timestamp( )
{
	SYSTEMTIME st;
	std::string timestamp = utils::time::unix_timestamp();
	GetSystemTime(&st);
	timestamp.append(utils::conversion::to_string( ( void* )&st.wMilliseconds, C_UNSIGNED | C_SHORT ));
	return timestamp;
}

std::string utils::conversion::to_string( const void* data, unsigned short flag )
{
	std::ostringstream out;

	unsigned short type = flag & 0x0FFF;

	switch ( type )
	{

	case C_BOOLEAN:
		if ( (*( bool* )data) == true ) return "true";
		return "false";

	case C_CHAR:
		if (FLAG_CONTAINS(flag,C_UNSIGNED)) out << 0+(*( unsigned char* )data);
		else out << 0+(*( char* )data);
		break;

	case C_SHORT:
		if (FLAG_CONTAINS(flag,C_UNSIGNED)) out << 0+(*( unsigned short* )data);
		else out << 0+(*( short* )data);
		break;

	case C_INTEGER:
		if (FLAG_CONTAINS(flag,C_UNSIGNED)) out << (*( unsigned int* )data);
		else out << (*( int* )data);
		break;

	case C_INTEGER64:
		if (FLAG_CONTAINS(flag,C_UNSIGNED)) out << (*( unsigned long long* )data);
		else out << (*( long long* )data);
		break;

	// http://upload.wikimedia.org/wikipedia/commons/d/d2/Float_example.svg
	case C_FLOAT:
		out << std::setprecision(std::numeric_limits<float>::digits10+5) << (*( float* )data);
		break;

	// http://upload.wikimedia.org/wikipedia/commons/a/a9/IEEE_754_Double_Floating_Point_Format.svg
	case C_DOUBLE:
		out << std::setprecision(std::numeric_limits<double>::digits10+2) << (*( double* )data);
		break;

	// http://upload.wikimedia.org/wikipedia/commons/2/26/IEEE_754_Extended_Floating_Point_Format.svg
	case C_LONGDOUBLE:
		out << std::setprecision(std::numeric_limits<long double>::digits10+2) << (*( long double* )data);
		break;

	case C_TIME_T:
		out << (*( time_t* )data);
		break;

	}

	return out.str( );
}

void utils::text::replace_first( std::string* data, std::string from, std::string to )
{
	std::string::size_type position = 0;

	if ( ( position = data->find(from, position) ) != std::string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

void utils::text::replace_all( std::string* data, std::string from, std::string to )
{
	std::string::size_type position = 0;

	while ( ( position = data->find( from, position ) ) != std::string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

unsigned int utils::text::count_all( std::string* data, std::string term )
{
	unsigned int count = 0;
	std::string::size_type position = 0;

	while ( ( position = data->find( term, position ) ) != std::string::npos )
	{
		count++;
		position++;
	}

	return count;
}

std::string utils::text::special_expressions_decode( std::string data )
{
	utils::text::replace_all( &data, "&amp;", "&" );
	utils::text::replace_all( &data, "&quot;", "\"" );
	utils::text::replace_all( &data, "&#039;", "'" );
	utils::text::replace_all( &data, "&lt;", "<" );
	utils::text::replace_all( &data, "&gt;", ">" );

	utils::text::replace_all( &data, "&hearts;", "\xE2\x99\xA5" ); // direct byte replacement
//	utils::text::replace_all( &data, "&hearts;", "\\u2665" );      // indirect slashu replacement

	utils::text::replace_all( &data, "\\/", "/" );
	utils::text::replace_all( &data, "\\\\", "\\" );
	utils::text::replace_all( &data, "\\n", "\n" );

	// TODO: Add more to comply general usage
	// http://www.utexas.edu/learn/html/spchar.html
	// http://www.webmonkey.com/reference/Special_Characters
	// http://www.degraeve.com/reference/specialcharacters.php
	// http://www.chami.com/tips/internet/050798i.html
	// http://www.w3schools.com/tags/ref_entities.asp
	// http://www.natural-innovations.com/wa/doc-charset.html
	// http://webdesign.about.com/library/bl_htmlcodes.htm
	return data;
}

std::string utils::text::remove_html( std::string data )
{
	std::string new_string = "";

	for ( std::string::size_type i = 0; i < data.length( ); i++ )
	{
		if ( data.at(i) == '<' && data.at(i+1) != ' ' )
		{
			i = data.find( ">", i );
			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::slashu_to_utf8( std::string data )
{
	std::string new_string = "";

	for ( std::string::size_type i = 0; i < data.length( ); i++ )
	{
		if ( data.at(i) == '\\' && data.at(i+1) == 'u' )
		{
			unsigned int udn = strtol( data.substr( i + 2, 4 ).c_str(), NULL, 16 );

			if ( udn >= 128 && udn <= 2047 ) // U+0080 .. U+07FF
			{
				new_string += ( char )( 192 + ( udn / 64 ) );
				new_string += ( char )( 128 + ( udn % 64 ) );
			}
			else if ( udn >= 2048 && udn <= 65535 ) // U+0800 .. U+FFFF
			{
				new_string += ( char )( 224 + ( udn / 4096 ) );
				new_string += ( char )( 128 + ( ( udn / 64 ) % 64 ) );
				new_string += ( char )( 128 + ( udn % 64  ) );
			}
			else if ( udn <= 127 ) // U+0000 .. U+007F (should not appear)
				new_string += ( char )udn;

			i += 5;
			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::trim( std::string data )
{
	std::string spaces = " \t"; // TODO: include "nbsp"
	std::string::size_type begin = data.find_first_not_of( spaces );
	std::string::size_type end = data.find_last_not_of( spaces ) + 1;

	return (begin != std::string::npos) ? data.substr( begin, end - begin ) : "";
}

std::string utils::text::source_get_value( std::string* data, unsigned int argument_count, ... )
{
	va_list arg;
	std::string ret = "";
	std::string::size_type start = 0, end = 0;
	
	va_start( arg, argument_count );
	
	for ( unsigned int i = argument_count; i > 0; i-- ) {
		if ( i == 1 ) {
			end = data->find( va_arg( arg, char* ), start );
			if ( start == std::string::npos || end == std::string::npos )
				break;
			ret = data->substr( start, end - start );
		} else {
			std::string term = va_arg( arg, char* );
			start = data->find( term, start );
			if ( start == std::string::npos )
				break;
			start += term.length();
		}
	}
	
	va_end( arg );	
	return ret;
}

int utils::number::random( )
{
	srand( ::time( NULL ) );
	return rand( );
}

void utils::debug::info( const char* info, HWND parent )
{
	CreateDialogParam( g_hInstance, MAKEINTRESOURCE( IDD_INFO ),
		parent, FBInfoDialogProc, ( LPARAM )mir_strdup(info) );
}

void utils::debug::test( FacebookProto* fbp )
{
	return;
}

int utils::debug::log(std::string file_name, std::string text)
{
	char szFile[MAX_PATH];
	GetModuleFileNameA(g_hInstance, szFile, SIZEOF(szFile));
	std::string path = szFile;
	path = path.substr( 0, path.rfind( "\\" ) );
	path = path.substr( 0, path.rfind( "\\" ) + 1 );
	path = path + file_name.c_str() + ".txt";

	SYSTEMTIME time;
	GetSystemTime( &time );

	std::ofstream out( path.c_str(), std::ios_base::out | std::ios_base::app | std::ios_base::ate );
	out << "[" << (time.wHour < 10 ? "0" : "") << time.wHour << ":" << (time.wMinute < 10 ? "0" : "") << time.wMinute << ":" << (time.wSecond < 10 ? "0" : "") << time.wSecond << "] " << text << std::endl;
	out.close( );

	return EXIT_SUCCESS;
}

void __fastcall utils::mem::detract(char** str )
{
	utils::mem::detract( ( void** )str );
}

void __fastcall utils::mem::detract(void** p)
{
	utils::mem::detract((void*)(*p));
}

void __fastcall utils::mem::detract(void* p)
{
	mir_free(p);
}

void* __fastcall utils::mem::allocate(size_t size)
{
	void* p = NULL;

	if (size)
	{
		p = malloc(size);

		if (p)
			ZeroMemory(p, size);
	}
	return p;
}

int ext_to_format(const std::string &ext)
{
	for(size_t i=0; i<SIZEOF(formats); i++)
	{
		if(ext == formats[i].ext)
			return formats[i].fmt;
	}
	
	return PA_FORMAT_UNKNOWN;
}


// OBSOLETE

void MB( const char* m )
{
	MessageBoxA( NULL, m, NULL, MB_OK );
}

void MBI( int a )
{
	char b[32];
	itoa( a, b, 10 );
	MB( b );
}
