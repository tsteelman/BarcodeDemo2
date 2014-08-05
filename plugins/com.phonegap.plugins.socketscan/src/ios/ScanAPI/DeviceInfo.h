//
//  DeviceInfo.h
//  ScannerSettings
//
//  Created by Jimmy Yang on 11-2-23.
//  Copyright 2011 Socket Mobile, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ScanApiIncludes.h"

@class DeviceInfo;

enum ENotificationType{
    kNotificationFriendlyName,
    kNotificationBluetoothAddress,
    kNotificationDeviceType,
    kNotificationFirmwareVersion,
    kNotificationBattery,
    kNotificationLocalDecodeAction,
    kNotificationCapabilities,
    kNotificationPostamble,
    kNotificationSymbology,
    kNotificationDecodedData,
    kNotificationSetPropertyError
};

@protocol Notification
-(void) OnNotify:(DeviceInfo*)deviceinfo notificationType:(enum ENotificationType)type;
@end

@interface SymbologyInfo : NSObject{
    NSString* _name;
    enum ESktScanSymbologyID _id;
    BOOL _enabled;
}
-(SymbologyInfo*)initWithSymbology:(id<ISktScanSymbology>)symbology;

-(NSString*)getName;
-(void)setName:(NSString*) name;

-(enum ESktScanSymbologyID)getId;
-(void)setId:(enum ESktScanSymbologyID)symbologyId;

-(BOOL)isEnabled;
-(void)setEnabled:(BOOL)enabled;

@end

@interface DecodedDataInfo : NSObject{
    NSString* _symbologyName;
    NSMutableData* _data;
    int _length;
}
-(DecodedDataInfo*)initWithDecodedData:(id<ISktScanDecodedData>)decodedData;

-(NSString*)getSymbologyName;
-(void)setSymbologyName:(NSString*)symbologyName;

-(uint8_t*)getData;
-(void)setData:(uint8_t*)data Length:(int)length;

-(int)getLength;
-(void)setLength:(int)length;

@end

@protocol ISktScanDevice;
@interface DeviceInfo : NSObject {

	id<ISktScanDevice> _device;
	id<Notification> _notification;

    // device properties
	NSString* _name;
	long _type;
	
	NSString *_bdAddress;
	NSString *_version;
	NSString *_batterylevel;
	int _localDecodeAction;
	BOOL _rumbleSupport;
	NSString *_postamble;
	DecodedDataInfo* _decodedData;
    
    NSMutableArray* _symbologies;
    
    long _setPropertyId;
    long _setPropertyError;
}

-(DeviceInfo*)init:(id<ISktScanDevice>)device name:(NSString*)name type:(long)type;

-(id<ISktScanDevice>) getSktScanDevice;

-(void) setNotification:(id)notification;
-(id) getNotification;

-(NSString*) getName;
-(void) setName:(NSString*)name;

-(NSString*)getBdAddress;
-(void)setBdAddress:(NSString*) bdAddress;

-(NSString*) getTypeString;
-(void) setType:(long)type;

-(NSString*)getFirmwareVersion;
-(void)setFirmwareVersion:(NSString*)version;

-(NSString*)getBatteryLevel;
-(void)setBatteryLevel:(NSString*)level;

-(int) getLocalDecodeAction;
-(void) setLocalDecodeAction:(int)decodeAction;

-(BOOL) getRumbleSupport;
-(void) setRumbleSupport:(BOOL)support;

-(NSString*)getPostamble;
-(void)setPostamble:(NSString*) postamble;

-(void) setDecodeData:(id<ISktScanDecodedData>)decodedData;
-(DecodedDataInfo*) getDecodedData;

-(SymbologyInfo*)getSymbologyInfo:(int)index;
-(void)addSymbologyInfo:(id<ISktScanSymbology>)symbologyInfo;

-(int)getSymbologyCount;

-(void)setPropertyError:(long)propertyId Error:(long)error;
-(long)getPropertyErrorId;
-(long)getPropertyError;
@end
