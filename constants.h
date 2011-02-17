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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/constants.h $
Revision       : $Revision: 99 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-02-12 21:32:51 +0100 (Sat, 12 Feb 2011) $

*/

#pragma once

// Version management
#include "build.h"
#define __VERSION_DWORD             PLUGIN_MAKE_VERSION(0, 1, 3, 6)
#define __PRODUCT_DWORD             PLUGIN_MAKE_VERSION(0, 9, 14, 0)
#define __VERSION_STRING            "0.1.3.6"
#define __PRODUCT_STRING            "0.9.14.0"
#define __VERSION_VS_FILE           0,1,3,6
#define __VERSION_VS_PROD           0,9,14,0
#define __VERSION_VS_FILE_STRING    "0, 1, 3, 6"
#define __VERSION_VS_PROD_STRING    "0, 9, 14, 0"
#define __API_VERSION_STRING        "3.2"

// API versions
// 3.2 -- minor change in Machine name, Chat server URL
// 3.1 -- minor change in Logout procedure
// 3.0 -- lots of source changes on Facebook site
// 2.1 -- minor Facebook Chat server URLs change
// 2.0 -- major Facebook redesign
// 1.3 -- m.facebook.com profile change of syntax
// 1.2 -- buddy_list updates allow non-cumulative update data
// 1.1 -- buddy_list now includes some reduntant data
// 1.0 -- initial implementation

// Product management
#define FACEBOOK_NAME               "Facebook"
#define FACEBOOK_URL_HOMEPAGE       "http://www.facebook.com/"
#define FACEBOOK_URL_REQUESTS       "http://www.facebook.com/reqs.php"
#define FACEBOOK_URL_MESSAGES       "http://www.facebook.com/n/?inbox"
#define PLUGIN_HOSTING_URL          "http://code.google.com/p/eternityplugins/"

// Connection
#define FACEBOOK_SERVER_REGULAR     "www.facebook.com"
#define FACEBOOK_SERVER_MOBILE      "m.facebook.com"
#define FACEBOOK_SERVER_CHAT        "%s.%s.facebook.com"
#define FACEBOOK_SERVER_LOGIN       "login.facebook.com"
#define FACEBOOK_SERVER_APPS        "apps.facebook.com"

// Limits
#define FACEBOOK_MESSAGE_LIMIT      1024
#define FACEBOOK_MESSAGE_LIMIT_TEXT "1024"
#define FACEBOOK_MIND_LIMIT         420
#define FACEBOOK_MIND_LIMIT_TEXT    "420"
#define FACEBOOK_TIMEOUTS_LIMIT     5
#define FACEBOOK_GROUP_NAME_LIMIT   100

// Defaults
#define FACEBOOK_MINIMAL_POLL_RATE      15
#define FACEBOOK_DEFAULT_POLL_RATE      24 // in seconds
#define FACEBOOK_MAXIMAL_POLL_RATE      60
#define FACEBOOK_USER_UPDATE_RATE       7200 // in seconds

#define DEFAULT_FORCE_HTTPS             0
#define DEFAULT_CLOSE_WINDOWS_ENABLE    0
#define DEFAULT_SET_MIRANDA_STATUS      0
#define DEFAULT_LOGGING_ENABLE          0
#define DEFAULT_EVENT_NOTIFICATIONS_ENABLE  1
#define DEFAULT_EVENT_FEEDS_ENABLE          1
#define DEFAULT_EVENT_OTHER_ENABLE          1
#define DEFAULT_EVENT_CLIENT_ENABLE         1
#define DEFAULT_EVENT_COLBACK           0x00ffffff
#define DEFAULT_EVENT_COLTEXT           0x00000000
#define DEFAULT_EVENT_TIMEOUT_TYPE      0
#define DEFAULT_EVENT_TIMEOUT           20
#define DEFAULT_EVENT_FEEDS_TYPE        0
#define DEFAULT_SYSTRAY_NOTIFY          0
#define DEFAULT_SHOW_OLD_FEEDS          0

#define FACEBOOK_DEFAULT_AVATAR_URL     "http://static.ak.fbcdn.net/pics/q_silhouette.gif"

// Event flags
#define FACEBOOK_EVENT_CLIENT          0x10000000 // Facebook error or info message
#define FACEBOOK_EVENT_NEWSFEED        0x20000000 // Facebook newsfeed (wall) message
#define FACEBOOK_EVENT_NOTIFICATION    0x40000000 // Facebook new notification
#define FACEBOOK_EVENT_OTHER           0x80000000 // Facebook other event - friend requests/new messages

