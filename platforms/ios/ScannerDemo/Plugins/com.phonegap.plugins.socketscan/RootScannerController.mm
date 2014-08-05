//
//  RootViewController.m
//  ScannerSettings
//
//  Created by Heiby He on 11-2-22.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

// include for ScanAPI Files
#import "ScanAPI/include/ISktScanApi.h"
#import "ScanAPI/include/ScanAPI.h"
#import "ScanAPI/include/SktScanPropIds.h"
#import "ScanAPI/include/SktScanErrors.h"

#import "RootScannerController.h"
#import "ScanAPI/DeviceInfo.h"

#import "ScanAPI/Debug.h"

#import "ScanApiHelper.h"


//static NSString *kCellIdentifier = @"MyIdentifier";
static NSString *kTitleKey = @"title";
static NSString *kExplainKey = @"explanation";
static NSString *kViewControllerKey = @"viewController";

BOOL firstLoad=YES;
unsigned char _dataConfirmationMode=kSktScanDataConfirmationModeApp;
static NSString* PLEASE_RESTART_THIS_APP=@"Please restart this application";

@implementation RootScannerController

@synthesize devicelist, scanCallback, deviceArrivalCallback, deviceRemovalCallback;

-(void)Debug:(NSString *)text{
#ifdef DEBUG
    NSLog(text);
#endif
}

#pragma mark -
#pragma mark View lifecycle

// Takes in a barcode and passes it to the scan callback on the javascript side.
-(void) callScanCallback:(NSString*)barcode withSymbology:(NSString*)symbology {
    // TODO escape these parameters before making the javascript string
    // Make the String for the function call.
    NSString* jsCallBack = [NSString stringWithFormat:@"%@(\"%@\",\"%@\");", self.scanCallback, barcode, symbology];
    [self writeJavascript: jsCallBack];
}


// Takes in a barcode and passes it to the scan callback on the javascript side.
-(void) callDeviceArrivalCallback:(NSString*)friendlyName withGuid:(NSString*)guid{
    // TODO escape these parameters before making the javascript string
    // Make the String for the function call.
    NSString* jsCallBack = [NSString stringWithFormat:@"%@(\"%@\",\"%@\");", self.deviceArrivalCallback, friendlyName, guid];
    [self writeJavascript: jsCallBack];
}


// Takes in a barcode and passes it to the scan callback on the javascript side.
-(void) callDeviceRemovalCallback:(NSString*)friendlyName withGuid:(NSString*)guid{
    // TODO escape these parameters before making the javascript string
    // Make the String for the function call.
    NSString* jsCallBack = [NSString stringWithFormat:@"%@(\"%@\",\"%@\");", self.deviceRemovalCallback, friendlyName, guid];
    [self writeJavascript: jsCallBack];
}

// Called from javascript. Tells scanner to rumble, beep, or flash led. Code pretty much copied from PostSetDataConfirmation
-(void) notifyScanner:(NSMutableArray*)arguments withDict:(NSMutableDictionary*)options {
    int rumble = [(NSString*)[arguments objectAtIndex:0] intValue];
    int beep = [(NSString*)[arguments objectAtIndex:1] intValue];
    int led = [(NSString*)[arguments objectAtIndex:2] intValue];
    
    // create a ScanObject instance
    id<ISktScanObject> newScanObject=[SktClassFactory createScanObject];
    
    // retrieve its property
	id<ISktScanProperty> property=[newScanObject Property];
    
    // set the property members
	[property setID:kSktScanPropIdDataConfirmationDevice];
	[property setType:kSktScanPropTypeUlong];
	[property setUlong:SKTDATACONFIRMATION(0, rumble,beep,led)];
    
    CommandContext* command=[[CommandContext alloc]initWithParam:FALSE 
                                                         ScanObj:newScanObject 
                                                      ScanDevice:[_selectedDevice getSktScanDevice] 
                                                          Device:_selectedDevice
                                                          Target:self
                                                        Response:nil];
    
    [_scanApiHelper addCommand:command];
}


// Handle events from device.
-(void) OnNotify:(DeviceInfo *)deviceinfo notificationType:(enum ENotificationType)type{
    if(type==kNotificationDecodedData){
        // The info from the scan.
		DecodedDataInfo* decodedDataInfo=[deviceinfo getDecodedData];
        // Make the barcode data into a string.
		int _datalength=[decodedDataInfo getLength];
		uint8_t* _decodeddata=[decodedDataInfo getData];
		NSString* decodeddatatext=[[NSString alloc] initWithBytes:_decodeddata length:_datalength encoding:NSUTF8StringEncoding];
        // Barcode comes in with newline at the end, if the CHS 7Xi is used. Get rid of it.
        NSString *trimmedString = [decodeddatatext stringByTrimmingCharactersInSet:
                                   [NSCharacterSet newlineCharacterSet]];
        
        // Get the symbology as a string.
        NSString* symbology = [decodedDataInfo getSymbologyName];
        
        // Call the javascript code.
        [self callScanCallback:trimmedString withSymbology:symbology];
        
        [decodeddatatext release];
    }
}


