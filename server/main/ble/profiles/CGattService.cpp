#include "CGattService.hpp"


// should be in a deifine.hpp file somehwere
#define NO_FLAGS 0

namespace
{
template<typename return_t>
requires std::is_same_v<return_t, ble_gatt_chr_def> || std::is_same_v<return_t, ble_gatt_svc_def>
[[nodiscard]] constexpr return_t end_of_array()
{
	if constexpr(std::is_same_v<return_t, ble_gatt_chr_def>)
	{
		return ble_gatt_chr_def {
			.uuid = nullptr,
			.access_cb = nullptr,
			.arg = nullptr,
			.descriptors = nullptr,
			.flags = NO_FLAGS,
			.min_key_size = 0u,
			.val_handle = nullptr
		};
	}
	else
	{
		return ble_gatt_svc_def {
			.type = BLE_GATT_SVC_TYPE_END,
			.uuid = nullptr,
			.includes = nullptr,
			.characteristics = nullptr
		};
	}
}
[[nodiscard]] std::vector<ble_gatt_chr_def> make_nimble_characteristics_arr(std::vector<ble::CCharacteristic>& characteristics)
{
	std::vector<ble_gatt_chr_def> nimbleArr{};
	nimbleArr.reserve(characteristics.size());
	for(auto&& characteristic : characteristics)
		nimbleArr.emplace_back(static_cast<ble_gatt_chr_def>(characteristic));
	nimbleArr.emplace_back(end_of_array<ble_gatt_chr_def>());

	return nimbleArr;
}
}	// namespace


namespace ble
{
CGattService::CGattService(std::vector<CCharacteristic>& characteristics)
	: m_pUUID{ std::make_unique<ble_uuid128_t>(/* stuff */) }
	, m_Characteristics{ make_nimble_characteristics_arr(characteristics) }
	, m_Service{}
{
	m_Service.emplace_back(static_cast<ble_gatt_svc_def>(*this));
	m_Service.emplace_back(end_of_array<ble_gatt_svc_def>());
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
	
	
	using TypeToCopy = std::remove_cvref_t<decltype(*source.m_pUUID)>;
	using DstType = std::remove_cvref_t<decltype(*cpy.m_pUUID)>;
	static_assert(std::is_trivially_copyable_v<TypeToCopy>);
	static_assert(std::is_trivially_constructible_v<DstType>);
	static_assert(sizeof(TypeToCopy) == sizeof(DstType));	
	std::memcpy(cpy.m_pUUID.get(), source.m_pUUID.get(), sizeof(DstType));
	
	
	cpy.m_Characteristics = source.m_Characteristics;
	cpy.m_Service = cpy.make_nimble_svc_arr();

	return cpy;
}
std::vector<ble_gatt_svc_def> CGattService::make_nimble_svc_arr() const
{
	std::vector<ble_gatt_svc_def> arr{};
	arr.emplace_back(static_cast<ble_gatt_svc_def>(*this));
	arr.emplace_back(end_of_array<ble_gatt_svc_def>());

	return arr;
}
CGattService::operator ble_gatt_svc_def() const
{
	return ble_gatt_svc_def{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = &(m_pUUID->u),
		.includes = nullptr, // for now
		.characteristics = m_Characteristics.data()
	};
}
const std::vector<ble_gatt_svc_def>& CGattService::as_nimble_arr() const
{
	return m_Service;
}	
}	// namespace ble