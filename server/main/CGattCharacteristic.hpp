#pragma once
// std
#include <memory>
// nimnble
#include "host/ble_uuid.h"


namespace ble
{
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



std::unique_ptr<ble_uuid128_t> make_ble_uuid_128(uint16_t uniqueValue)
{
	UUID uuid = BaseUID;
	uuid.custom = uniqueValue;

	auto m_pNimbleUUID = std::make_unique<ble_uuid128_t>();
	m_pNimbleUUID->u = BLE_UUID_TYPE_128;
	static_assert(std::is_trivially_copyable_v<decltype(uuid)>);
	static_assert(ARRAY_SIZE(m_pNumbleUUID->value) == sizeof(decltype(uuid)));
	std::memcpy(&(m_pNumbleUUID->value[0]), &uuid, ARRAY_SIZE(m_pNumbleUUID->value));

	return m_pNimbleUUID;
}


struct CharsCallbackArgs
{
}

template<typename invocable_t>
requires std::invocable<invocable_t, CharsCallbackArgs>
class CGattCharacteristic
{
public:
	template<typename... property_flag_t>
	CGattCharacteristic(uint16_t uuid, invocable_t&& callback, property_flag_t&&... flags)
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
		return static_cast<ble_gatt_chr_def_t>(*this);
	}
private:
	std::unique_ptr<ble_uuid128_t> m_pUUID;
	invokable_t m_Callback;
	CharsCbArgs m_Args;
	CharsPropertyFlag m_Flags;
	uint16_t m_ValueHandle;
};
}	// namespace ble