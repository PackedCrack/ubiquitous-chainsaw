- change to .hpp
- no header in cmaketextlist
- no namespace in header
- no indent in namespace
- namespace == static -> tell linker to not link to it
- static -> compiler
- preprocessor #ifdef.. etc
- static funcs are not declared in headers, due to linker


- does nimble_port_run() hijack main thread?

- default constructors, void* will break it
- const member will break move constructor

- RAII PRIO

- Test if nimble_port_init() cannot be done twice

- fatal_fmt use {} for formating


- make fields a free func
- make flags etc

data vs resource.
no need to have calss that only holds data

- back to basics move klaus

-----------------------------

https://mynewt.apache.org/latest/network/ble_setup/ble_sync_cb.html
    /*
    In the NimBLE stack, the reset_cb callback is invoked when a soft reset event occurs. 
    A soft reset is typically triggered by application code or stack internals and does not necessarily indicate a crash. 
    For instance, the reset_cb might be called in response to a call to ble_hs_reset() or when the stack detects a configuration change requiring a reset.

    If the system crashes due to a watchdog timeout, buffer overflow, or other low-level issues, it's unlikely that the reset_cb callback will be called.


    In the case of a hard reset, such as a watchdog reset or a system crash, the behavior can vary. 
    Some systems may automatically restart the application after a hard reset, in which case app_main() 
    would be called again during the startup process. However, in other systems, a hard reset may result in a 
    complete system restart where the firmware reboots without automatically restarting the application.


    TESTED: This callback is not called when buffer overflow occurs! the device will instead reboot and call app_main() again
    TESTED: Not called when pressing the hardware reset button
    */



    ______________________________________________________________
     // docs reference for gap events
    // https://mynewt.apache.org/latest/tutorials/ble/bleprph/bleprph-sections/bleprph-gap-event.html?highlight=ble_gap_event_connect
    // No restrictions on NimBLE operations
    // All context data is transient
    // If advertising results in a connection, the connection inherits this callback as its event-reporting mechanism.

    //struct ble_gap_conn_desc desc;
    //int result;
   
    // static advertise {}
    //static_assert(std::is_trivially_copyable_v<ble_gap_adv_params>::value);
    //static CAdvertiser advertiser {m_params}; // as lvalue or rvalue? its a trivially copiable struct 

/*    CHATGPT
No, you do not need to advertise to find a bonded device in a Bluetooth Low Energy (BLE) context.
Bonding is a process where two devices establish a trusted relationship by exchanging encryption keys 
and storing them for future use. Once two devices have bonded, they can communicate securely without 
needing to perform certain security procedures again. This is useful for scenarios where you want to 
maintain a secure connection between devices over multiple sessions.
When you want to establish a connection with a bonded device, you typically use the device's address or 
identity information stored during the bonding process. Advertising is not necessary because the bonded 
device's address or identity information can be used to directly initiate a connection attempt.
In summary, bonding establishes a trusted relationship between devices, and once bonded, you can use the
 stored information to connect without needing to advertise. However, advertising is still relevant for 
 discovering and connecting to new devices that are not yet bonded.
*/

/*
TESTING:

ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params, gap_event_handler, NULL);

This will start advertising. When a client connects, it does not end advertising.
The one that triggered the callback, will inherit the callback aswell.
A new advertise procedure will be initiated (see below): (a different thread i assume)
I (1068) NimBLE: GAP procedure initiated: advertise; 
I (1068) NimBLE: disc_mode=2
I (1068) NimBLE:  adv_channel_map=0 own_addr_type=1 adv_filter_policy=0 adv_itvl_min=0 adv_itvl_max=0
I (1068) NimBLE:

TESTING:
When the client disconnects, the procedure gets removed. (i hope), and advertising on the other threads will continue

TESTING:
If when a client disconnects and i call ble_gap_adv_stop(); the following is shown:
I (42788) NimBLE: GAP procedure initiated: stop advertising.
The advertising will continue anyway.

TESTING:
if when a client connects and i call ble_gap_adv_stop();
The advertising will still continue

TESTING:
Calling start advertise, if already advertising, the following error code is returned:
BLE_HS_EALREADY 0x02 Operation already in progress or completed.

https://github.com/espressif/esp-idf/issues/4243
t he Controller shall continue advertising until the Host issues an HCI_LE_Set_Advertising_Enable command with 
Advertising_Enable set to 0x00 (Advertising is disabled) or until a connection is created or until the Advertising 
is timed out due to high duty cycle Directed Advertising. 

TESTING:
if i set the max number of connections to 1 in sdk configs
starting advertising after a connection will result in error code:
BLE_HS_ENOMEM 0x06 Operation failed due to resource exhaustion.

*/