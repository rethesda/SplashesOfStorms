#pragma once

namespace util
{
	inline std::pair<bool, float> point_in_water(const RE::NiPoint3& a_pos)
	{
		if (auto waterSystem = RE::TESWaterSystem::GetSingleton(); waterSystem->enabled) {
			for (const auto& waterObject : waterSystem->waterObjects) {
				if (waterObject) {
					for (const auto& bound : waterObject->multiBounds) {
						if (bound) {
							if (auto size{ bound->size }; size.z <= 10.0f) {  //avoid sloped water
								auto center{ bound->center };
								const auto boundMin = center - size;
								const auto boundMax = center + size;
								if (!(a_pos.x < boundMin.x || a_pos.x > boundMax.x || a_pos.y < boundMin.y || a_pos.y > boundMax.y)) {
									return { true, center.z };
								}
							}
						}
					}
				}
			}
		}
		return { false, 0.0f };
	}
}

namespace RayCast
{
	using namespace util;

	struct Input
	{
		RE::NiPoint3 rayOrigin{};
	};

	struct Output
	{
		RE::NiPoint3 hitPos{};
		RE::NiMatrix3 normal{};
		bool hitActor{ false };
		bool hitWater{ false };
	};

	inline std::optional<RE::NiPoint3> GenerateRandomPointAroundPlayer(float a_radius, const RE::NiPoint3& a_posIn, bool a_inPlayerFOV)
	{
		auto rng = REX::TRandom<float>();
		
		const float r = a_radius * std::sqrtf(rng.Generate(0.0f,1.0f));
		const float theta = rng.Generate(0.0f, 1.0f) * RE::NI_TWO_PI;

		const RE::NiPoint3 randPoint{
			a_posIn.x + r * std::cosf(theta),
			a_posIn.y + r * std::sinf(theta),
			a_posIn.z
		};

		if (!a_inPlayerFOV || RE::Main::WorldRootCamera()->PointInFrustum(randPoint, 32.0f)) {
			return randPoint;
		}

		return std::nullopt;
	}

	inline std::optional<Output> GenerateRayCast(RE::TESObjectCELL* a_cell, const Input& a_input)
	{
		if (!a_cell || a_cell != RE::PlayerCharacter::GetSingleton()->GetParentCell()) {
			return std::nullopt;
		}

		const auto bhkWorld = a_cell->GetbhkWorld();
		if (!bhkWorld) {
			return std::nullopt;
		}

		RE::NiPoint3 rayStart = a_input.rayOrigin;
		RE::NiPoint3 rayEnd = a_input.rayOrigin;

		constexpr auto height = 9999.0f;

		rayStart.z += height;
		rayEnd.z -= height;

		RE::bhkPickData pickData{};

		const auto havokWorldScale = RE::bhkWorld::GetWorldScale();
		pickData.rayInput.from = rayStart * havokWorldScale;
		pickData.rayInput.to = rayEnd * havokWorldScale;
		pickData.rayInput.enableShapeCollectionFilter = false;
		pickData.rayInput.filterInfo.SetCollisionLayer(RE::COL_LAYER::kLOS);

		if (bhkWorld->PickObject(pickData); pickData.rayOutput.HasHit()) {
			Output output;

			const auto distance = rayEnd - rayStart;
			output.hitPos = rayStart + (distance * pickData.rayOutput.hitFraction);

			output.normal.SetEulerAnglesXYZ({ -0, -0, REX::TRandom<float>().Generate(-RE::NI_PI, RE::NI_PI) });

			switch (pickData.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo.GetCollisionLayer()) {
			case RE::COL_LAYER::kCharController:
			case RE::COL_LAYER::kBiped:
			case RE::COL_LAYER::kDeadBip:
			case RE::COL_LAYER::kBipedNoCC:
				output.hitActor = true;
				break;
			default:
				{
					if (auto [inWater, waterHeight] = point_in_water(output.hitPos); inWater && waterHeight > output.hitPos.z) {
						output.hitWater = true;
						output.hitPos.z = waterHeight;
					}
				}
				break;
			}

			return output;
		}

		return std::nullopt;
	}
}

namespace Ripples
{
	struct Static
	{
		static inline bool showProceduralWater = false;

		static void ToggleWaterRipples(RE::TESWaterSystem* a_waterSystem, bool a_enabled, float a_fadeAmount)
		{
			float fadeAmount = a_fadeAmount;
			if (a_enabled && a_fadeAmount > 0.0f) {
				showProceduralWater = true;
			} else {
				if (!showProceduralWater) {
					return;
				}
				showProceduralWater = false;
				fadeAmount = 0.0f;
			}
			for (auto& waterObject : a_waterSystem->waterObjects) {
				if (waterObject) {
					if (const auto& rippleObject = waterObject->waterRippleObject; rippleObject) {
						if (a_enabled) {
							rippleObject->SetAppCulled(false);
						} else {
							rippleObject->SetAppCulled(true);
						}

						RE::BSVisit::TraverseScenegraphGeometries(rippleObject.get(), [&](RE::BSGeometry* a_geometry) -> RE::BSVisit::BSVisitControl {
							using Feature = RE::BSShaderMaterial::Feature;

							if (const auto& effect = a_geometry->shaderProperty.get()) {
								if (const auto effectShaderProp = netimmerse_cast<RE::BSEffectShaderProperty*>(effect)) {
									if (const auto material = static_cast<RE::BSEffectShaderMaterial*>(effectShaderProp->material)) {
										material->baseColor.alpha = fadeAmount;
									}
								}
							}

							return RE::BSVisit::BSVisitControl::kContinue;
						});
					}
				}
			}
		}
	};

	struct Dynamic
	{
		static inline float rippleTimer = 0.0f;
		static constexpr float rippleDelay = 0.01f;

		static void ToggleWaterRipples(Rain* a_rain, RE::TESWaterSystem* a_waterSystem)
		{
			rippleTimer += RE::GetSecondsSinceLastFrame();

			if (rippleTimer > rippleDelay) {
				rippleTimer = 0.0f;

				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto cell = player->GetParentCell();
				if (!cell) {
					return;
				}

				const auto playerPos = RE::PlayerCharacter::GetSingleton()->GetPosition();

				const auto rayCastRadius = a_rain->ripple.rayCastRadius;
				const auto rayCastIterations = a_rain->ripple.rayCastIterations;

				static const auto enableDebugMarker = Settings::Manager::GetSingleton()->enableDebugMarkerRipple;

				for (std::size_t i = 0; i < rayCastIterations; i++) {
					SKSE::GetTaskInterface()->AddTask([=] {
						const RayCast::Input rayCastInput{
							*RayCast::GenerateRandomPointAroundPlayer(rayCastRadius, playerPos, false),
						};
						if (const auto rayCastOutput = GenerateRayCast(cell, rayCastInput); rayCastOutput) {
							if (rayCastOutput->hitWater) {
								if (!enableDebugMarker) {
									a_waterSystem->AddRipple(rayCastOutput->hitPos, a_rain->ripple.rippleDisplacementAmount * 0.0099999998f);
								} else {
									RE::BSTempEffectParticle::Spawn(cell, 1.6f, "MarkerX.nif", rayCastOutput->normal, rayCastOutput->hitPos, 0.5f, 7, nullptr);
								}
							}
						}
					});
				}
			}
		}
	};
}
