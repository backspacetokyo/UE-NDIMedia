#pragma once

#include "MediaIOCoreTextureSampleBase.h"
#include "MediaIOCoreBinarySampleBase.h"

class FNDIMediaTextureSample
	: public FMediaIOCoreTextureSampleBase
{};

class FNDIMediaTextureSamplePool
	: public TMediaObjectPool<FNDIMediaTextureSample>
{};

///

class FNDIMediaBinarySample
	: public FMediaIOCoreBinarySampleBase
{};

class FNDIMediaBinarySamplePool
	: public TMediaObjectPool<FNDIMediaBinarySample>
{};