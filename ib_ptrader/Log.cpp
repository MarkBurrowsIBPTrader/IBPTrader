/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "ib_ptraderdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



void IB_PTraderDlg::Log (LPCTSTR message, bool log)
{
    if (! log)
        return;
    if (this->_statusmessageslistbox.GetCount () > 19999)
    {
        this->_statusmessageslistbox.ResetContent ();
    }
    CTime now = CTime::GetCurrentTime ();
    CString str;
    str.Format ("%0.2d:%0.2d:%0.2d - %s", now.GetHour (), now.GetMinute (), now.GetSecond (), message);
    int index = this->_statusmessageslistbox.AddString ((LPCTSTR) str);
    if (index == LB_ERR)
    {
        if (! _have_shown_LB_ERR)
        {
            AfxMessageBox ("Status List Box - LB_ERR");
        }
        _have_shown_LB_ERR = true;
    }
    else if (index == LB_ERRSPACE)
    {
        if (! _have_shown_LB_ERRSPACE)
        {
            AfxMessageBox ("Status List Box - LB_ERRSPACE");
        }
        _have_shown_LB_ERRSPACE = true;
    }
}


void IB_PTraderDlg::Log (LPCTSTR message)
{
    Log (message, true);
}


void IB_PTraderDlg::Log (CString &message)
{
    Log ((LPCTSTR) message);
}


void IB_PTraderDlg::Log (const wchar_t *message)
{
    char *charmess = xercesc::XMLString::transcode (message);
    Log (charmess);
    xercesc::XMLString::release (&charmess);
}


void IB_PTraderDlg::LogDateTime ()
{
    CTime now = CTime::GetCurrentTime ();
    CString timeasstr;
    timeasstr.Format ("%i/%i/%i", now.GetDay (), now.GetMonth (), now.GetYear ());
    Log (timeasstr);
}
