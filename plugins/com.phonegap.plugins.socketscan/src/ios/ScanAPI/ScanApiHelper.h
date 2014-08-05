//
//  ScanApiHelper.h
//  ScannerSettings
//
//  Created by Eric Glaenzer on 7/29/11.
//  Copyright 2011 Socket Mobile, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ScanApiIncludes.h"

#import "DeviceInfo.h"

@protocol ISktScanObject;
@protocol ISktScanDevice;
@protocol ISktScanDecodedData;
@protocol ISktScanApi;

#define CMD_MAX_RETRY 3 // maximum retry for a command

/**
 * CommandContextDelegate
 * Each time a property command has completed,
 * this optional delegate will be called so
 * the ScanApiHelper user can either refresh the UI
 * with a property response and/or ask for another
 * property
 */
@protocol CommandContextDelegate <NSObject> 
-(void)run:(id<ISktScanObject>)scanObj;
@end

@interface CommandContext : NSObject{
    id _target;
    SEL _response;
    enum eStatus{
        statusReady=1,
        statusNotCompleted,
        statusCompleted
    }status;
    BOOL _getOperation;
    id<ISktScanObject>_scanObj;
    id<ISktScanDevice>_device;
    DeviceInfo* _deviceInfo;
    int _symbologyId;
    int retry;
}

@property (nonatomic,readwrite)int retry;
@property (nonatomic,readwrite)enum eStatus status;

-(id)initWithParam:(BOOL)getOperation 
           ScanObj:(id<ISktScanObject>)scanObj 
        ScanDevice:(id<ISktScanDevice>)scanDevice 
            Device:(DeviceInfo*)device 
            Target:(id)target
          Response:(SEL)response;
-(void)dealloc;

-(id<ISktScanDevice>)getScanDevice;
-(id<ISktScanObject>)getScanObject;
-(void)doCallback:(id<ISktScanObject>)scanObj;
-(SKTRESULT)doCommand;

@end

@protocol ScanApiHelperDelegate <NSObject>
/**
 * called each time a device connects to the host
 * @param result contains the result of the connection
 * @param newDevice contains the device information
 */
-(void)onDeviceArrival:(SKTRESULT)result Device:(DeviceInfo*)deviceInfo;

/**
 * called each time a device disconnect from the host
 * @param deviceRemoved contains the device information
 */
-(void) onDeviceRemoval:(DeviceInfo*) deviceRemoved;

/**
 * called each time ScanAPI is reporting an error
 * @param result contains the error code
 */
-(void) onError:(SKTRESULT) result;

/**
 * called each time ScanAPI receives decoded data from scanner
 * @param deviceInfo contains the device information from which
 * the data has been decoded
 * @param decodedData contains the decoded data information
 */
-(void) onDecodedData:(DeviceInfo*) device DecodedData:(id<ISktScanDecodedData>) decodedData;

/**
 * called when ScanAPI initialization has been completed
 * @param result contains the initialization result
 */
-(void) onScanApiInitializeComplete:(SKTRESULT) result;

/**
 * called when ScanAPI has been terminated. This will be
 * the last message received from ScanAPI
 */
-(void) onScanApiTerminated;

/**
 * called when an error occurs during the retrieval
 * of a ScanObject from ScanAPI.
 * @param result contains the retrieval error code
 */
-(void) onErrorRetrievingScanObject:(SKTRESULT) result;

@end


