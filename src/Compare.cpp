/*
 * This file is part of Compare plugin for Notepad++
 * Copyright (C)2011 Jean-Sebastien Leroy (jean.sebastien.leroy@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <exception>
#include <vector>
#include <memory>

#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>

#include "Tools.h"
#include "Compare.h"
#include "NppHelpers.h"
#include "LibHelpers.h"
#include "ProgressDlg.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "NavDialog.h"
#include "Engine.h"
#include "NppInternalDefines.h"


NppData nppData;
UserSettings Settings;


const TCHAR UserSettings::mainSection[]					= TEXT("Main");

const TCHAR UserSettings::oldIsFirstSetting[]			= TEXT("Old is First");
const TCHAR UserSettings::oldFileOnLeftSetting[]		= TEXT("Old on Left");
const TCHAR UserSettings::compareToPrevSetting[]		= TEXT("Default Compare is to Prev");

const TCHAR UserSettings::detectMovesLineModeSetting[]	= TEXT("Detect Moves Line Mode");

const TCHAR UserSettings::encodingsCheckSetting[]		= TEXT("Check Encodings");
const TCHAR UserSettings::promptCloseOnMatchSetting[]	= TEXT("Prompt to Close on Match");
const TCHAR UserSettings::alignReplacementsSetting[]	= TEXT("Align Replacements");
const TCHAR UserSettings::wrapAroundSetting[]			= TEXT("Wrap Around");
const TCHAR UserSettings::reCompareOnSaveSetting[]		= TEXT("Re-Compare on Save");
const TCHAR UserSettings::gotoFirstDiffSetting[]		= TEXT("Go to First Diff");
const TCHAR UserSettings::updateOnChangeSetting[]		= TEXT("Update on Change");
const TCHAR UserSettings::compactNavBarSetting[]		= TEXT("Compact NavBar");

const TCHAR UserSettings::ignoreSpacesSetting[]			= TEXT("Ignore Spaces");
const TCHAR UserSettings::ignoreCaseSetting[]			= TEXT("Ignore Case");
const TCHAR UserSettings::detectMovesSetting[]			= TEXT("Detect Moves");
const TCHAR UserSettings::navBarSetting[]				= TEXT("Navigation Bar");

const TCHAR UserSettings::colorsSection[]				= TEXT("Colors");

const TCHAR UserSettings::addedColorSetting[]			= TEXT("Added");
const TCHAR UserSettings::removedColorSetting[]			= TEXT("Removed");
const TCHAR UserSettings::changedColorSetting[]			= TEXT("Changed");
const TCHAR UserSettings::movedColorSetting[]			= TEXT("Moved");
const TCHAR UserSettings::highlightColorSetting[]		= TEXT("Highlight");
const TCHAR UserSettings::highlightAlphaSetting[]		= TEXT("Alpha");


void UserSettings::load()
{
	TCHAR iniFile[MAX_PATH];

	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)_countof(iniFile), (LPARAM)iniFile);

	::PathAppend(iniFile, TEXT("ComparePlugin.ini"));

	OldFileIsFirst			= ::GetPrivateProfileInt(mainSection, oldIsFirstSetting,
			DEFAULT_OLD_IS_FIRST, iniFile) == 1;
	OldFileViewId			= ::GetPrivateProfileInt(mainSection, oldFileOnLeftSetting,
			DEFAULT_OLD_ON_LEFT, iniFile) == 1 ? MAIN_VIEW : SUB_VIEW;
	CompareToPrev			= ::GetPrivateProfileInt(mainSection, compareToPrevSetting,
			DEFAULT_COMPARE_TO_PREV, iniFile) == 1;
	DetectMovesLineMode		= ::GetPrivateProfileInt(mainSection, detectMovesLineModeSetting,
			DEFAULT_DETECT_MOVE_LINE_MODE, iniFile) == 1;
	EncodingsCheck			= ::GetPrivateProfileInt(mainSection, encodingsCheckSetting,
			DEFAULT_ENCODINGS_CHECK, iniFile) == 1;
	PromptToCloseOnMatch	= ::GetPrivateProfileInt(mainSection, promptCloseOnMatchSetting,
			DEFAULT_PROMPT_CLOSE_ON_MATCH, iniFile) == 1;
	AlignReplacements		= ::GetPrivateProfileInt(mainSection, alignReplacementsSetting,
			DEFAULT_ALIGN_REPLACEMENTS, iniFile) == 1;
	WrapAround				= ::GetPrivateProfileInt(mainSection, wrapAroundSetting,
			DEFAULT_WRAP_AROUND, iniFile) == 1;
	RecompareOnSave			= ::GetPrivateProfileInt(mainSection, reCompareOnSaveSetting,
			DEFAULT_RECOMPARE_ON_SAVE, iniFile) == 1;
	GotoFirstDiff			= ::GetPrivateProfileInt(mainSection, gotoFirstDiffSetting,
			DEFAULT_GOTO_FIRST_DIFF, iniFile) == 1;
	UpdateOnChange			= ::GetPrivateProfileInt(mainSection, updateOnChangeSetting,
			DEFAULT_UPDATE_ON_CHANGE, iniFile) == 1;
	CompactNavBar			= ::GetPrivateProfileInt(mainSection, compactNavBarSetting,
			DEFAULT_COMPACT_NAVBAR, iniFile) == 1;

	IgnoreSpaces	= ::GetPrivateProfileInt(mainSection, ignoreSpacesSetting,	1, iniFile) == 1;
	IgnoreCase		= ::GetPrivateProfileInt(mainSection, ignoreCaseSetting,	0, iniFile) == 1;
	DetectMoves		= ::GetPrivateProfileInt(mainSection, detectMovesSetting,	1, iniFile) == 1;
	UseNavBar		= ::GetPrivateProfileInt(mainSection, navBarSetting,		1, iniFile) == 1;

	colors.added		= ::GetPrivateProfileInt(colorsSection, addedColorSetting,
			DEFAULT_ADDED_COLOR, iniFile);
	colors.deleted		= ::GetPrivateProfileInt(colorsSection, removedColorSetting,
			DEFAULT_DELETED_COLOR, iniFile);
	colors.changed		= ::GetPrivateProfileInt(colorsSection, changedColorSetting,
			DEFAULT_CHANGED_COLOR, iniFile);
	colors.moved		= ::GetPrivateProfileInt(colorsSection, movedColorSetting,
			DEFAULT_MOVED_COLOR, iniFile);
	colors.highlight	= ::GetPrivateProfileInt(colorsSection, highlightColorSetting,
			DEFAULT_HIGHLIGHT_COLOR, iniFile);
	colors.alpha		= ::GetPrivateProfileInt(colorsSection, highlightAlphaSetting,
			DEFAULT_HIGHLIGHT_ALPHA, iniFile);

	dirty = false;
}


void UserSettings::save()
{
	if (!dirty)
		return;

	TCHAR iniFile[MAX_PATH];

	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)_countof(iniFile), (LPARAM)iniFile);
	::PathAppend(iniFile, TEXT("ComparePlugin.ini"));

	::WritePrivateProfileString(mainSection, oldIsFirstSetting,
			OldFileIsFirst ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, oldFileOnLeftSetting,
			OldFileViewId == MAIN_VIEW ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, compareToPrevSetting,
			CompareToPrev ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, detectMovesLineModeSetting,
			DetectMovesLineMode ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, encodingsCheckSetting,
			EncodingsCheck ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, promptCloseOnMatchSetting,
			PromptToCloseOnMatch ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, alignReplacementsSetting,
			AlignReplacements ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, wrapAroundSetting,
			WrapAround ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, reCompareOnSaveSetting,
			RecompareOnSave ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, gotoFirstDiffSetting,
			GotoFirstDiff ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, updateOnChangeSetting,
			UpdateOnChange ? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, compactNavBarSetting,
			CompactNavBar ? TEXT("1") : TEXT("0"), iniFile);

	::WritePrivateProfileString(mainSection, ignoreSpacesSetting,	IgnoreSpaces	? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, ignoreCaseSetting,	    IgnoreCase		? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, detectMovesSetting,	DetectMoves		? TEXT("1") : TEXT("0"), iniFile);
	::WritePrivateProfileString(mainSection, navBarSetting,			UseNavBar		? TEXT("1") : TEXT("0"), iniFile);

	TCHAR buffer[64];

	_itot_s(colors.added, buffer, 64, 10);
	::WritePrivateProfileString(colorsSection, addedColorSetting, buffer, iniFile);

	_itot_s(colors.deleted, buffer, 64, 10);
	::WritePrivateProfileString(colorsSection, removedColorSetting, buffer, iniFile);

	_itot_s(colors.changed, buffer, 64, 10);
	::WritePrivateProfileString(colorsSection, changedColorSetting, buffer, iniFile);

	_itot_s(colors.moved, buffer, 64, 10);
	::WritePrivateProfileString(colorsSection, movedColorSetting, buffer, iniFile);

	_itot_s(colors.highlight, buffer, 64, 10);
	::WritePrivateProfileString(colorsSection, highlightColorSetting, buffer, iniFile);

	_itot_s(colors.alpha, buffer, 64, 10);
	::WritePrivateProfileString(colorsSection, highlightAlphaSetting, buffer, iniFile);

	dirty = false;
}


namespace // anonymous namespace
{

const TCHAR PLUGIN_NAME[] = TEXT("Compare");


/**
 *  \class
 *  \brief
 */
class NppSettings
{
public:
	static NppSettings& get()
	{
		static NppSettings instance;
		return instance;
	}

	void enableClearCommands(bool enable) const;
	void enableNppScrollCommands(bool enable) const;
	void updatePluginMenu();
	void save();
	void setNormalMode();
	void setCompareMode(bool clearHorizontalScroll = false);
	void toSingleLineTab();
	void restoreMultilineTab();

	bool	compareMode;

private:
	NppSettings() : compareMode(false), _restoreMultilineTab(false) {}

	void refreshTabBar(HWND hTabBar);
	void refreshTabBars();

	bool	_restoreMultilineTab;

	bool	_syncVScroll;
	bool	_syncHScroll;
};


/**
 *  \struct
 *  \brief
 */
struct DeletedSection
{
	DeletedSection(int action, int line, int len) : startLine(line), lineReplace(false)
	{
		restoreAction = (action == SC_PERFORMED_UNDO) ? SC_PERFORMED_REDO : SC_PERFORMED_UNDO;

		markers.resize(len, 0);
	}

	int					startLine;
	bool				lineReplace;
	int					restoreAction;
	std::vector<int>	markers;
};


/**
 *  \struct
 *  \brief
 */
struct DeletedSectionsList
{
	DeletedSectionsList() : skipPush(0), lastPushTimeMark(0) {}

	void push(int currAction, int startLine, int endLine);
	void pop(int currAction, int startLine);

	void clear()
	{
		skipPush = 0;
		sections.clear();
	}

private:
	int							skipPush;
	DWORD						lastPushTimeMark;
	std::vector<DeletedSection>	sections;
};


