//
//  ISktScanDecodedData.h
//  ScanApi
//
//  Created by Eric Glaenzer on 5/19/11.
//  Copyright 2011 SocketMobile. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol ISktScanDecodedData <NSObject>
-(enum ESktScanSymbologyID)ID;
-(NSString*)Name;
-(uint8_t*)getData;
-(int)getDataSize;

@end
