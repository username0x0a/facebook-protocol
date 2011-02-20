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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/connection.cpp $
Revision       : $Revision: 99 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-02-12 21:32:51 +0100 (Sat, 12 Feb 2011) $

*/

#include "common.h"

void FacebookProto::KillThreads( void* )
{
	HANDLE local_msgLoop = m_hMsgLoop, local_updLoop = m_hUpdLoop;
	m_hMsgLoop = NULL; m_hUpdLoop = NULL;

	// Kill the old threads if they are still around
	if(local_msgLoop)
	{
		LOG("***** Requesting MessageLoop to exit");
		WaitForSingleObject(local_msgLoop,60000);
		CloseHandle(local_msgLoop);
	}
	if(local_updLoop)
	{
		LOG("***** Requesting UpdateLoop to exit");
		WaitForSingleObject(local_updLoop,10000);
		CloseHandle(local_updLoop);
	}
}

void FacebookProto::SignOn(void*)
{
	ScopedLock s(signon_lock_);
	LOG("***** Beginning SignOn process");

	if ( NegotiateConnection( ) )
	{
		if (!getByte(FACEBOOK_KEY_SHOW_OLD_FEEDS, DEFAULT_SHOW_OLD_FEEDS))
			facy.last_feeds_update_ = ::time( NULL );

		setDword( "LogonTS", (DWORD)time(NULL) );
		m_hUpdLoop = ForkThreadEx( &FacebookProto::UpdateLoop,  this );
		m_hMsgLoop = ForkThreadEx( &FacebookProto::MessageLoop, this );
	}
	ToggleStatusMenuItems(isOnline());

	LOG("***** SignOn complete");
}

void FacebookProto::SignOff(void*)
{
	ScopedLock s(signon_lock_);
	ScopedLock b(facy.buddies_lock_);
	LOG("##### Beginning SignOff process");

	ForkThreadEx( &FacebookProto::KillThreads, this );

	deleteSetting( "LogonTS" );

	facy.logout( );
	facy.clear_cookies( );
	facy.buddies.clear( );

	int old_status = m_iStatus;
	m_iStatus = facy.self_.status_id = ID_STATUS_OFFLINE;

	ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
		(HANDLE)old_status,m_iStatus);

	SetAllContactStatuses( ID_STATUS_OFFLINE );

	ToggleStatusMenuItems(isOnline());

	LOG("##### SignOff complete");
}

bool FacebookProto::NegotiateConnection( )
{
	LOG("***** Negotiating connection with Facebook");

	int old_status = m_iStatus;
	std::string user, pass;
	DBVARIANT dbv = {0};

	if( !DBGetContactSettingString(NULL,m_szModuleName,FACEBOOK_KEY_LOGIN,&dbv) )
	{
		user = dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	else
	{
		NotifyEvent(m_tszUserName,TranslateT("Please enter a username."),NULL,FACEBOOK_EVENT_CLIENT);
		goto error;
	}

	if( !DBGetContactSettingString(NULL,m_szModuleName,FACEBOOK_KEY_PASS,&dbv) )
	{
		CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
			reinterpret_cast<LPARAM>(dbv.pszVal));
		pass = dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	else
	{
		NotifyEvent(m_tszUserName,TranslateT("Please enter a password."),NULL,FACEBOOK_EVENT_CLIENT);
		goto error;
	}
	if( !DBGetContactSettingString(NULL,m_szModuleName,FACEBOOK_KEY_DEVICE_ID,&dbv) )
	{
		facy.cookies["datr"] = dbv.pszVal;
		DBFreeVariant(&dbv);
	}

	bool success;
	{
		success = facy.login( user, pass );
		if (success) success = facy.home( );
		if (success) success = facy.reconnect( );
		if (success) success = facy.buddy_list( );
	}

	if(!success)
	{
error:
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_FAILED,
			(HANDLE)old_status,m_iStatus);

		// Set to offline
		old_status = m_iStatus;
		m_iStatus = m_iDesiredStatus = facy.self_.status_id = ID_STATUS_OFFLINE;

		SetAllContactStatuses(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		return false;
	}
	else
	{
		m_iStatus = facy.self_.status_id = m_iDesiredStatus;

		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		return true;
	}
}

void FacebookProto::UpdateLoop(void *)
{
	ScopedLock s(update_loop_lock_); // TODO: Required?
	time_t thread_current = m_tUpdLoop = time(NULL);
	LOG( ">>>>> Entering FacebookProto::UpdateLoop [%d]", thread_current );

	for ( DWORD i = 0; ; i = ++i % 42 )
	{
		if ( !isOnline( ) )
			break;
		if ( thread_current != m_tUpdLoop )
			break;
		if ( i != 0 )
			if ( !facy.buddy_list( ) )
				break;
		if ( !isOnline( ) )
			break;
		if ( i % 6 == 3 && getByte( FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE ) )
			if ( !facy.feeds( ) )
				break;
		if ( !isOnline( ) )
			break;
		if ( i % 7 == 6 )
			if ( !facy.keep_alive( ) )
				break;
		if ( !isOnline( ) )
			break;
		LOG( "***** FacebookProto::UpdateLoop [%d] going to sleep...",
		    thread_current );
		if ( SleepEx( GetPollRate( ) * 1000, true ) == WAIT_IO_COMPLETION )
			break;
		LOG( "***** FacebookProto::UpdateLoop [%d] waking up...",
		    thread_current );
	}

	LOG( "<<<<< Exiting FacebookProto::UpdateLoop [%d]", thread_current );
}

void FacebookProto::MessageLoop(void *)
{
	ScopedLock s(message_loop_lock_); // TODO: Required?
	time_t thread_current = m_tMsgLoop = time(NULL);
	LOG( ">>>>> Entering FacebookProto::MessageLoop [%d]", thread_current );

	while ( true )
	{
		if ( !isOnline( ) )
			break;
		if ( thread_current != m_tMsgLoop )
			break;
		if ( !facy.channel( ) )
			break;
		LOG( "***** FacebookProto::MessageLoop [%d] refreshing...",
		    thread_current );
	}

	LOG( "<<<<< Exiting FacebookProto::MessageLoop [%d]", thread_current );
}

BYTE FacebookProto::GetPollRate( )
{
	BYTE poll_rate = getByte( FACEBOOK_KEY_POLL_RATE, FACEBOOK_DEFAULT_POLL_RATE );

	return (
	    ( poll_rate >= FACEBOOK_MINIMAL_POLL_RATE &&
	      poll_rate <= FACEBOOK_MAXIMAL_POLL_RATE )
	    ? poll_rate : FACEBOOK_DEFAULT_POLL_RATE );
}
