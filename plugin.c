#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"

static struct TS3Functions ts3Functions;
static char* pluginID = NULL;

#define PLUGIN_API_VERSION 20
#define MENU_ITEM_ID 1

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif


const char* ts3plugin_name() {
	return "New Mission";
}


const char* ts3plugin_version() {
	return "1.0";
}


int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}


const char* ts3plugin_author() {
	return "Starfox64";
}


const char* ts3plugin_description() {
	return "This plugin adds a context menu option to poke an entire channel regarding a new mission.";
}


void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}


int ts3plugin_init() {
	#ifdef _WIN32
	freopen("CONOUT$", "wb", stdout);
	#endif
	return 0;
}


void ts3plugin_shutdown() {
	if (pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}


void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);
}


int ts3plugin_requestAutoload() {
	return 1;
}


void ts3plugin_freeMemory(void* data) {
	free(data);
}


/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}


#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);


void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
	BEGIN_CREATE_MENUS(1);
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ITEM_ID, "Poke", NULL);
	END_CREATE_MENUS;
}


void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	if (type == PLUGIN_MENU_TYPE_CHANNEL && menuItemID == MENU_ITEM_ID)
	{
		anyID* clients;

		if (ts3Functions.getChannelClientList(serverConnectionHandlerID, selectedItemID, &clients) == ERROR_ok)
		{
			anyID selfID;

			if (ts3Functions.getClientID(serverConnectionHandlerID, &selfID) == ERROR_ok)
			{
				unsigned int count = 0;

				for (anyID* client = clients; *client != (anyID) NULL; client++)
				{
					if (*client != selfID)
					{
						ts3Functions.requestClientPoke(serverConnectionHandlerID, *client, "New Mission!", "NewMissionPokeReturnCode");
						count++;
					}
				}

				char result[32];
				sprintf(result, "Poked %u players.", count);

				ts3Functions.printMessage(serverConnectionHandlerID, result, PLUGIN_MESSAGE_TARGET_CHANNEL);
				ts3Functions.freeMemory(clients);
			}
		}
	}
}
