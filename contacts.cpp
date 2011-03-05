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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/contacts.cpp $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#include "common.h"

bool FacebookProto::IsMyContact(HANDLE hContact)
{
	const char *proto = reinterpret_cast<char*>( CallService(MS_PROTO_GETCONTACTBASEPROTO,
		reinterpret_cast<WPARAM>(hContact),0) );

	if(proto && strcmp(m_szModuleName,proto) == 0)
		return true;
	else
		return false;
}

HANDLE FacebookProto::ContactIDToHContact(std::string user_id)
{
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	    hContact;
	    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0) )
	{
		if(!IsMyContact(hContact))
			continue;

		DBVARIANT dbv;
		if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
		{
			if(strcmp(user_id.c_str(),dbv.pszVal) == 0) {
				DBFreeVariant(&dbv);
				return hContact; }
			else
				DBFreeVariant(&dbv);
		}
	}

	return 0;
}

HANDLE FacebookProto::AddToContactList(facebook_user* fbu)
{
	// First, check if this contact exists
	HANDLE hContact = ContactIDToHContact(fbu->user_id);
	if(hContact)
		return hContact;

	// If not, make a new contact!
	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if(hContact)
	{
		if(CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact,(LPARAM)m_szModuleName) == 0)
		{
			DBWriteContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,fbu->user_id.c_str());
			std::string profile_url = FACEBOOK_URL_HOMEPAGE;
			profile_url += "profile.php?id=";
			profile_url += fbu->user_id;
			DBWriteContactSettingString(hContact,m_szModuleName,"Homepage",profile_url.c_str());
			DBDeleteContactSetting(hContact, "CList", "MyHandle");
			DBVARIANT dbv;
			if( !DBGetContactSettingTString(NULL,m_szModuleName,FACEBOOK_KEY_DEF_GROUP,&dbv) )
			{
				DBWriteContactSettingTString(hContact,"CList","Group",dbv.ptszVal);
				DBFreeVariant(&dbv);
			}
			if (getByte(FACEBOOK_KEY_DISABLE_STATUS_NOTIFY, 0)) // TODO: Obsolete?
				CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (LPARAM)IGNOREEVENT_USERONLINE);
			return hContact;
		}
		else
			CallService(MS_DB_CONTACT_DELETE,(WPARAM)hContact,0);
	}

	return 0;
}

bool FacebookProto::ContactNeedsUpdate(facebook_user* fbu)
{
	return ( ::time(NULL) - fbu->last_update ) > FACEBOOK_USER_UPDATE_RATE;
}