void DeletedSectionsList::push(int currAction, int startLine, int endLine)
{
	if (endLine <= startLine)
		return;

	if (skipPush)
	{
		--skipPush;
		return;
	}

	// Is it line replacement revert operation?
	if (!sections.empty() && sections.back().restoreAction == currAction && sections.back().lineReplace)
		return;

	DeletedSection delSection(currAction, startLine, endLine - startLine + 1);

	HWND currentView = getCurrentView();

	const int startPos = ::SendMessage(currentView, SCI_POSITIONFROMLINE, startLine, 0);
	clearChangedIndicator(currentView,
			startPos, ::SendMessage(currentView, SCI_POSITIONFROMLINE, endLine, 0) - startPos);

	for (int line = ::SendMessage(currentView, SCI_MARKERPREVIOUS, endLine, MARKER_MASK_LINE);
			line >= startLine; line = ::SendMessage(currentView, SCI_MARKERPREVIOUS, line - 1, MARKER_MASK_LINE))
	{
		delSection.markers[line - startLine] = ::SendMessage(currentView, SCI_MARKERGET, line, 0) & MARKER_MASK_ALL;
		if (line != endLine)
			clearMarks(currentView, line);
	}

	sections.push_back(delSection);

	lastPushTimeMark = ::GetTickCount();
}


void DeletedSectionsList::pop(int currAction, int startLine)
{
	if (sections.empty())
	{
		++skipPush;
		return;
	}

	DeletedSection& last = sections.back();

	if (last.restoreAction != currAction)
	{
		// Try to guess if this is the insert part of line replacement operation
		if (::GetTickCount() < lastPushTimeMark + 40)
			last.lineReplace = true;
		else
			++skipPush;

		return;
	}

	if (last.startLine != startLine)
		return;

	HWND currentView = getCurrentView();

	const int linesCount = static_cast<int>(last.markers.size());

	const int startPos = ::SendMessage(currentView, SCI_POSITIONFROMLINE, last.startLine, 0);
	clearChangedIndicator(currentView,
			startPos, ::SendMessage(currentView, SCI_POSITIONFROMLINE, last.startLine + linesCount, 0) - startPos);

	for (int i = 0; i < linesCount; ++i)
	{
		clearMarks(currentView, last.startLine + i);

		if (last.markers[i])
			::SendMessage(currentView, SCI_MARKERADDSET, last.startLine + i, last.markers[i]);
	}

	sections.pop_back();
}


enum Temp_t
{
	NO_TEMP = 0,
	LAST_SAVED_TEMP,
	SVN_TEMP,
	GIT_TEMP
};


/**
 *  \class
 *  \brief
 */
class ComparedFile
{
public:
	ComparedFile() : isTemp(NO_TEMP) {}

	void initFromCurrent(bool currFileIsNew);
	void updateFromCurrent();
	void updateView();
	void clear();
	void clear(const section_t& section);
	void onBeforeClose() const;
	void onClose() const;
	void close() const;
	void restore() const;
	bool isOpen() const;

    inline void removeAndStoreBlanks();
    inline void restoreBlanks();
    inline void clearStoredBlanks();

	Temp_t	isTemp;
	bool	isNew;

	int		originalViewId;
	int		originalPos;
	int		compareViewId;

	LRESULT	buffId;
	int		sciDoc;
	TCHAR	name[MAX_PATH];

	DeletedSectionsList deletedSections;

private:
	BlankSections_t     blankSections;
};


/**
 *  \class
 *  \brief
 */
class ComparedPair
{
public:
	inline ComparedFile& getFileByViewId(int viewId);
	inline ComparedFile& getFileByBuffId(LRESULT buffId);
	inline ComparedFile& getOtherFileByBuffId(LRESULT buffId);
	inline ComparedFile& getFileBySciDoc(int sciDoc);
	inline ComparedFile& getOldFile();
	inline ComparedFile& getNewFile();

	void positionFiles();
	void restoreFiles(int currentBuffId);

	void setStatus();

	ComparedFile	file[2];
	int				relativePos;

	bool			isFullCompare;
	bool			SpacesIgnored;
	bool			CaseIgnored;
	bool			MovesDetected;
};


/**
 *  \class
 *  \brief
 */
class NewCompare
{
public:
	NewCompare(bool currFileIsNew, bool markFirstName);
	~NewCompare();

	ComparedPair	pair;

private:
	TCHAR	_firstTabText[64];
};


using CompareList_t = std::vector<ComparedPair>;


enum class CompareResult
{
	COMPARE_ERROR,
	COMPARE_CANCELLED,
	COMPARE_MATCH,
	COMPARE_MISMATCH
};


/**
 *  \class
 *  \brief
 */
class DelayedActivate : public DelayedWork
{
public:
	DelayedActivate() : DelayedWork() {}
	virtual void operator()();

	inline void operator()(LRESULT buff)
	{
		buffId = buff;
		operator()();
	}

	LRESULT buffId;
};


/**
 *  \class
 *  \brief
 */
class DelayedClose : public DelayedWork
{
public:
	DelayedClose() : DelayedWork() {}
	virtual void operator()();

	std::vector<LRESULT> closedBuffs;
};


/**
 *  \class
 *  \brief
 */
class DelayedUpdate : public DelayedWork
{
public:
	DelayedUpdate() : DelayedWork(), linesAdded(0), linesDeleted(0), fullCompare(false) {}
	virtual void operator()();

	int		changePos;
	int		linesAdded;
	int		linesDeleted;

	bool	fullCompare;
};


/**
 *  \struct
 *  \brief
 */
struct TempMark_t
{
	const TCHAR*	fileMark;
	const TCHAR*	tabMark;
};


static const TempMark_t tempMark[] =
{
	{ TEXT(""),				TEXT("") },
	{ TEXT("_LastSave"),	TEXT(" ** Last Save") },
	{ TEXT("_SVN"),			TEXT(" ** SVN") },
	{ TEXT("_Git"),			TEXT(" ** Git") }
};


CompareList_t compareList;
std::unique_ptr<NewCompare> newCompare;

volatile unsigned notificationsLock = 0;

DelayedActivate	delayedActivation;
DelayedClose	delayedClosure;
DelayedUpdate	delayedUpdate;

AboutDialog   	AboutDlg;
SettingsDialog	SettingsDlg;
NavDialog     	NavDlg;

toolbarIcons  tbSetFirst;
toolbarIcons  tbCompare;
toolbarIcons  tbCompareLines;
toolbarIcons  tbClearCompare;
toolbarIcons  tbFirst;
toolbarIcons  tbPrev;
toolbarIcons  tbNext;
toolbarIcons  tbLast;
toolbarIcons  tbNavBar;

HINSTANCE hInstance;
FuncItem funcItem[NB_MENU_COMMANDS] = { 0 };

// Declare local functions that appear before they are defined
void First();
void onBufferActivated(LRESULT buffId, bool delay = true);
void forceViewsSync(HWND focalView, bool syncCurrentLine = true);


void NppSettings::enableClearCommands(bool enable) const
{
	HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, NPPPLUGINMENU, 0);

	::EnableMenuItem(hMenu, funcItem[CMD_CLEAR_ACTIVE]._cmdID,
			MF_BYCOMMAND | ((!enable && !compareMode) ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED));

	::EnableMenuItem(hMenu, funcItem[CMD_CLEAR_ALL]._cmdID,
			MF_BYCOMMAND | ((!enable && compareList.empty()) ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED));

	HWND hNppToolbar = NppToolbarHandleGetter::get();
	if (hNppToolbar)
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, funcItem[CMD_CLEAR_ACTIVE]._cmdID, enable || compareMode);
}


void NppSettings::enableNppScrollCommands(bool enable) const
{
	HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, NPPMAINMENU, 0);
	const int flag = MF_BYCOMMAND | (enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED));

	::EnableMenuItem(hMenu, IDM_VIEW_SYNSCROLLH, flag);
	::EnableMenuItem(hMenu, IDM_VIEW_SYNSCROLLV, flag);

	HWND hNppToolbar = NppToolbarHandleGetter::get();
	if (hNppToolbar)
	{
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, IDM_VIEW_SYNSCROLLH, enable);
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, IDM_VIEW_SYNSCROLLV, enable);
	}
}


void NppSettings::updatePluginMenu()
{
	HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, NPPPLUGINMENU, 0);
	const int flag = MF_BYCOMMAND | (compareMode ? MF_ENABLED : (MF_DISABLED | MF_GRAYED));

	::EnableMenuItem(hMenu, funcItem[CMD_CLEAR_ACTIVE]._cmdID,
			MF_BYCOMMAND | ((!compareMode && !newCompare) ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED));

	::EnableMenuItem(hMenu, funcItem[CMD_CLEAR_ALL]._cmdID,
			MF_BYCOMMAND | ((compareList.empty() && !newCompare) ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED));

	if (compareList.empty())
		restoreMultilineTab();

	::EnableMenuItem(hMenu, funcItem[CMD_FIRST]._cmdID, flag);
	::EnableMenuItem(hMenu, funcItem[CMD_PREV]._cmdID, flag);
	::EnableMenuItem(hMenu, funcItem[CMD_NEXT]._cmdID, flag);
	::EnableMenuItem(hMenu, funcItem[CMD_LAST]._cmdID, flag);

	HWND hNppToolbar = NppToolbarHandleGetter::get();
	if (hNppToolbar)
	{
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, funcItem[CMD_CLEAR_ACTIVE]._cmdID, compareMode || newCompare);
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, funcItem[CMD_FIRST]._cmdID, compareMode);
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, funcItem[CMD_PREV]._cmdID, compareMode);
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, funcItem[CMD_NEXT]._cmdID, compareMode);
		::SendMessage(hNppToolbar, TB_ENABLEBUTTON, funcItem[CMD_LAST]._cmdID, compareMode);
	}
}


void NppSettings::save()
{
	HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, NPPMAINMENU, 0);

	_syncVScroll = (::GetMenuState(hMenu, IDM_VIEW_SYNSCROLLV, MF_BYCOMMAND) & MF_CHECKED) != 0;
	_syncHScroll = (::GetMenuState(hMenu, IDM_VIEW_SYNSCROLLH, MF_BYCOMMAND) & MF_CHECKED) != 0;
}


void NppSettings::setNormalMode()
{
	if (compareMode == false)
		return;

	compareMode = false;

	if (NavDlg.isVisible())
		NavDlg.Hide();

	if (!isSingleView())
	{
		enableNppScrollCommands(true);

		HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, NPPMAINMENU, 0);

		bool syncScroll = (::GetMenuState(hMenu, IDM_VIEW_SYNSCROLLV, MF_BYCOMMAND) & MF_CHECKED) != 0;
		if (syncScroll != _syncVScroll)
			::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_SYNSCROLLV);

		syncScroll = (::GetMenuState(hMenu, IDM_VIEW_SYNSCROLLH, MF_BYCOMMAND) & MF_CHECKED) != 0;
		if (syncScroll != _syncHScroll)
			::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_SYNSCROLLH);
	}

	updatePluginMenu();
}


