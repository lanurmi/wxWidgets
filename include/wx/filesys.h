/////////////////////////////////////////////////////////////////////////////
// Name:        wx/filesys.h
// Purpose:     class for opening files - virtual file system
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __FILESYS_H__
#define __FILESYS_H__

#include "wx/defs.h"

#if wxUSE_FILESYSTEM

#include "wx/stream.h"
#include "wx/datetime.h"
#include "wx/filename.h"
#include "wx/list.h"

#include <unordered_map>

class WXDLLIMPEXP_FWD_BASE wxFSFile;
class WXDLLIMPEXP_FWD_BASE wxFileSystemHandler;
class WXDLLIMPEXP_FWD_BASE wxFileSystem;

//--------------------------------------------------------------------------------
// wxFSFile
//                  This class is a file opened using wxFileSystem. It consists of
//                  input stream, location, mime type & optional anchor
//                  (in 'index.htm#chapter2', 'chapter2' is anchor)
//--------------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxFSFile : public wxObject
{
public:
    wxFSFile(wxInputStream *stream, const wxString& loc,
             const wxString& mimetype, const wxString& anchor
#if wxUSE_DATETIME
             , wxDateTime modif
#endif // wxUSE_DATETIME
             )
        : m_Location(loc)
        , m_MimeType(mimetype.Lower())
        , m_Anchor(anchor)
#if wxUSE_DATETIME
        , m_Modif(modif)
#endif
    {
        m_Stream = stream;
    }

    virtual ~wxFSFile() { delete m_Stream; }

    // returns stream. This doesn't give away ownership of the stream object.
    wxInputStream *GetStream() const { return m_Stream; }

    // gives away the ownership of the current stream.
    wxInputStream *DetachStream()
    {
        wxInputStream *stream = m_Stream;
        m_Stream = nullptr;
        return stream;
    }

    // deletes the current stream and takes ownership of another.
    void SetStream(wxInputStream *stream)
    {
        delete m_Stream;
        m_Stream = stream;
    }

    // returns file's mime type
    const wxString& GetMimeType() const;

    // returns the original location (aka filename) of the file
    const wxString& GetLocation() const { return m_Location; }

    const wxString& GetAnchor() const { return m_Anchor; }

#if wxUSE_DATETIME
    wxDateTime GetModificationTime() const { return m_Modif; }
#endif // wxUSE_DATETIME

private:
    wxInputStream *m_Stream;
    wxString m_Location;
    wxString m_MimeType;
    wxString m_Anchor;
#if wxUSE_DATETIME
    wxDateTime m_Modif;
#endif // wxUSE_DATETIME

    wxDECLARE_ABSTRACT_CLASS(wxFSFile);
    wxDECLARE_NO_COPY_CLASS(wxFSFile);
};





//--------------------------------------------------------------------------------
// wxFileSystemHandler
//                  This class is FS handler for wxFileSystem. It provides
//                  interface to access certain
//                  kinds of files (HTPP, FTP, local, tar.gz etc..)
//--------------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxFileSystemHandler : public wxObject
{
public:
    wxFileSystemHandler() : wxObject() {}

    // returns true if this handler is able to open given location
    virtual bool CanOpen(const wxString& location) = 0;

    // opens given file and returns pointer to input stream.
    // Returns nullptr if opening failed.
    // The location is always absolute path.
    virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) = 0;

    // Finds first/next file that matches spec wildcard. flags can be wxDIR for restricting
    // the query to directories or wxFILE for files only or 0 for either.
    // Returns filename or empty string if no more matching file exists
    virtual wxString FindFirst(const wxString& spec, int flags = 0);
    virtual wxString FindNext();

    // Returns MIME type of the file - w/o need to open it
    // (default behaviour is that it returns type based on extension)
    static wxString GetMimeTypeFromExt(const wxString& location);

protected:
    // returns protocol ("file", "http", "tar" etc.) The last (most right)
    // protocol is used:
    // {it returns "tar" for "file:subdir/archive.tar.gz#tar:/README.txt"}
    static wxString GetProtocol(const wxString& location);

    // returns left part of address:
    // {it returns "file:subdir/archive.tar.gz" for "file:subdir/archive.tar.gz#tar:/README.txt"}
    static wxString GetLeftLocation(const wxString& location);

    // returns anchor part of address:
    // {it returns "anchor" for "file:subdir/archive.tar.gz#tar:/README.txt#anchor"}
    // NOTE:  anchor is NOT a part of GetLeftLocation()'s return value
    static wxString GetAnchor(const wxString& location);

    // returns right part of address:
    // {it returns "/README.txt" for "file:subdir/archive.tar.gz#tar:/README.txt"}
    static wxString GetRightLocation(const wxString& location);

    wxDECLARE_ABSTRACT_CLASS(wxFileSystemHandler);
};




//--------------------------------------------------------------------------------
// wxFileSystem
//                  This class provides simple interface for opening various
//                  kinds of files (HTPP, FTP, local, tar.gz etc..)
//--------------------------------------------------------------------------------