-(void) PostSetLocalDecodeAction:(Byte)mode{    
    [_scanApiHelper postSetDecodeAction:_selectedDevice DecodeAction:mode Target:self Response:nil];
}



// timer handler for consuming ScanObject from ScanAPI
// if ScanAPI is not initialized this handler doesn nothing
-(void)onTimer{
    if(_scanApiInitialized==true)
        [_scanApiHelper doScanApiReceive];
}

// Kicks everything off. Called from javascript.
- (void) initScanApi:(NSMutableArray*)arguments withDict:(NSMutableDictionary*)options {
    self.scanCallback = [arguments objectAtIndex:0];
    self.deviceArrivalCallback = [arguments objectAtIndex:1];
    self.deviceRemovalCallback = [arguments objectAtIndex:2];
    
    _propertySetPending=NO;
	_iPad=NO;
    
#ifdef UI_USER_INTERFACE_IDIOM
	_iPad=(UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPad);
#endif
    
    _propertiesToRequest=[[NSMutableArray alloc]init];
    _propertiesToSet=[[NSMutableArray alloc]init];
    _scanApiVersion=@"requesting...";
    
	_scanApiInitialized=false;
	_devicecount=0;
	self.devicelist=[NSMutableArray array];
	[self.devicelist addObject:[NSDictionary dictionaryWithObjectsAndKeys:
								@"Please wait while initializing ScanAPI...", kTitleKey,
								@"Connected Scanner", kExplainKey,
								nil, kViewControllerKey,
								nil]];    
    
    
    // create a ScanApiHelper and open it
    _scanApiHelper=[[ScanApiHelper alloc]init];
    [_scanApiHelper setDelegate:self];
    [_scanApiHelper open];
    
    // start the ScanAPI Consumer timer to check if ScanAPI has a ScanObject for us to consume
    // all the asynchronous events coming from ScanAPI or property get/set complete operation
    // will be received in this consumer timer
    _scanApiConsumer=[[NSTimer scheduledTimerWithTimeInterval:0.2 target:self selector:@selector(onTimer) userInfo:nil repeats:YES] retain];

    
}

- (void)dealloc {
	[devicelist removeAllObjects];
	[devicelist release];
    devicelist=nil;
    [_scanApiConsumer release];
    _scanApiConsumer=nil;
    [super dealloc];
}


#pragma mark -

#pragma mark ScanApiHelper delegate
// most of these delegates can be received if there
// is a ScanApiHelper consummer in place calling the 
// ScanApiHelper doScanApiReceive which is this application
// is done using a timer
// The exception is the message:OnScanApiInitializeComplete
// which is called from the temporary thread that initializes
// ScanAPI


/**
 * called each time a device connects to the host
 * @param result contains the result of the connection
 * @param newDevice contains the device information
 */
-(void)onDeviceArrival:(SKTRESULT)result Device:(DeviceInfo*)deviceInfo friendlyName:friendlyName guid:guid {
    // information in the table view
	if (SKTSUCCESS(result)) {
		[self.devicelist removeObjectAtIndex:0];
		[self.devicelist addObject:[NSMutableDictionary dictionaryWithObjectsAndKeys:
									[deviceInfo getName], kTitleKey,
									@"Connected Scanner", kExplainKey,
									deviceInfo, kViewControllerKey,
									nil]];
		_devicecount++;
        _lastConnected=deviceInfo;
//        _selectedDevice=nil;// for now no device is selected
        // TODO this is probably a bad idea...but just use whatever device comes in as the selected device.
       	_selectedDevice = deviceInfo;
        // Listen for input...
        [_selectedDevice setNotification:self];
        
        // Notify javascript.
        [self callDeviceArrivalCallback:friendlyName withGuid:guid];
        // Disable local decode. Device will not rumble or beep unless we tell it to.
        [self PostSetLocalDecodeAction:kSktScanLocalDecodeActionNone];
	}
	else {
		UIAlertView *alert=[[UIAlertView alloc]
							initWithTitle:@"Error"
							message:@"Unable to open the scanner"
							delegate:self
							cancelButtonTitle:@"OK"
							otherButtonTitles:nil];
		[alert show];
		[alert release];
	}
    
}