void NppSettings::setCompareMode(bool clearHorizontalScroll)
{
	if (compareMode == true)
		return;

	compareMode = true;

	save();

	if (clearHorizontalScroll)
	{
		int pos = ::SendMessage(nppData._scintillaMainHandle, SCI_POSITIONFROMLINE,
				getCurrentLine(nppData._scintillaMainHandle), 0);
		::SendMessage(nppData._scintillaMainHandle, SCI_SETSEL, pos, pos);

		pos = ::SendMessage(nppData._scintillaSecondHandle, SCI_POSITIONFROMLINE,
				getCurrentLine(nppData._scintillaSecondHandle), 0);
		::SendMessage(nppData._scintillaSecondHandle, SCI_SETSEL, pos, pos);
	}

	// Disable N++ vertical scroll - we handle it manually because of the Word Wrap
	if (_syncVScroll)
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_SYNSCROLLV);

	// Yaron - Enable N++ horizontal scroll sync
	if (!_syncHScroll)
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_SYNSCROLLH);

	// synchronize zoom levels
	int zoom = ::SendMessage(getCurrentView(), SCI_GETZOOM, 0, 0);
	::SendMessage(getOtherView(), SCI_SETZOOM, zoom, 0);

	enableNppScrollCommands(false);
	updatePluginMenu();
}


void NppSettings::refreshTabBar(HWND hTabBar)
{
	if (::IsWindowVisible(hTabBar) && (TabCtrl_GetItemCount(hTabBar) > 1))
	{
		const int currentTabIdx = TabCtrl_GetCurSel(hTabBar);

		TabCtrl_SetCurFocus(hTabBar, (currentTabIdx) ? 0 : 1);
		TabCtrl_SetCurFocus(hTabBar, currentTabIdx);
	}
}


void NppSettings::refreshTabBars()
{
	HWND currentView = getCurrentView();

	HWND hNppTabBar = NppTabHandleGetter::get(SUB_VIEW);

	if (hNppTabBar)
		refreshTabBar(hNppTabBar);

	hNppTabBar = NppTabHandleGetter::get(MAIN_VIEW);

	if (hNppTabBar)
		refreshTabBar(hNppTabBar);

	::SetFocus(currentView);
}


void NppSettings::toSingleLineTab()
{
	if (!_restoreMultilineTab)
	{
		HWND hNppMainTabBar = NppTabHandleGetter::get(MAIN_VIEW);
		HWND hNppSubTabBar = NppTabHandleGetter::get(SUB_VIEW);

		if (hNppMainTabBar && hNppSubTabBar)
		{
			RECT tabRec;
			::GetWindowRect(hNppMainTabBar, &tabRec);
			const int mainTabYPos = tabRec.top;

			::GetWindowRect(hNppSubTabBar, &tabRec);
			const int subTabYPos = tabRec.top;

			// Both views are side-by-side positioned
			if (mainTabYPos == subTabYPos)
			{
				LONG_PTR tabStyle = ::GetWindowLongPtr(hNppMainTabBar, GWL_STYLE);

				if ((tabStyle & TCS_MULTILINE) && !(tabStyle & TCS_VERTICAL))
				{
					::SetWindowLongPtr(hNppMainTabBar, GWL_STYLE, tabStyle & ~TCS_MULTILINE);
					::SendMessage(hNppMainTabBar, WM_TABSETSTYLE, 0, 0);

					tabStyle = ::GetWindowLongPtr(hNppSubTabBar, GWL_STYLE);
					::SetWindowLongPtr(hNppSubTabBar, GWL_STYLE, tabStyle & ~TCS_MULTILINE);
					::SendMessage(hNppSubTabBar, WM_TABSETSTYLE, 0, 0);

					refreshTabBars();

					_restoreMultilineTab = true;
				}
			}
		}
	}
}


void NppSettings::restoreMultilineTab()
{
	if (_restoreMultilineTab)
	{
		_restoreMultilineTab = false;

		HWND hNppMainTabBar = NppTabHandleGetter::get(MAIN_VIEW);
		HWND hNppSubTabBar = NppTabHandleGetter::get(SUB_VIEW);

		if (hNppMainTabBar && hNppSubTabBar)
		{
			LONG_PTR tabStyle = ::GetWindowLongPtr(hNppMainTabBar, GWL_STYLE);
			::SetWindowLongPtr(hNppMainTabBar, GWL_STYLE, tabStyle | TCS_MULTILINE);
			::SendMessage(hNppMainTabBar, WM_TABSETSTYLE, 0, 0);

			tabStyle = ::GetWindowLongPtr(hNppSubTabBar, GWL_STYLE);
			::SetWindowLongPtr(hNppSubTabBar, GWL_STYLE, tabStyle | TCS_MULTILINE);
			::SendMessage(hNppSubTabBar, WM_TABSETSTYLE, 0, 0);

			refreshTabBars();
		}
	}
}


void ComparedFile::initFromCurrent(bool currFileIsNew)
{
	isNew = currFileIsNew;
	buffId = getCurrentBuffId();
	originalViewId = getCurrentViewId();
	compareViewId = originalViewId;
	originalPos = posFromBuffId(buffId);
	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, _countof(name), (LPARAM)name);

	updateFromCurrent();
}


void ComparedFile::updateFromCurrent()
{
	sciDoc = getDocId(getCurrentView());

	if (isTemp)
	{
		HWND hNppTabBar = NppTabHandleGetter::get(getCurrentViewId());

		if (hNppTabBar)
		{
			TCHAR tabName[MAX_PATH];
			_tcscpy_s(tabName, _countof(tabName), ::PathFindFileName(name));

			int i = _tcslen(tabName) - 1 - _tcslen(tempMark[isTemp].fileMark);
			for (; i > 0 && tabName[i] != TEXT('_'); --i);

			if (i > 0)
			{
				tabName[i] = 0;
				_tcscat_s(tabName, _countof(tabName), tempMark[isTemp].tabMark);

				TCITEM tab;
				tab.mask = TCIF_TEXT;
				tab.pszText = tabName;

				TabCtrl_SetItem(hNppTabBar, posFromBuffId(buffId), &tab);
			}
		}
	}
}


void ComparedFile::updateView()
{
	compareViewId = isNew ? ((Settings.OldFileViewId == MAIN_VIEW) ? SUB_VIEW : MAIN_VIEW) : Settings.OldFileViewId;
}


void ComparedFile::clear()
{
	clearWindow(getView(viewIdFromBuffId(buffId)));

	deletedSections.clear();
}


void ComparedFile::clear(const section_t& section)
{
	clearMarksAndBlanks(getView(viewIdFromBuffId(buffId)), section.off, section.len);

	deletedSections.clear();
}


void ComparedFile::onBeforeClose() const
{
	if (buffId != getCurrentBuffId())
		activateBufferID(buffId);

	HWND view = getCurrentView();
	clearWindow(view);

	if (isTemp)
		::SendMessage(view, SCI_SETSAVEPOINT, true, 0);
}


void ComparedFile::onClose() const
{
	if (isTemp)
	{
		::SetFileAttributes(name, FILE_ATTRIBUTE_NORMAL);
		::DeleteFile(name);
	}
}


void ComparedFile::close() const
{
	onBeforeClose();

	::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);

	onClose();
}


void ComparedFile::restore() const
{
	if (isTemp)
	{
		close();
		return;
	}

	if (buffId != getCurrentBuffId())
		activateBufferID(buffId);

	clearWindow(getCurrentView());

	if (viewIdFromBuffId(buffId) != originalViewId)
	{
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_GOTO_ANOTHER_VIEW);

		if (!isOpen())
			return;

		const int currentPos = posFromBuffId(buffId);

		if (originalPos >= currentPos)
			return;

		for (int i = currentPos - originalPos; i; --i)
			::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_TAB_MOVEBACKWARD);
	}
}


bool ComparedFile::isOpen() const
{
	return (::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, buffId, (LPARAM)NULL) >= 0);
}


void ComparedFile::removeAndStoreBlanks()
{
    blankSections = removeBlankLines(getView(viewIdFromBuffId(buffId)), true);
}


void ComparedFile::restoreBlanks()
{
    if (!blankSections.empty())
        addBlankLines(getView(viewIdFromBuffId(buffId)), blankSections);
}


void ComparedFile::clearStoredBlanks()
{
    blankSections.clear();
}


ComparedFile& ComparedPair::getFileByViewId(int viewId)
{
	return (viewIdFromBuffId(file[0].buffId) == viewId) ? file[0] : file[1];
}


ComparedFile& ComparedPair::getFileByBuffId(LRESULT buffId)
{
	return (file[0].buffId == buffId) ? file[0] : file[1];
}


ComparedFile& ComparedPair::getOtherFileByBuffId(LRESULT buffId)
{
	return (file[0].buffId == buffId) ? file[1] : file[0];
}


ComparedFile& ComparedPair::getFileBySciDoc(int sciDoc)
{
	return (file[0].sciDoc == sciDoc) ? file[0] : file[1];
}


ComparedFile& ComparedPair::getOldFile()
{
	return file[0].isNew ? file[1] : file[0];
}


ComparedFile& ComparedPair::getNewFile()
{
	return file[0].isNew ? file[0] : file[1];
}


void ComparedPair::positionFiles()
{
	// sync both views zoom
	const int zoom = ::SendMessage(getCurrentView(), SCI_GETZOOM, 0, 0);
	::SendMessage(getOtherView(), SCI_SETZOOM, zoom, 0);

	const LRESULT currentBuffId = getCurrentBuffId();

	ComparedFile& oldFile = getOldFile();
	ComparedFile& newFile = getNewFile();

	oldFile.updateView();
	newFile.updateView();

	relativePos = (oldFile.originalViewId != newFile.originalViewId) ? 0 :
			(oldFile.originalViewId == oldFile.compareViewId) ?
			newFile.originalPos - oldFile.originalPos : oldFile.originalPos - newFile.originalPos;

	if (viewIdFromBuffId(oldFile.buffId) != oldFile.compareViewId)
	{
		if (oldFile.buffId != getCurrentBuffId())
			activateBufferID(oldFile.buffId);
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_GOTO_ANOTHER_VIEW);
		oldFile.updateFromCurrent();
	}

	if (viewIdFromBuffId(newFile.buffId) != newFile.compareViewId)
	{
		if (newFile.buffId != getCurrentBuffId())
			activateBufferID(newFile.buffId);
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_GOTO_ANOTHER_VIEW);
		newFile.updateFromCurrent();
	}

	if (oldFile.sciDoc != getDocId(getView(oldFile.compareViewId)))
		activateBufferID(oldFile.buffId);

	if (newFile.sciDoc != getDocId(getView(newFile.compareViewId)))
		activateBufferID(newFile.buffId);

	activateBufferID(currentBuffId);
}


void ComparedPair::restoreFiles(int currentBuffId = -1)
{
	// Check if position update is needed -
	// this is for relative re-positioning to keep files initial order consistent
	if (relativePos)
	{
		ComparedFile* biasFile;
		ComparedFile* movedFile;

		// One of the files is in its original view and won't be moved - this is the bias file
		if (viewIdFromBuffId(file[0].buffId) == file[0].originalViewId)
		{
			biasFile = &file[0];
			movedFile = &file[1];
		}
		else
		{
			biasFile = &file[1];
			movedFile = &file[0];
		}

		if (biasFile->originalPos > movedFile->originalPos)
		{
			const int newPos = posFromBuffId(biasFile->buffId);

			if (newPos != biasFile->originalPos && newPos < movedFile->originalPos)
				movedFile->originalPos = newPos;
		}
	}

	if (currentBuffId == -1)
	{
		file[0].restore();
		file[1].restore();
	}
	else
	{
		getOtherFileByBuffId(currentBuffId).restore();
		getFileByBuffId(currentBuffId).restore();
	}
}


