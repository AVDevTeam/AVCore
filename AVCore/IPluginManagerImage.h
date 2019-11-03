#pragma once
#include "PluginManager.h"
#include "PluginInterface.h"

// Позволяет получить доступ к PluginManager
class IPluginManagerImage
{
public:
	virtual ~IPluginManagerImage() {}
	virtual PluginManager * getPluginManager() = 0;
};