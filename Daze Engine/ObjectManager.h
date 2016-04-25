#pragma once

typedef uint64_t EngineID;

class ObjectManager
{
public:
	static EngineID NewID(void);
private:
	static EngineID ID;
};