void ComparedPair::setStatus()
{
	HWND hNppStatusBar = NppStatusBarHandleGetter::get();

	if (hNppStatusBar)
	{
		TCHAR msg[512];

		_sntprintf_s(msg, _countof(msg), _TRUNCATE,
				TEXT("Compare (%s)    Ignore Spaces (%s)    Ignore Case (%s)    Detect Moves (%s)"),
				isFullCompare	? TEXT("Full")	: TEXT("Sel"),
				SpacesIgnored	? TEXT("Y")	: TEXT("N"),
				CaseIgnored		? TEXT("Y")	: TEXT("N"),
				MovesDetected	? TEXT("Y")	: TEXT("N"));

		::SendMessage(hNppStatusBar, SB_SETTEXT, 0, static_cast<LPARAM>((LONG_PTR)msg));
	}
}


NewCompare::NewCompare(bool currFileIsNew, bool markFirstName)
{
	_firstTabText[0] = 0;

	pair.file[0].initFromCurrent(currFileIsNew);

	// Enable commands to be able to clear the first file that was just set
	NppSettings::get().enableClearCommands(true);

	if (markFirstName)
	{
		HWND hNppTabBar = NppTabHandleGetter::get(pair.file[0].originalViewId);

		if (hNppTabBar)
		{
			TCITEM tab;
			tab.mask = TCIF_TEXT;
			tab.pszText = _firstTabText;
			tab.cchTextMax = _countof(_firstTabText);

			TabCtrl_GetItem(hNppTabBar, pair.file[0].originalPos, &tab);

			TCHAR tabText[MAX_PATH];
			tab.pszText = tabText;

			_sntprintf_s(tabText, _countof(tabText), _TRUNCATE, TEXT("%s ** %s to Compare"),
					_firstTabText, Settings.OldFileIsFirst ? TEXT("Old") : TEXT("New"));

			TabCtrl_SetItem(hNppTabBar, pair.file[0].originalPos, &tab);
		}
	}
}


NewCompare::~NewCompare()
{
	if (_firstTabText[0] != 0)
	{
		HWND hNppTabBar = NppTabHandleGetter::get(pair.file[0].originalViewId);

		if (hNppTabBar)
		{
			// This is workaround for Wine issue with tab bar refresh
			::InvalidateRect(hNppTabBar, NULL, FALSE);

			TCITEM tab;
			tab.mask = TCIF_TEXT;
			tab.pszText = _firstTabText;

			TabCtrl_SetItem(hNppTabBar, posFromBuffId(pair.file[0].buffId), &tab);

			::UpdateWindow(hNppTabBar);
		}
	}

	if (!NppSettings::get().compareMode)
		NppSettings::get().enableClearCommands(false);
}


CompareList_t::iterator getCompare(LRESULT buffId)
{
	for (CompareList_t::iterator it = compareList.begin(); it < compareList.end(); ++it)
	{
		if (it->file[0].buffId == buffId || it->file[1].buffId == buffId)
			return it;
	}

	return compareList.end();
}


CompareList_t::iterator getCompareBySciDoc(int sciDoc)
{
	for (CompareList_t::iterator it = compareList.begin(); it < compareList.end(); ++it)
	{
		if (it->file[0].sciDoc == sciDoc || it->file[1].sciDoc == sciDoc)
			return it;
	}

	return compareList.end();
}


void resetCompareView(HWND view)
{
	if (!::IsWindowVisible(view))
		return;

	CompareList_t::iterator cmpPair = getCompareBySciDoc(getDocId(view));
	if (cmpPair != compareList.end())
		setCompareView(view);
}


void updateWrap()
{
	if (::SendMessage(nppData._scintillaMainHandle, SCI_GETWRAPMODE, 0, 0) == SC_WRAP_NONE)
		return;

	ScopedIncrementer incr(notificationsLock);

	::UpdateWindow(nppData._nppHandle);

	adjustBlanksWrap();

	::UpdateWindow(nppData._scintillaMainHandle);
	::UpdateWindow(nppData._scintillaSecondHandle);
}


void showNavBar()
{
	NavDlg.SetConfig(Settings);
	NavDlg.Show();
	updateWrap();
}


bool isFileCompared(HWND view)
{
	const int sciDoc = getDocId(view);

	CompareList_t::iterator cmpPair = getCompareBySciDoc(sciDoc);
	if (cmpPair != compareList.end())
	{
		const TCHAR* fname = ::PathFindFileName(cmpPair->getFileBySciDoc(sciDoc).name);

		TCHAR msg[MAX_PATH];
		_sntprintf_s(msg, _countof(msg), _TRUNCATE,
				TEXT("File \"%s\" is already compared - operation ignored."), fname);
		::MessageBox(nppData._nppHandle, msg, TEXT("Compare Plugin"), MB_OK);

		return true;
	}

	return false;
}


bool isEncodingOK(const ComparedPair& cmpPair)
{
	// Warn about encoding mismatches as that might compromise the compare
	if (getEncoding(cmpPair.file[0].buffId) != getEncoding(cmpPair.file[1].buffId))
	{
		if (::MessageBox(nppData._nppHandle,
			TEXT("Trying to compare files with different encodings - \n")
			TEXT("the result might be inaccurate and misleading.\n\n")
			TEXT("Compare anyway?"), TEXT("Compare Plugin"), MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
		{
			return false;
		}
	}

	return true;
}


// Call it with no arguments when re-comparing (the files are active in both views)
bool areSelectionsValid(LRESULT currentBuffId = -1, LRESULT otherBuffId = -1)
{
	HWND view1 = (currentBuffId == otherBuffId) ?
			nppData._scintillaMainHandle : getView(viewIdFromBuffId(currentBuffId));
	HWND view2 = (currentBuffId == otherBuffId) ?
			nppData._scintillaSecondHandle : getView(viewIdFromBuffId(otherBuffId));

	if (view1 == view2)
		activateBufferID(otherBuffId);

	std::pair<int, int> viewSel = getSelectionLines(view2);
	bool valid = !(viewSel.first < 0);
	bool notOnlyBlanks = true;

	if (valid && (currentBuffId == otherBuffId))
		valid = notOnlyBlanks = !areOnlyBlanks(view2, viewSel);

	if (view1 == view2)
		activateBufferID(currentBuffId);

	if (valid)
	{
		viewSel = getSelectionLines(view1);
		valid = !(viewSel.first < 0);

		if (valid && (currentBuffId == otherBuffId))
			valid = notOnlyBlanks = !areOnlyBlanks(view1, viewSel);
	}

	if (!valid)
	{
		if (notOnlyBlanks)
			::MessageBox(nppData._nppHandle, TEXT("No selected lines to compare - operation ignored."),
					TEXT("Compare Plugin"), MB_OK);
		else
			::MessageBox(nppData._nppHandle, TEXT("Only blank lines selected - operation ignored."),
					TEXT("Compare Plugin"), MB_OK);
	}

	return valid;
}


bool setFirst(bool currFileIsNew, bool markName = false)
{
	HWND view = getCurrentView();

	if (isFileCompared(view))
		return false;

	// Done on purpose: First wipe the std::unique_ptr so ~NewCompare is called before the new object constructor.
	// This is important because the N++ plugin menu is updated on NewCompare construct/destruct.
	newCompare.reset();
	newCompare.reset(new NewCompare(currFileIsNew, markName));

	return true;
}


void setContent(const char* content)
{
	HWND view = getCurrentView();

	ScopedViewUndoCollectionBlocker undoBlock(view);
	ScopedViewWriteEnabler writeEn(view);

	::SendMessage(view, SCI_SETTEXT, 0, (LPARAM)content);
	::SendMessage(view, SCI_SETSAVEPOINT, true, 0);
}


bool createTempFile(const TCHAR *file, Temp_t tempType)
{
	if (::PathFileExists(file) == FALSE)
	{
		::MessageBox(nppData._nppHandle, TEXT("File is not written to disk - operation ignored."),
				TEXT("Compare Plugin"), MB_OK);
		return false;
	}

	if (!setFirst(true))
		return false;

	TCHAR tempFile[MAX_PATH];

	if (::GetTempPath(_countof(tempFile), tempFile))
	{
		const TCHAR* fileName = ::PathFindFileName(newCompare->pair.file[0].name);

		if (::PathAppend(tempFile, fileName))
		{
			_tcscat_s(tempFile, _countof(tempFile), tempMark[tempType].fileMark);

			unsigned idxPos = _tcslen(tempFile);

			// Make sure temp file is unique
			for (int i = 1; ; ++i)
			{
				TCHAR idx[32];

				_itot_s(i, idx, _countof(idx), 10);

				if (_tcslen(idx) + idxPos + 1 > _countof(tempFile))
				{
					idxPos = _countof(tempFile);
					break;
				}

				_tcscat_s(tempFile, _countof(tempFile), idx);

				if (!::PathFileExists(tempFile))
					break;

				tempFile[idxPos] = 0;
			}

			if ((idxPos + 1 <= _countof(tempFile)) && ::CopyFile(file, tempFile, TRUE))
			{
				::SetFileAttributes(tempFile, FILE_ATTRIBUTE_TEMPORARY);

				const int langType = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERLANGTYPE,
						newCompare->pair.file[0].buffId, 0);

				ScopedIncrementer incr(notificationsLock);

				if (::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)tempFile))
				{
					const LRESULT buffId = getCurrentBuffId();

					::SendMessage(nppData._nppHandle, NPPM_SETBUFFERLANGTYPE, buffId, langType);
					::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_EDIT_SETREADONLY);

					newCompare->pair.file[1].isTemp = tempType;

					return true;
				}
			}
		}
	}

	::MessageBox(nppData._nppHandle, TEXT("Creating temp file failed - operation aborted."),
			TEXT("Compare Plugin"), MB_OK);

	newCompare.reset();

	return false;
}


void clearComparePair(LRESULT buffId)
{
	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
		return;

	ScopedIncrementer incr(notificationsLock);

	cmpPair->restoreFiles(buffId);

	compareList.erase(cmpPair);

	onBufferActivated(getCurrentBuffId(), false);
}


void closeComparePair(CompareList_t::iterator& cmpPair)
{
	HWND currentView = getCurrentView();

	ScopedIncrementer incr(notificationsLock);

	// First close the file in the SUB_VIEW as closing a file may lead to a single view mode
	// and if that happens we want to be in single main view
	cmpPair->getFileByViewId(SUB_VIEW).close();
	cmpPair->getFileByViewId(MAIN_VIEW).close();

	compareList.erase(cmpPair);

	if (::IsWindowVisible(currentView))
		::SetFocus(currentView);

	onBufferActivated(getCurrentBuffId(), false);
}


