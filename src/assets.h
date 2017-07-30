#ifndef LD39_ASSETS_H_
#define LD39_ASSETS_H_

#include <heightmap.h>

typedef enum
{
	MAX_HEIGHTMAP_MODEL = MAX_ACTIVE_MAPS-1,
	MDL_THERMAL,
	MDL_TOWER,
} ld39_model_enum;

typedef enum
{
	SOUND_WIND,
	SOUND_STALL,
	SOUND_CRASH,
	SOUND_LIVE_WIRE,
	SOUND_POWER_ON,
	SOUND_BOOST,
	SOUND_THERMAL,
} ld39_sound_enum;

#endif // LD39_ASSETS_H_
