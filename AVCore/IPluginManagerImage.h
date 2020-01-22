#pragma once
#include "PluginManager.h"
#include "PluginInterface.h"
#include "MessageManager.h"

// Позволяет получить доступ к PluginManager
class IPluginManagerImage
{
public:
	virtual ~IPluginManagerImage() {}
	virtual PluginManager * getPluginManager() = 0;
	virtual MessageManager * getMessageManager() = 0;
};