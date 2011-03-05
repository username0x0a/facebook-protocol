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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/json.cpp $
Revision       : $Revision: 92 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-20 21:38:59 +0100 (Thu, 20 Jan 2011) $

*/

#include "common.h"
#include "JSON_CAJUN/reader.h"
#include "JSON_CAJUN/writer.h"
#include "JSON_CAJUN/elements.h"

int facebook_json_parser::parse_buddy_list( void* data, List::List< facebook_user >* buddy_list )
{
	using namespace json;

	try
	{
		facebook_user* current = NULL;
		std::string buddyData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( buddyData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
		const Array& wasAvailableIDs = objRoot["payload"]["buddy_list"]["wasAvailableIDs"];

		for ( Array::const_iterator itWasAvailable( wasAvailableIDs.Begin() );
			itWasAvailable != wasAvailableIDs.End(); ++itWasAvailable)
		{
			const Number& member = *itWasAvailable;
			char was_id[32];
			lltoa( member.Value(), was_id, 10 );

			current = buddy_list->find( std::string( was_id ) );
			if ( current != NULL )
				current->status_id = ID_STATUS_OFFLINE;
		}

		const Object& nowAvailableList = objRoot["payload"]["buddy_list"]["nowAvailableList"];

		for (Object::const_iterator itAvailable(nowAvailableList.Begin());
			itAvailable != nowAvailableList.End(); ++itAvailable)
		{
			const Object::Member& member = *itAvailable;

			current = buddy_list->find( member.name );
			if ( current == NULL )
				buddy_list->insert( std::make_pair( member.name, new facebook_user( ) ) );

			current = buddy_list->find( member.name );
			const Object& objMember = member.element;
			const Boolean idle = objMember["i"];

			current->user_id = current->real_name = member.name;
			current->status_id = ID_STATUS_ONLINE;
			current->is_idle = ( idle.Value( ) == 1 );
		}

		const Object& userInfosList = objRoot["payload"]["buddy_list"]["userInfos"];

		for (Object::const_iterator itUserInfo(userInfosList.Begin());
			itUserInfo != userInfosList.End(); ++itUserInfo)
		{
			const Object::Member& member = *itUserInfo;

			current = buddy_list->find( member.name );
			if ( current == NULL )
				continue;

			const Object& objMember = member.element;
			const String& realName = objMember["name"];
			const String& imageUrl = objMember["thumbSrc"];

			current->real_name = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( realName.Value( ) ) );
			current->image_url = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( imageUrl.Value( ) ) );
		}
	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log( "!!!!! Caught json::Exception: %s", e.what() );
	}
	catch (const std::exception& e)
	{
		proto->Log( "!!!!! Caught std::exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_messages( void* data, std::vector< facebook_message* >* messages, std::vector< facebook_notification* >* notifications )
{
	using namespace json;

	try
	{
		std::string messageData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( messageData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
		const Array& messagesArray = objRoot["ms"];

		for (Array::const_iterator itMessage(messagesArray.Begin());
			itMessage != messagesArray.End(); ++itMessage)
		{
			const Object& objMember = *itMessage;

			const String& type = objMember["type"];

			if ( type.Value( ) == "msg" ) // chat message
			{
				const Number& from = objMember["from"];
				char was_id[32];
				lltoa( from.Value(), was_id, 10 );
				
				const Object& messageContent = objMember["msg"];

				const String& messageID = messageContent["msgID"];
				bool duplicate = false;
				for ( std::list<std::string>::iterator i = proto->facy.messageIDs.begin(); i != proto->facy.messageIDs.end(); i++ )
					if (*i == messageID.Value()) {
						duplicate = true; break; }

				if ( duplicate ) {
					proto->facy.messageIDs.remove( messageID.Value() ); continue; }
				else
					proto->facy.messageIDs.push_back( messageID.Value() );

				const String& text = messageContent["text"];

				facebook_message* message = new facebook_message( );
				message->message_text= utils::text::special_expressions_decode(
					utils::text::slashu_to_utf8( text.Value( ) ) );
				message->time = ::time( NULL );
				message->user_id = was_id;

				messages->push_back( message );
			}
			else if ( type.Value( ) == "app_msg" ) // event notification
			{
				const String& text = objMember["response"]["payload"]["title"];
				const String& link = objMember["response"]["payload"]["link"];

				facebook_notification* notification = new facebook_notification( );
				notification->text = utils::text::remove_html(
					utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( text.Value( ) ) ) );

				notification->link = utils::text::special_expressions_decode( link.Value( ) );

				notifications->push_back( notification );
			}
			else if ( type.Value( ) == "typ" ) // chat typing notification
			{
				const Number& from = objMember["from"];
				char user_id[32];
				lltoa( from.Value(), user_id, 10 );

				facebook_user fbu;
				fbu.user_id = user_id;

				HANDLE hContact = proto->AddToContactList(&fbu);
				DBWriteContactSettingWord(hContact,proto->m_szModuleName,"Status",ID_STATUS_ONLINE);

				const Number& state = objMember["st"];
				if (state.Value() == 1)
					CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)60);
				else
					CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
			}
			else if ( type.Value( ) == "inbox" )
			{
// Temporarily(?) disabled
// due to new inbox
//
//				TCHAR info[512]; char num[32];
//				const Number& unseen = objMember["unseen"];
//
//				lltoa( unseen.Value(), num, 10 );
//				mir_sntprintf(info, 500, TranslateT("You have %s unseen messages"), num);
//				proto->NotifyEvent(TranslateT("Unseen messages"), info, NULL, FACEBOOK_EVENT_OTHER, TEXT( FACEBOOK_URL_MESSAGES ) );
			}
			else
				continue;
		}
	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log ( "!!!!! Caught json::Exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}
