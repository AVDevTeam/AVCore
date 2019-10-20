#pragma once
#include "PluginInterface.h"
#include "EventsParser.h"
#include <string>
#include <list>
#include <map>
#include <iostream>


typedef std::pair<int, IPlugin*> callback;
typedef std::map<int, callback> priorityMap;
typedef std::map <AV_EVENT_TYPE, priorityMap*> eventsMap;

class PluginManager : public IManager
{
public:
	IPlugin * loadPlugin(std::string path);
	int registerCallback(IPlugin * plugin, int callbackId, AV_EVENT_TYPE eventType, int priority);
	AV_EVENT_RETURN_STATUS processEvent(AV_EVENT_TYPE eventType, void*);
	void addEventParser(AV_EVENT_TYPE, EventParser*);

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