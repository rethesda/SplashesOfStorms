#pragma once

class RainObject
{
public:
	RainObject() = delete;

	RainObject(std::string_view a_section) :
		enabled(a_section, "Enabled", true),
		rayCastRadius(a_section, "RaycastRadius", 1024.0f),
		rayCastIterations(a_section, "RaycastIterations", 1)
	{}

	RainObject(const RainObject&) = delete;
	RainObject(RainObject&&) = delete;
	RainObject& operator=(const RainObject&) = delete;
	RainObject& operator=(RainObject&&) = delete;

	REX::TTomlSetting<bool>          enabled;
	REX::TTomlSetting<float>         rayCastRadius;
	REX::TTomlSetting<std::uint32_t> rayCastIterations;
};

class Splash : public RainObject
{
public:
	Splash(std::string_view a_rain) :
		RainObject(std::format("{}.splashes", a_rain)),
		nif(std::format("{}.splashes", a_rain), "NifPath", "Effects\\rainSplashNoSpray.NIF"),
		nifActor(std::format("{}.splashes", a_rain), "NifPathActor", "Effects\\rainSplashNoSpray.NIF"),
		nifScale(std::format("{}.splashes", a_rain), "NifScale", 0.6f),
		nifScaleActor(std::format("{}.splashes", a_rain), "NifScaleActor", 0.2f)
	{}

	[[nodiscard]] const std::string& GetNif() const { return static_cast<const std::string&>(nif); }
	[[nodiscard]] const std::string& GetNifActor() const { return static_cast<const std::string&>(nifActor); }

	REX::TTomlSetting<std::string> nif;
	REX::TTomlSetting<std::string> nifActor;
	REX::TTomlSetting<float>       nifScale;
	REX::TTomlSetting<float>       nifScaleActor;
};

class Ripple : public RainObject
{
public:
	Ripple(std::string_view a_rain, std::uint32_t a_iterations = 15) :
		RainObject(std::format("{}.ripples", a_rain)),
		rippleDisplacementAmount(std::format("{}.ripples", a_rain), "RippleDisplacementMult", 0.4f)
	{
		rayCastIterations.SetValue(a_iterations);
	}

	REX::TTomlSetting<float> rippleDisplacementAmount;
};

class Rain
{
public:
	enum class TYPE
	{
		kNone,
		kLight,
		kMedium,
		kHeavy,
		kInvalid  // Snow
	};

	Rain(TYPE a_type, std::string_view a_section) :
		type(a_type),
		splash(a_section),
		ripple(a_section)
	{}

	TYPE   type;
	Splash splash;
	Ripple ripple;
};

namespace Settings
{
	class Manager : public REX::TSingleton<Manager>
	{
	public:
		bool LoadSettings();

		Rain* GetRain(float a_particleDensity);
		Rain* GetRain();

		[[nodiscard]] Rain::TYPE GetRainType() const { return currentRainType; }
		void                     SetRainType(Rain::TYPE a_type) { currentRainType = a_type; }

		REX::TTomlSetting<bool> enableDebugMarkerSplash{ "Settings", "DebugSplashes", false };
		REX::TTomlSetting<bool> enableDebugMarkerRipple{ "Settings", "DebugRipples", false };

	private:
		static inline constexpr auto path = R"(Data\SKSE\Plugins\po3_SplashesOfStorms.toml)"sv;

		Rain light{ Rain::TYPE::kLight, "LightRain" };
		Rain medium{ Rain::TYPE::kMedium, "MediumRain" };
		Rain heavy{ Rain::TYPE::kHeavy, "HeavyRain" };

		Rain::TYPE currentRainType{ Rain::TYPE::kNone };
	};
}