void FacebookProto::SetAllContactStatuses(int status)
{
	for (HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	    hContact;
	    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{
		if (!IsMyContact(hContact))
			continue;

		if (DBGetContactSettingWord(hContact,m_szModuleName,"Status",ID_STATUS_OFFLINE) == status)
			continue;

		if (status == ID_STATUS_OFFLINE)
			DBDeleteContactSetting(hContact,m_szModuleName,"IdleTS");

		DBWriteContactSettingWord(hContact,m_szModuleName,"Status",status);
	}
}

void FacebookProto::UpdateContactWorker(void *p)
{
	if ( p == NULL ) return;

	facebook_user* fbu = ( facebook_user* )p;

	if ( this->isOffline( ) )
		goto exit;

	if ( fbu->status_id == ID_STATUS_OFFLINE )
	{
		DBWriteContactSettingWord(fbu->handle,m_szModuleName,"Status",ID_STATUS_OFFLINE );
		DBDeleteContactSetting(fbu->handle,m_szModuleName,"IdleTS");
		goto exit;
	}
	else // ID_STATUS_ONLINE + _CONNECTING for self-contact
	{
		if ( fbu->user_id != facy.self_.user_id ) // if real contact
		{
			if (!fbu->handle) // just been added
				fbu->handle = AddToContactList(fbu);

			if (!fbu->last_update) // just come online
				DBWriteContactSettingWord(fbu->handle,m_szModuleName,"Status",ID_STATUS_ONLINE );

			// Update idle flag
			if ( fbu->is_idle ) {
				if ( !DBGetContactSettingDword(fbu->handle,m_szModuleName,"IdleTS",0) )
					DBWriteContactSettingDword(fbu->handle,m_szModuleName,"IdleTS",(DWORD)time(NULL)); }
			else {
				if ( DBGetContactSettingDword(fbu->handle,m_szModuleName,"IdleTS",0) )
					DBWriteContactSettingDword(fbu->handle,m_szModuleName,"IdleTS",0); }
		}

		if ( fbu->user_id == facy.self_.user_id || ContactNeedsUpdate( fbu ) )
		{
			bool update_required = false;
			DBVARIANT dbv;

			// Update Real name

			if ( !DBGetContactSettingString(fbu->handle,m_szModuleName,FACEBOOK_KEY_ID,&dbv) ) {
				update_required = strcmp( dbv.pszVal, fbu->real_name.c_str() ) != 0;
				DBFreeVariant(&dbv); }
			else update_required = true;

			if ( update_required ) {
				DBWriteContactSettingUTF8String(fbu->handle,m_szModuleName,FACEBOOK_KEY_NAME,fbu->real_name.c_str());
				DBWriteContactSettingUTF8String(fbu->handle,m_szModuleName,"Nick",fbu->real_name.c_str()); }

			// Get Status message and Avatar URL if needed

			facy.get_profile( fbu );

			// Update status message

			if ( fbu->user_id == facy.self_.user_id )
				update_required = true;
			else if ( !DBGetContactSettingUTF8String(fbu->handle,"CList","StatusMsg",&dbv) ) {
				update_required = strcmp( dbv.pszVal, fbu->status.c_str() ) != 0;
				DBFreeVariant(&dbv); }
			else update_required = true;

			if ( update_required ) {
				if ( fbu->user_id == facy.self_.user_id )
					setU8String("StatusMsg",fbu->status.c_str());
				else
					DBWriteContactSettingUTF8String(fbu->handle,"CList","StatusMsg",fbu->status.c_str());
			}

			// Update avatar

			if ( !DBGetContactSettingString(fbu->handle,m_szModuleName,FACEBOOK_KEY_AV_URL,&dbv) ) {
				update_required = strcmp( dbv.pszVal, fbu->image_url.c_str() ) != 0;
				DBFreeVariant(&dbv); }
			else update_required = true;

			update_required = update_required || !AvatarExists(fbu->user_id);

			if ( update_required ) {
				DBWriteContactSettingString(fbu->handle,m_szModuleName,FACEBOOK_KEY_AV_URL,fbu->image_url.c_str());
				ProcessAvatar(fbu->handle,&fbu->image_url); }

			// Update update timestamp

			fbu->last_update = ::time( NULL );
		}
	}

exit:
	if ( fbu->status_id == ID_STATUS_OFFLINE && fbu->user_id != this->facy.self_.user_id )
		delete fbu;
}

void FacebookProto::GetAwayMsgWorker(void *hContact)
{
	if(hContact == 0) return;

	DBVARIANT dbv;
	if( !DBGetContactSettingString(hContact,"CList","StatusMsg",&dbv) )
	{
		ProtoBroadcastAck(m_szModuleName,hContact,ACKTYPE_AWAYMSG,ACKRESULT_SUCCESS,
			(HANDLE)1,(LPARAM)dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else
	{
		ProtoBroadcastAck(m_szModuleName,hContact,ACKTYPE_AWAYMSG,ACKRESULT_FAILED,
			(HANDLE)1,(LPARAM)0);
	}
}

HANDLE FacebookProto::GetAwayMsg(HANDLE hContact)
{
	ForkThread(&FacebookProto::GetAwayMsgWorker, this,hContact);
	return (HANDLE)1;
}

int FacebookProto::OnBuildContactMenu(WPARAM,LPARAM)
{
	return NULL;
}

int FacebookProto::OnContactDeleted(WPARAM wparam,LPARAM)
{
	HANDLE hContact = (HANDLE)wparam;

	ScopedLock s(facy.buddies_lock_);

	for (List::Item< facebook_user >* i = facy.buddies.begin( ); i != NULL; i = i->next )
		if (hContact == i->data->handle) {
			facy.buddies.erase(i->key); break; }

	return NULL;
}
