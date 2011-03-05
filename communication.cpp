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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/communication.cpp $
Revision       : $Revision: 99 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-02-12 21:32:51 +0100 (Sat, 12 Feb 2011) $

*/

#include "common.h"

void facebook_client::client_notify( TCHAR* message )
{
	parent->NotifyEvent( parent->m_tszUserName, message, NULL, FACEBOOK_EVENT_CLIENT );
}

http::response facebook_client::flap( const int request_type, std::string* request_data )
{
	NETLIBHTTPREQUEST nlhr = {sizeof( NETLIBHTTPREQUEST )};
	nlhr.requestType = choose_method( request_type );
	std::string url = choose_request_url( request_type, request_data );
	nlhr.szUrl = (char*)url.c_str( );
	nlhr.flags = NLHRF_HTTP11 | NLHRF_NODUMP | choose_security_level( request_type ) | NLHRF_GENERATEHOST;
	nlhr.headers = get_request_headers( request_type, &nlhr.headersCount );
	nlhr.timeout = 1000 * (( request_type == FACEBOOK_REQUEST_MESSAGES_RECEIVE ) ? 60 : 15);

	if ( request_data != NULL )
	{
		nlhr.pData = (char*)(*request_data).c_str();
		nlhr.dataLength = (int)request_data->length( );
	}

	parent->Log("@@@@@ Sending request to '%s'", nlhr.szUrl);

	NETLIBHTTPREQUEST* pnlhr = ( NETLIBHTTPREQUEST* )CallService( MS_NETLIB_HTTPTRANSACTION, (WPARAM)handle_, (LPARAM)&nlhr );

	http::response resp;

	if ( pnlhr != NULL )
	{
		parent->Log("@@@@@ Got response with code %d", pnlhr->resultCode);
		store_headers( &resp, pnlhr->headers, pnlhr->headersCount );
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	}
	else
	{
		parent->Log("!!!!! No response from server (time-out)");
		resp.code = HTTP_CODE_FAKE_DISCONNECTED;
		// Better to have something set explicitely as this value
	    // is compaired in all communication requests
	}

	return resp;
}

bool facebook_client::validate_response( http::response* resp )
{
	if ( resp->code == HTTP_CODE_FAKE_DISCONNECTED ) {
		parent->Log(" ! !  Request has timed out, connection or server error");
		return false; }

	if ( resp->data.find( "{\"error\":" ) != std::string::npos ) try
	{
		std::string error = resp->data.substr( resp->data.find( "{\"error\":" ) + 9, 128 );
		int error_num = atoi( error.substr( 0, error.find( "," ) ).c_str() );
		if ( error_num != 0 )
		{
			error = error.substr( error.find( "\"errorDescription\":\"" ) + 40 );
			error = error.substr( 0, error.find( "\"," ) );
			resp->error_number = error_num;
			resp->error_text = error;
			parent->Log(" ! !  Received Facebook error: %d -- %s", error_num, error.c_str());
//			client_notify( ... );
			resp->code = HTTP_CODE_FAKE_ERROR;
			return false;
		}
	} catch (const std::exception &e) {
		parent->Log(" @ @  validate_response: Exception: %s",e.what());return false; }

	return true;
}

bool facebook_client::handle_entry( std::string method )
{
	parent->Log("   >> Entering %s()", method.c_str());
	return true;
}

bool facebook_client::handle_success( std::string method )
{
	parent->Log("   << Quitting %s()", method.c_str());
	reset_error();
	return true;
}

bool facebook_client::handle_error( std::string method, bool force_disconnect )
{
	bool result;
	increment_error();
	parent->Log("!!!!! %s(): Something with Facebook went wrong", method.c_str());

	if ( force_disconnect )
		result = false;
	else if ( error_count_ <= (UINT)DBGetContactSettingByte(NULL,parent->m_szModuleName,FACEBOOK_KEY_TIMEOUTS_LIMIT,FACEBOOK_TIMEOUTS_LIMIT))
		result = true;
	else
		result = false;

	if ( result == false ) {
		reset_error(); parent->SetStatus(ID_STATUS_OFFLINE); }

	return result;
}

//////////////////////////////////////////////////////////////////////////////