bool initNewCompare()
{
	bool firstIsSet = (bool)newCompare;

	// Compare to self?
	if (firstIsSet && (newCompare->pair.file[0].buffId == getCurrentBuffId()))
		firstIsSet = false;

	if (!firstIsSet)
	{
		const bool singleView = isSingleView();
		const bool isNew = singleView ? true : getCurrentViewId() != Settings.OldFileViewId;

		if (!setFirst(isNew))
			return false;

		if (singleView)
		{
			if (getNumberOfFiles(getCurrentViewId()) < 2)
			{
				::MessageBox(nppData._nppHandle, TEXT("Only one file opened - operation ignored."),
						TEXT("Compare Plugin"), MB_OK);
				return false;
			}

			::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0,
					Settings.CompareToPrev ? IDM_VIEW_TAB_PREV : IDM_VIEW_TAB_NEXT);
		}
		else
		{
			HWND otherView = getOtherView();

			// Check if the file in the other view is compared already
			if (isFileCompared(otherView))
				return false;

			::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_SWITCHTO_OTHER_VIEW);
		}
	}

	newCompare->pair.file[1].initFromCurrent(!newCompare->pair.file[0].isNew);

	return true;
}


CompareList_t::iterator addComparePair()
{
	compareList.push_back(newCompare->pair);
	newCompare.reset();

	return compareList.end() - 1;
}


CompareResult compareViews(progress_ptr& progress, section_t mainViewSection, section_t subViewSection)
{
	DocCmpInfo doc1;
	DocCmpInfo doc2;

	if (Settings.OldFileViewId == MAIN_VIEW)
	{
		doc1.view		= nppData._scintillaMainHandle;
		doc1.section	= mainViewSection;
		doc2.view		= nppData._scintillaSecondHandle;
		doc2.section	= subViewSection;
	}
	else
	{
		doc2.view		= nppData._scintillaMainHandle;
		doc2.section	= mainViewSection;
		doc1.view		= nppData._scintillaSecondHandle;
		doc1.section	= subViewSection;
	}

	std::pair<std::vector<diff_info>, bool> cmpResults = compareDocs(doc1, doc2, Settings, progress);
	std::vector<diff_info>& blockDiff = cmpResults.first;

	if (progress && !progress->NextPhase())
		return CompareResult::COMPARE_CANCELLED;

	const int blockDiffSize = static_cast<int>(blockDiff.size());

	if (blockDiffSize == 0)
		return CompareResult::COMPARE_MATCH;

	if (progress)
		progress->SetMaxCount(blockDiffSize);

	// Do block compares
	for (int i = 0; i < blockDiffSize; ++i)
	{
		if (blockDiff[i].type == diff_type::DIFF_INSERT)
		{
			if (i > 0) // Should always be the case but check it anyway for safety
			{
				diff_info& blockDiff1 = blockDiff[i - 1];
				diff_info& blockDiff2 = blockDiff[i];

				// Check if the DELETE/INSERT pair includes changed lines or it's a completely new block
				if (blockDiff1.type == diff_type::DIFF_DELETE)
				{
					blockDiff1.matchedDiff = &blockDiff2;
					blockDiff2.matchedDiff = &blockDiff1;

					compareBlocks(doc1, doc2, Settings, blockDiff1, blockDiff2);
				}
			}
		}

		if (progress && !progress->Advance())
			return CompareResult::COMPARE_CANCELLED;
	}

	if (progress && !progress->NextPhase())
		return CompareResult::COMPARE_CANCELLED;

	if (!showDiffs(doc1, doc2, Settings, cmpResults, progress))
		return CompareResult::COMPARE_CANCELLED;

	return CompareResult::COMPARE_MISMATCH;
}


CompareResult runCompare(CompareList_t::iterator& cmpPair, bool selectionCompare)
{
	cmpPair->positionFiles();

	section_t mainViewSection = { 0, 0 };
	section_t subViewSection = { 0, 0 };

	if (selectionCompare)
	{
		const std::pair<int, int> mainViewSel = getSelectionLines(nppData._scintillaMainHandle);
		const std::pair<int, int> subViewSel = getSelectionLines(nppData._scintillaSecondHandle);

		mainViewSection.off = mainViewSel.first;
		mainViewSection.len = mainViewSel.second - mainViewSel.first + 1;

		subViewSection.off = subViewSel.first;
		subViewSection.len = subViewSel.second - subViewSel.first + 1;
	}

	setStyles(Settings);

	CompareResult result = CompareResult::COMPARE_ERROR;

	progress_ptr& progress = ProgressDlg::Open();

	if (progress)
	{
		const TCHAR* newName = ::PathFindFileName(cmpPair->getNewFile().name);
		const TCHAR* oldName = ::PathFindFileName(cmpPair->getOldFile().name);

		TCHAR msg[MAX_PATH];
		_sntprintf_s(msg, _countof(msg), _TRUNCATE, selectionCompare ?
				TEXT("Comparing selected lines in \"%s\" vs. selected lines in \"%s\"...") :
				TEXT("Comparing \"%s\" vs. \"%s\"..."), newName, oldName);

		progress->SetInfo(msg);
	}

	try
	{
		result = compareViews(progress, mainViewSection, subViewSection);
		progress.reset();
	}
	catch (std::exception& e)
	{
		progress.reset();

		char msg[128];
		_snprintf_s(msg, _countof(msg), _TRUNCATE, "Exception occurred: %s", e.what());
		MessageBoxA(nppData._nppHandle, msg, "Compare Plugin", MB_OK | MB_ICONWARNING);
	}
	catch (...)
	{
		progress.reset();

		MessageBoxA(nppData._nppHandle, "Unknown exception occurred.", "Compare Plugin", MB_OK | MB_ICONWARNING);
	}

	return result;
}


void compare(bool selectionCompare = false)
{
	ScopedIncrementer incr(notificationsLock);

	const bool				doubleView		= !isSingleView();
	const LRESULT			currentBuffId	= getCurrentBuffId();
	CompareList_t::iterator	cmpPair			= getCompare(currentBuffId);
	const bool				recompare		= (cmpPair != compareList.end());

	ViewLocation location;

	if (recompare)
	{
		newCompare.reset();

		if (selectionCompare && !areSelectionsValid())
			return;

		location.save(currentBuffId);

		cmpPair->getOldFile().clear();
		cmpPair->getNewFile().clear();
	}
	// New compare
	else
	{
		if (!initNewCompare())
		{
			newCompare.reset();
			return;
		}

		cmpPair = addComparePair();

		if (cmpPair->getOldFile().isTemp)
		{
			activateBufferID(cmpPair->getNewFile().buffId);
		}
		else
		{
			activateBufferID(currentBuffId);

			if (selectionCompare &&
				!areSelectionsValid(currentBuffId, cmpPair->getOtherFileByBuffId(currentBuffId).buffId))
			{
				compareList.erase(cmpPair);
				return;
			}
		}

		if (Settings.EncodingsCheck && !isEncodingOK(*cmpPair))
		{
			clearComparePair(getCurrentBuffId());
			return;
		}
	}

	CompareResult cmpResult = runCompare(cmpPair, selectionCompare);

	switch (cmpResult)
	{
		case CompareResult::COMPARE_MISMATCH:
		{
			cmpPair->isFullCompare	= !selectionCompare;
			cmpPair->SpacesIgnored	= Settings.IgnoreSpaces;
			cmpPair->CaseIgnored	= Settings.IgnoreCase;
			cmpPair->MovesDetected	= Settings.DetectMoves;

			cmpPair->setStatus();

			NppSettings::get().setCompareMode(true);

			setCompareView(nppData._scintillaMainHandle);
			setCompareView(nppData._scintillaSecondHandle);

			NppSettings::get().toSingleLineTab();

			if (Settings.UseNavBar)
				showNavBar();
			else
				updateWrap();

			if (!Settings.GotoFirstDiff && recompare)
			{
				location.restore();
			}
			else
			{
				if (!doubleView)
					activateBufferID(cmpPair->getNewFile().buffId);

				First();
			}

			if (selectionCompare)
				clearSelection(getOtherView());

			// Synchronize views
			forceViewsSync(getCurrentView());
		}
		break;

		case CompareResult::COMPARE_MATCH:
		{
			const ComparedFile& oldFile = cmpPair->getOldFile();

			const TCHAR* newName = ::PathFindFileName(cmpPair->getNewFile().name);

			TCHAR msg[2 * MAX_PATH];

			int choice = IDNO;

			if (oldFile.isTemp)
			{
				if (recompare)
				{
					_sntprintf_s(msg, _countof(msg), _TRUNCATE,
							TEXT("%s \"%s\" and \"%s\" match.\n\nTemp file will be closed."), selectionCompare ?
							TEXT("Selected lines in files") : TEXT("Files"),
							newName, ::PathFindFileName(oldFile.name));
				}
				else
				{
					if (oldFile.isTemp == LAST_SAVED_TEMP)
						_sntprintf_s(msg, _countof(msg), _TRUNCATE,
								TEXT("File \"%s\" has not been modified since last Save."), newName);
					else
						_sntprintf_s(msg, _countof(msg), _TRUNCATE,
								TEXT("File \"%s\" has no changes against %s."), newName,
								oldFile.isTemp == GIT_TEMP ? TEXT("Git") : TEXT("SVN"));
				}

				::MessageBox(nppData._nppHandle, msg, TEXT("Compare Plugin"), MB_OK);
			}
			else
			{
				_sntprintf_s(msg, _countof(msg), _TRUNCATE,
						TEXT("%s \"%s\" and \"%s\" match.%s"),
						selectionCompare ? TEXT("Selected lines in files") : TEXT("Files"),
						newName, ::PathFindFileName(oldFile.name),
						Settings.PromptToCloseOnMatch ? TEXT("\n\nClose compared files?") : TEXT(""));

				if (Settings.PromptToCloseOnMatch)
					choice = ::MessageBox(nppData._nppHandle, msg, TEXT("Compare Plugin"),
							MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1);
				else
					::MessageBox(nppData._nppHandle, msg, TEXT("Compare Plugin"), MB_OK);
			}

			if (choice == IDYES)
				closeComparePair(cmpPair);
			else
				clearComparePair(getCurrentBuffId());
		}
		break;

		default:
			clearComparePair(getCurrentBuffId());
	}
}


void SetAsFirst()
{
	if (!setFirst(!Settings.OldFileIsFirst, true))
		newCompare.reset();
}


void CompareWhole()
{
	compare();
}


void CompareSelectedLines()
{
	compare(true);
}


void ClearActiveCompare()
{
	newCompare.reset();

	if (NppSettings::get().compareMode)
		clearComparePair(getCurrentBuffId());
}


void ClearAllCompares()
{
	newCompare.reset();

	if (!compareList.size())
		return;

	const LRESULT buffId = getCurrentBuffId();

	ScopedIncrementer incr(notificationsLock);

	::SetFocus(getOtherView());

	const LRESULT otherBuffId = getCurrentBuffId();

	for (int i = static_cast<int>(compareList.size()) - 1; i >= 0; --i)
		compareList[i].restoreFiles();

	compareList.clear();

	NppSettings::get().setNormalMode();

	if (!isSingleView())
		activateBufferID(otherBuffId);

	activateBufferID(buffId);
}


void LastSaveDiff()
{
	TCHAR file[MAX_PATH];

	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, _countof(file), (LPARAM)file);

	if (createTempFile(file, LAST_SAVED_TEMP))
		compare();
}