// Facebook request types // TODO: Provide MS_ and release in FB plugin API?
#define FACEBOOK_REQUEST_API_CHECK              50  // check latest API version
#define FACEBOOK_REQUEST_LOGIN                  100 // connecting physically
#define FACEBOOK_REQUEST_SETUP_MACHINE          102 // setting machine name
#define FACEBOOK_REQUEST_KEEP_ALIVE             104 // keeping online status alive without idle
#define FACEBOOK_REQUEST_LOGOUT                 106 // disconnecting physically
#define FACEBOOK_REQUEST_HOME                   110 // getting __post_form_id__ + __fb_dtsg__ + ...
#define FACEBOOK_REQUEST_BUDDY_LIST             120 // getting regular updates (friends online, ...)
#define FACEBOOK_REQUEST_FEEDS                  125 // getting feeds
#define FACEBOOK_REQUEST_RECONNECT              130 // getting __sequence_num__ and __channel_id__
#define FACEBOOK_REQUEST_PROFILE_GET            200 // getting others' profiles
#define FACEBOOK_REQUEST_STATUS_SET             251 // setting my "What's on my mind?"
#define FACEBOOK_REQUEST_MESSAGE_SEND           300 // sending message
#define FACEBOOK_REQUEST_MESSAGES_RECEIVE       301 // receiving messages
#define FACEBOOK_REQUEST_TYPING_SEND            304 // sending typing notification
#define FACEBOOK_REQUEST_CHAT_SETTINGS          305 // closing message window, setting chat visibility

// Chat settings flags
#define FACEBOOK_CHAT_VISIBILITY    0x01
#define FACEBOOK_CHAT_CLOSE_WINDOW  0x02

// Reconnect flags
#define FACEBOOK_RECONNECT_LOGIN        "6" // When logging in // TODO: 5?
#define FACEBOOK_RECONNECT_KEEP_ALIVE   "0" // After a period, used to keep session alive // TODO: 6?
// TODO: And what about 7? :))

// User-Agents
static const struct {
	const char* user_name;
	const char* user_agent_string;
} user_agents[] = {
	{ "Miranda IM (default)", "" },
//	{ "Facebook for iPhone", "FacebookTouch2.5" },
//	{ "Facebook for iPhone Alternative", "Facebook/2.5 CFNetwork/342.1 Darwin/9.4.1" },
//	{ "Lynx 2.8.4", "Lynx/2.8.4rel.1 libwww-FM/2.14" },
	{ "Internet Explorer 8", "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0)" },
	{ "Internet Explorer 9", "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)" },
//	{ "Mozilla 4.08", "Mozilla/4.08 [en] (WinNT; U ;Nav)" },
	{ "Firefox 4.0", "Mozilla/5.0 (Windows NT 6.1; rv:2.0) Gecko/20100101 Firefox/4.0" },
//	{ "Opera Mini 3.0", "Opera/8.01 (J2ME/MIDP; Opera Mini/3.0.6306/1528; nb; U; ssr)" },
//	{ "Opera 9.27 (Windows XP)", "Opera/9.27 (Windows NT 5.1; U; en)" },
//	{ "Opera 9.64 (Windows XP)", "Opera/9.64 (Windows NT 5.1; U; en)" },
	{ "Opera 11.01 (Windows XP)", "Opera/9.80 (Windows NT 5.1; U; en) Presto/2.7.62 Version/11.01" },
	{ "Opera 11.01 (Mac OS X 10.5.8)", "Opera/9.80 (Macintosh; Intel Mac OS X 10.5.8; U; en) Presto/2.7.62 Version/11.01" },
	{ "Safari 5.0.3 (Mac OS X 10.6.7)", "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_7; en-us) AppleWebKit/534.16+ (KHTML, like Gecko) Version/5.0.3 Safari/533.19.4" },
	{ "Google Chrome 11.0.661 (Windows XP)", "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/534.19 (KHTML, like Gecko) Chrome/11.0.661.0 Safari/534.19" },
//	{ "Mobile Safari 3.1.1", "Mozilla/5.0 (iPod; U; CPU iPhone OS 2_2_1 like Mac OS X; en-us) AppleWebKit/525.18.1 (KHTML, like Gecko) Version/3.1.1 Mobile/5H11a Safari/525.20" },
//	{ "IE Mobile 7.6", "HTC-8900/1.2 Mozilla/4.0 (compatible; MSIE 6.0; Windows CE; IEMobile 7.6) UP.Link/6.3.0.0.0" },
//	{ "Blackberry 8320", "BlackBerry8320/4.3.1 Profile/MIDP-2.0 Configuration/CLDC-1.1" },
//	{ "Opera Mini 4.2", "Opera/9.60 (J2ME/MIDP; Opera Mini/4.2.13337/504; U; en) Presto/2.2.0" },
//	{ "Nokia 6230", "Nokia6230/2.0+(04.43)+Profile/MIDP-2.0+Configuration/CLDC-1.1+UP.Link/6.3.0.0.0" },
//	{ "Palm Pre (webOS 1.0)", "Mozilla/5.0 (webOS/1.0; U; en-US) AppleWebKit/525.27.1 (KHTML, like Gecko) Version/1.0 Safari/525.27.1 Pre/1.0" }
};

// News Feed types
static const struct {
	const char* user_name;
	const char* value;
} feed_types[] = {
	{ "Most recent", "lf" },
	{ "Status Updates", "app_2915120374" }
};

