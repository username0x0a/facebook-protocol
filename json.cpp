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

int facebook_json_parser::parse_buddy_list( void* data, List::List< facebook_user >* buddy_list )
{
	try
	{
		facebook_user* current = NULL;
        Json::Value root;

        if ( !reader.parse( static_cast< std::string* >( data )->substr( 9 ), root) )
            return EXIT_FAILURE;

		const Json::Value was_available = root["payload"]["buddy_list"]["wasAvailableIDs"];
        for ( size_t i = 0; i < was_available.size(); ++i ) {
			current = buddy_list->find( was_available[i].asString() );
			if ( current != NULL )
				current->status_id = ID_STATUS_OFFLINE;
        }

        const Json::Value now_available = root["payload"]["buddy_list"]["nowAvailableList"];
		for ( Json::Value::iterator i = now_available.begin(); i != now_available.end(); ++i )
		{
			std::string user_id = i.key().asString();
            const Json::Value member = *i;
			current = buddy_list->find( user_id );
			if ( current == NULL )
				buddy_list->insert( std::make_pair( user_id, new facebook_user( ) ) );
			current = buddy_list->find( user_id );
            current->user_id = current->real_name = user_id;
            current->status_id = ID_STATUS_ONLINE;
			current->is_idle = member["i"].asBool();
        }

        const Json::Value user_infos = root["payload"]["buddy_list"]["userInfos"];
		for ( Json::Value::iterator i = user_infos.begin(); i != user_infos.end(); ++i )
		{
            std::string user_id = i.key().asString();
			const Json::Value member = *i;
			current = buddy_list->find( user_id );
			if ( current == NULL )
                continue;

            current->real_name = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( member["name"].asString() ) );
			current->image_url = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( member["thumbSrc"].asString() ) );
        }
	}
	catch (const std::runtime_error& e)
	{
		proto->Log( "!!!!! Caught std::runtime_error: %s", e.what() );
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_messages( void* data, std::vector< facebook_message* >* messages, std::vector< facebook_notification* >* notifications )
{
	try
	{
        Json::Value root;

        if ( !reader.parse( static_cast< std::string* >( data )->substr( 9 ), root) )
            return EXIT_FAILURE;

        const Json::Value message_array = root["ms"];
        for ( size_t i = 0; i < message_array.size(); ++i ) {
            const Json::Value message = message_array[i];
            std::string type = message["type"].asString();

            if ( type == "msg" ) // chat message
            {
				std::string message_id = message["msg"]["msgID"].asString();
                bool duplicate = false;
				for ( std::list<std::string>::iterator i = proto->facy.messageIDs.begin(); i != proto->facy.messageIDs.end(); i++ )
					if (*i == message_id) {
						duplicate = true; break; }
				if ( duplicate ) {
					proto->facy.messageIDs.remove( message_id ); continue; }
				else
					proto->facy.messageIDs.push_back( message_id );

                facebook_message* m = new facebook_message();
				unsigned __int64 from = message["from"].asUInt64();
				m->user_id = utils::conversion::to_string(
					&from, C_UNSIGNED | C_INTEGER64 );
                m->time = ::time(NULL);
                m->message_text = utils::text::special_expressions_decode(
					utils::text::slashu_to_utf8( message["msg"]["text"].asString() ));
                messages->push_back(m);
            }
            else if ( type == "app_msg" )
            {
                facebook_notification* n = new facebook_notification();
                n->text = utils::text::remove_html(
					utils::text::special_expressions_decode(
						utils::text::slashu_to_utf8( message["response"]["payload"]["title"].asString() )));
                n->link = utils::text::special_expressions_decode(
                    message["response"]["payload"]["link"].asString() );
				notifications->push_back(n);
            }
            else if ( type == "typ" )
            {
                facebook_user u;
				unsigned __int64 from = message["from"].asUInt64();
				u.user_id = utils::conversion::to_string( &from, C_UNSIGNED | C_INTEGER64 );
				HANDLE hContact = proto->AddToContactList(&u);
                if (!hContact) continue;
				DBWriteContactSettingWord(hContact,proto->m_szModuleName,"Status",ID_STATUS_ONLINE);
				if (message["st"] == "1")
					CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)60);
				else
					CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
            }
            else if ( type == "inbox" )
            {
// Disabled due to new inbox
//
//				TCHAR info[512]; char num[32];
//				std::string unseen = message["unseen"].asString();
//
//				mir_sntprintf(info, 500, TranslateT("You have %s unseen messages"), unseen.c_str());
//				proto->NotifyEvent(TranslateT("Unseen messages"), info, NULL, FACEBOOK_EVENT_OTHER, TEXT( FACEBOOK_URL_MESSAGES ) );
            }
            else
                continue;
        }
	}
	catch (const std::runtime_error& e)
	{
		proto->Log( "!!!!! Caught std::runtime_error: %s", e.what() );
	}

	return EXIT_SUCCESS;
}
