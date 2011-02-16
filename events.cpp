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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/events.cpp $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#include "common.h"

int FacebookProto::Test( WPARAM wparam, LPARAM lparam )
{
	facy.feeds( );
	return FALSE;
}

int FacebookProto::Log(const char *fmt,...)
{
	if ( getByte( FACEBOOK_KEY_LOGGING_ENABLE, 0 ) != TRUE )
		return EXIT_SUCCESS;

	va_list va;
	char text[65535];
	ScopedLock s(log_lock_);

	va_start(va,fmt);
	mir_vsnprintf(text,sizeof(text),fmt,va);
	va_end(va);

	return utils::debug::log( m_szModuleName, text );
}

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {

	case WM_COMMAND: {
		//Get the plugin data (we need the PopUp service to do it)
		TCHAR* data = (TCHAR*)PUGetPluginData(hwnd);
		if (data != NULL) {
//			char *url = mir_t2a_cp(data,CP_UTF8);
			std::string url2 = mir_t2a_cp(data,CP_UTF8);
			std::string url = "http://www.facebook.com";
			if ( url2.substr(0,4) != "http" ) {
				url.append(url2);
				CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) url.c_str() );
			} else
				CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) url2.c_str() );
		}

		// After a click, destroy popup
		PUDeletePopUp(hwnd);
		} break;

	case WM_CONTEXTMENU: 
		PUDeletePopUp(hwnd);
		break;

	case UM_FREEPLUGINDATA: {
		// After close, free
		TCHAR* url = (TCHAR*)PUGetPluginData(hwnd);
		if (url != NULL)
			mir_free(url);
		} return FALSE;

	default:
		break; 

	}

	return DefWindowProc(hwnd, message, wParam, lParam);
};

int FacebookProto::NotifyEvent(TCHAR* title, TCHAR* info, HANDLE contact, DWORD flags, TCHAR* szUrl)
{
	int ret; int timeout; COLORREF colorBack = 0; COLORREF colorText = 0;

	switch ( flags ) {

	case FACEBOOK_EVENT_CLIENT:
		if ( !getByte( FACEBOOK_KEY_EVENT_CLIENT_ENABLE, 0 ) )
			return EXIT_SUCCESS;
		if ( getByte( FACEBOOK_KEY_EVENT_CLIENT_DEFAULT, 0 ) != TRUE ) {
			colorBack = getDword( FACEBOOK_KEY_EVENT_CLIENT_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_CLIENT_COLTEXT, DEFAULT_EVENT_COLTEXT ); }
		timeout = getDword( FACEBOOK_KEY_EVENT_CLIENT_TIMEOUT, 0 );
		flags |= NIIF_WARNING;
		break;

	case FACEBOOK_EVENT_NEWSFEED:
		if ( !getByte( FACEBOOK_KEY_EVENT_FEEDS_ENABLE, 0 ) )
			return EXIT_SUCCESS;
		if ( getByte( FACEBOOK_KEY_EVENT_FEEDS_DEFAULT, 0 ) != TRUE ) {
			colorBack = getDword( FACEBOOK_KEY_EVENT_FEEDS_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_FEEDS_COLTEXT, DEFAULT_EVENT_COLTEXT ); }
		timeout = getDword( FACEBOOK_KEY_EVENT_FEEDS_TIMEOUT, 0 );
		SkinPlaySound( "NewsFeed" );
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_NOTIFICATION:
		if ( !getByte( FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, 0 ) )
			return EXIT_SUCCESS;
		if ( getByte( FACEBOOK_KEY_EVENT_NOTIFICATIONS_DEFAULT, 0 ) != TRUE ) {
			colorBack = getDword( FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLTEXT, DEFAULT_EVENT_COLTEXT ); }
		timeout = getDword( FACEBOOK_KEY_EVENT_NOTIFICATIONS_TIMEOUT, 0 );
		SkinPlaySound( "Notification" );
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_OTHER:
		if ( !getByte( FACEBOOK_KEY_EVENT_OTHER_ENABLE, 0 ) )
			return EXIT_SUCCESS;
		if ( getByte( FACEBOOK_KEY_EVENT_OTHER_DEFAULT, 0 ) != TRUE ) {
			colorBack = getDword( FACEBOOK_KEY_EVENT_OTHER_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_OTHER_COLTEXT, DEFAULT_EVENT_COLTEXT ); }
		timeout = getDword( FACEBOOK_KEY_EVENT_OTHER_TIMEOUT, 0 );
		SkinPlaySound( "OtherEvent" );
		flags |= NIIF_INFO;
		break;

	}

	if ( !getByte(FACEBOOK_KEY_SYSTRAY_NOTIFY,DEFAULT_SYSTRAY_NOTIFY) ) {
		if (ServiceExists(MS_POPUP_ADDPOPUP)) {
			POPUPDATAT pd;
			pd.colorBack = colorBack;
			pd.colorText = colorText;
			pd.iSeconds = timeout;
			pd.lchContact = contact;
			pd.lchIcon = GetIcon(1); // TODO: Icon test
			pd.PluginData = szUrl;
			pd.PluginWindowProc = (WNDPROC)PopupDlgProc;
			lstrcpy(pd.lptzContactName, title);
			lstrcpy(pd.lptzText, info);
			ret = PUAddPopUpT(&pd);

			if (ret == 0)
				return EXIT_FAILURE;
		}
	} else {
		if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
			MIRANDASYSTRAYNOTIFY err;
			int niif_flags = flags;
			REMOVE_FLAG( niif_flags, FACEBOOK_EVENT_CLIENT |
			                         FACEBOOK_EVENT_NEWSFEED |
			                         FACEBOOK_EVENT_NOTIFICATION |
			                         FACEBOOK_EVENT_OTHER );
			err.szProto = m_szModuleName;
			err.cbSize = sizeof(err);
			err.dwInfoFlags = NIIF_INTERN_TCHAR | niif_flags;
			err.tszInfoTitle = title;
			err.tszInfo = info;
			err.uTimeout = 1000 * timeout;
			ret = CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & err);

			if (ret == 0)
				return EXIT_FAILURE;
		} 
	}

	if (FLAG_CONTAINS(flags, FACEBOOK_EVENT_CLIENT))
		MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);

	return EXIT_SUCCESS;
}