DWORD facebook_client::choose_security_level( int request_type )
{
	if ( DBGetContactSettingByte( NULL, parent->m_szProtoName, FACEBOOK_KEY_FORCE_HTTPS, 0 ) )
		if ( request_type != FACEBOOK_REQUEST_MESSAGES_RECEIVE )
			return NLHRF_SSL;

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
		return NLHRF_SSL;

//	case FACEBOOK_REQUEST_API_CHECK:
//	case FACEBOOK_REQUEST_LOGOUT:
//	case FACEBOOK_REQUEST_KEEP_ALIVE:
//	case FACEBOOK_REQUEST_HOME:
//	case FACEBOOK_REQUEST_BUDDY_LIST:
//	case FACEBOOK_REQUEST_FEEDS:
//	case FACEBOOK_REQUEST_RECONNECT:
//	case FACEBOOK_REQUEST_PROFILE_GET:
//	case FACEBOOK_REQUEST_STATUS_SET:
//	case FACEBOOK_REQUEST_MESSAGE_SEND:
//	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
//	case FACEBOOK_REQUEST_SETTINGS:
//	case FACEBOOK_REQUEST_TYPING_SEND:
	default:
		return ( DWORD )0;
	}
}

int facebook_client::choose_method( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
	case FACEBOOK_REQUEST_KEEP_ALIVE:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	case FACEBOOK_REQUEST_CHAT_SETTINGS:
	case FACEBOOK_REQUEST_TYPING_SEND:
	case FACEBOOK_REQUEST_LOGOUT:
		return REQUEST_POST;

//	case FACEBOOK_REQUEST_API_CHECK:
//	case FACEBOOK_REQUEST_HOME:
//	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
//	case FACEBOOK_REQUEST_FEEDS:
//	case FACEBOOK_REQUEST_RECONNECT:
//	case FACEBOOK_REQUEST_PROFILE_GET:
	default:
		return REQUEST_GET;
	}
}

std::string facebook_client::choose_proto( int request_type )
{
	if ( DBGetContactSettingByte( NULL, parent->m_szProtoName, FACEBOOK_KEY_FORCE_HTTPS, 0 ) )
		if ( request_type != FACEBOOK_REQUEST_MESSAGES_RECEIVE )
			return HTTP_PROTO_SECURE;

	switch ( request_type )
	{
//	case FACEBOOK_REQUEST_API_CHECK:
//	case FACEBOOK_REQUEST_LOGOUT:
//	case FACEBOOK_REQUEST_HOME:
//	case FACEBOOK_REQUEST_FEEDS:
//	case FACEBOOK_REQUEST_RECONNECT:
//	case FACEBOOK_REQUEST_PROFILE_GET:
//	case FACEBOOK_REQUEST_KEEP_ALIVE:
//	case FACEBOOK_REQUEST_BUDDY_LIST:
//	case FACEBOOK_REQUEST_STATUS_SET:
//	case FACEBOOK_REQUEST_MESSAGE_SEND:
//	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
//	case FACEBOOK_REQUEST_SETTINGS:
//	case FACEBOOK_REQUEST_TYPING_SEND:
	default:
		return HTTP_PROTO_REGULAR;

	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
		return HTTP_PROTO_SECURE;
	}
}

std::string facebook_client::choose_server( int request_type, std::string* data )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_API_CHECK:
		return "code.google.com";

	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
		return FACEBOOK_SERVER_LOGIN;

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE: {
		std::string server = FACEBOOK_SERVER_CHAT;
		utils::text::replace_first( &server, "%s", "0" );
		utils::text::replace_first( &server, "%s", this->chat_channel_host_ );
		return server; }

//	case FACEBOOK_REQUEST_PROFILE_GET:
//		return FACEBOOK_SERVER_MOBILE;

//	case FACEBOOK_REQUEST_LOGOUT:
//	case FACEBOOK_REQUEST_KEEP_ALIVE:
//	case FACEBOOK_REQUEST_HOME:
//	case FACEBOOK_REQUEST_PROFILE_GET:
//	case FACEBOOK_REQUEST_BUDDY_LIST:
//	case FACEBOOK_REQUEST_FEEDS:
//	case FACEBOOK_REQUEST_RECONNECT:
//	case FACEBOOK_REQUEST_STATUS_SET:
//	case FACEBOOK_REQUEST_MESSAGE_SEND:
//	case FACEBOOK_REQUEST_SETTINGS:
//	case FACEBOOK_REQUEST_TYPING_SEND:
	default:
		return FACEBOOK_SERVER_REGULAR;
	}
}

