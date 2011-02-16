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

File name      : $HeadURL: https://n3weRm0re.ewer@eternityplugins.googlecode.com/svn/trunk/facebook/main.cpp $
Revision       : $Revision: 92 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-20 21:38:59 +0100 (Thu, 20 Jan 2011) $

*/

#include "common.h"

// TODO: Make following as "globals" structure?
PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
LIST_INTERFACE li;
CLIST_INTERFACE* pcli;
UTF8_INTERFACE utfi;
MD5_INTERFACE md5i;

HINSTANCE g_hInstance;
std::string g_strUserAgent;
DWORD g_mirandaVersion;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Facebook Protocol",
	__VERSION_DWORD,
	"Provides basic support for Facebook Chat protocol. [Built: "__DATE__" "__TIME__"]",
	"Michal Zelinka",
	"jarvis@jabber.cz",
	"© 2009-11 Michal Zelinka",
	"http://dev.miranda.im/~jarvis/",
	UNICODE_AWARE, //not transient
	0,             //doesn't replace anything built-in
	// {DDE897AD-E40A-4e2c-98D8-D20B575E96BC}
	{ 0xdde897ad, 0xe40a, 0x4e2c, { 0x98, 0xd8, 0xd2, 0xb, 0x57, 0x5e, 0x96, 0xbc } }

};

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int compare_protos(const FacebookProto *p1, const FacebookProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<FacebookProto> g_Instances(1, compare_protos);

DWORD WINAPI DllMain(HINSTANCE hInstance,DWORD,LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if(mirandaVersion > PLUGIN_MAKE_VERSION(0,10,0,0) &&
	   mirandaVersion < PLUGIN_MAKE_VERSION(0,10,0,2))
	{
		MessageBox(0,_T("The Facebook protocol plugin cannot be loaded. ")
			_T("It requires Miranda IM 0.10 alpha build #2 or later."),_T("Miranda"),
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}

	else if(mirandaVersion < PLUGIN_MAKE_VERSION(0,9,14,0))
	{
		MessageBox(0,_T("The Facebook protocol plugin cannot be loaded. ")
			_T("It requires Miranda IM 0.9.14 or later."),_T("Miranda"),
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}

	g_mirandaVersion = mirandaVersion;
	return &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	// TODO: Make product version controlled via definitions
	MessageBox(0,_T("The Facebook protocol plugin cannot be loaded. ")
		_T("It requires Miranda IM 0.9.14 or later."),_T("Miranda"),
		MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char *proto_name,const TCHAR *username )
{
	FacebookProto *proto = new FacebookProto(proto_name,username);
	g_Instances.insert(proto);
	return proto;
}

static int protoUninit(PROTO_INTERFACE* proto)
{
	g_Instances.remove(( FacebookProto* )proto);
	return EXIT_SUCCESS;
}

int OnModulesLoaded(WPARAM,LPARAM)
{
	if ( ServiceExists( MS_UPDATE_REGISTER ) )
	{
		Update upd = {sizeof(upd)};
		char curr_version[30];

		upd.szComponentName = pluginInfo.shortName;
		upd.szUpdateURL = UPDATER_AUTOREGISTER;
		upd.szBetaVersionURL     = "http://dev.miranda.im/~jarvis/";
		upd.szBetaChangelogURL   = "http://dev.miranda.im/~jarvis/";
		upd.pbBetaVersionPrefix  = reinterpret_cast<BYTE*>("Facebook</a> ");
		upd.cpbBetaVersionPrefix = strlen(reinterpret_cast<char*>(upd.pbBetaVersionPrefix));
		upd.szBetaUpdateURL      = "http://dev.miranda.im/~jarvis/";
		upd.pbVersion = reinterpret_cast<BYTE*>( CreateVersionStringPlugin(
			reinterpret_cast<PLUGININFO*>(&pluginInfo),curr_version) );
		upd.cpbVersion = strlen(reinterpret_cast<char*>(upd.pbVersion));
		CallService(MS_UPDATE_REGISTER,0,(LPARAM)&upd);
	}

	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
#if defined _DEBUG
	_CrtDumpMemoryLeaks( );
#endif
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getUTFI(&utfi);
	mir_getMD5I(&md5i);

	pcli = reinterpret_cast<CLIST_INTERFACE*>( CallService(
	    MS_CLIST_RETRIEVE_INTERFACE,0,reinterpret_cast<LPARAM>(g_hInstance)) );

	PROTOCOLDESCRIPTOR pd = {sizeof(pd)};
	pd.szName = "Facebook";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE,0,reinterpret_cast<LPARAM>(&pd));

	HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);

	InitIcons();
	//InitContactMenus();

	// Init native User-Agent
	{
		std::stringstream agent;
//		DWORD mir_ver = ( DWORD )CallService( MS_SYSTEM_GETVERSION, NULL, NULL );
		agent << "MirandaIM/";
		agent << (( g_mirandaVersion >> 24) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion >> 16) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion >>  8) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion      ) & 0xFF);
		agent << " FacebookProtocol/";
		agent << __VERSION_STRING;
		g_strUserAgent = agent.str( );
	}

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	//UninitContactMenus();
	return 0;
}
