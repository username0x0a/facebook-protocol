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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/messages.cpp $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#include "common.h"

struct send_direct
{
	send_direct(HANDLE hContact,const std::string &msg) : hContact(hContact),msg(msg) {}
	HANDLE hContact;
	std::string msg;
};

struct send_typing
{
	send_typing(HANDLE hContact,const int status) : hContact(hContact),status(status) {}
	HANDLE hContact;
	int status;
};

int FacebookProto::RecvMsg(HANDLE hContact, PROTORECVEVENT *pre)
{
	DBVARIANT dbv;

	if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) ) {
		std::string* data = new std::string( dbv.pszVal );
		ForkThread( &FacebookProto::CloseChatWorker, this, (void*)data );
		DBFreeVariant(&dbv); }

	CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);

	CCSDATA ccs = { hContact,PSR_MESSAGE,0,reinterpret_cast<LPARAM>(pre) };
	return CallService(MS_PROTO_RECVMSG,0,reinterpret_cast<LPARAM>(&ccs));
}

void FacebookProto::SendMsgWorker(void *p)
{
	if(p == NULL)
		return;

	send_direct *data = static_cast<send_direct*>(p);

	DBVARIANT dbv;
	if( !DBGetContactSettingString(data->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
	{
		if ( facy.send_message(dbv.pszVal, data->msg) )
			ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_MESSAGE,ACKRESULT_SUCCESS, (HANDLE)1,0);
		else
			ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_MESSAGE,ACKRESULT_FAILED, (HANDLE)1,(LPARAM)Translate("Error with sending message."));

		std::string* data = new std::string( dbv.pszVal );
		CloseChatWorker( (void*)data );
		DBFreeVariant(&dbv);
	}

	delete data;
}

int FacebookProto::SendMsg(HANDLE hContact, int flags, const char *msg)
{
	if ( !isOnline( ) ) {
		ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)Translate("You cannot send messages when you are offline."));
		return 0; }

	if ( DBGetContactSettingWord( hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE ) == ID_STATUS_OFFLINE ) {
		ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)Translate("Facebook protocol don't support offline messages."));
		return 0; }

	// TODO: msg comes as Unicode (retyped wchar_t*), why should we convert it as ANSI to UTF-8? o_O
	if ( flags & PREF_UNICODE )
        msg = mir_utf8encode(msg);

	ForkThread( &FacebookProto::SendMsgWorker, this,new send_direct(hContact,msg) );

	return 1;
}

int FacebookProto::UserIsTyping(HANDLE hContact,int type)
{ 
	if (hContact && isOnline())
		ForkThread(&FacebookProto::SendTypingWorker, this,new send_typing(hContact,type));
		// TODO: Thread required?

	return 0;
}

void FacebookProto::SendTypingWorker(void *p)
{
	if(p == NULL)
		return;

	send_typing *typing = static_cast<send_typing*>(p);

		
	DBVARIANT dbv;
	if( !DBGetContactSettingString(typing->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
	{
		std::string data = "typ=";
		data += ( typing->status == PROTOTYPE_SELFTYPING_ON ) ? "1" : "0"; // PROTOTYPE_SELFTYPING_OFF
		data += "&to=";
		data += dbv.pszVal;
		data += "&source=chat";
		data += "&fb_dtsg=" + facy.dtsg_;
		data += "&post_form_id=";
		data += ( facy.post_form_id_.length( ) ) ? facy.post_form_id_ : "0";
		data += "&post_form_id_source=AsyncRequest";

		http::response resp = facy.flap( FACEBOOK_REQUEST_TYPING_SEND, &data );

		DBFreeVariant(&dbv);
	}		

	delete typing;
}

void FacebookProto::CloseChatWorker(void *p)
{
	if (p == NULL)
		return;

	if ( DBGetContactSettingByte(NULL, m_szModuleName, FACEBOOK_KEY_CLOSE_WINDOWS_ENABLE, DEFAULT_CLOSE_WINDOWS_ENABLE ) == TRUE )
		facy.close_chat( *(std::string*)p );

	delete p;
}