/**
 * this class provides a set of common functions to retrieve
 * or configure a scanner or ScanAPI and to receive decoded
 * data from a scanner.<p>
 * This helper manages a commands list so the application
 * can send multiple command in a row, the helper will send
 * them one at a time. Each command has an optional callback 
 * function that will be called each time a command complete.
 * By example, to get a device friendly name, use the 
 * PostGetFriendlyName method and pass a callback function in 
 * which you can update the UI with the newly fetched friendly 
 * name. This operation will be completely asynchronous.<p>
 * ScanAPI Helper manages a list of device information. Most of 
 * the time only one device is connected to the host. This list
 * could be configured to have always one item, that will be a 
 * "No device connected" item in the case where there is no device
 * connected, or simply a device name when there is one device
 * connected. Use isDeviceConnected method to know if there is at
 * least one device connected to the host.<br> 
 * Common usage scenario of ScanAPIHelper:<br>
 * <li> create an instance of ScanApiHelper: _scanApi=new ScanApiHelper();
 * <li> [optional] if a UI device list is used a no device connected 
 * string can be specified:_scanApi.setNoDeviceText(getString(R.string.no_device_connected));
 * <li> register for notification: _scanApi.setNotification(_scanApiNotification);
 * <li> derive from ScanApiHelperNotification to handle the notifications coming
 * from ScanAPI including "Device Arrival", "Device Removal", "Decoded Data" etc...
 * <li> open ScanAPI to start using it:_scanApi.open();
 * <li> check the ScanAPI initialization result in the notifications: 
 * _scanApiNotification.onScanApiInitializeComplete(long result){}
 * <li> monitor a scanner connection by using the notifications:
 * _scanApiNotification.onDeviceArrival(long result,DeviceInfo newDevice){}
 * _scanApiNotification.onDeviceRemoval(DeviceInfo deviceRemoved){}
 * <li> retrieve the decoded data from a scanner
 * _scanApiNotification.onDecodedData(DeviceInfo device,ISktScanDecodedData decodedData){}
 * <li> once the application is done using ScanAPI, close it using:
 * _scanApi.close();
 * @author ericg
 *
 */
@interface ScanApiHelper : NSObject {
    id<ScanApiHelperDelegate> _delegate;
    NSString* _noDeviceText;// text to display when no device is connected
    NSMutableDictionary* _deviceInfoList;// list of device info
    BOOL _scanApiOpen;
    BOOL _scanApiTerminated;
    NSMutableArray* _commandContexts;
    id<ISktScanApi>_scanApi;
    id<ISktScanObject>_scanObjectReceived;
    NSObject* _commandContextsLock;
}


/**
 * register for notifications in order to receive notifications such as
 * "Device Arrival", "Device Removal", "Decoded Data"...etc...
 * @param delegate
 */
-(void)setDelegate:(id<ScanApiHelperDelegate>)delegate;

/**
 * specifying a name to display when no device is connected
 * will add a no device connected item in the list with 
 * the name specified, otherwise if there is no device connected
 * the list will be empty.
 */
-(void)setNoDeviceText:(NSString*) noDeviceText;

/**
 * get the list of devices. If there is no device
 * connected and a text has been specified for
 * when there is no device then the list will
 * contain one item which is the no device in the 
 * list
 * @return
 */
-(NSDictionary*) getDevicesList;

/**
 * check if there is a device connected
 * @return
 */
-(BOOL) isDeviceConnected;

/**
 * flag to know if ScanAPI is open
 * @return
 */
-(BOOL)isScanApiOpen;


/**
 * open ScanAPI and initialize ScanAPI
 * The result of opening ScanAPI is returned in the callback
 * onScanApiInitializeComplete
 */
-(void)open;

/**
 * close ScanAPI. The callback onScanApiTerminated
 * is invoked as soon as ScanAPI is completely closed.
 * If a device is connected, a device removal will be received
 * during the process of closing ScanAPI.
 */
-(void)close;

/**
 * Check and consume asynchronous event from ScanAPI.
 * Typically this message could be called from a timer
 * handler
 */
-(SKTRESULT)doScanApiReceive;

/**
 * remove the pending commands for a specific device
 * or all the pending commands if null is passed as
 * iDevice parameter
 * @param iDevice reference to the device for which
 * the commands must be removed from the list or <b>null</b>
 * if all the commands must be removed.
 */
-(void)removeCommand:(DeviceInfo*)deviceInfo;


/**
 * postGetScanAPIVersion
 * retrieve the ScanAPI Version
 */
-(void)postGetScanApiVersion:(id)target Response:(SEL)response;


/**
 * postSetConfirmationMode
 * Configures ScanAPI so that scanned data must be confirmed by this application before the
 * scanner can be triggered again.
 */
-(void)postSetConfirmationMode:(unsigned char)mode Target:(id)target Response:(SEL)response;


/**
 * postScanApiAbort
 * 
 * Request ScanAPI to shutdown. If there is some devices connected
 * we will receive Remove event for each of them, and once all the
 * outstanding devices are closed, then ScanAPI will send a 
 * Terminate event upon which we can close this application.
 * If the ScanAPI Abort command failed, then the callback will
 * close ScanAPI
 */
