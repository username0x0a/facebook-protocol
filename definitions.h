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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/definitions.h $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#pragma once

// Verbose allocation functions (detection of memory leaks)
#if defined _DEBUG
#define _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAPALLOC
#endif

#define CODE_BLOCK_BEGIN        {
#define CODE_BLOCK_TRY          try {
#define CODE_BLOCK_CATCH        } catch(const std::exception &e) {
#define CODE_BLOCK_INFINITE     while( true ) {
#define CODE_BLOCK_END          }

#define FLAG_CONTAINS(x,y)      ( ( x & y ) == y )
#define REMOVE_FLAG(x,y)        ( x = ( x & ~y ) )

#define LOG Log

#define LOG_NOTIFY              0
#define LOG_WARNING             1
#define LOG_ALERT               2
#define LOG_FAILURE             3
#define LOG_CRITICAL            4

#define lltoa _i64toa
#define atoll _atoi64
#define strtoll _strtoi64
#define strtoull _strtoui64
#define strtof strtod
#define strtold strtod

#if defined( _UNICODE )
#define NIIF_INTERN_TCHAR NIIF_INTERN_UNICODE // m_clist.h
#define mir_tstrdup mir_wstrdup // m_system.h
#else
#define NIIF_INTERN_TCHAR 0
#define mir_tstrdup mir_strdup
#endif