std::string facebook_client::choose_action( int request_type, std::string* data )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_API_CHECK:
		return "/p/eternityplugins/wiki/FacebookProtocol_DevelopmentProgress";

	case FACEBOOK_REQUEST_LOGIN:
		return "/login.php?login_attempt=1";

	case FACEBOOK_REQUEST_SETUP_MACHINE:
		return "/loginnotify/setup_machine.php?persistent=1";

	case FACEBOOK_REQUEST_LOGOUT:
		return "/logout.php";

	case FACEBOOK_REQUEST_KEEP_ALIVE:
		return "/ajax/presence/update.php?__a=1";

	case FACEBOOK_REQUEST_HOME:
		return "/home.php?";

	case FACEBOOK_REQUEST_BUDDY_LIST:
		return "/ajax/chat/buddy_list.php?__a=1";

	case FACEBOOK_REQUEST_FEEDS: {
		// Filters: lf = live feed, h = news feed
		// TODO: Make filter selection customizable?
		std::string action = "/ajax/intent.php?filter=%s&request_type=1&__a=1&newest=%s&ignore_self=true";
		unsigned int type_id = DBGetContactSettingByte( NULL, parent->m_szModuleName,
		    FACEBOOK_KEY_EVENT_FEEDS_TYPE, DEFAULT_EVENT_FEEDS_TYPE );
		if ( type_id >= SIZEOF( feed_types ) || type_id < 0 )
			type_id = DEFAULT_EVENT_FEEDS_TYPE;
		utils::text::replace_first( &action, "%s", feed_types[type_id].value );
		std::string newest = utils::conversion::to_string((void*)&this->last_feeds_update_, UTILS_CONV_TIME_T);
		utils::text::replace_first( &action, "%s", newest );
		return action; }

	case FACEBOOK_REQUEST_RECONNECT: {
		std::string action = "/ajax/presence/reconnect.php?reason=%s&iframe_loaded=false&post_form_id=%s&__a=1&nctr[n]=1";
		std::string reason = ( this->chat_first_touch_ ) ? FACEBOOK_RECONNECT_LOGIN : FACEBOOK_RECONNECT_KEEP_ALIVE;
		utils::text::replace_first( &action, "%s", reason );
		utils::text::replace_first( &action, "%s", this->post_form_id_ );
		return action; }

// Backup for old mobile site profile getter
//
//	case FACEBOOK_REQUEST_PROFILE_GET: {
//		std::string action = "/profile.php?id=%s&v=info";
//		utils::text::replace_first( &action, "%s", (*data) );
//		return action; }

	case FACEBOOK_REQUEST_PROFILE_GET: {
		std::string action = "/ajax/hovercard/user.php?id=%s&__a=1";
		utils::text::replace_first( &action, "%s", (*data) );
		return action; }

	case FACEBOOK_REQUEST_STATUS_SET:
		return "/ajax/updatestatus.php?__a=1";

	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return "/ajax/chat/send.php?__a=1";

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE: {
		std::string action = "/x/%s/%s/p_%s=%d";
		std::string first_time;
		if ( this->chat_first_touch_ ) { first_time = "true"; this->chat_first_touch_ = false; }
		else first_time = "false";
		utils::text::replace_first( &action, "%s", utils::time::unix_timestamp() );
		utils::text::replace_first( &action, "%s", first_time );
		utils::text::replace_first( &action, "%s", self_.user_id );
		utils::text::replace_first( &action, "%d", utils::conversion::to_string( (void*)&chat_sequence_num_, UTILS_CONV_UNSIGNED | UTILS_CONV_INTEGER ) );
		return action; }

	case FACEBOOK_REQUEST_CHAT_SETTINGS:
		return "/ajax/chat/settings.php?__a=1";

	case FACEBOOK_REQUEST_TYPING_SEND:
		return "/ajax/chat/typ.php?__a=1";

	default:
		return "/";
	}
}

std::string facebook_client::choose_request_url( int request_type, std::string* data )
{
	std::string url = choose_proto( request_type );
	url.append( choose_server( request_type, data ) );
	url.append( choose_action( request_type, data ) );
	return url;
}


NETLIBHTTPHEADER* facebook_client::get_request_headers( int request_type, int* headers_count )
{
	ScopedLock s( headers_lock_ );

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_PROFILE_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	case FACEBOOK_REQUEST_CHAT_SETTINGS:
	case FACEBOOK_REQUEST_TYPING_SEND:
		*headers_count = 7;
		break;
	case FACEBOOK_REQUEST_HOME:
	case FACEBOOK_REQUEST_FEEDS:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	default:
		*headers_count = 6;
		break;
	case FACEBOOK_REQUEST_API_CHECK:
		*headers_count = 5;
		break;
	}

	NETLIBHTTPHEADER* headers = ( NETLIBHTTPHEADER* )utils::mem::allocate( sizeof( NETLIBHTTPHEADER )*( *headers_count ) );

	refresh_headers( ); // TODO: Refresh only when required

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_PROFILE_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	case FACEBOOK_REQUEST_CHAT_SETTINGS:
	case FACEBOOK_REQUEST_TYPING_SEND:
		set_header( &headers[6], "Content-Type" );
	case FACEBOOK_REQUEST_HOME:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	default:
		set_header( &headers[5], "Cookie" );
	case FACEBOOK_REQUEST_API_CHECK:
		set_header( &headers[4], "Connection" );
		set_header( &headers[3], "User-Agent" );
		set_header( &headers[2], "Accept" );
		set_header( &headers[1], "Accept-Encoding" );
		set_header( &headers[0], "Accept-Language" );
		break;
	}

	return headers;
}

