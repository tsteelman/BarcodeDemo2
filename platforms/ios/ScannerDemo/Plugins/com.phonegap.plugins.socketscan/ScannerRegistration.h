//
//  ScannerRegistration.h
//  ScannerRegistrationLib
//
//  Created by Jimmy Yang on 6/21/11.
//  Copyright 2011 Socket Mobile, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>


enum eRegistrationStatus
{
	registered,
	notRegistered,
	unableRegister,
	cached
};

@protocol ScannerRegistrationDelegate <NSObject>
-(void)scannerRegistrationStatusDidRespond:(enum eRegistrationStatus)status;    

@end

@interface ScannerRegistration : NSObject {
    NSURLConnection* _connection;
    id _delegate;
    enum eRegistrationStatus _status;
    BOOL _pending;// a request is pending
}
-(id)init;
+(enum eRegistrationStatus) getScannerStatus:(NSString*)scannerBluetoothAddress RefreshCache:(BOOL)refreshCache;
-(long)startGetScannerStatus:(NSString*)scannerBluetoothAddress RefreshCache:(BOOL)refreshCache Delegate:(id)delegate;
-(long)stopGetScannerStatus;
+(long) registerScanner:(NSString*)scannerBluetoothAddress AppID:(NSString*)applicationId URL:(NSString*)webFormUrl;
+(void)DbgMsg:(NSString*)message;
@end
