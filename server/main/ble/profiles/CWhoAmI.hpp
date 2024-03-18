#pragma once
#include "../../server_common.hpp"
#include "CGattService.hpp"


namespace ble
{
class CWhoAmI
{
public:
	enum class Error
	{
		none,
		outOfHeapMemory,
		invalidResource
	};
public:
	[[nodiscard]] static Result<CWhoAmI, CWhoAmI::Error> make_whoami();
	~CWhoAmI() = default;
	CWhoAmI(const CWhoAmI& other) = default;
	CWhoAmI(CWhoAmI&& other) = default;
	CWhoAmI& operator=(const CWhoAmI& other) = default;
	CWhoAmI& operator=(CWhoAmI&& other) = default;
private:
	CWhoAmI();
private:
	//void register_service();
private:
	std::vector<CCharacteristic> m_Characteristics;
	CGattService m_Service;
};
}	// namespace ble