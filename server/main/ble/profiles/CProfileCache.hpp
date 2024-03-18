#pragma once
#include "../../server_common.hpp"
// std
#include <memory>
#include <vector>
#include <map>
#include <concepts>
// esp
#include "host/ble_gatt.h"


namespace ble
{
template<typename profile_t>
concept NimbleProfile = requires(profile_t profile)
{
	{ profile.as_nimble_service() } -> std::convertible_to<ble_gatt_svc_def>;
};
class CProfile
{
	class Concept
	{
	public:
		virtual ~Concept() = default;
	protected:
		Concept() = default;
		Concept(const Concept& other) = default;
		Concept(Concept&& other) = default;
		Concept& operator=(const Concept& other) = default;
		Concept& operator=(Concept&& other) = default;
	public:
		virtual std::unique_ptr<Concept> copy() const = 0;
		virtual ble_gatt_svc_def exec_nimble_service() const = 0;
	};
	template<typename profile_t>
	requires NimbleProfile<profile_t>
	class Model final : public Concept
	{
	public:
		Model(profile_t&& profile)
			: m_Profile{ std::forward<profile_t>(profile) }
		{};
		~Model() override = default;
		Model(const Model& other) = default;
		Model(Model&& other) = default;
		Model& operator=(const Model& other) = default;
		Model& operator=(Model&& other) = default;
		[[nodiscard]] std::unique_ptr<Concept> copy() const override { return std::make_unique<Model>(*this); }
	public:
	[[nodiscard]] ble_gatt_svc_def exec_nimble_service() const override
	{
		return m_Profile.as_nimble_service();
	}
	private:
		profile_t m_Profile;
	};
public:
	template<typename profile_t>
	requires NimbleProfile<profile_t>
	explicit CProfile(profile_t&& profile)
		: m_pProfile{ std::make_unique<Model<profile_t>>( std::forward<profile_t>(profile)) }
	{}
	~CProfile() = default;
	CProfile(const CProfile& other)
		: m_pProfile{ other.m_pProfile->copy() }
	{};
	CProfile(CProfile&& other) = default;
	CProfile& operator=(const CProfile& other)
	{
		if(this != &other)
			m_pProfile = other.m_pProfile->copy();

		return *this;
	};
	CProfile& operator=(CProfile&& other) = default;
public:
	[[nodiscard]] ble_gatt_svc_def nimble_service() const
	{
		return m_pProfile->exec_nimble_service();
	}
private:
	std::unique_ptr<Concept> m_pProfile = nullptr;
};

template<typename profile_t, typename... ctor_args_t>
[[nodiscard]] CProfile make_profile(ctor_args_t&&... args)
{
	return CProfile{ profile_t{ std::forward(args)... } };
}

class CProfileCache
{
	using KeyType = std::string_view;
public:
	enum class Error
	{
		none,
		outOfHeapMemory,
		invalidResource
	};
	static constexpr std::string_view KEY_WHOAMI = "whoami";
	static constexpr std::string_view KEY_RANGE = "range";
public:
	[[nodiscard]] static Result<CProfileCache, CProfileCache::Error> make_profile_cache(std::map<KeyType, CProfile>&& profiles);
	~CProfileCache() = default;
	CProfileCache(const CProfileCache& other) = default;
	CProfileCache(CProfileCache&& other) = default;
	CProfileCache& operator=(const CProfileCache& other) = default;
	CProfileCache& operator=(CProfileCache&& other) = default;
private:
	CProfileCache(std::map<KeyType, CProfile>&& profiles);
private:
	std::map<KeyType, CProfile> m_Profiles;
	std::vector<ble_gatt_svc_def> m_Services;
};
}	// namespace ble