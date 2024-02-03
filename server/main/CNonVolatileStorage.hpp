#pragma once


namespace storage
{
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#introduction
class CNonVolatileStorage
{
public:
	CNonVolatileStorage();
	~CNonVolatileStorage();
	CNonVolatileStorage(const CNonVolatileStorage& other) = delete;	// Deleted for now..
	CNonVolatileStorage(CNonVolatileStorage&& other) = delete;
	CNonVolatileStorage& operator=(const CNonVolatileStorage& other) = delete;
	CNonVolatileStorage& operator=(CNonVolatileStorage&& other) = delete;

	[[nodiscard]] static CNonVolatileStorage& instance();
	// TODO:: read/write
private:
};
}	// namespace storage