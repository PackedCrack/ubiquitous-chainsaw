#include "CGattService.hpp"


// should be in a deifine.hpp file somehwere
#define NO_FLAGS 0

namespace
{
[[nodiscard]] constexpr ble_gatt_chr_def end_of_array()
{
	return ble_gatt_chr_def{
		.uuid = nullptr,
		.access_cb = nullptr,
		.arg = nullptr,
		.descriptors = nullptr,
		.flags = NO_FLAGS,
		.min_key_size = 0u,
		.val_handle = nullptr
	};
}
}	// namespace


namespace ble
{
CGattService::CGattService(std::vector<CGattCharacteristic>& characteristics)
	: m_pUUID{ std::make_unique<ble_uuid128_t>(/* stuff */) }
	, m_Characteristics{}
{
	m_Characteristics.reserve(characteristics.size());
	for(auto&& characteristic : characteristics)
		m_Characteristics.emplace_back(static_cast<ble_gatt_chr_def>(characteristic));
	m_Characteristics.emplace_back(end_of_array());
}
CGattService::CGattService(const CGattService& other)
	: m_pUUID{ nullptr }
	, m_Characteristics{}
{
	*this = copy(other);
}
CGattService& CGattService::operator=(const CGattService& other)
{
	if(this != &other)
	{
		*this = copy(other);
	}

	return *this;
}
CGattService CGattService::copy(const CGattService& source) const
{
	CGattService cpy{};
	cpy.m_pUUID = std::make_unique<ble_uuid128_t>();
	static_assert(std::is_trivially_constructible_v<std::remove_cvref_t<decltype(*cpy.m_pUUID)>>);
	static_assert(sizeof(std::remove_cvref_t<decltype(*source.m_pUUID)>) == sizeof(std::remove_cvref_t<decltype(*cpy.m_pUUID)>));
	std::memcpy(cpy.m_pUUID.get(), source.m_pUUID.get(), sizeof(std::remove_cvref_t<decltype(*cpy.m_pUUID)>));
	cpy.m_Characteristics = source.m_Characteristics;
	return cpy;
}
CGattService::operator ble_gatt_svc_def()
{
	return ble_gatt_svc_def{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = m_pUUID.get(),
		.includes = nullptr, // for now
		.characteristics = m_Characteristics.data()
	};
}
}	// namespace ble