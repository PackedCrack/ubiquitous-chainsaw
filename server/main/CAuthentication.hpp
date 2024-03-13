#pragma once
#include "CGattService.hpp"


namespace ble
{
class CAuthentication
{
public:
	CAuthentication();
	~CAuthentication() = default;
	CAuthentication(const CAuthentication& other) = default;
	CAuthentication(CAuthentication&& other) = default;
	CAuthentication& operator=(const CAuthentication& other) = default;
	CAuthentication& operator=(CAuthentication&& other) = default;
public:

private:
	[[nodiscard]] std::vector<CCharacteristic> make_characteristics() const;
	void register_service();
private:
	std::vector<CCharacteristic> m_Characteristics;
	CGattService m_Service;
};
}	// namespace ble