#pragma once
#include "PluginManager.h"
#include "PluginInterface.h"
#include "MessageManager.h"

// ��������� �������� ������ � PluginManager
class IPluginManagerImage
{
public:
	virtual ~IPluginManagerImage() {}
	virtual PluginManager * getPluginManager() = 0;
	virtual MessageManager * getMessageManager() = 0;
};