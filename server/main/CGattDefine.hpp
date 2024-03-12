#pragma once

#include <memory>

// nimnble
#include "host/ble_uuid.h"


namespace ble
{

struct UUID
{
    uint32_t data1 = 0u;
    uint16_t custom = 0u;
    uint16_t data3 = 0u;
    uint16_t data4 = 0u;
    uint16_t data5 = 0u;
    uint16_t data6 = 0u;
    uint16_t data7 = 0u;
};

static constexpr UUID BaseUID
{
        .data1 = 0u,
        .custom = 0u,
        .data3 = 0x1000,
        .data4 = 0x8000,
        .data5 = 0x0080,
        .data6 = 0x5F9B,
        .data7 = 0x34FB
};

static constexpr uint16_t ID_SERVICE_WHOAMI = 0xAAAA;

enum class UUIDType
{
	type128 = BLE_UUID_TYPE_128
}



std::unique_ptr<ble_uuid128_t> make_ble_uuid_128(uint16_t uniqueValue)
{
	UUID uuid = BaseUID;
	uuid.custom = uniqueValue;

	auto m_pNimbleUUID = std::make_unique<ble_uuid128_t>();
	m_pNimbleUUID->u = BLE_UUID_TYPE_128;
	static_assert(std::is_trivially_copyable_v<decltype(uuid)>);
	std::memcpy(&(m_pNumbleUUID->value[0]), &uuid, ARRAY_SIZE(m_pNumbleUUID->value));

	return m_pNimbleUUID;
}







enum class CharsPropertyFlag : uint16_t
{
	broadcast = BLE_GATT_CHR_F_BROADCAST,
	read = BLE_GATT_CHR_F_READ,
	writeNoRespond = BLE_GATT_CHR_F_WRITE_NO_RSP,
	write = BLE_GATT_CHR_F_WRITE,
	notify = BLE_GATT_CHR_F_NOTIFY,
	indicate = BLE_GATT_CHR_F_INDICATE,
	authSignWrite = BLE_GATT_CHR_F_AUTH_SIGN_WRITE,
	reliableWrite = BLE_GATT_CHR_F_RELIABLE_WRITE,
	auxWrite = BLE_GATT_CHR_F_AUX_WRITE,
	readEnc = BLE_GATT_CHR_F_READ_ENC,
	readAuthen = BLE_GATT_CHR_F_READ_AUTHEN,
	readAuthor = BLE_GATT_CHR_F_READ_AUTHOR,
	writeEnc = BLE_GATT_CHR_F_WRITE_ENC,
	writeAuthen = BLE_GATT_CHR_F_WRITE_ENC,
	writeAuthor = BLE_GATT_CHR_F_WRITE_AUTHOR
}
[[nodiscard]] constexpr CharsPropertyFlag operator&(CharsPropertyFlag lhs, CharsPropertyFlag rhs)
{
	return CharsPropertyFlag{ static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs) };
}
[[nodiscard]] constexpr CharsPropertyFlag operator|(CharsPropertyFlag lhs, CharsPropertyFlag rhs)
{
	return CharsPropertyFlag{ static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs) };
}
[[nodiscard]] constexpr CharsPropertyFlag operator^(CharsPropertyFlag lhs, CharsPropertyFlag rhs)
{
	return CharsPropertyFlag{ static_cast<uint16_t>(lhs) ^ static_cast<uint16_t>(rhs) };
}
[[nodiscard]] constexpr CharsPropertyFlag operator~(CharsPropertyFlag value)
{
	return CharsPropertyFlag{ ~static_cast<uint16_t>(value) };
}




struct CharsCbArgs
{
	std::string str;
}
auto my_lambda(){ return [](CharsCbArgs){}; }
auto my_lambda2(){ return [](CharsCbArgs){}; }

