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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/list.hpp $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (Sat, 08 Jan 2011) $

*/

#pragma once

namespace List
{
	template< typename T > class Item
	{
	public:
		std::string key;
		T* data;
		Item< T >* prev;
		Item< T >* next;
		
		Item( )
		{
			this->data = NULL;
			this->prev = NULL;
			this->next = NULL;
		}

		~Item( )
		{
			delete this->data;
		}
	};

	template< typename T > class List
	{
	private:
		Item< T >* first;
		Item< T >* last;
		unsigned int count;
		
	public:
		List( )
		{
			this->first = this->last = NULL;
			this->count = 0;
		}

		~List( )
		{
			this->clear( );
		}

		Item< T >* begin( )
		{
			return first;
		}
		Item< T >* end( )
		{
			return last;
		}

		unsigned int size( )
		{
			return count;
		}

		bool empty( )
		{
			return ( this->count == 0 );
		}

		void insert( Item< T >* item )
		{
			if ( this->empty( ) )
				this->first = this->last = item;
			else {
				// TODO: key sorting/comparation
				item->next = this->first;
				this->first->prev = item;
				this->first = item; }
			this->count++;
		}

		void insert( std::pair< std::string, T* > item )
		{
			Item<T>* ins = new Item<T>;
			ins->key = item.first;
			ins->data = item.second;
			this->insert( ins );
		}
		void erase( std::string key )
		{
			Item< T >* help = this->first;
			while ( help != NULL ) {
				if ( help->key.compare( key ) != 0 ) help = help->next;
				else break; }
			if ( help != NULL ) {
				if ( help == this->first ) this->first = first->next;
				else help->prev->next = help->next;
				if ( help == this->last ) this->last = help->prev;
				else help->next->prev = help->prev;
				this->count--;
				delete help;
			}
		}

		void erase( Item< T >* item )
		{
			erase( item->key );
		}

		T* find( std::string key )
		{
			Item< T >* help = this->begin( );
			while ( help != NULL && help->key != key )
				help = help->next;
			if ( help != NULL ) return help->data;
			else return NULL;
		}

		T* at( const unsigned int item )
		{
			Item< T >* help = this->begin( );
			for ( unsigned int i = 0; i < item; i++ )
				help = help->next;
			return help->item;
		}

		T* operator[]( const unsigned int item )
		{
			return at( item );
		}

		void clear( )
		{
			while ( !this->empty( ) && this->begin( ) != NULL )
				this->erase( this->begin( ) );
		}
	};
};