-(void)postScanApiAbort:(id)target Response:(SEL)response;

/**
 * postSetDataConfirmation
 * acknowledge the decoded data<p>
 * This is only required if the scanner Confirmation Mode is set to kSktScanDataConfirmationModeApp
 */
-(void)postSetDataConfirmation:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;

/**
 * postGetBtAddress
 * Creates a SktScanObject and initializes it to perform a request for the
 * Bluetooth address in the scanner.
 */
-(void)postGetBtAddress:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;

/**
 * postGetDeviceType
 * Creates a SktScanObject and initializes it to perform a request for the
 * device type of the scanner.
 */
-(void)postGetDeviceType:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;

/**
 * postGetFirmwareVersion
 * Creates a SktScanObject and initializes it to perform a request for the
 * firmware revision in the scanner.
 */
-(void)postGetFirmwareVersion:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;


/**
 * postGetBattery
 * Creates a SktScanObject and initializes it to perform a request for the
 * battery level in the scanner.
 */
-(void)postGetBattery:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;

/**
 * postGetDecodeAction
 * 
 * Creates a TSktScanObject and initializes it to perform a request for the
 * Decode Action in the scanner.
 * 
 */
-(void)postGetDecodeAction:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;

/**
 * postGetCapabilitiesDevice
 * 
 * Creates a SktScanObject and initializes it to perform a request for the
 * Capabilities Device in the scanner.
 */
-(void)postGetCapabilitiesDevice:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;

/**
 * postGetPostambleDevice
 * 
 * Creates a SktScanObject and initializes it to perform a request for the
 * Postamble Device in the scanner.
 * 
 */
-(void)postGetPostambleDevice:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;


/**
 * postSetPostamble
 * 
 * Configure the postamble of the device
 * @param deviceInfo
 * @param postamble
 */
-(void)postSetPostambleDevice:(DeviceInfo*)deviceInfo Postamble:(NSString*)postamble Target:(id)target Response:(SEL)response;

/**
 * postGetSymbologyInfo
 * 
 * Creates a SktScanObject and initializes it to perform a request for the
 * Symbology Info in the scanner.
 * 
 */
-(void)postGetSymbologyInfo:(DeviceInfo*)deviceInfo SymbologyId:(int)symbologyId Target:(id)target Response:(SEL)response;

/**
 * postSetSymbologyInfo
 * Constructs a request object for setting the Symbology Info in the scanner
 * 
 */
-(void)postSetSymbologyInfo:(DeviceInfo*)deviceInfo SymbologyId:(int)symbologyId Status:(BOOL)status Target:(id)target Response:(SEL)response;


/**
 * postGetFriendlyName
 * 
 * Creates a SktScanObject and initializes it to perform a request for the
 * friendly name in the scanner.
 * 
 */
-(void)postGetFriendlyName:(DeviceInfo*)deviceInfo Target:(id)target Response:(SEL)response;

/** 
 * postSetFriendlyName
 * Constructs a request object for setting the Friendly Name in the scanner
 * 
 */
-(void)postSetFriendlyName:(DeviceInfo*)deviceInfo FriendlyName:(NSString*)friendlyName Target:(id)target Response:(SEL)response;

/**
 * postSetDecodeAction
 * 
 * Configure the local decode action of the device
 * 
 * @param deviceInfo
 * @param decodeAction
 */
-(void)postSetDecodeAction:(DeviceInfo*)deviceInfo DecodeAction:(int)decodeAction Target:(id)target Response:(SEL)response;


-(void)addCommand:(CommandContext*)command;
-(void)initializeScanAPIThread:(id)arg;
-(BOOL)handleScanObject:(id<ISktScanObject>)scanObj;
-(SKTRESULT)handleDeviceArrival:(id<ISktScanObject>)scanObj;
-(SKTRESULT)handleDeviceRemoval:(id<ISktScanObject>)scanObj;

-(SKTRESULT)handleSetOrGetComplete:(id<ISktScanObject>)scanObj;
-(SKTRESULT)handleEvent:(id<ISktScanObject>)scanObj;
-(SKTRESULT)handleDecodedData:(id<ISktScanObject>)scanObj;
-(SKTRESULT)sendNextCommand;

@end
