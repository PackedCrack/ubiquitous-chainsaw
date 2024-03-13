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
	void register_service();
private:
	CGattService m_Service;
	std::vector<CCharacteristic> m_Characteristics;
};
}	// namespace ble