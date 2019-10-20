#include "PluginManager.h"
#include <Windows.h>

typedef IPlugin* (* GetPlugin)();

IPlugin* PluginManager::loadPlugin(char* path)
{
	HMODULE pluginDll = LoadLibraryA(path);
	if (pluginDll == 0)
	{
		DWORD lastError = GetLastError();
		throw "PLUGIN NOT FOUND";
	}
	GetPlugin getPlugin = (GetPlugin)GetProcAddress(pluginDll, "GetPlugin");
	if (getPlugin == 0)
		throw "PLUGIN NOT FOUND";
	IPlugin * plugin = getPlugin();
	plugin->init(this);
	return plugin;
}

int PluginManager::registerCallback(IPlugin * plugin, int callbackId)
{
	this->callbacks.push_back(std::pair<IPlugin *, int>(plugin, callbackId));
	return 0;
}

AV_EVENT_RETURN_STATUS PluginManager::processEvent(void* event)
{
	for (std::list<std::pair<IPlugin*, int>>::iterator it = this->callbacks.begin(); it != this->callbacks.end(); ++it)
	{
		(*it).first->callback((*it).second, event);
	}
	return AvEventStatusAllow;
}