void facebook_client::set_header( NETLIBHTTPHEADER* header, char* name )
{
	header->szName  = name;
	header->szValue = (char*)this->headers[name].c_str();
}

void facebook_client::refresh_headers( )
{
	ScopedLock s( headers_lock_ );

	if ( headers.size() < 5 )
	{
		this->headers["Connection"] = "close";
		this->headers["Accept"] = "*/*";
#ifdef _DEBUG
		this->headers["Accept-Encoding"] = "none";
#else
		this->headers["Accept-Encoding"] = "deflate, gzip, x-gzip, identity, *;q=0";
#endif
		this->headers["Accept-Language"] = "en,en-US;q=0.9";
		this->headers["Content-Type"] = "application/x-www-form-urlencoded";
	}
	this->headers["User-Agent"] = get_user_agent( );
	this->headers["Cookie"] = load_cookies( );
}

std::string facebook_client::get_user_agent( )
{
	BYTE user_agent = DBGetContactSettingByte(NULL, parent->m_szModuleName, FACEBOOK_KEY_USER_AGENT, 0);
	if (user_agent > 0 && user_agent < SIZEOF(user_agents))
		return user_agents[user_agent].user_agent_string;
	else return g_strUserAgent;
}

std::string facebook_client::load_cookies( )
{
	ScopedLock s( cookies_lock_ );

	std::string cookieString = "isfbe=false;";

	if ( !cookies.empty( ) )
		for ( std::map< std::string, std::string >::iterator iter = cookies.begin(); iter != cookies.end(); ++iter ) {
			cookieString.append( iter->first );
			cookieString.append( 1, '=' );
			cookieString.append( iter->second );
			cookieString.append( 1, ';' ); }
	return cookieString;
}

void facebook_client::store_headers( http::response* resp, NETLIBHTTPHEADER* headers, int headersCount )
{
	ScopedLock h( headers_lock_ );
	ScopedLock c( cookies_lock_ );

	for ( int i = 0; i < headersCount; i++ )
	{
		std::string header_name = headers[i].szName; // TODO: Casting?
		std::string header_value = headers[i].szValue; // TODO: Casting?

		if ( header_name == "Set-Cookie" )
		{
			std::string cookie_name = header_value.substr( 0, header_value.find( "=" ) );
			std::string cookie_value = header_value.substr( header_value.find( "=" ) + 1, header_value.find( ";" ) - header_value.find( "=" ) - 1 );
			if ( cookie_value == "deleted" ) {
				parent->Log("      Deleted cookie '%s'", cookie_name.c_str());
				cookies.erase( cookie_name ); }
			else {
				parent->Log("      New cookie '%s': %s", cookie_name.c_str(), cookie_value.c_str());
				cookies[cookie_name] = cookie_value; }
		}
		else {
			parent->Log("----- Got header '%s': %s", header_name.c_str(), header_value.c_str() );
			resp->headers[header_name] = header_value;
		}
	}
}

void facebook_client::clear_cookies( )
{
	ScopedLock s( cookies_lock_ );

	if ( !cookies.empty( ) )
		cookies.clear( );
}

//////////////////////////////////////////////////////////////////////////////

bool facebook_client::api_check( )
{
	handle_entry( "api_check" );

	http::response resp = flap( FACEBOOK_REQUEST_API_CHECK );

	// Process result

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		std::string api_version_latest = utils::text::source_get_value( &resp.data, 2, "</h1><p><var>", "</var>" );

		if ( api_version_latest.length() && std::string( __API_VERSION_STRING ) != api_version_latest )
			client_notify( TranslateT( "Facebook API version has changed, wait and watch for the Facebook protocol update." ) );

	}

	// Clear Google Code cookies
	clear_cookies();

	return handle_success( "api_check" );
}