void SvnDiff()
{
	TCHAR file[MAX_PATH];
	TCHAR svnFile[MAX_PATH];

	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, _countof(file), (LPARAM)file);

	if (!GetSvnFile(file, svnFile, _countof(svnFile)))
		return;

	if (createTempFile(svnFile, SVN_TEMP))
		compare();
}


void GitDiff()
{
	TCHAR file[MAX_PATH];

	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, _countof(file), (LPARAM)file);

	std::vector<char> content = GetGitFileContent(file);

	if (content.empty())
		return;

	if (!createTempFile(file, GIT_TEMP))
		return;

	setContent(content.data());
	content.clear();

	compare();
}


void IgnoreSpaces()
{
	Settings.IgnoreSpaces = !Settings.IgnoreSpaces;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_IGNORE_SPACES]._cmdID,
			(LPARAM)Settings.IgnoreSpaces);
	Settings.markAsDirty();
}


void IgnoreCase()
{
	Settings.IgnoreCase = !Settings.IgnoreCase;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_IGNORE_CASE]._cmdID,
			(LPARAM)Settings.IgnoreCase);
	Settings.markAsDirty();
}


void DetectMoves()
{
	Settings.DetectMoves = !Settings.DetectMoves;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_DETECT_MOVES]._cmdID,
			(LPARAM)Settings.DetectMoves);
	Settings.markAsDirty();
}


void Prev()
{
	if (NppSettings::get().compareMode)
		jumpToNextChange(false, Settings.WrapAround);
}


void Next()
{
	if (NppSettings::get().compareMode)
		jumpToNextChange(true, Settings.WrapAround);
}


void First()
{
	if (NppSettings::get().compareMode)
		jumpToFirstChange();
}


void Last()
{
	if (NppSettings::get().compareMode)
		jumpToLastChange();
}


void OpenSettingsDlg(void)
{
	if (SettingsDlg.doDialog(&Settings) == IDOK)
	{
		Settings.save();

		newCompare.reset();

		if (!compareList.empty())
		{
			setStyles(Settings);

			NavDlg.SetConfig(Settings);
		}
	}
}


void OpenAboutDlg()
{
	AboutDlg.doDialog();
}


void createMenu()
{
	_tcscpy_s(funcItem[CMD_SET_FIRST]._itemName, nbChar, TEXT("Set as First to Compare"));
	funcItem[CMD_SET_FIRST]._pFunc				= SetAsFirst;
	funcItem[CMD_SET_FIRST]._pShKey				= new ShortcutKey;
	funcItem[CMD_SET_FIRST]._pShKey->_isAlt		= true;
	funcItem[CMD_SET_FIRST]._pShKey->_isCtrl	= true;
	funcItem[CMD_SET_FIRST]._pShKey->_isShift	= false;
	funcItem[CMD_SET_FIRST]._pShKey->_key		= '1';

	_tcscpy_s(funcItem[CMD_COMPARE]._itemName, nbChar, TEXT("Compare"));
	funcItem[CMD_COMPARE]._pFunc			= CompareWhole;
	funcItem[CMD_COMPARE]._pShKey			= new ShortcutKey;
	funcItem[CMD_COMPARE]._pShKey->_isAlt	= true;
	funcItem[CMD_COMPARE]._pShKey->_isCtrl	= true;
	funcItem[CMD_COMPARE]._pShKey->_isShift	= false;
	funcItem[CMD_COMPARE]._pShKey->_key		= 'C';

	_tcscpy_s(funcItem[CMD_COMPARE_LINES]._itemName, nbChar, TEXT("Compare Selected Lines"));
	funcItem[CMD_COMPARE_LINES]._pFunc				= CompareSelectedLines;
	funcItem[CMD_COMPARE_LINES]._pShKey				= new ShortcutKey;
	funcItem[CMD_COMPARE_LINES]._pShKey->_isAlt		= true;
	funcItem[CMD_COMPARE_LINES]._pShKey->_isCtrl	= true;
	funcItem[CMD_COMPARE_LINES]._pShKey->_isShift	= false;
	funcItem[CMD_COMPARE_LINES]._pShKey->_key		= 'N';

	_tcscpy_s(funcItem[CMD_CLEAR_ACTIVE]._itemName, nbChar, TEXT("Clear Active Compare"));
	funcItem[CMD_CLEAR_ACTIVE]._pFunc				= ClearActiveCompare;
	funcItem[CMD_CLEAR_ACTIVE]._pShKey 				= new ShortcutKey;
	funcItem[CMD_CLEAR_ACTIVE]._pShKey->_isAlt 		= true;
	funcItem[CMD_CLEAR_ACTIVE]._pShKey->_isCtrl		= true;
	funcItem[CMD_CLEAR_ACTIVE]._pShKey->_isShift	= false;
	funcItem[CMD_CLEAR_ACTIVE]._pShKey->_key 		= 'X';

	_tcscpy_s(funcItem[CMD_CLEAR_ALL]._itemName, nbChar, TEXT("Clear All Compares"));
	funcItem[CMD_CLEAR_ALL]._pFunc 				= ClearAllCompares;

	_tcscpy_s(funcItem[CMD_LAST_SAVE_DIFF]._itemName, nbChar, TEXT("Diff since last Save"));
	funcItem[CMD_LAST_SAVE_DIFF]._pFunc				= LastSaveDiff;
	funcItem[CMD_LAST_SAVE_DIFF]._pShKey 			= new ShortcutKey;
	funcItem[CMD_LAST_SAVE_DIFF]._pShKey->_isAlt 	= true;
	funcItem[CMD_LAST_SAVE_DIFF]._pShKey->_isCtrl 	= true;
	funcItem[CMD_LAST_SAVE_DIFF]._pShKey->_isShift	= false;
	funcItem[CMD_LAST_SAVE_DIFF]._pShKey->_key 		= 'D';

	_tcscpy_s(funcItem[CMD_SVN_DIFF]._itemName, nbChar, TEXT("SVN Diff"));
	funcItem[CMD_SVN_DIFF]._pFunc 				= SvnDiff;
	funcItem[CMD_SVN_DIFF]._pShKey 				= new ShortcutKey;
	funcItem[CMD_SVN_DIFF]._pShKey->_isAlt 		= true;
	funcItem[CMD_SVN_DIFF]._pShKey->_isCtrl 	= true;
	funcItem[CMD_SVN_DIFF]._pShKey->_isShift	= false;
	funcItem[CMD_SVN_DIFF]._pShKey->_key 		= 'V';

	_tcscpy_s(funcItem[CMD_GIT_DIFF]._itemName, nbChar, TEXT("Git Diff"));
	funcItem[CMD_GIT_DIFF]._pFunc 				= GitDiff;
	funcItem[CMD_GIT_DIFF]._pShKey 				= new ShortcutKey;
	funcItem[CMD_GIT_DIFF]._pShKey->_isAlt 		= true;
	funcItem[CMD_GIT_DIFF]._pShKey->_isCtrl 	= true;
	funcItem[CMD_GIT_DIFF]._pShKey->_isShift	= false;
	funcItem[CMD_GIT_DIFF]._pShKey->_key 		= 'G';

	_tcscpy_s(funcItem[CMD_IGNORE_SPACES]._itemName, nbChar, TEXT("Ignore Spaces"));
	funcItem[CMD_IGNORE_SPACES]._pFunc = IgnoreSpaces;

	_tcscpy_s(funcItem[CMD_IGNORE_CASE]._itemName, nbChar, TEXT("Ignore Case"));
	funcItem[CMD_IGNORE_CASE]._pFunc = IgnoreCase;

	_tcscpy_s(funcItem[CMD_DETECT_MOVES]._itemName, nbChar, TEXT("Detect Moves"));
	funcItem[CMD_DETECT_MOVES]._pFunc = DetectMoves;

	_tcscpy_s(funcItem[CMD_NAV_BAR]._itemName, nbChar, TEXT("Navigation Bar"));
	funcItem[CMD_NAV_BAR]._pFunc = ViewNavigationBar;

	_tcscpy_s(funcItem[CMD_PREV]._itemName, nbChar, TEXT("Previous"));
	funcItem[CMD_PREV]._pFunc 				= Prev;
	funcItem[CMD_PREV]._pShKey 				= new ShortcutKey;
	funcItem[CMD_PREV]._pShKey->_isAlt 		= true;
	funcItem[CMD_PREV]._pShKey->_isCtrl 	= false;
	funcItem[CMD_PREV]._pShKey->_isShift	= false;
	funcItem[CMD_PREV]._pShKey->_key 		= VK_PRIOR;

	_tcscpy_s(funcItem[CMD_NEXT]._itemName, nbChar, TEXT("Next"));
	funcItem[CMD_NEXT]._pFunc 				= Next;
	funcItem[CMD_NEXT]._pShKey 				= new ShortcutKey;
	funcItem[CMD_NEXT]._pShKey->_isAlt 		= true;
	funcItem[CMD_NEXT]._pShKey->_isCtrl 	= false;
	funcItem[CMD_NEXT]._pShKey->_isShift	= false;
	funcItem[CMD_NEXT]._pShKey->_key 		= VK_NEXT;

	_tcscpy_s(funcItem[CMD_FIRST]._itemName, nbChar, TEXT("First"));
	funcItem[CMD_FIRST]._pFunc 				= First;
	funcItem[CMD_FIRST]._pShKey 			= new ShortcutKey;
	funcItem[CMD_FIRST]._pShKey->_isAlt 	= true;
	funcItem[CMD_FIRST]._pShKey->_isCtrl 	= true;
	funcItem[CMD_FIRST]._pShKey->_isShift	= false;
	funcItem[CMD_FIRST]._pShKey->_key 		= VK_PRIOR;

	_tcscpy_s(funcItem[CMD_LAST]._itemName, nbChar, TEXT("Last"));
	funcItem[CMD_LAST]._pFunc 				= Last;
	funcItem[CMD_LAST]._pShKey 				= new ShortcutKey;
	funcItem[CMD_LAST]._pShKey->_isAlt 		= true;
	funcItem[CMD_LAST]._pShKey->_isCtrl 	= true;
	funcItem[CMD_LAST]._pShKey->_isShift	= false;
	funcItem[CMD_LAST]._pShKey->_key 		= VK_NEXT;

	_tcscpy_s(funcItem[CMD_SETTINGS]._itemName, nbChar, TEXT("Settings..."));
	funcItem[CMD_SETTINGS]._pFunc = OpenSettingsDlg;

	_tcscpy_s(funcItem[CMD_ABOUT]._itemName, nbChar, TEXT("Help / About..."));
	funcItem[CMD_ABOUT]._pFunc = OpenAboutDlg;
}


