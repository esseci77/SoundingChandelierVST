//
//  osx_utils.h
//  PaDrive
//
//  Created by Simone Campanini on 14/04/16.
//  Copyright (c) 2015 Fons Adriaensen. All rights reserved.
//


#include <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>

#include <cstring>

#include "osx_utils.h"

bool OSXUtils::findFilterPath(const char* name, char* path, const size_t maxlen)
{
    NSBundle* bundle = [NSBundle mainBundle];
    
    [[bundle resourcePath] getCString:path
                            maxLength:maxlen - (strlen(name) + 4)
                             encoding:NSASCIIStringEncoding];
    strcat(path, "/");
    strcat(path, name);
    
    if (strstr(name,".ald") == NULL)
    {
        strcat(path, ".ald");
    }
    
    FILE* f = fopen(path, "r");
    if(f == NULL) return false;
    
    fclose(f);
    
    return true;
}

bool OSXUtils::findImageInBundle(char* name, const unsigned int len)
{
    //    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSBundle* bundle = [NSBundle mainBundle];
    
    char path [1024];
    [[bundle resourcePath] getCString:path
                            maxLength:1024
                             encoding:NSASCIIStringEncoding];
    strcat(path, "/");
    strcat(path, name);
    
    if (strstr(name,".png") == NULL)
    {
        strcat(path, ".png");
    }

    unsigned int l = 1024;
    if(len < l) l = len;
    
    strcpy(name, path);
    
    FILE* f = fopen(name, "r");
    if(f == NULL) return false;
    
    fclose(f);
    
    //    [pool release];
    return true;
}