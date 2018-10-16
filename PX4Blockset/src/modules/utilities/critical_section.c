#include "critical_section.h"

#define CRITICAL_SECTION_MAX_CNT 20
static int32_t id = 0;
static int32_t sections[CRITICAL_SECTION_MAX_CNT];

int32_t register_critical_section()
{
	int32_t ret = 0;
	if(id < CRITICAL_SECTION_MAX_CNT)
	{
		sections[id] = 0;
		ret = id;
		id++;
	}
	else 
	{
		ret = -1;
	}
	return ret;
}

ErrorStatus enter_critical_section(int32_t cs)
{
	if(cs < 0 || cs > CRITICAL_SECTION_MAX_CNT)
	{
		return ERROR;
	}
	
	sections[cs]++;
	
	if(sections[cs] == 1)
	{
		return SUCCESS;
	}
	else
	{
		sections[cs]--;
		return ERROR;
	}
}

inline void leave_critical_section(int32_t cs)
{
	sections[cs]--;
}