void deinitPlugin()
{
	// Always close it, else N++'s plugin manager would call 'ViewNavigationBar'
	// on startup, when N++ has been shut down before with opened navigation bar
	if (NavDlg.isVisible())
		NavDlg.Hide();

	if (tbSetFirst.hToolbarBmp)
		::DeleteObject(tbSetFirst.hToolbarBmp);

	if (tbCompare.hToolbarBmp)
		::DeleteObject(tbCompare.hToolbarBmp);

	if (tbCompareLines.hToolbarBmp)
		::DeleteObject(tbCompareLines.hToolbarBmp);

	if (tbClearCompare.hToolbarBmp)
		::DeleteObject(tbClearCompare.hToolbarBmp);

	if (tbFirst.hToolbarBmp)
		::DeleteObject(tbFirst.hToolbarBmp);

	if (tbPrev.hToolbarBmp)
		::DeleteObject(tbPrev.hToolbarBmp);

	if (tbNext.hToolbarBmp)
		::DeleteObject(tbNext.hToolbarBmp);

	if (tbLast.hToolbarBmp)
		::DeleteObject(tbLast.hToolbarBmp);

	if (tbNavBar.hToolbarBmp)
		::DeleteObject(tbNavBar.hToolbarBmp);

	SettingsDlg.destroy();
	AboutDlg.destroy();
	NavDlg.destroy();

	// Deallocate shortcut
	for (int i = 0; i < NB_MENU_COMMANDS; i++)
		if (funcItem[i]._pShKey != NULL)
        {
			delete funcItem[i]._pShKey;
            funcItem[i]._pShKey = NULL;
        }
}


void forceViewsSync(HWND focalView, bool syncCurrentLine)
{
	HWND otherView = getOtherView(focalView);

	const int activeLine = getCurrentLine(focalView);
	const int firstVisibleLine1 = ::SendMessage(focalView, SCI_GETFIRSTVISIBLELINE, 0, 0);

	if (syncCurrentLine)
	{
		const int visibleLine = ::SendMessage(focalView, SCI_VISIBLEFROMDOCLINE, activeLine, 0);
		// If current line is not visible then sync views on the first visible line
		syncCurrentLine = (visibleLine >= firstVisibleLine1) &&
				(visibleLine <= firstVisibleLine1 + ::SendMessage(focalView, SCI_LINESONSCREEN, 0, 0));
	}

	const int line = syncCurrentLine ? activeLine :
			::SendMessage(focalView, SCI_DOCLINEFROMVISIBLE, firstVisibleLine1, 0);
	const int offset =
			::SendMessage(focalView, SCI_VISIBLEFROMDOCLINE, syncCurrentLine ? line : line + 1, 0) - firstVisibleLine1;

	ScopedIncrementer incr(notificationsLock);

	::SendMessage(otherView, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
	int firstVisibleLine2 =
		::SendMessage(otherView, SCI_VISIBLEFROMDOCLINE, syncCurrentLine ? line : line + 1, 0) - offset;

	if (!syncCurrentLine && (line != ::SendMessage(otherView, SCI_DOCLINEFROMVISIBLE, firstVisibleLine2, 0) ||
			firstVisibleLine1 == ::SendMessage(focalView, SCI_VISIBLEFROMDOCLINE, line, 0)))
		firstVisibleLine2 = ::SendMessage(otherView, SCI_VISIBLEFROMDOCLINE, line, 0);

	if ((activeLine != getCurrentLine(otherView)) && !isSelection(otherView))
	{
		int pos = ::SendMessage(focalView, SCI_GETCURRENTPOS, 0, 0) -
				::SendMessage(focalView, SCI_POSITIONFROMLINE, activeLine, 0);
		pos += ::SendMessage(otherView, SCI_POSITIONFROMLINE, activeLine, 0);

		const int lineEndPos = ::SendMessage(otherView, SCI_GETLINEENDPOSITION, activeLine, 0);
		if (lineEndPos < pos)
			pos = lineEndPos;

		::SendMessage(otherView, SCI_SETSEL, pos, pos);
	}

	::SendMessage(otherView, SCI_SETFIRSTVISIBLELINE, firstVisibleLine2, 0);

	::UpdateWindow(otherView);
}


void comparedFileActivated(const ComparedFile& cmpFile)
{
	HWND syncView = getCurrentView();

	if (!NppSettings::get().compareMode)
	{
		NppSettings::get().setCompareMode();

		if (Settings.UseNavBar && !NavDlg.isVisible())
			showNavBar();
	}

	if (cmpFile.originalViewId != cmpFile.compareViewId)
		syncView = getOtherView(syncView);

	setCompareView(nppData._scintillaMainHandle);
	setCompareView(nppData._scintillaSecondHandle);

	forceViewsSync(syncView);
}


void onToolBarReady()
{
	UINT style = (LR_SHARED | LR_LOADTRANSPARENT | LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS);

	const bool isRTL = ((::GetWindowLongPtr(nppData._nppHandle, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) != 0);

	if (isRTL)
		tbSetFirst.hToolbarBmp =
				(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_SETFIRST_RTL), IMAGE_BITMAP, 0, 0, style);
	else
		tbSetFirst.hToolbarBmp =
				(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_SETFIRST), IMAGE_BITMAP, 0, 0, style);

	tbCompare.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_COMPARE), IMAGE_BITMAP, 0, 0, style);
	tbCompareLines.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_COMPARE_LINES), IMAGE_BITMAP, 0, 0, style);
	tbClearCompare.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_CLEARCOMPARE), IMAGE_BITMAP, 0, 0, style);
	tbFirst.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_FIRST),	IMAGE_BITMAP, 0, 0, style);
	tbPrev.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_PREV),	IMAGE_BITMAP, 0, 0, style);
	tbNext.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_NEXT),	IMAGE_BITMAP, 0, 0, style);
	tbLast.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_LAST),	IMAGE_BITMAP, 0, 0, style);
	tbNavBar.hToolbarBmp =
			(HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_NAVBAR), IMAGE_BITMAP, 0, 0, style);

	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_SET_FIRST]._cmdID,			(LPARAM)&tbSetFirst);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_COMPARE]._cmdID,			(LPARAM)&tbCompare);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_COMPARE_LINES]._cmdID,		(LPARAM)&tbCompareLines);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_CLEAR_ACTIVE]._cmdID,		(LPARAM)&tbClearCompare);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_FIRST]._cmdID,				(LPARAM)&tbFirst);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_PREV]._cmdID,				(LPARAM)&tbPrev);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_NEXT]._cmdID,				(LPARAM)&tbNext);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_LAST]._cmdID,				(LPARAM)&tbLast);
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON,
			(WPARAM)funcItem[CMD_NAV_BAR]._cmdID,			(LPARAM)&tbNavBar);
}


void onNppReady()
{
	// It's N++'s job actually to disable its scroll menu commands but since it's not the case provide this as a patch
	if (isSingleView())
		NppSettings::get().enableNppScrollCommands(false);

	NppSettings::get().updatePluginMenu();

	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_IGNORE_SPACES]._cmdID,
			(LPARAM)Settings.IgnoreSpaces);
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_IGNORE_CASE]._cmdID,
			(LPARAM)Settings.IgnoreCase);
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_DETECT_MOVES]._cmdID,
			(LPARAM)Settings.DetectMoves);
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_NAV_BAR]._cmdID,
			(LPARAM)Settings.UseNavBar);
}


void onSciUpdateUI(SCNotification *notifyCode)
{
	if (notifyCode->updated & (SC_UPDATE_SELECTION | SC_UPDATE_V_SCROLL))
		forceViewsSync((HWND)notifyCode->nmhdr.hwndFrom);
}


void DelayedUpdate::operator()()
{
	if (fullCompare)
	{
		linesAdded = 0;
		linesDeleted = 0;
		fullCompare = false;

		compare();

		return;
	}

	HWND changeView = getCurrentView();

	const int startLine = ::SendMessage(changeView, SCI_LINEFROMPOSITION, changePos, 0);

	section_t mainViewSec = { startLine, 1 };
	section_t subViewSec = { startLine, 1 };

	ScopedIncrementer incr(notificationsLock);

	// Adjust views re-compare range
	if (linesAdded || linesDeleted)
	{
		HWND otherView = getOtherView();

		section_t& changeViewSec = (changeView == nppData._scintillaMainHandle) ? mainViewSec : subViewSec;
		section_t& otherViewSec = (changeView == nppData._scintillaMainHandle) ? subViewSec : mainViewSec;

		const int startOff = startLine - getPrevUnmarkedLine(otherView, startLine, MARKER_MASK_LINE);

		changeViewSec.off -= startOff;
		otherViewSec.off -= startOff;

		changeViewSec.len += (startOff + linesAdded);
		otherViewSec.len += (startOff + linesDeleted);

		const int endLine = otherViewSec.off + otherViewSec.len - 1;
		const int endOff = getNextUnmarkedLine(otherView, endLine, MARKER_MASK_LINE) - endLine;

		changeViewSec.len += endOff;
		otherViewSec.len += endOff;

		mainViewSec.len -= clearMarksAndBlanks(nppData._scintillaMainHandle, mainViewSec.off, mainViewSec.len);
		subViewSec.len -= clearMarksAndBlanks(nppData._scintillaSecondHandle, subViewSec.off, subViewSec.len);

		progress_ptr& progress = ProgressDlg::Open();

		if (progress)
			progress->SetInfo(TEXT("Re-comparing changes..."));

		compareViews(progress, mainViewSec, subViewSec);
		progress.reset();
	}
	else
	{
		clearMarks(nppData._scintillaMainHandle, mainViewSec.off, mainViewSec.len);
		clearMarks(nppData._scintillaSecondHandle, subViewSec.off, subViewSec.len);

		progress_ptr progress;
		compareViews(progress, mainViewSec, subViewSec);
	}

	linesAdded = 0;
	linesDeleted = 0;

	// Force NavBar redraw
	if (NavDlg.isVisible())
		NavDlg.Show();

	// Synchronize views
	forceViewsSync(changeView);
}


void onSciModified(SCNotification *notifyCode)
{
	const LRESULT buffId = getCurrentBuffId();

	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
		return;

	if (notifyCode->modificationType & SC_MOD_BEFOREDELETE)
	{
		HWND currentView = getCurrentView();

		const int startLine = ::SendMessage(currentView, SCI_LINEFROMPOSITION, notifyCode->position, 0);
		const int endLine =
			::SendMessage(currentView, SCI_LINEFROMPOSITION, notifyCode->position + notifyCode->length, 0);

		// Change is on single line?
		if (endLine <= startLine)
			return;

		const int currAction =
			notifyCode->modificationType & (SC_PERFORMED_USER | SC_PERFORMED_UNDO | SC_PERFORMED_REDO);

		cmpPair->getFileByBuffId(buffId).deletedSections.push(currAction, startLine, endLine);
	}
	else if ((notifyCode->modificationType & SC_MOD_INSERTTEXT) && notifyCode->linesAdded)
	{
		HWND currentView = getCurrentView();

		const int startLine = ::SendMessage(currentView, SCI_LINEFROMPOSITION, notifyCode->position, 0);

		const int currAction =
			notifyCode->modificationType & (SC_PERFORMED_USER | SC_PERFORMED_UNDO | SC_PERFORMED_REDO);

		cmpPair->getFileByBuffId(buffId).deletedSections.pop(currAction, startLine);
	}
}