template<typename invocable_t, typename... property_flags_t>
requires std::invocable<invocable_t, CharsCbArgs>
class CGattCharacteristic
{
public:
	CGattCharacteristic(uint16_t uuid, invocable_t&& callback, property_flags_t&&... flags)
		: m_pUUID{ make_ble_uuid_128(uuid) }
		, m_Callback{ std::forward<invocable_t>(callback) }
		, m_Args{}
		, m_Flags{}
		, m_ValHandle{}
	{
		m_Flags = (flags | ...);
	}
	explicit operator ble_gatt_chr_def()
	{
		return ble_gett_char_def{
			.uuid = m_pUUID.get(),
			.ble_gatt_access_fn = &m_Callback,
			.arg = static_cast<void*>(&m_Args),
			.descriptors = nullptr,
			.flags = static_cast<ble_gatt_chr_flags>(m_Flags),
			.min_key_size = 0,
			.val_handle = &m_ValueHandle
		};
	}
	[[nodiscard]] ble_gatt_chr_def_t to_nimble_api()
	{

	}
private:
	std::unique_ptr<ble_uuid128_t> m_pUUID;
	invokable_t m_Callback;
	CharsCbArgs m_Args;
	CharsPropertyFlag m_Flags;
	uint16_t m_ValueHandle;
};


CGattCharacteristic chars{ ID_SERVICE_WHOAMI, my_lambda(), CharsPropertyFlag::write, CharsPropertyFlag::notify, CharsPropertyFlag::read };




[[nodiscard]] ble_gatt_chr_def end_of_array()
{
	return ble_gatt_svc_def{
			.type = BLE_GATT_SVC_TYPE_END,
			.uuid = nullptr,
			.includes = nullptr,
			.characteristics = nullptr
		};
}
class CGattService
{
public:
	CGattService(std::vector<CGattCharacteristic>& characteristics)
		: m_pUUID{ std::make_unique<ble_uuid128_t>(/* stuff */) }
		, m_Characteristics{}
	{
		m_Characteristics.reserve(characteristics.size());
		for(auto&& characteristic : characteristics)
			m_Characteristics.emplace_back(static_cast<ble_gatt_chr_def>(characteristic));

		m_Characteristics.emplace_back(end_of_array());
	}
	CGattService(const CGattService& other)
		: m_pUUID{ nullptr }
		, m_Characteristics{}
	{
		*this = copy(other);
	}
	CGattService(CGattService&& other) = default;
	CGattService& operator=(const CGattService& other)
	{
		if(this != &other)
		{
			*this = copy(other);
		}

		return *this;
	}
	CGattService& operator=(CGattService&& other) = default;
	explicit operator ble_gatt_svc_def()
	{
		return ble_gatt_svc_def{
			.type = BLE_GATT_SVC_TYPE_PRIMARY,
			.uuid = m_pUUID.get(),
			.includes = nullptr, // for now
			.characteristics = m_Characteristics.data()
		};
	}
private:
	CGattService() = default;
	[[nodiscard]] CGattService copy(CGattService& source)
	{
		CGattService cpy{};
		cpy.m_pUUID = std::make_unique<ble_uuid128_t>();
		static_assert(std::is_trivially_constructable_v<decltype(*cpy.m_pUUID)>);
		std::memcpy(cpy.m_pUUID.get(), source.m_pUUID.get(), sizeof(decltype(*cpy.m_pUUID)));

		cpy.m_Characteristics = source.m_Characteristics;

		return cpy;
	}
private:
	std::unique_ptr<ble_uuid128_t> m_pUUID;
	std::vector<ble_gatt_chr_def> m_Characteristics;
};



class CAuthentication : public std::enable_shared_from_this<CAuthentication>
{
public:
	CAuthentication(std::string_view macAddress)
	{
		// sign mac
		// maybe encrypt mac
		// write to characteristics
		// launch service
	}

	[[nodiscard]] std::shared_ptr<CAuthentication> make_shared()
	{
		shared_from_this();
	}
private:
	void register_service()
	{
		// ble_register_service(static_cast<ble_gatt_svc_def>(m_Service));

		/*
		result = ble_gatts_count_cfg(m_services.svc.data()); // BLE_HS_EINVAL 
    	if (result != SUCCESS) 
        	return result;

    	return ble_gatts_add_svcs(m_services.svc.data()); // BLE_HS_ENOMEM 
		*/
	}
private:
	CGattService m_Service;
	std::vector<CGattCharacteristic> m_Characteristics;
}

class CRange
{
public:
private:
	CGattService m_Services;
	std::vector<CGattCharacteristic> m_Characteristics;
	std::vector<CGattDescriptor> m_Descriptors;
}
}	// namespace ble