// Open Bit Flags
enum wxFileSystemOpenFlags
{
    wxFS_READ = 1,      // Open for reading
    wxFS_SEEKABLE = 4   // Returned stream will be seekable
};

using wxFSHandlerHash = std::unordered_map<void*, wxFileSystemHandler*>;

class WXDLLIMPEXP_BASE wxFileSystem : public wxObject
{
public:
    wxFileSystem() : wxObject() { m_FindFileHandler = nullptr;}
    virtual ~wxFileSystem();

    // sets the current location. Every call to OpenFile is
    // relative to this location.
    // NOTE !!
    // unless is_dir = true 'location' is *not* the directory but
    // file contained in this directory
    // (so ChangePathTo("dir/subdir/xh.htm") sets m_Path to "dir/subdir/")
    void ChangePathTo(const wxString& location, bool is_dir = false);

    wxString GetPath() const {return m_Path;}

    // opens given file and returns pointer to input stream.
    // Returns nullptr if opening failed.
    // It first tries to open the file in relative scope
    // (based on ChangePathTo()'s value) and then as an absolute
    // path.
    wxFSFile* OpenFile(const wxString& location, int flags = wxFS_READ);

    // Finds first/next file that matches spec wildcard. flags can be wxDIR for restricting
    // the query to directories or wxFILE for files only or 0 for either.
    // Returns filename or empty string if no more matching file exists
    wxString FindFirst(const wxString& spec, int flags = 0);
    wxString FindNext();

    // find a file in a list of directories, returns false if not found
    bool FindFileInPath(wxString *pStr,
                        const wxString& path, const wxString& file);

    // Adds FS handler.
    // In fact, this class is only front-end to the FS handlers :-)
    static void AddHandler(wxFileSystemHandler *handler);

    // Removes FS handler
    static wxFileSystemHandler* RemoveHandler(wxFileSystemHandler *handler);

    // Returns true if there is a handler which can open the given location.
    static bool HasHandlerForPath(const wxString& location);

    // remove all items from the m_Handlers list
    static void CleanUpHandlers();

    // Returns the native path for a file URL
    static wxFileName URLToFileName(const wxString& url);

    // Returns the file URL for a native path
    static wxString FileNameToURL(const wxFileName& filename);


protected:
    wxFileSystemHandler *MakeLocal(wxFileSystemHandler *h);

    wxString m_Path;
            // the path (location) we are currently in
            // this is path, not file!
            // (so if you opened test/demo.htm, it is
            // "test/", not "test/demo.htm")
    wxString m_LastName;
            // name of last opened file (full path)
    static wxList m_Handlers;
            // list of FS handlers
    wxFileSystemHandler *m_FindFileHandler;
            // handler that succeed in FindFirst query
    wxFSHandlerHash m_LocalHandlers;
            // Handlers local to this instance

    wxDECLARE_DYNAMIC_CLASS(wxFileSystem);
    wxDECLARE_NO_COPY_CLASS(wxFileSystem);
};


/*

'location' syntax:

To determine FS type, we're using standard KDE notation:
file:/absolute/path/file.htm
file:relative_path/xxxxx.html
/some/path/x.file               ('file:' is default)
http://www.gnome.org
file:subdir/archive.tar.gz#tar:/README.txt

special characters :
  ':' - FS identificator is before this char
  '#' - separator. It can be either HTML anchor ("index.html#news")
            (in case there is no ':' in the string to the right from it)
        or FS separator
            (example : http://www.wxhtml.org/wxhtml-0.1.tar.gz#tar:/include/wxhtml/filesys.h"
             this would access tgz archive stored on web)
  '/' - directory (path) separator. It is used to determine upper-level path.
        HEY! Don't use \ even if you're on Windows!

*/


class WXDLLIMPEXP_BASE wxLocalFSHandler : public wxFileSystemHandler
{
public:
    virtual bool CanOpen(const wxString& location) override;
    virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;
    virtual wxString FindFirst(const wxString& spec, int flags = 0) override;
    virtual wxString FindNext() override;

    // wxLocalFSHandler will prefix all filenames with 'root' before accessing
    // files on disk. This effectively makes 'root' the top-level directory
    // and prevents access to files outside this directory.
    // (This is similar to Unix command 'chroot'.)
    static void Chroot(const wxString& root) { ms_root = root; }

protected:
    static wxString ms_root;
};

// Stream reading data from wxFSFile: this allows to use virtual files with any
// wx functions accepting streams.
class WXDLLIMPEXP_BASE wxFSInputStream : public wxWrapperInputStream
{
public:
    // Notice that wxFS_READ is implied in flags.
    wxFSInputStream(const wxString& filename, int flags = 0);
    virtual ~wxFSInputStream();

private:
    wxFSFile* m_file;

    wxDECLARE_NO_COPY_CLASS(wxFSInputStream);
};

#endif
  // wxUSE_FILESYSTEM

#endif
  // __FILESYS_H__
