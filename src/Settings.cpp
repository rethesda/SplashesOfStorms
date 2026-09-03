#include "Settings.h"

namespace Settings
{
	bool Manager::LoadSettings()
	{
		const auto store = REX::FTomlSettingStore::GetSingleton();
		store->Init(path.data(), "");

		store->Load();
		store->Save();

		return true;
	}

	Rain* Manager::GetRain(float a_particleDensity)
	{
		currentRainType = a_particleDensity < 5.0f ? Rain::TYPE::kLight :
		                  a_particleDensity < 9.0f ? Rain::TYPE::kMedium :
		                                             Rain::TYPE::kHeavy;
		return GetRain();
	}

	Rain* Manager::GetRain()
	{
		switch (currentRainType) {
		case Rain::TYPE::kLight:
			return &light;
		case Rain::TYPE::kMedium:
			return &medium;
		case Rain::TYPE::kHeavy:
			return &heavy;
		default:
			return nullptr;
		}
	}
}
