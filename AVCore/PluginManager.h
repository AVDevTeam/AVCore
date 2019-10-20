#pragma once
#include "PluginInterface.h"
#include "EventsParser.h"
#include <string>
#include <list>
#include <map>
#include <iostream>


typedef std::pair<int, IPlugin*> callback;
typedef std::map<int, callback> priorityMap;
typedef std::map <int, priorityMap*> eventsMap;

class PluginManager : public IManager
{
public:
	IPlugin * loadPlugin(char* path);
	int registerCallback(IPlugin * plugin, int callbackId, int, int);
	AV_EVENT_RETURN_STATUS processEvent(int, void*);
	void addEventParser(int, EventParser*);

private:
	/*
	Callbacks map (two levels):
	{
		"eventType" : 
		{
			Priority1 : (callbackId, Plutin)
			...
		}
	}
	*/
	eventsMap callbacksMap;
	std::map<int, EventParser*> parsersMap;
};