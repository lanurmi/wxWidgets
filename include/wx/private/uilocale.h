///////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/uilocale.h
// Purpose:     wxUILocaleImpl class declaration
// Author:      Vadim Zeitlin
// Created:     2021-08-01
// Copyright:   (c) 2021 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_UILOCALE_H_
#define _WX_PRIVATE_UILOCALE_H_

#include "wx/localedefs.h"
#include "wx/object.h"
#include "wx/string.h"
#include "wx/vector.h"

typedef wxVector<wxLanguageInfo> wxLanguageInfos;

// Return the vector of all languages known to wx.
const wxLanguageInfos& wxGetLanguageInfos();

// Function returning hard-coded values for the "C" locale.
wxString wxGetStdCLocaleInfo(wxLocaleInfo index, wxLocaleCategory cat);

// ----------------------------------------------------------------------------
// wxUILocaleImpl provides the implementation of public wxUILocale functions
// ----------------------------------------------------------------------------

class wxUILocaleImpl : public wxRefCounter
{
public:
    // This function is implemented in platform-specific code and returns the
    // object used by default, i.e. if wxUILocale::UseDefault() is not called.
    // This object corresponds to the traditional "C" locale.
    //
    // It should never return nullptr.
    static wxUILocaleImpl* CreateStdC();

    // Similarly, this one returns the object corresponding to the default user
    // locale settings which is used if wxUILocale::UseDefault() was called.
    //
    // It may return nullptr in case of failure.
    static wxUILocaleImpl* CreateUserDefault();

    // Create locale object for the given locale.
    //
    // It may return nullptr in case of failure.
    static wxUILocaleImpl* CreateForLocale(const wxLocaleIdent& locId);

    // This function exists only for wxLocale compatibility and creates the
    // locale corresponding to the given language. It shouldn't be used
    // anywhere else.
    //
    // It is implemented in terms of CreateForLocale() for non-MSW platforms,
    // but under MSW it is different for compatibility reasons.
    //
    // The language passed to this function is a valid language, i.e. neither
    // wxLANGUAGE_UNKNOWN nor wxLANGUAGE_DEFAULT.
    //
    // It may return nullptr in case of failure, but never does so for English
    // languages because wxLocale(wxLANGUAGE_ENGLISH) is always supposed to
    // work, so it just falls back on CreateStdC() if it fails to create it.
    static wxUILocaleImpl* CreateForLanguage(const wxLanguageInfo& info);

    // This function retrieves a list of preferred UI languages.
    // The list is in the order of preference, if it has more than one entry.
    // The entries contain platform-dependent identifiers.
    static wxVector<wxString> GetPreferredUILanguages();

#if wxUSE_DATETIME
    // Helper function used by GetMonthName/GetWeekDayName(): returns 0 if flags is
    // wxDateTime::Name_Full, 1 if it is wxDateTime::Name_Abbr, and 2 if it is
    // wxDateTime::Name_Shortest or -1 if the flags is incorrect (and asserts in this case)
    //
    // the return value of this function is used as an index into 2D array
    // containing full names in its first row and abbreviated ones in the 2nd one
    static int ArrayIndexFromFlag(wxDateTime::NameFlags flags);
#endif // wxUSE_DATETIME

    // Use this locale in the UI.
    //
    // This is not implemented for all platforms, notably not for Mac where the
    // UI locale is determined at application startup, but we can't do anything
    // about it anyhow, so we don't even bother returning an error code from it.
    virtual void Use() = 0;

    // Functions corresponding to wxUILocale ones.
    virtual wxString GetName() const = 0;
    virtual wxLocaleIdent GetLocaleId() const = 0;
    virtual wxString GetInfo(wxLocaleInfo index, wxLocaleCategory cat) const = 0;
    virtual wxString GetLocalizedName(wxLocaleName name, wxLocaleForm form) const = 0;
#if wxUSE_DATETIME
    virtual wxString GetMonthName(wxDateTime::Month month, wxDateTime::NameForm form) const = 0;
    virtual wxString GetWeekDayName(wxDateTime::WeekDay weekday, wxDateTime::NameForm form) const = 0;
#endif // wxUSE_DATETIME

    virtual wxLayoutDirection GetLayoutDirection() const = 0;
    virtual int CompareStrings(const wxString& lhs, const wxString& rhs,
                               int flags) const = 0;

    virtual ~wxUILocaleImpl() = default;

    // These two methods are for internal use only. First one creates the
    // global language database if it doesn't already exist, second one destroys
    // it.
    static void CreateLanguagesDB();
    static void DestroyLanguagesDB();

    // Creates the global tables of languages and scripts called by CreateLanguagesDB
    static void InitLanguagesDB();

    // These two methods are for internal use only.
    // wxLocaleIdent expects script identifiers as listed in ISO 15924.
    // However, directory names for translation catalogs follow the
    // Unix convention, using script aliases as listed  in ISO 15924.
    // First one converts a script name to its alias, second converts
    // a script alias to its corresponding script name.
    // Both methods return empty strings, if the script name or alias
    // couldn't be found.
    static wxString GetScriptAliasFromName(const wxString& scriptName);
    static wxString GetScriptNameFromAlias(const wxString& scriptAlias);

    // These three methods are for internal use only.
    // The new algorithm for determine the best translation language
    // uses them.
    // First one expands a locale tag using most likely subtags for script
    // and region. The method returns an empty string, if a matching tag
    // couldn't be found.
    // Second one determines the matching distance between locale tags.
    // The method returns -1, if no match was found.
    // Third one determines whether 2 regions belong to the same region
    // group of the given language. The method returns false, if no
    // region group is defined for the given language.
    static wxString GetLikelySubtags(const wxString & fromTag);
    static int GetMatchDistance(const wxString& desired, const wxString& supported);
    static bool SameRegionGroup(const wxString& language, const wxString& desiredRegion, const wxString& supportedRegion);
};

#endif // _WX_PRIVATE_UILOCALE_H_
