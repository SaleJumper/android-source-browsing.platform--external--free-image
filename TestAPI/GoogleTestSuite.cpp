// Google test suite
#include <string>
using namespace std;

#include <stdio.h>
#include <windows.h>

#include "FreeImage.h"

/* =====================================================================================================================
 ======================================================================================================================= */
void scaleImage(wstring &path)
{
    wstring     path_in = wstring(L"c:\\temp\\unsorted\\") + path;
    wstring     path_out = wstring(L"c:\\temp\\scaled\\") + path;

    FIBITMAP    *inputImage = NULL;

    wstring     ext((const wchar_t *) path_in.c_str() + path_in.length() - 4);
    if((ext == L".jpg") || (ext == L".JPG")) {
        inputImage = FreeImage_LoadU(FIF_JPEG, path_in.c_str(), 0);
    }
    else if((ext == L".png") || (ext == L".PNG")) {
        inputImage = FreeImage_LoadU(FIF_PNG, path_in.c_str(), 0);
    }
    else if((ext == L".gif") || (ext == L".GIF")) {
        inputImage = FreeImage_LoadU(FIF_GIF, path_in.c_str(), 0);
    }
    else {
        return;
    }

    if(!inputImage) {
        return;
    }

    FIBITMAP    *outputImage = FreeImage_Rescale(inputImage, FreeImage_GetWidth(inputImage) / 2, FreeImage_GetHeight(inputImage) / 2, FILTER_BILINEAR);
    FreeImage_Unload(inputImage);

    if(!outputImage) {
        return;
    }

    BOOL    ret;
    if((ext == L".jpg") || (ext == L".JPG")) {
        ret = FreeImage_SaveU(FIF_JPEG, outputImage, path_out.c_str(), 0);
    }
    else if((ext == L".png") || (ext == L".PNG")) {
        ret = FreeImage_SaveU(FIF_PNG, outputImage, path_out.c_str(), 0);
    }
    else if((ext == L".gif") || (ext == L".GIF")) {
        ret = FreeImage_SaveU(FIF_GIF, outputImage, path_out.c_str(), 0);
    }

    // we are done
    FreeImage_Unload(outputImage);
}

#define THUMBNAIL_SIZE  90  // fit inside a square whose size is 90 pixels

/* =====================================================================================================================
 ======================================================================================================================= */
void makeThumbnail(wstring &path)
{
    wstring     path_in = wstring(L"c:\\temp\\unsorted\\") + path;
    wstring     path_out = wstring(L"c:\\temp\\thumbs\\") + path + wstring(L".jpg");


    FIBITMAP            *dib = NULL;
    int                 flags = 0;          // default load flag
    int                 originalWidth = 0;  // original image width
    int                 originalHeight = 0; // original image height
	FREE_IMAGE_FORMAT   fif = FreeImage_GetFileTypeU(path_in.c_str());
    if(fif == FIF_UNKNOWN) {
        return ;
    }

    if(fif == FIF_JPEG) {
        FITAG   *tag = NULL;

        // for JPEG images, we can speedup the loading part using LibJPEG downsampling feature while loading the
        // image...
        flags |= THUMBNAIL_SIZE << 16;

        // load the dib
        dib = FreeImage_LoadU(fif, path_in.c_str(), flags);
        if(!dib) {
            return ;
        }

        // the dib may have been downscaled by 2x, 4x or 8x retrieve the original width & height (stored as comments
        // for this special case)
        if(FreeImage_GetMetadata(FIMD_COMMENTS, dib, "OriginalJPEGWidth", &tag)) {
            originalWidth = atoi((char *) FreeImage_GetTagValue(tag));
        }
        else {
            originalWidth = FreeImage_GetWidth(dib);
        }

        if(FreeImage_GetMetadata(FIMD_COMMENTS, dib, "OriginalJPEGHeight", &tag)) {
            originalHeight = atoi((char *) FreeImage_GetTagValue(tag));
        }
        else {
            originalHeight = FreeImage_GetHeight(dib);
        }
    }
    else {

        // any cases other than the JPEG case: load the dib
        dib = FreeImage_LoadU(fif, path_in.c_str(), flags);
        if(!dib) {
            return ;
        }

        originalWidth = FreeImage_GetWidth(dib);
        originalHeight = FreeImage_GetHeight(dib);
    }

    // store ‘originalWidth’ and ‘originalHeight’ for later use … store any other metadata (such as Exif) for later
    // use … ... create the requested thumbnail
    FIBITMAP    *thumbnail = FreeImage_MakeThumbnail(dib, THUMBNAIL_SIZE, TRUE);
    FreeImage_Unload(dib);

	// Save thumbnail
	if(thumbnail) {
	FreeImage_SaveU(FIF_JPEG, thumbnail, path_out.c_str(), 0);
    FreeImage_Unload(thumbnail);
	}

}

/* =====================================================================================================================
 ======================================================================================================================= */
int main(int argc, char *argv[])
{
    unsigned    width = 512;
    unsigned    height = 512;

#if defined(_DEBUG) && defined(WIN32)
    // check for memory leaks at program exit (after the 'return 0') through a call to _CrtDumpMemoryLeaks note that
    // in debug mode, objects allocated with the new operator may be destroyed *after* the end of the main function.
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif
#if defined(FREEIMAGE_LIB) || !defined(WIN32)
    FreeImage_Initialise();
#endif

    WIN32_FIND_DATAW    fileData;
    HANDLE              hList = FindFirstFileW(L"c:\\temp\\unsorted\\*", &fileData);

    if(hList == INVALID_HANDLE_VALUE) {

        // empty dir: nothing to do
        return 0;
    }

    while(1) {

        // Make sure that files and directories are 'non-special'
        if(fileData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE)) {

            // TpnTrace::Trace(TRACE_INFO, "Skipping file(%S) because of attributes (0x%X)", fileData.cFileName, fileData.dwFileAttributes);
        }
        else {
            wstring fileName(fileData.cFileName, wcslen(fileData.cFileName));

            if((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (fileName == wstring(L".")) || (fileName == wstring(L".."))) {

                // folder, nothing to do
            }
            else {

                // process files
                //scaleImage(fileName);
				makeThumbnail(fileName);
            }
        }

        if(!FindNextFileW(hList, &fileData)) {
            if(GetLastError() == ERROR_NO_MORE_FILES) {
                break;
            }

            break;  // throw !!!
        }
    }

    FindClose(hList);

#if defined(FREEIMAGE_LIB) || !defined(WIN32)
    FreeImage_DeInitialise();
#endif
    return 0;
}