/**
 * called each time a device disconnect from the host
 * @param deviceRemoved contains the device information
 */
-(void) onDeviceRemoval:(DeviceInfo*) deviceRemoved friendlyName:friendlyName guid:guid {
    // remove the device info from the list
    NSDictionary* dico=[self.devicelist objectAtIndex:0];
    DeviceInfo* deviceInfo=[dico valueForKey:kViewControllerKey];
	[self.devicelist removeObjectAtIndex:0];
    if(_selectedDevice==deviceInfo)
        _selectedDevice=nil;
	_devicecount--;
    
    // add the "No device connected" in the list
	[self.devicelist addObject:[NSDictionary dictionaryWithObjectsAndKeys:
								@"No device connected", kTitleKey,
								@"Connected Scanner", kExplainKey,
								nil, kViewControllerKey,
								nil]];
    
    [self callDeviceRemovalCallback:friendlyName withGuid:guid];
}

/**
 * called each time ScanAPI is reporting an error
 * @param result contains the error code
 */
-(void) onError:(SKTRESULT) result{
    
    NSString* errstr=nil;
    if(result==ESKT_UNABLEINITIALIZE)
        errstr=[NSString stringWithFormat:@"ScanAPI is reporting an error %d. Please turn off and on the scanner.", result];
    else
        errstr=[NSString stringWithFormat:@"ScanAPI is reporting an error %d", result];
    
    
    UIAlertView *alert=[[UIAlertView alloc]
                        initWithTitle:@"Error"
                        message:errstr
                        delegate:self
                        cancelButtonTitle:@"OK"
                        otherButtonTitles:nil];
    [alert show];
    [alert release];
}

/**
 * called each time ScanAPI receives decoded data from scanner
 * @param deviceInfo contains the device information from which
 * the data has been decoded
 * @param decodedData contains the decoded data information
 */
-(void) onDecodedData:(DeviceInfo*) device DecodedData:(id<ISktScanDecodedData>) decodedData{
    [_selectedDevice setDecodeData:decodedData];
}

/**
 * called when ScanAPI initialization has been completed
 * @param result contains the initialization result
 */
-(void) onScanApiInitializeComplete:(SKTRESULT) result{
    if(SKTSUCCESS(result)){
		// replace the text by No Device Connected
        [self.devicelist removeObjectAtIndex:0];
		[self.devicelist addObject:[NSDictionary dictionaryWithObjectsAndKeys:
									@"No device connected", kTitleKey,
									@"Connected Scanner", kExplainKey,
									nil, kViewControllerKey,
									nil]];
        
        // set the confirmation mode to be local on the device (more responsive)
        [_scanApiHelper postSetConfirmationMode:_dataConfirmationMode Target:self Response:@selector(onSetProperty:)];
        
        // this flag will tell the ScanAPI consumer timer handler
        // that it is ok now to consume ScanObject from ScanAPI since
        // it is correctly initialized
        _scanApiInitialized=true;
    }
    else{
		UIAlertView *alert=[[UIAlertView alloc]
							initWithTitle:@"Error"
							message:[NSString stringWithFormat:@"Unable to initialize ScanAPI: %d",result]
							delegate:self
							cancelButtonTitle:@"OK"
							otherButtonTitles:nil];
		[alert show];
		[alert release];
		[self.devicelist removeObjectAtIndex:0];
		[self.devicelist addObject:[NSDictionary dictionaryWithObjectsAndKeys:
									PLEASE_RESTART_THIS_APP, kTitleKey,
									@"Connected Scanner", kExplainKey,
									nil, kViewControllerKey,
									nil]];
    }
}

/**
 * called when ScanAPI has been terminated. This will be
 * the last message received from ScanAPI
 */
-(void) onScanApiTerminated{
    _scanApiInitialized=FALSE;
    [_scanApiHelper close];
}

/**
 * called when an error occurs during the retrieval
 * of a ScanObject from ScanAPI.
 * @param result contains the retrieval error code
 */
-(void) onErrorRetrievingScanObject:(SKTRESULT) result{
    [self Debug:[NSString stringWithFormat:@"Error retrieving ScanObject from ScanAPI:%d",result]];
}





#pragma mark -

#pragma mark ScanApiHelper utility function and Complete event handlers

-(NSString*)getScanApiVersion{
    return _scanApiVersion;
}


// return a reference to ScanAPIHelper for the other
// views
-(ScanApiHelper*)getScanApiHelper{
    return _scanApiHelper;
}

