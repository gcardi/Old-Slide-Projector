//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include <shlobj.h>

#include "BrowseFolder.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{

    if(uMsg == BFFM_INITIALIZED)
    {
        std::wstring tmp = reinterpret_cast<LPCTSTR>( lpData );
        std::wcout << "path: " << tmp << std::endl;
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }

    return 0;
}

String BrowseFolder( String SavedPath )
{
    TCHAR path[MAX_PATH];

    LPCTSTR path_param = SavedPath.c_str();

    BROWSEINFO bi = { 0 };
    bi.lpszTitle  = _D( "Browse for folder..." );
    bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn       = BrowseCallbackProc;
    bi.lParam     = (LPARAM) path_param;

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );

    if ( pidl != 0 )
    {
        //get the name of the folder and put it in path
        SHGetPathFromIDList ( pidl, path );

        //free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }

        return path;
    }

    return {};
}