bool facebook_client::login(const std::string &username,const std::string &password)
{
	handle_entry( "login" );

	username_ = username;
	password_ = password;

	// Access homepage to get initial cookies
	flap( FACEBOOK_REQUEST_HOME );

	// Prepare login data
	std::string data = "charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&locale=en&email=";
	data += utils::url::encode( username );
	data += "&pass=";
	data += utils::url::encode( password );
	data += "&pass_placeHolder=Password&login=Login&persistent=1";

	// Send validation
	http::response resp = flap( FACEBOOK_REQUEST_LOGIN, &data );

	// Process result data
	validate_response(&resp);

	// Check whether setting Machine name is required
	if ( resp.code == HTTP_CODE_FOUND && resp.headers.find("Location") != resp.headers.end() && resp.headers["Location"].find("loginnotify/setup_machine.php") != std::string::npos ) {
		std::string inner_data = "charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&locale=en&remembercomputer=1&machinename=";
		inner_data += "MirandaIM";
		flap( FACEBOOK_REQUEST_SETUP_MACHINE, &inner_data ); }

	// Check for Device ID
	if ( cookies["datr"].length() )
		DBWriteContactSettingString( NULL, parent->m_szModuleName, FACEBOOK_KEY_DEVICE_ID, cookies["datr"].c_str() );

	switch ( resp.code )
	{

	case HTTP_CODE_OK: { // OK page returned, but that is regular login page we don't want in fact
		// Get error message
		std::string error_str = utils::text::remove_html(
		    utils::text::source_get_value( &resp.data, 2, "id=\"standard_error\">", "</h2>" ) );
		if ( !error_str.length() )
			error_str = "Unknown login error";
		parent->Log(" ! !  Login error: %s", error_str.c_str());

		TCHAR* message = TranslateT( "Login error: " );
		TCHAR* error = mir_a2t_cp( error_str.c_str( ), CP_UTF8 );
		TCHAR* info = ( TCHAR* )malloc( ( lstrlen( message ) + lstrlen( error ) ) * sizeof( TCHAR ) );
		lstrcpy( info, message );
		lstrcat( info, error );
		
		client_notify( info );
		mir_free( message );
		mir_free( error );
		mir_free( info );
	}
	case HTTP_CODE_FORBIDDEN: // Forbidden
	case HTTP_CODE_NOT_FOUND: // Not Found
	default:
		return handle_error( "login", FORCE_DISCONNECT );

	case HTTP_CODE_FOUND: // Found and redirected to Home, Logged in, everything is OK
		if ( cookies.find("c_user") != cookies.end() ) {
			this->self_.user_id = cookies.find("c_user")->second;
			DBWriteContactSettingString(NULL,parent->m_szModuleName,FACEBOOK_KEY_ID,this->self_.user_id.c_str());
			parent->Log("      Got self user id: %s", this->self_.user_id.c_str());
			return handle_success( "login" ); }
		else {
			client_notify(TranslateT("Login error, probably bad login credentials."));
			return handle_error( "login", FORCE_DISCONNECT );
		}
	}
}

bool facebook_client::logout( )
{
	if ( parent->isOffline() )
		return true;

	if ( DBGetContactSettingByte(NULL, parent->m_szModuleName, FACEBOOK_KEY_DISABLE_LOGOUT, 0) )
		return true;

	handle_entry( "logout" );

	std::string data = "post_form_id=";
	data += ( this->post_form_id_.length( ) ) ? this->post_form_id_ : "0";
	data += "&fb_dtsg=";
	data += ( this->dtsg_.length( ) ) ? this->dtsg_ : "0";
	data += "&ref=mb&h=";
	data += this->logout_hash_;

	http::response resp = flap( FACEBOOK_REQUEST_LOGOUT, &data );


	// Process result
	username_ = password_ = self_.user_id = "";

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
	case HTTP_CODE_FOUND:
		return handle_success( "logout" );

	default:
		return false; // Logout not finished properly, but..okay, who cares :P

	}
}

bool facebook_client::keep_alive( )
{
	if ( parent->isOffline() )
		return false;

	handle_entry( "keep_alive" );

	// Keep us connected

	std::string data = "user=" + this->self_.user_id + "&popped_out=false&force_render=true&buddy_list=1&notifications=0&post_form_id=" + this->post_form_id_ + "&fb_dtsg=" + this->dtsg_ + "&post_form_id_source=AsyncRequest&__a=1&nctr[n]=1";

	{
		ScopedLock s(buddies_lock_);

		for (List::Item< facebook_user >* i = buddies.begin(); i != buddies.end(); i = i->next ) {
			data += "&available_list[";
			data += i->data->user_id;
			data += "][i]=";
			data += ( i->data->is_idle ) ? "1" : "0"; } }

	http::response resp = flap( FACEBOOK_REQUEST_KEEP_ALIVE, &data );

	// Keep us marked as Online
	// TODO: Online/Away application can be toggled here
	home();

	// Process result
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
	case HTTP_CODE_FOUND:
		return handle_success( "keep_alive" );

	default:
		return handle_error( "keep_alive" );

	}
}

