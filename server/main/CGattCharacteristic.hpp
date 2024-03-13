#pragma once
#include "ble_common.hpp"
// std
#include <memory>


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
};
[[nodiscard]] constexpr CharsPropertyFlag operator&(CharsPropertyFlag lhs, CharsPropertyFlag rhs)
{
	return CharsPropertyFlag{ static_cast<uint16_t>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs)) };
}
[[nodiscard]] constexpr CharsPropertyFlag operator|(CharsPropertyFlag lhs, CharsPropertyFlag rhs)
{
	return CharsPropertyFlag{ static_cast<uint16_t>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs)) };
}
[[nodiscard]] constexpr CharsPropertyFlag operator^(CharsPropertyFlag lhs, CharsPropertyFlag rhs)
{
	return CharsPropertyFlag{ static_cast<uint16_t>(static_cast<uint16_t>(lhs) ^ static_cast<uint16_t>(rhs)) };
}
[[nodiscard]] constexpr CharsPropertyFlag operator~(CharsPropertyFlag value)
{
	return CharsPropertyFlag{ static_cast<uint16_t>(~static_cast<uint16_t>(value)) };
}


struct CharsCallbackArgs
{
};

template<typename invocable_t>
requires std::is_invocable_r_v<int32_t, invocable_t, uint16_t, uint16_t, ble_gatt_access_ctxt*, void*>
class CGattCharacteristic
{
public:
	template<typename... property_flag_t>
	CGattCharacteristic(uint16_t uuid, invocable_t&& callback, property_flag_t&&... flags)
		: m_UUID{ make_ble_uuid128(uuid) }
		, m_Callback{ std::forward<invocable_t>(callback) }
		, m_Args{}
		, m_Flags{}
		, m_ValueHandle{}
	{
		m_Flags = (flags | ...);
	}
	explicit operator ble_gatt_chr_def()
	{
		return ble_gatt_chr_def{
			.uuid = &(m_UUID.u),
			.access_cb = m_Callback,
			.arg = static_cast<void*>(&m_Args),
			.descriptors = nullptr,
			.flags = static_cast<ble_gatt_chr_flags>(m_Flags),
			.min_key_size = 0,
			.val_handle = &m_ValueHandle
		};
	}
	[[nodiscard]] ble_gatt_chr_def to_nimble_t()
	{
		return static_cast<ble_gatt_chr_def>(*this);
	}
private:
	//std::unique_ptr<ble_uuid128_t> m_pUUID;
	ble_uuid128_t m_UUID;
	invocable_t m_Callback;
	CharsCallbackArgs m_Args;	// needed at all?
	CharsPropertyFlag m_Flags;
	uint16_t m_ValueHandle;
};


template<typename characteristic_t>
concept GattCharacteristicDefine = requires(characteristic_t characteristic)
{
	{ characteristic.to_nimble_t() } -> std::convertible_to<ble_gatt_chr_def>;
	{ static_cast<ble_gatt_chr_def>(characteristic) } -> std::convertible_to<ble_gatt_chr_def>;
};

class CCharacteristic
{
	// todo: friend functions
class Concept
{
public:
	virtual ~Concept() = default;
public:
	virtual ble_gatt_chr_def exec_to_nimble_t() = 0;
protected:
	Concept() = default;
	Concept(const Concept& other) = default;
	Concept(Concept&& other) = default;
	Concept& operator=(const Concept& other) = default;
	Concept& operator=(Concept&& other) = default;
};
template<typename characteristic_t>
requires GattCharacteristicDefine<characteristic_t>
class Model : public Concept
{
public:
	Model(characteristic_t&& characteristic)
		: m_Characteristic{ std::forward<characteristic_t>(characteristic) }
	{};
	~Model() override = default;
	Model(const Model& other) = default;
	Model(Model&& other) = default;
	Model& operator=(const Model& other) = default;
	Model& operator=(Model&& other) = default;
public:
	[[nodiscard]] ble_gatt_chr_def exec_to_nimble_t() override
	{
		return m_Characteristic.to_nimble_t();
	}
private:
	characteristic_t m_Characteristic;
};
public:
	template<typename characteristic_t>
	explicit CCharacteristic(characteristic_t&& characteristic) requires GattCharacteristicDefine<characteristic_t>
		: m_pCharacteristic{ std::make_unique<Model<characteristic_t>>(std::forward<characteristic_t>(characteristic)) }
	{}
	~CCharacteristic() = default;
	CCharacteristic(const CCharacteristic& other) = default;
	CCharacteristic(CCharacteristic&& other) = default;
	CCharacteristic& operator=(const CCharacteristic& other) = default;
	CCharacteristic& operator=(CCharacteristic&& other) = default;
public:
	[[nodiscard]] ble_gatt_chr_def to_nimble_t()
	{
		return m_pCharacteristic->exec_to_nimble_t();
	}
	explicit operator ble_gatt_chr_def()
	{
		return m_pCharacteristic->exec_to_nimble_t();
	}
private:
	std::unique_ptr<Concept> m_pCharacteristic;
};

template<typename... ctor_args_t>
[[nodiscard]] CCharacteristic make_characteristic(ctor_args_t... args)
{
	return CCharacteristic{ CGattCharacteristic{ std::forward<ctor_args_t>(args)... }};
}
}	// namespace ble