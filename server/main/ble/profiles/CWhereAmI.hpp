#pragma once
#include "../../server_common.hpp"
#include "CGattService.hpp"
#include "profiles.hpp"
#include <memory>

namespace ble
{
class CWhereAmI
{
public:
	CWhereAmI();
	~CWhereAmI() = default;
	CWhereAmI(const CWhereAmI& other);
	CWhereAmI(CWhereAmI&& other) = default;
	CWhereAmI& operator=(const CWhereAmI& other);
	CWhereAmI& operator=(CWhereAmI&& other) noexcept;
public:
	void register_with_nimble(const std::shared_ptr<Profile>& pProfile);
	[[nodiscard]] ble_gatt_svc_def as_nimble_service() const;
private:
	[[nodiscard]] std::vector<CCharacteristic> make_characteristics(const std::shared_ptr<Profile>& pProfile);

	[[nodiscard]] auto make_callback_client_query(const std::shared_ptr<Profile>& pProfile);
	[[nodiscard]] CCharacteristic make_characteristic_client_query(const std::shared_ptr<Profile>& pProfile);
private:
	std::vector<CCharacteristic> m_Characteristics;
	CGattService m_Service;
};
}	// namespace ble