bool facebook_client::home( )
{
	handle_entry( "home" );

	http::response resp = flap( FACEBOOK_REQUEST_HOME );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		if ( resp.data.find( "id=\"navAccountName\"" ) != std::string::npos )
		{
			// Get real_name
			this->self_.real_name = utils::text::special_expressions_decode( utils::text::source_get_value( &resp.data, 2, " id=\"navAccountName\">", "</a" ) );
			DBWriteContactSettingUTF8String(NULL,parent->m_szModuleName,FACEBOOK_KEY_NAME,this->self_.real_name.c_str());
			DBWriteContactSettingUTF8String(NULL,parent->m_szModuleName,"Nick",this->self_.real_name.c_str());
			parent->Log("      Got self real name: %s", this->self_.real_name.c_str());

			// Get post_form_id
			this->post_form_id_ = utils::text::source_get_value( &resp.data, 2, "post_form_id:\"", "\"" );
			parent->Log("      Got self post form id: %s", this->post_form_id_.c_str());

			// Get dtsg
			this->dtsg_ = utils::text::source_get_value( &resp.data, 2, ",fb_dtsg:\"", "\"" );
			parent->Log("      Got self dtsg: %s", this->dtsg_.c_str());

			// Get logout hash
			this->logout_hash_ = utils::text::source_get_value( &resp.data, 2, "<input type=\"hidden\" autocomplete=\"off\" name=\"h\" value=\"", "\"" );
			parent->Log("      Got self logout hash: %s", this->logout_hash_.c_str());

			// Get friend requests count and messages count and notify it
			if ( DBGetContactSettingByte( NULL, parent->m_szModuleName, FACEBOOK_KEY_EVENT_OTHER_ENABLE, DEFAULT_EVENT_OTHER_ENABLE ) ) {
				std::string str_count = utils::text::source_get_value( &resp.data, 2, "<span id=\"jewelRequestCount\">", "</span>" );
				if ( str_count.length() && str_count != std::string( "0" ) ) {
					TCHAR* message = TranslateT( "Got new friend requests: " );
					TCHAR* count = mir_a2t_cp( str_count.c_str( ), CP_UTF8 );
					TCHAR* info = ( TCHAR* )malloc( ( lstrlen( message ) + lstrlen( count ) ) * sizeof( TCHAR ) );
					lstrcpy( info, message );
					lstrcat( info, count );
					parent->NotifyEvent( parent->m_tszUserName, info, NULL, FACEBOOK_EVENT_OTHER, TEXT(FACEBOOK_URL_REQUESTS) );
					mir_free( message );
					mir_free( count );
					mir_free( info ); }

// TODO: Use ever again?
//				str_count = utils::text::source_get_value( &resp.data, 2, "<span id=\"jewelInnerUnseenCount\">", "</span>" );
//				if ( str_count.length() && str_count != std::string( "0" ) ) {
//					TCHAR* message = TranslateT( "Got new messages: " );
//					TCHAR* count = mir_a2t_cp( str_count.c_str( ), CP_UTF8 );
//					TCHAR* info = ( TCHAR* )malloc( ( lstrlen( message ) + lstrlen( count ) ) * sizeof( TCHAR ) );
//					lstrcpy( info, message );
//					lstrcat( info, count );
//					parent->NotifyEvent( parent->m_tszUserName, info, NULL, FACEBOOK_EVENT_OTHER, TEXT(FACEBOOK_URL_MESSAGES) );
//					mir_free( message );
//					mir_free( count );
//					mir_free( info ); }
			}

			// Set first touch flag
			this->chat_first_touch_ = true;

			// Update self-contact
			ForkThread(&FacebookProto::UpdateContactWorker, this->parent, (void*)&this->self_);

			return handle_success( "home" );
		}
		else {
			client_notify(TranslateT("Something happened to Facebook. Maybe there was some major update so you should wait for an update."));
			return handle_error( "home", FORCE_DISCONNECT );
		}

	case HTTP_CODE_FOUND:
		parent->Log("      REPLICA_DOWN is back in force! What a regression, muhahaha! Revert to revision 88 to take care about this...");
	default:
		return handle_error( "home", FORCE_DISCONNECT );

	}
}

bool facebook_client::chat_settings( BYTE flag, void* input )
{
	handle_entry( "chat_settings" );

	std::string data = "window_id=0";
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : "0";
	data += "&post_form_id_source=AsyncRequest";
	data += "&fb_dtsg=" + this->dtsg_;

	if ( FLAG_CONTAINS( flag, FACEBOOK_CHAT_VISIBILITY ) )
		// Set online "status" for chat
		// - also turns on manually logged-out chat
		// TODO: Chat online/offline state can be toggled here
		data += "&visibility=true&lsd=";

	if ( FLAG_CONTAINS( flag, FACEBOOK_CHAT_CLOSE_WINDOW ) ) {
		// Close contact's window when requested
		std::string* contact_id = ( std::string* )input;
		data += "&close_chat=";
		data += *contact_id; }

	http::response resp = flap( FACEBOOK_REQUEST_CHAT_SETTINGS, &data );

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		return handle_success( "chat_settings" );

	default:
		return handle_error( "chat_settings" );

	}
}