void onSciModifiedUpdate(SCNotification *notifyCode)
{
	const LRESULT buffId = getCurrentBuffId();

	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
		return;

	if (notifyCode->modificationType & SC_MOD_BEFOREDELETE)
	{
		HWND currentView = getCurrentView();

		const int startLine = ::SendMessage(currentView, SCI_LINEFROMPOSITION, notifyCode->position, 0);
		const int endLine =
			::SendMessage(currentView, SCI_LINEFROMPOSITION, notifyCode->position + notifyCode->length, 0);

		if (endLine > startLine)
		{
			ScopedIncrementer incr(notificationsLock);

			clearMarks(currentView, startLine, endLine - startLine + 1);
		}
	}
	else if (notifyCode->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))
	{
		if (!delayedUpdate.fullCompare)
		{
			if (delayedUpdate.isPending())
			{
				delayedUpdate.cancel();

				if (delayedUpdate.changePos > notifyCode->position)
					delayedUpdate.changePos = notifyCode->position;
			}
			else
			{
				delayedUpdate.changePos = notifyCode->position;
			}

			if (notifyCode->modificationType & SC_MOD_INSERTTEXT)
				delayedUpdate.linesAdded += notifyCode->linesAdded;
			else
				delayedUpdate.linesDeleted += (-notifyCode->linesAdded);

			delayedUpdate.post(10);
		}
	}
}


void onSciZoom()
{
	CompareList_t::iterator cmpPair = getCompare(getCurrentBuffId());
	if (cmpPair == compareList.end())
		return;

	ScopedIncrementer incr(notificationsLock);

	// sync both views zoom
	const int zoom = ::SendMessage(getCurrentView(), SCI_GETZOOM, 0, 0);
	::SendMessage(getOtherView(), SCI_SETZOOM, zoom, 0);
}


void DelayedActivate::operator()()
{
	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
		return;

	const ComparedFile& otherFile = cmpPair->getOtherFileByBuffId(buffId);

	// When compared file is activated make sure its corresponding pair file is also active in the other view
	if (getDocId(getOtherView()) != otherFile.sciDoc)
	{
		ScopedIncrementer incr(notificationsLock);

		activateBufferID(otherFile.buffId);
		activateBufferID(buffId);
	}

	cmpPair->setStatus();

	comparedFileActivated(cmpPair->getFileByBuffId(buffId));
}


void onBufferActivated(LRESULT buffId, bool delay)
{
	delayedActivation.cancel();

	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
	{
		NppSettings::get().setNormalMode();
		setNormalView(getCurrentView());
		resetCompareView(getOtherView());
	}
	else
	{
		const ComparedFile& otherFile = cmpPair->getOtherFileByBuffId(buffId);

		// The other compared file is active in the other view - perhaps we are simply switching between views
		if (getDocId(getOtherView()) == otherFile.sciDoc)
		{
			cmpPair->setStatus();

			comparedFileActivated(cmpPair->getFileByBuffId(buffId));
		}
		// The other compared file is not active in the other view - we must activate it but let's wait because
		// if the view is also switched this might not be the user activated buffer
		// (it might be intermediate buffer activation notification)
		else
		{
			if (delay)
			{
				delayedActivation.buffId = buffId;
				delayedActivation.post(30);
			}
			else
			{
				delayedActivation(buffId);
			}
		}
	}
}


void DelayedClose::operator()()
{
	const LRESULT currentBuffId = getCurrentBuffId();

	ScopedIncrementer incr(notificationsLock);

	for (int i = static_cast<int>(closedBuffs.size()); i; --i)
	{
		const LRESULT buffId = closedBuffs[i - 1];

		CompareList_t::iterator cmpPair = getCompare(buffId);
		if (cmpPair == compareList.end())
			continue;

		ComparedFile& closedFile = cmpPair->getFileByBuffId(buffId);
		ComparedFile& otherFile = cmpPair->getOtherFileByBuffId(buffId);

		if (closedFile.isTemp)
		{
			if (closedFile.isOpen())
				closedFile.close();
			else
				closedFile.onClose();
		}

		if (otherFile.isTemp)
		{
			if (otherFile.isOpen())
				otherFile.close();
			else
				otherFile.onClose();
		}
		else
		{
			if (otherFile.isOpen())
				otherFile.restore();
		}

		compareList.erase(cmpPair);
	}

	closedBuffs.clear();

	activateBufferID(currentBuffId);
	onBufferActivated(currentBuffId, false);

	// If it is the last file and it is not in the main view - move it there
	if (getNumberOfFiles() == 1 && getCurrentViewId() == SUB_VIEW)
	{
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

		const LRESULT newBuffId = getCurrentBuffId();

		activateBufferID(currentBuffId);
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_GOTO_ANOTHER_VIEW);
		activateBufferID(newBuffId);
		::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
	}
}


void onFileBeforeClose(LRESULT buffId)
{
	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
		return;

	delayedUpdate.cancel();
	delayedActivation.cancel();

	delayedClosure.cancel();
	delayedClosure.closedBuffs.push_back(buffId);

	const LRESULT currentBuffId = getCurrentBuffId();

	ScopedIncrementer incr(notificationsLock);

	ComparedFile& closedFile = cmpPair->getFileByBuffId(buffId);
	closedFile.onBeforeClose();

	if (cmpPair->relativePos && (closedFile.originalViewId == viewIdFromBuffId(buffId)))
	{
		ComparedFile& otherFile = cmpPair->getOtherFileByBuffId(buffId);

		otherFile.originalPos = posFromBuffId(buffId) + cmpPair->relativePos;

		if (cmpPair->relativePos > 0)
			--otherFile.originalPos;
		else
			++otherFile.originalPos;

		if (otherFile.originalPos < 0)
			otherFile.originalPos = 0;
	}

	if (currentBuffId != buffId)
		activateBufferID(currentBuffId);

	delayedClosure.post(30);
}


void onFileBeforeSave(LRESULT buffId)
{
	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
		return;

    ComparedFile& savedFile = cmpPair->getFileByBuffId(buffId);
    const LRESULT currentBuffId = getCurrentBuffId();

	ScopedIncrementer incr(notificationsLock);

    if (currentBuffId != buffId)
        activateBufferID(buffId);

    savedFile.removeAndStoreBlanks();

    if (currentBuffId != buffId)
        activateBufferID(currentBuffId);
}


void onFileSaved(LRESULT buffId)
{
	CompareList_t::iterator cmpPair = getCompare(buffId);
	if (cmpPair == compareList.end())
		return;

    ComparedFile& savedFile = cmpPair->getFileByBuffId(buffId);
    const ComparedFile& otherFile = cmpPair->getOtherFileByBuffId(buffId);

    const LRESULT currentBuffId = getCurrentBuffId();
    const bool pairIsActive = (currentBuffId == buffId || currentBuffId == otherFile.buffId);

	ScopedIncrementer incr(notificationsLock);

    if (!pairIsActive)
        activateBufferID(buffId);

    if (pairIsActive && Settings.RecompareOnSave)
    {
        delayedUpdate.cancel();
        delayedUpdate.fullCompare = true;
        delayedUpdate.post(30);
    }
    else
    {
        savedFile.restoreBlanks();
    }

    savedFile.clearStoredBlanks();

    if (otherFile.isTemp == LAST_SAVED_TEMP)
    {
        HWND hNppTabBar = NppTabHandleGetter::get(otherFile.compareViewId);

        if (hNppTabBar)
        {
            TCHAR tabText[MAX_PATH];

            TCITEM tab;
            tab.mask = TCIF_TEXT;
            tab.pszText = tabText;
            tab.cchTextMax = _countof(tabText);

            const int tabPos = posFromBuffId(otherFile.buffId);
            TabCtrl_GetItem(hNppTabBar, tabPos, &tab);

            _tcscat_s(tabText, _countof(tabText), TEXT(" - Outdated"));

            TabCtrl_SetItem(hNppTabBar, tabPos, &tab);
        }
    }

    if (!pairIsActive)
    {
        activateBufferID(currentBuffId);
        onBufferActivated(currentBuffId, false);
    }
}

} // anonymous namespace


void ViewNavigationBar()
{
	Settings.UseNavBar = !Settings.UseNavBar;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[CMD_NAV_BAR]._cmdID,
			(LPARAM)Settings.UseNavBar);
	Settings.markAsDirty();

	if (NppSettings::get().compareMode)
	{
		if (Settings.UseNavBar)
		{
			showNavBar();
		}
		else
		{
			NavDlg.Hide();
			updateWrap();
		}

		forceViewsSync(getCurrentView());
	}
}


// Main plugin DLL function
BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD  reasonForCall, LPVOID)
 {
	hInstance = hinstDLL;

	switch (reasonForCall)
	{
		case DLL_PROCESS_ATTACH:
			createMenu();
		break;

		case DLL_PROCESS_DETACH:
			deinitPlugin();
		break;

		case DLL_THREAD_ATTACH:
		break;

		case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}


//
// Notepad++ API functions below
//

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;

	Settings.load();

	AboutDlg.init(hInstance, nppData);
	SettingsDlg.init(hInstance, nppData);
	NavDlg.init(hInstance);
}


extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return PLUGIN_NAME;
}


extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = NB_MENU_COMMANDS;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	switch (notifyCode->nmhdr.code)
	{
		// Handle wrap refresh
		case SCN_PAINTED:
			if (NppSettings::get().compareMode && !notificationsLock &&
					!delayedActivation.isPending() && !delayedClosure.isPending() && !delayedUpdate.isPending())
				NavDlg.Update();
		break;

		// Emulate word-wrap aware vertical scroll sync
		case SCN_UPDATEUI:
			if (NppSettings::get().compareMode && !notificationsLock &&
					!delayedActivation.isPending() && !delayedClosure.isPending() && !delayedUpdate.isPending())
				onSciUpdateUI(notifyCode);
		break;

		case NPPN_BUFFERACTIVATED:
			if (!notificationsLock && !compareList.empty() && !delayedClosure.isPending())
				onBufferActivated(notifyCode->nmhdr.idFrom);
		break;

		case NPPN_FILEBEFORECLOSE:
			if (newCompare && (newCompare->pair.file[0].buffId == static_cast<LRESULT>(notifyCode->nmhdr.idFrom)))
				newCompare.reset();
			else if (!notificationsLock && !compareList.empty())
				onFileBeforeClose(notifyCode->nmhdr.idFrom);
		break;

		case NPPN_FILEBEFORESAVE:
			if (!compareList.empty())
				onFileBeforeSave(notifyCode->nmhdr.idFrom);
		break;

		case NPPN_FILESAVED:
			if (!notificationsLock && !compareList.empty())
				onFileSaved(notifyCode->nmhdr.idFrom);
		break;

		// This is used to monitor either:
		// - text change to automatically update results or
		// - deletion of lines to properly clear their compare markings
		case SCN_MODIFIED:
			if (NppSettings::get().compareMode && !notificationsLock)
			{
				if (Settings.UpdateOnChange)
					onSciModifiedUpdate(notifyCode);
				else
					onSciModified(notifyCode);
			}
		break;

		case SCN_ZOOM:
			if (NppSettings::get().compareMode && !notificationsLock)
				onSciZoom();
		break;

		case NPPN_WORDSTYLESUPDATED:
			setStyles(Settings);
			NavDlg.SetConfig(Settings);
		break;

		case NPPN_TBMODIFICATION:
			onToolBarReady();
		break;

		case NPPN_READY:
			onNppReady();
		break;

		case NPPN_BEFORESHUTDOWN:
			ClearAllCompares();
		break;

		case NPPN_SHUTDOWN:
			Settings.save();
			deinitPlugin();
		break;
	}
}


extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM)
{
	return TRUE;
}


extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}
