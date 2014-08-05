//
//  ISktScanVersion.h
//  ScanApi
//
//  Created by Eric Glaenzer on 5/20/11.
//  Copyright 2011 SocketMobile. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol ISktScanVersion
- (unsigned short) getMajor;
- (unsigned short) getMiddle;
- (unsigned short) getMinor;
- (unsigned long) getBuild;
- (unsigned short) getMonth;
- (unsigned short) getDay;
- (unsigned short) getYear;
- (unsigned short) getHour;
- (unsigned short) getMinute;

@end