bool facebook_client::reconnect( )
{
	handle_entry( "reconnect" );

	// Update chat settings
	chat_settings( FACEBOOK_CHAT_VISIBILITY );

	// Request reconnect
	http::response resp = flap( FACEBOOK_REQUEST_RECONNECT );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK: {
		this->chat_channel_host_ = utils::text::source_get_value( &resp.data, 2, "\"host\":\"", "\"" );
		parent->Log("      Got self channel host: %s", this->chat_channel_host_.c_str());

		this->chat_sequence_num_ = atoi( utils::text::source_get_value( &resp.data, 2, "\"seq\":", "," ).c_str() );
		parent->Log("      Got self sequence number: %d", this->chat_sequence_num_); }

		return handle_success( "reconnect" );

	default:
		return handle_error( "reconnect", FORCE_DISCONNECT );

	}
}

bool facebook_client::buddy_list( )
{
	handle_entry( "buddy_list" );

	// Prepare update data
	std::string data = "user=" + this->self_.user_id + "&popped_out=false&force_render=true&buddy_list=1&notifications=0&post_form_id=" + this->post_form_id_ + "&fb_dtsg=" + this->dtsg_ + "&post_form_id_source=AsyncRequest&__a=1&nctr[n]=1";

	{
		ScopedLock s(buddies_lock_);

		for (List::Item< facebook_user >* i = buddies.begin(); i != NULL; i = i->next ) {
			data += "&available_list[";
			data += i->data->user_id;
			data += "][i]=";
			data += ( i->data->is_idle ) ? "1" : "0"; } }

	// Get buddy list
	http::response resp = flap( FACEBOOK_REQUEST_BUDDY_LIST, &data );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		if ( resp.data.find( "\"listChanged\":true" ) != std::string::npos ) {
			std::string* response_data = new std::string( resp.data );
			ForkThread( &FacebookProto::ProcessBuddyList, this->parent, ( void* )response_data ); }
		return handle_success( "buddy_list" );

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "buddy_list" );

	}
}

bool facebook_client::feeds( )
{
	handle_entry( "feeds" );

	// Get feeds
	http::response resp = flap( FACEBOOK_REQUEST_FEEDS );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		if ( resp.data.find( "\"storyCount\":" ) != std::string::npos ) {
			std::string* response_data = new std::string( resp.data );
			ForkThread( &FacebookProto::ProcessFeeds, this->parent, ( void* )response_data ); }
		return handle_success( "feeds" );

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "feeds" );

	}
}

bool facebook_client::channel( )
{
	handle_entry( "channel" );

	// Get update
	http::response resp = flap( FACEBOOK_REQUEST_MESSAGES_RECEIVE );

	// Process result data
	validate_response(&resp);

	if ( resp.code != HTTP_CODE_OK )
	{
		// Something went wrong
	}
	else if ( resp.data.find( "\"t\":\"continue\"" ) != std::string::npos )
	{
		// Everything is OK, no new message received
	}
	else if ( resp.data.find( "\"t\":\"refresh\"" ) != std::string::npos )
	{
		// Something went wrong with the session (has expired?), refresh it
		return this->reconnect( );
	}
	else if ( resp.data.find( "\"t\":\"fullReload\"" ) != std::string::npos )
	{
		// Something went wrong (server flooding?)
		return this->reconnect( );
	}
	else
	{
		// Something has been received, throw to new thread to process
		std::string* response_data = new std::string( resp.data );
		ForkThread( &FacebookProto::ProcessMessages, this->parent, ( void* )response_data );

		// Increment sequence number
		this->chat_sequence_num_ += utils::text::count_all( &resp.data, "\"type\":\"" );
	}

	// Return
	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		return handle_success( "channel" );

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "channel" );

	}
}

