#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"

#include "sys/CNonVolatileStorage.hpp"

#include "esp_system.h"



void print_chip_info()
{
	sys::CSystem system{};
	sys::CChip chip = system.chip_info();

	std::printf("\nChip information");
	std::printf("\nRevision: %s", chip.revision().c_str());
	std::printf("\nNumber of Cores: %i", chip.cores());
	std::printf("\nFlash Memory: %s", chip.embedded_psram() ? "Embedded" : "External");
	std::printf("\nPSRAM: %s", chip.embedded_flash_memory() ? "Embedded" : "External");
	std::printf("\nSupports Wifi 2.4ghz: %s", chip.wifi() ? "True" : "False");
	std::printf("\nSupports Bluetooth LE: %s", chip.bluetooth_le() ? "True" : "False");
	std::printf("\nSupports Bluetooth Classic: %s", chip.bluetooth_classic() ? "True" : "False");
	std::printf("\nSupports IEEE 802.15.4: %s", chip.IEEE_802_15_4() ? "True" : "False");
	std::printf("\nCurrent minimum free heap: %lu bytes\n\n", system.min_free_heap());
}


esp_err_t print_what_saved(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open("STORAGE", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Read restart counter
    int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
    err = nvs_get_i32(my_handle, "restart_conter", &restart_counter);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    printf("Restart counter = %" PRIu32 "\n", restart_counter);

	// READ BINARY DATA
	
	// Read binary data from NVS
	size_t required_size = 0;
	err = nvs_get_blob(my_handle, "BinaryData", nullptr, &required_size);
	if (err == ESP_OK) {
		std::vector<uint8_t> retrieved_data(required_size);
		err = nvs_get_blob(my_handle, "BinaryData", retrieved_data.data(), &required_size);
		if (err == ESP_OK) {
			printf("Retrieved Binary Data: ");
        	for (const auto& byte : retrieved_data) 
			{
            	printf("%02X ", byte); // Print each byte in hexadecimal format
        	}
        	printf("\n");
			// Binary data retrieved successfully
			// Use retrieved_data as needed
		} else {
			// Handle error
			nvs_close(my_handle);
			return ESP_FAIL;
		}
	} else if (err == ESP_ERR_NVS_NOT_FOUND) {
		// Key not found in NVS
	} else {
		// Handle error
		nvs_close(my_handle);
		return ESP_FAIL;
	}


	// READ STRING
//	// Read the string value
//	size_t required_size = 0;
//	err = nvs_get_str(my_handle, "TestingString", NULL, &required_size);
//	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
//	    nvs_close(my_handle);
//	    return err;
//	}
//	
//	// Allocate memory to store the string
//	char* value = (char*)malloc(required_size);
//	if (value == NULL) {
//	    nvs_close(my_handle);
//	    return ESP_ERR_NO_MEM; // Failed to allocate memory
//	}
//
//// Read the string from NVS
//err = nvs_get_str(my_handle, "TestingString", value, &required_size);
//if (err != ESP_OK) {
//    free(value);
//    nvs_close(my_handle);
//    return err;
//}




    //// Read run time blob
    //size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    //// obtain required memory space to store blob being read from NVS
    //err = nvs_get_blob(my_handle, "run_time", NULL, &required_size);
    //if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Run time:\n");
    //if (required_size == 0) {
    //    printf("Nothing saved yet!\n");
    //} else {
    //    uint32_t* run_time = static_cast<uint32_t*>(malloc(required_size));
    //    err = nvs_get_blob(my_handle, "run_time", run_time, &required_size);
    //    if (err != ESP_OK) {
    //        free(run_time);
    //        return err;
    //    }
    //    for (int i = 0; i < required_size / sizeof(uint32_t); i++) {
    //        printf("%d: %" PRIu32 "\n", i + 1, run_time[i]);
    //    }
    //    free(run_time);
    //}


    // Close
    nvs_close(my_handle);
	
	// Print the string value
	//printf("TestingString: %s\n", value);
	//free(value);
	
	return ESP_OK;
}


esp_err_t save_restart_counter(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open("STORAGE", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Read
    int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
    err = nvs_get_i32(my_handle, "restart_conter", &restart_counter);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Write
    restart_counter++;
    err = nvs_set_i32(my_handle, "restart_conter", restart_counter);
    if (err != ESP_OK) return err;

    // Commit written value.
    // After setting any values, nvs_commit() must be called to ensure changes are written
    // to flash storage. Implementations may write to storage at other times,
    // but this is not guaranteed.
    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(my_handle);
    return ESP_OK;
}


bool vectors_is_equal(const std::vector<uint8_t>& v1, const std::vector<uint8_t>& v2) 
{
    if (v1.size() != v2.size()) 
	{
        return false; // Vectors are of different sizes, so they can't be equal
    }

    for (size_t i = 0; i < v1.size(); ++i) 
	{
        if (v1[i] != v2[i]) 
		{
            return false; // Elements at index i are different, so vectors are not equal
        }
    }

    return true; // All elements are equal, so vectors are equal
}

esp_err_t save_run_time(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

	// SAVE STRING
	const char* new_value = "testing";
	// OPEN STORAGE
	err = nvs_open("STORAGE", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) return err;

	// Set the string value in NVS
	err = nvs_set_str(my_handle, "TestingString", new_value);
	if (err != ESP_OK) {
		nvs_close(my_handle);
		return err;
	}


	// SAVE BINARY DATA
	std::vector<uint8_t> binaryData = { 0xBA, 0xBE, 0xBA, 0xBe, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE };

	size_t existingSize = 0;
	err = nvs_get_blob(my_handle, "BinaryData", NULL, &existingSize);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) 
	{
	    nvs_close(my_handle);
	    return err;
	}

	// If the key exists and has a non-zero size, read the existing blob
	if (existingSize > 0) 
	{
		// Allocate memory to store the existing blob
		std::vector<uint8_t> existingBinaryData(existingSize);
		err = nvs_get_blob(my_handle, "BinaryData", existingBinaryData.data(), &existingSize);
		if (err != ESP_OK) 
		{
			nvs_close(my_handle);
			return err;
		}

		bool isEqual = vectors_is_equal(binaryData, existingBinaryData);
		if (isEqual)
		{
			std::printf("vectors are equal no need to add again!\n");
			nvs_close(my_handle);
			return err;
		}
	}

	// Write binary data to NVS
	err = nvs_set_blob(my_handle, "BinaryData", binaryData.data(), binaryData.size() * sizeof(uint8_t));
	if (err != ESP_OK) {
		// Handle error
		nvs_close(my_handle);
		return err;
	}


	// SAVE BLOB
   //// Read the size of memory space required for blob
   //size_t required_size = 0;  // value will default to 0, if not set yet in NVS
   //err = nvs_get_blob(my_handle, "run_time", NULL, &required_size);
   //if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
//
   //// Read previously saved blob if available
   //uint32_t* run_time = static_cast<uint32_t*>(malloc(required_size + sizeof(uint32_t)));
   //if (required_size > 0) {
   //    err = nvs_get_blob(my_handle, "run_time", run_time, &required_size);
   //    if (err != ESP_OK) {
   //        free(run_time);
   //        return err;
   //    }
   //}
   //// Write value including previously saved blob if available
   //required_size += sizeof(uint32_t);
   //run_time[required_size / sizeof(uint32_t) - 1] = xTaskGetTickCount() * portTICK_PERIOD_MS;
   //err = nvs_set_blob(my_handle, "run_time", run_time, required_size);
   //free(run_time);
//
   //if (err != ESP_OK) return err;
//
   //// Commit
   //err = nvs_commit(my_handle);
   //if (err != ESP_OK) return err;
//


	err = nvs_commit(my_handle);
	if (err != ESP_OK) return err;


	// Close
	nvs_close(my_handle);
	return ESP_OK;
}


extern "C" void app_main(void)
{
	sys::CSystem system{};
	try 
	{
		print_chip_info();


		storage::CNonVolatileStorage nvs{};
		std::optional<storage::CNonVolatileStorage::CReader> optReader = nvs.make_reader("STORAGE");
		if (!optReader.has_value())
		{
			LOG_FATAL("CReader creation error");
		}


		// TODO CHECK IF OPEN NEEDS TO BE CALLED WHITIN SAME SCOPE


		//storage::CNonVolatileStorage::CReader reader = std::move(optReader.value());

		storage::CNonVolatileStorage::ReadBinaryResult tmp = optReader.value().read_binary("BinaryData");
		if (!tmp.data.has_value())
		{
			std::printf("Error %d\n", static_cast<esp_err_t>(tmp.code)); // invalidHandle ???
			LOG_ERROR("Error reading data");
		}
	//
		std::vector<uint8_t> data = tmp.data.value();
		std::printf("Data size=%d\n", data.size());
		for (uint8_t byte : data) 
		{
        	std::printf("%02X ", byte);
    	}

		//if (!result.has_value()) 
		//{
		//	LOG_ERROR("Failed to read binary data from NVS");
		//} 
//
		//ReadBinaryResult data = result.value();



		//std::optional<storage::CNonVolatileStorage::CReadWriter> writer = nvs.make_read_writer("STORAGE");
		//if (!writer.has_value())
		//{
		//	LOG_FATAL("Writer creation error");
		//}


		esp_err_t err = print_what_saved();
    	if (err != ESP_OK) 
			std::printf("Error (%d) reading data from NVS!\n", err);
		
		//err = save_restart_counter();
    	//if (err != ESP_OK) 
		//	std::printf("Error (%d) saving restart counter to NVS!\n", err);
//
		//err = save_run_time();
        //if (err != ESP_OK) 
		//	std::printf("Error (%d) saving run time blob to NVS!\n", err);
//
//
//
		//std::printf("Total entries: %d\n", stats.total_entries); // This represents the total number of key-value pairs that can be stored in the NVS partition
		//std::printf("Used entries: %d\n", stats.used_entries); //: This indicates the number of key-value pairs that are currently stored in the NVS partition.
		//std::printf("Free entries: %d\n", stats.free_entries); // This represents the number of available slots for storing additional key-value pairs in the NVS partition. 
		//std::printf("Namespace entries: %d\n", stats.namespace_count);
//
//
//
//


		//// CHECK SIZE OF PARTITION
		// esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA,
        //                                              ESP_PARTITION_SUBTYPE_DATA_NVS,
        //                                              NULL);
		//if (it == NULL) 
		//{
		//	std::printf("failed to find the default partition\n");
    	//}	
//
		//const esp_partition_t* partition = esp_partition_get(it);
    	//esp_partition_iterator_release(it);
//
    	//if (partition == NULL) {
    	//    // Failed to get partition information
		//	std::printf("Failed to get partition information\n");
    	//}
//
		//LOG_INFO_FMT("Partition size = {}",  partition->size);

		// esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key)
		// esp_err_t nvs_flash_erase(void) // Erase the default NVS partition.
		// esp_err_t nvs_erase_all(nvs_handle_t handle) // Erase all key-value pairs in a namespace.
		// esp_err_t nvs_get_stats(const char *part_name, nvs_stats_t *nvs_stats)
		// esp_err_t nvs_get_used_entry_count(nvs_handle_t handle, size_t *used_entries)
		// esp_err_t nvs_get_stats(const char *part_name, nvs_stats_t *nvs_stats)


		// https://mynewt.apache.org/latest/

		//{
        //ble::CNimble tmp {};
		//}
		//ble::CNimble nimble {};
		//ble::CNimble nimble {std::move(tmp)};
		//ble::CNimble nimble = std::move(tmp);


		while (true)
		{
			//chainsawServer.rssi();
			// Perform any periodic tasks here
			vTaskDelay(pdMS_TO_TICKS(3000)); // milisecs
		}
    } 
	catch (const exception::fatal_error& error) 
	{
		LOG_ERROR_FMT("Caught exception: {}", error.what());
		system.restart();
    }
}