// OnSetProperty
// update the progress bar and in case of error
// save the property ID and the error in the device info object
// so the configuration screen can use this information to display 
// an error to the user and to put back the previous setting
-(SKTRESULT)onSetProperty:(id<ISktScanObject>)scanObject{
    SKTRESULT result=ESKT_NOERROR;
    if(_selectedDevice!=nil){
        result=[[scanObject Msg]Result];
        if(!SKTSUCCESS(result)){
            [_selectedDevice setPropertyError:[[scanObject Property]getID] Error:result];
        }
    }
    
    return result;
}



-(SKTRESULT) OnScanApiVersion:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
        id<ISktScanProperty>property=[scanObject Property];
        if([property getType]==kSktScanPropTypeVersion){
            _scanApiVersion=[NSString stringWithFormat:@"%x.%x.%x %d",
                             [[property getVersion]getMajor],
                             [[property getVersion]getMiddle],
                             [[property getVersion]getMinor],
                             [[property getVersion]getBuild]];
        }
        else
            _scanApiVersion=@"incorrect format";
		[[NSNotificationCenter defaultCenter] postNotificationName:@"UpdateScanApiVersion" object:nil userInfo:nil];
	}
	return result;
}

-(SKTRESULT) OnFriendlyName:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
        id<ISktScanProperty>property=[scanObject Property];
        if(_selectedDevice!=nil)
            [_selectedDevice setName:[property getString]];
	}
	return result;
}

-(SKTRESULT) OnBtAddress:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanProperty> property=[scanObject Property];
		unsigned char array[6];
		memcpy(array, [property getArrayValue], [property getArraySize]);
		NSString* strBtAddr=[[NSString alloc] initWithFormat:@"%02x:%02x:%02x:%02x:%02x:%02x", array[0],
							 array[1],
							 array[2],
							 array[3],
							 array[4],
							 array[5]];
		
        if(_selectedDevice!=nil)
            [_selectedDevice setBdAddress:strBtAddr];
		
	}
	return result;
}
-(SKTRESULT) OnScannerType:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanProperty> property=[scanObject Property];
		long type=[property getUlong];
		
        if(_selectedDevice!=nil)
            [_selectedDevice setType:type];
		
	}
	return result;
}

-(SKTRESULT) OnScannerFirmware:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanProperty> property=[scanObject Property];
		id<ISktScanVersion> version=[property getVersion];
		NSString* szVersion=[[NSString alloc] initWithFormat:@"%x.%x.%x %d %02x.%02x.%04x", 
							 [version getMajor],
							 [version getMiddle],
							 [version getMinor],
							 [version getBuild],
							 [version getMonth],
							 [version getDay],
							 [version getYear],
							 [version getHour],
							 [version getMinute]];
		
        if(_selectedDevice!=nil)
            [_selectedDevice setFirmwareVersion:szVersion];		
		
	}
	return result;
}

-(SKTRESULT) OnBatteryLevel:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanProperty> property=[scanObject Property];
		int level=SKTBATTERY_GETCURLEVEL([property getUlong]);
		
        if(_selectedDevice!=nil){
            if (level>0) {
                [_selectedDevice setBatteryLevel:[NSString stringWithFormat:@"%d%%",level]];
            }
            else {
                [_selectedDevice setBatteryLevel:@"0%%"];
            }
        }
		
	}
	return result;
}

-(SKTRESULT) OnDecodeAction:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanProperty> property=[scanObject Property];
        if(_selectedDevice!=nil)
            [_selectedDevice setLocalDecodeAction:[property getByte]];		
		
	}
	return result;
}

-(SKTRESULT) OnCapabilitiesDevice:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanProperty> property=[scanObject Property];
		BOOL bRumble=(([property getUlong]&kSktScanCapabilityLocalFunctionRumble)==kSktScanCapabilityLocalFunctionRumble);
        if(_selectedDevice!=nil)
            [_selectedDevice setRumbleSupport:bRumble];
		
	}
	return result;
}

-(SKTRESULT) OnPostambleDevice:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanProperty> property=[scanObject Property];
        if(_selectedDevice!=nil)
            [_selectedDevice setPostamble:[property getString]];
		
	}
	return result;
}

-(SKTRESULT) OnSymbologyInfo:(id<ISktScanObject>)scanObject{
	SKTRESULT result=ESKT_NOERROR;
	result=[[scanObject Msg] Result];
	if (SKTSUCCESS(result)) {
		id<ISktScanSymbology> symbology=[[scanObject Property]getSymbology];
        if(_selectedDevice!=nil)
            [_selectedDevice addSymbologyInfo:symbology];
		
	}
	return result;
}

	
@end

