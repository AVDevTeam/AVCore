#pragma once
#include "PluginManager.h"
#include "PluginInterface.h"

// ��������� �������� ������ � PluginManager
class IPluginManagerImage
{
public:
	virtual ~IPluginManagerImage() {}
	virtual PluginManager * getPluginManager() = 0;
};