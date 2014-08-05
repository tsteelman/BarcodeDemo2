//====================================================================
//      COMMERCIAL SOURCE CODE
//     Copyright (C) 2001 - 2012 Vorpalware Corp. All rights reserved.           
//====================================================================

// This class handles pretty much everything to do with scanning from the Socket Scanner.


SocketScan = new Object();

SocketScan.DEVICE_ARRIVAL_MESSAGE = 'Connected';
SocketScan.DEVICE_REMOVAL_MESSAGE = 'Disconnected';
SocketScan.SCAN_SUCCESS_MESSAGE = 'Match Found';

// The notifications that can be sent to scanner. These match the enums for the kSktScanPropIdDataConfirmationDevice property,
// defined in the ScanAPI (in file SktScanPropIds.h).
// I don't think this will be a problem because I think that API has been around for years so the enums probably won't change.
SocketScan.NOTIFICATIONS = new Object();
SocketScan.NOTIFICATIONS.RUMBLE = {
		NONE: 0,
		GOOD: 1,
		BAD: 2
};
SocketScan.NOTIFICATIONS.BEEP = {
		NONE: 0,
		GOOD: 1,
		BAD: 2
};
SocketScan.NOTIFICATIONS.LED = {
		NONE: 0,
		GREEN: 1,
		RED: 2
};


// The name of the callbacks for native code to call. If you change these strings, change the name
// of the functions in this class, as well.
SocketScan.scanCallbackFunctionName = "SocketScan.scanCallback";
SocketScan.deviceArrivalCallbackFunctionName = "SocketScan.deviceArrivalCallback";
SocketScan.deviceRemovalCallbackFunctionName = "SocketScan.deviceRemovalCallback";


SocketScan.scanCallbacks = new Array();
SocketScan.deviceArrivalCallbacks = new Array();
SocketScan.deviceRemovalCallbacks = new Array();

// Used to know if the device was disconnected because the app was backgrounded. BugzID: 3914.
SocketScan.deviceArrivedSinceLastResume = false;


// Makes the call to native code to start up the API.
SocketScan.initScanApi = function() {
	$(document)
	.bind('resume', SocketScan.onResume);
	
	cordova.exec("RootScannerController.initScanApi",
			SocketScan.scanCallbackFunctionName,
			SocketScan.deviceArrivalCallbackFunctionName,
			SocketScan.deviceRemovalCallbackFunctionName);
};


// Called when application resumes from background.
SocketScan.onResume = function() {
	SocketScan.deviceArrivedSinceLastResume = false;
};


// Controls the rumble, beep and led on the scanner.
// Each argument should be of type "SocketScan.NOTIFICATIONS"
SocketScan.notifyScanner = function(rumble, beep, led) {
	cordova.exec("RootScannerController.notifyScanner",
			rumble, beep, led);
};


SocketScan.notifyScannerOfSuccess = function() {
	SocketScan.notifyScanner(
			SocketScan.NOTIFICATIONS.RUMBLE.NONE,
			SocketScan.NOTIFICATIONS.BEEP.GOOD,
			SocketScan.NOTIFICATIONS.LED.GREEN);
};


SocketScan.notifyScannerOfFailure = function() {
	SocketScan.notifyScanner(
			SocketScan.NOTIFICATIONS.RUMBLE.GOOD,
			SocketScan.NOTIFICATIONS.BEEP.NONE,
			SocketScan.NOTIFICATIONS.LED.GREEN);
};


SocketScan.enableDefaultScanHandler = function() {
	SocketScan.addScanCallback(SocketScan.defaultScanHandler);
};


// This is what native code will call when it gets a scan.
SocketScan.scanCallback = function(barcode, symbology) {
	var scanSuccess = false;
	
	var callbacks = SocketScan.scanCallbacks;
	for (var i = 0; i < callbacks.length; i++) {
		var funk = callbacks[i];
		// If any of the callbacks report a successful scan, we treat it as a successful scan.
		scanSuccess = scanSuccess || funk(barcode, symbology);
	}
	
	if (scanSuccess)
		SocketScan.notifyScannerOfSuccess();
	else
		SocketScan.notifyScannerOfFailure();
};


//Just like native code, for now we will assume that whatever device connects is the device
//we're using.
SocketScan.deviceArrivalCallback = function(friendlyName, guid) {
	var callbacks = SocketScan.deviceArrivalCallbacks;
	for (var i = 0; i < callbacks.length; i++) {
		var funk = callbacks[i];
		funk(friendlyName, guid);
	}
};


SocketScan.deviceRemovalCallback = function(friendlyName, guid) {
	var callbacks = SocketScan.deviceRemovalCallbacks;
	for (var i = 0; i < callbacks.length; i++) {
		var funk = callbacks[i];
		funk(friendlyName, guid);
	}
};


SocketScan.defaultScanHandler = function(barcode, symbology) {	
	var barcodeObj = new Barcode(barcode, symbology);
	
	var sys = SocketScan.getSystemByBarcode(barcodeObj.decryptedBarcode);
	if (sys === null) {
		return false;
	}
	else {
		// Do something with the successful scan decode.
		return true;
	}
};


// Just like native code, for now we will assume that whatever device connects is the device
// we're using.
SocketScan.defaultDeviceArrivalCallback = function(friendlyName, guid) {
	SocketScan.deviceArrivedSinceLastResume = true;
	
	SocketScan.connectedDevice = {
		friendlyName: friendlyName,
		guid: guid
	};	
};


SocketScan.defaultDeviceRemovalCallback = function(friendlyName, guid) {
	if (guid === SocketScan.connectedDevice.guid)
		delete SocketScan.connectedDevice;
	
	// If the device was disconnected because the app was backgrounded, don't notify the user. BugzID: 3914
	if (! SocketScan.deviceArrivedSinceLastResume)
		return;	
};


// Scan callbacks take parameters: (barcode, symbology).
// They should return true for a successful scan and false for a failure.
SocketScan.addScanCallback = function(callback) {
	SocketScan.scanCallbacks.push(callback);
};

SocketScan.removeScanCallback = function(removeMe) {
	var callbacks = SocketScan.scanCallbacks;
	for (var i = 0; i < callbacks.length; i++) {
		var funk = callbacks[i];
		if (removeMe === funk) {
			// Remove callback from array.
			callbacks.splice(i, 1);
		}
	}
};


SocketScan.addDeviceArrivalCallback = function(callback) {
	SocketScan.deviceArrivalCallbacks.push(callback);
};


SocketScan.removeDeviceArrivalCallback = function(removeMe) {
	var callbacks = SocketScan.deviceArrivalCallbacks;
	for (var i = 0; i < callbacks.length; i++) {
		var funk = callbacks[i];
		if (removeMe === funk) {
			// Remove callback from array.
			callbacks.splice(i, 1);
		}
	}
};


SocketScan.addDeviceRemovalCallback = function(callback) {
	SocketScan.deviceRemovalCallbacks.push(callback);
};


SocketScan.removeDeviceRemovalCallback = function(removeMe) {
	var callbacks = SocketScan.deviceRemovalCallbacks;
	for (var i = 0; i < callbacks.length; i++) {
		var funk = callbacks[i];
		if (removeMe === funk) {
			// Remove callback from array.
			callbacks.splice(i, 1);
		}
	}
};


// Do initialization for SocketScan here...
SocketScan.addDeviceArrivalCallback(SocketScan.defaultDeviceArrivalCallback);
SocketScan.addDeviceRemovalCallback(SocketScan.defaultDeviceRemovalCallback);