bool facebook_client::send_message( std::string message_recipient, std::string message_text )
{
	handle_entry( "send_message" );

	std::string data = "msg_text=";
	data += utils::url::encode( message_text );
	data += "&msg_id=";
	data += utils::time::unix_timestamp( );
	data += "&to=";
	data += message_recipient;
	data += "&client_time=";
	data += utils::time::mili_timestamp( );
	data += "&to_offline=false";
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : "0";
	data += "&fb_dtsg=";
	data += ( this->dtsg_.length( ) ) ? this->dtsg_ : "0";
    data += "&post_form_id_source=AsyncRequest";

	http::response resp = flap( FACEBOOK_REQUEST_MESSAGE_SEND, &data );

	validate_response(&resp);

	switch ( resp.error_number ) {

	case 0: // Everything is OK
          break;

	case 1356003: { // Contact is offline
		HANDLE hContact = parent->ContactIDToHContact( message_recipient );
		DBWriteContactSettingWord(hContact,parent->m_szModuleName,"Status",ID_STATUS_OFFLINE);
		} break;  

	case 1356026: // Contact has alternative client
          /*
          post na url http://www.facebook.com/ajax/chat/post_application_settings.php?__a=1

          enable_and_send      Povolit a odeslat                                                                                                                                                                                                                                                                                                                                                                                                                               
          to_send              AQCoweMPeszBoKpd4iahcOyhmh0kiTYIhv1b5wCtuBiD0AaPVZIdEp3Pf5JMBmQ-9wf0ju-xdi-VRuk0ERk_I7XzI5dVJCs6-B0FExTZhspD-4-kTZLmZI-_M6fIuF2328yMyT3R3UEUmMV8P9MHcZwu-_pS3mOhsaHf6rIVcQ2rocSqLKi03wLKCfg0m8VsptPADWpOI-UNcIo-xl1PAoC1yVnL2wEXEtnF1qI_xFcmlJZ40AOONfIF_LS_lBrGYA-oCWLUK-GLHtQAHjO8aDeNXDU8Jk7Z_ES-_oAHee2PVLHcG_ACHXpasE7Iu3XFLMrdN2hjM96AjPRIf0Vk8gBZzfW_lUspakZmXxMI7iSNQE8lourK_6B3Z1s4UHxDZCNXYuc9gh70nm_xnaxnF9K1bR00s4MltnFjUT_3ypThzA  
          __d                  1                                                                                                                                                                                                                                                                                                                                                                                                                                               
          post_form_id         c73ebd9d94b7449c40e6965410fcdcf6                                                                                                                                                                                                                                                                                                                                                                                                                
          fb_dtsg              Tb-T9                                                                                                                                                                                                                                                                                                                                                                                                                                           
          lsd                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
          post_form_id_source  AsyncRequest                                                                                                                                                                                                                                                                                                                                                                                                                                    
          */
          break;

	default:
		break;
	}

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		return handle_success( "send_message" );

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "send_message" );

	}
}

bool facebook_client::close_chat( std::string contact_id )
{
	return chat_settings( FACEBOOK_CHAT_CLOSE_WINDOW, &contact_id );
}

bool facebook_client::get_profile(facebook_user* fbu)
{
	handle_entry( "get_profile" );

	http::response resp = flap( FACEBOOK_REQUEST_PROFILE_GET, &fbu->user_id );

	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK: {
		// TODO: More items?
		fbu->status = "";			

		std::string image = utils::text::source_get_value( &resp.data, 2, "background-image: url(", ")" );
		if ( image.length() )
			fbu->image_url = utils::text::special_expressions_decode( image );
		else
			fbu->image_url = FACEBOOK_DEFAULT_AVATAR_URL;
	}
	case HTTP_CODE_FOUND:
		return handle_success( "get_profile" );

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "get_profile" );

	}
}

bool facebook_client::set_status(const std::string &status_text)
{
	handle_entry( "set_status" );

	std::string data = "post_form_id_source=AsyncRequest&post_form_id=";
	data += ( this->post_form_id_.length( ) ) ? this->post_form_id_ : "0";
	data += "&fb_dtsg=";
	data += ( this->dtsg_.length( ) ) ? this->dtsg_ : "0";
	data += "&target_id=";
	data += this->self_.user_id;

	if ( status_text.length( ) ) {
		data += "&action=PROFILE_UPDATE&app_id=&hey_kid_im_a_composer=true&display_context=profile&_log_display_context=profile&ajax_log=1&status=";
		data += utils::url::encode( status_text );
		data += "&profile_id=";
		data += this->self_.user_id; }
	else
		data += "&clear=1&nctr[_mod]=pagelet_top_bar";

	http::response resp = flap( FACEBOOK_REQUEST_STATUS_SET, &data );

	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		return handle_success( "set_status" );

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "set_status" );

	}
}

//////////////////////////////////////////////////////////////////////////////

bool facebook_client::save_url(const std::string &url,const std::string &filename)
{
	NETLIBHTTPREQUEST req = {sizeof(req)};
	NETLIBHTTPREQUEST *resp;
	req.requestType = REQUEST_GET;
	req.szUrl = const_cast<char*>(url.c_str());

	resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService( MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(this->parent->m_hNetlibUser), reinterpret_cast<LPARAM>(&req) ));

	if ( resp )
	{
		parent->Log( "@@@@@ Saving avatar URL %s to path %s", url.c_str(), filename.c_str() );

		// Create folder if necessary
		std::string dir = filename.substr(0,filename.rfind('\\'));
		if(_access(dir.c_str(),0))
			CallService(MS_UTILS_CREATEDIRTREE, 0, (LPARAM)dir.c_str());

		// Write to file
		FILE *f = fopen(filename.c_str(),"wb");
		fwrite(resp->pData,1,resp->dataLength,f);
		fclose(f);

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)resp);
		return true;
	}
	else
		return false;
}
