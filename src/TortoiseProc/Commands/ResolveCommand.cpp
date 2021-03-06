// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2009-2014 - TortoiseGit
// Copyright (C) 2007-2008,2012 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "ResolveCommand.h"
#include "AppUtils.h"
#include "MessageBox.h"
#include "ResolveDlg.h"
#include "GitProgressDlg.h"
#include "ProgressCommands/ResolveProgressCommand.h"

bool ResolveCommand::Execute()
{
	CResolveDlg dlg;
	dlg.m_pathList = pathList;
	INT_PTR ret = IDOK;
	if (!parser.HasKey(_T("noquestion")))
		ret = dlg.DoModal();
	if (ret == IDOK)
	{
		if (!dlg.m_pathList.IsEmpty())
		{
			if (parser.HasKey(L"silent"))
			{
				for (int i = 0; i < dlg.m_pathList.GetCount(); ++i)
				{
					CString cmd, out;
					cmd.Format(_T("git.exe add -f -- \"%s\""), dlg.m_pathList[i].GetGitPathString());
					if (g_Git.Run(cmd, &out, CP_UTF8))
					{
						CMessageBox::Show(NULL, out, _T("TortoiseGit"), MB_OK | MB_ICONERROR);
						return false;
					}

					CAppUtils::RemoveTempMergeFile((CTGitPath &)dlg.m_pathList[i]);
				}

				HWND resolveMsgWnd = parser.HasVal(L"resolvemsghwnd") ? (HWND)parser.GetLongLongVal(L"resolvemsghwnd") : 0;
				if (resolveMsgWnd)
				{
					static UINT WM_REVERTMSG = RegisterWindowMessage(_T("GITSLNM_NEEDSREFRESH"));
					::PostMessage(resolveMsgWnd, WM_REVERTMSG, NULL, NULL);
				}
				return true;
			}
			else
			{
				CGitProgressDlg progDlg(CWnd::FromHandle(hWndExplorer));
				theApp.m_pMainWnd = &progDlg;
				ResolveProgressCommand resolveProgressCommand;
				progDlg.SetCommand(&resolveProgressCommand);
				resolveProgressCommand.SetPathList(dlg.m_pathList);
				progDlg.DoModal();
				return !progDlg.DidErrorsOccur();
			}
		}
	}
	return false;
}
