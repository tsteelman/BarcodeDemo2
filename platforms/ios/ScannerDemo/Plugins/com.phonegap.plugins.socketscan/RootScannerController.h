//
//  RootViewController.h
//  ScannerSettings
//
//  Created by Heiby He on 11-2-22.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ScanApiHelper.h"
//#import "PGPlugin.h"
#ifdef CORDOVA_FRAMEWORK
#import <Cordova/CDVPlugin.h>
#else
#import "CDVPlugin.h"
#endif

@protocol ISktScanObject;
@class DeviceInfo;
@protocol ISktScanApi;
@protocol ISktScanDevice;

@interface RootScannerController : CDVPlugin<ScanApiHelperDelegate> {
    // Javascript callbacks. Passed in from javascript.
    NSString* scanCallback;
    NSString* deviceArrivalCallback;
    NSString* deviceRemovalCallback;
    
  	NSMutableArray* devicelist;
    NSTimer* _scanApiConsumer;
    Boolean _scanApiInitialized;
    id<ISktScanObject> _scanObjectReceived;
    NSMutableArray* _propertiesToRequest;
    NSMutableArray* _propertiesToSet;
    BOOL _iPad;
    BOOL _propertySetPending;
    id<ISktScanApi> _scanapi;
    int _devicecount;
    DeviceInfo* _selectedDevice;
    DeviceInfo* _lastConnected;
    NSString* _scanApiVersion;
    
    ScanApiHelper* _scanApiHelper;
}
@property (nonatomic,retain) NSMutableArray *devicelist;
@property (nonatomic,retain) NSString* scanCallback;
@property (nonatomic,retain) NSString* deviceArrivalCallback;
@property (nonatomic,retain) NSString* deviceRemovalCallback;


-(void) Debug:(NSString*)text;
-(void) displayErrorAndGoBackToTop:(NSString*)errorText;
-(int) preparePropertyToRequest;

 
-(NSString*)getScanApiVersion;

-(ScanApiHelper*)getScanApiHelper;
-(SKTRESULT) onSetProperty:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onScanApiVersion:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onFriendlyName:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onBtAddress:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onScannerType:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onScannerFirmware:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onBatteryLevel:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onDecodeAction:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onCapabilitiesDevice:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onPostambleDevice:(id<ISktScanObject>)scanObject;
-(SKTRESULT) onSymbologyInfo:(id<ISktScanObject>)scanObject;
@end
