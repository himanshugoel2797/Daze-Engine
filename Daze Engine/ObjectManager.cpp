#include "stdafx.h"
#include "ObjectManager.h"

EngineID ObjectManager::ID = 0;

EngineID ObjectManager::NewID(void)
{
	return ++ID;
}