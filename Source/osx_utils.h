//
//  osx_utils.h
//  PaDrive
//
//  Created by Simone Campanini on 14/04/16.
//  Copyright (c) 2015 Fons Adriaensen. All rights reserved.
//

#ifndef CdsRemote_osx_utils_h
#define CdsRemote_osx_utils_h

namespace OSXUtils
{
    bool findFilterPath(const char* name, char* path, const size_t maxlen);
    bool findImageInBundle(char* name, const unsigned int len);
}
#endif
