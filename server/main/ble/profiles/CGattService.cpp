#include "CGattService.hpp"


// should be in a deifine.hpp file somehwere
#define NO_FLAGS 0

namespace
{
[[nodiscard]] constexpr ble_gatt_chr_def end_of_array()
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
[[nodiscard]] std::vector<ble_gatt_chr_def> make_nimble_characteristics_arr(std::vector<ble::CCharacteristic>& characteristics)
{
	std::vector<ble_gatt_chr_def> nimbleArr{};
	nimbleArr.reserve(characteristics.size());
	for(auto&& characteristic : characteristics)
		nimbleArr.emplace_back(static_cast<ble_gatt_chr_def>(characteristic));
	nimbleArr.emplace_back(end_of_array());

	return nimbleArr;
}
}	// namespace


namespace ble
{
CGattService::CGattService(uint16_t uuid, std::vector<CCharacteristic>& characteristics)
	: m_pUUID{ std::make_unique<ble_uuid128_t>(make_ble_uuid128(uuid)) }
	, m_Characteristics{ make_nimble_characteristics_arr(characteristics) }
{}
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

	return cpy;
}
CGattService::operator ble_gatt_svc_def() const
{
	//LOG_WARN_FMT("Service pointers: uuid - {:p}. characteristic array - {:p}",
	//					(void*)&(m_pUUID->u), (void*)m_Characteristics.data());
	return ble_gatt_svc_def{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = &(m_pUUID->u),
		.includes = nullptr, // for now
		.characteristics = m_Characteristics.data()
	};
}
}	// namespace ble