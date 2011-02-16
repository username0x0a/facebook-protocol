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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/entities.h $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#pragma once

struct facebook_user
{
	HANDLE handle;

	std::string user_id;
	std::string real_name;

	unsigned int status_id;
	std::string status;
	bool is_idle;

	std::string image_url;

	time_t last_update;

	facebook_user( )
	{
		this->handle = NULL;
		this->user_id = this->real_name = this->status = this->image_url = "";
		this->is_idle = false;
		this->status_id = ID_STATUS_OFFLINE;
		this->last_update = 0;
	}

	facebook_user( facebook_user* fu )
	{
		this->handle = fu->handle;
		this->image_url = fu->image_url;
		this->is_idle = fu->is_idle;
		this->last_update = fu->last_update;
		this->real_name = fu->real_name;
		this->status = fu->status;
		this->status_id = fu->status_id;
		this->user_id = fu->user_id;
	}
};

struct facebook_message
{
	std::string user_id;
	std::string message_text;
	time_t time;

	facebook_message( )
	{
		this->user_id = this->message_text = "";
		this->time = 0;
	}

	facebook_message( const facebook_message& msg )
	{
		this->user_id = msg.user_id;
		this->message_text = msg.message_text;
		this->time = msg.time;
	}
};

struct facebook_notification
{
	std::string user_id;
	std::string text;
	std::string link;

	facebook_notification( ) {
		this->user_id = this->text = this->link = ""; }
};

struct facebook_newsfeed
{
	std::string user_id;
	std::string title;
	std::string text;
	std::string link;

	facebook_newsfeed( ) {
		this->user_id = this->title = this->text = this->link = ""; }
};
