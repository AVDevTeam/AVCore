#pragma once
#include <list>
#include <thread>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>


#include <windows.h>
#include <fltUser.h>
#include "PluginInterface.h"
#include "EventsUMStructures.h"
#include <mutex>

#include <sstream> // to convert thread::id to string

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#endif

#define  KM_EVENTS_LISTENER_THREAD_COUNT   4 // DEBUG !!!      // the number of scanning worker threads.

/*
Implements logic of the worker thread that receives events
from kernel via communication port.
*/
class CommPortListener
{
public:
	CommPortListener(IManager*);
	void listen();
	void signalStop() { this->stop = true;  }
	// worker thread.
	std::thread * thread;
	void setEventsPort(HANDLE eventsPort) { this->eventsPort = eventsPort; }
	void setCompletionPort(HANDLE completionPort) { this->completionPort = completionPort; }

private:
	IManager* pluginManager;
	boolean stop;
	HANDLE eventsPort;
	HANDLE completionPort;
};

/*
This class implements logic of KM-UM communication via fltmgr
communication port.
*/
class CommPortServer
{
public:
	/*
	Method description:
		Starts listening to events.
	Arguments:
		Pointer to the pluginManager that will be passed
		to listeners.
	*/
	void start(IManager *);
	void stop();
	
private:
	void signalCancel();
	void closePorts();

	std::list<CommPortListener *> listeners;

	IManager* pluginManager;

	// Client side communication port.
	HANDLE eventsPort;
	// IO Completion port for multi-thread event processing.
	HANDLE completionPort;
};