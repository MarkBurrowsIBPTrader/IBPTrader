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



void IB_PTraderDlg::SetStopGoButtonText ()
{
    if (_allowedorders == Allow)
    {
        GetDlgItem (IDC_STARTSTOP)->SetWindowText ("Stop");
    }
    else
    {
        GetDlgItem (IDC_STARTSTOP)->SetWindowText ("Go!");
    }
    SetWindowTitle ();
}


#if ! PUBLIC_BUILD
void IB_PTraderDlg::SetSellStartsButtonText ()
{
    CString str;
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        if (ticker->_sellsearchspeed == Ticker::Slow)
        {
            str += "Goto Fast";
        }
        else
        {
            str += "Goto Slow";
        }
    }
    if (_lastsearchspeedchecktime.GetLength () > 0)
    {
        CString format;
        if (str.GetLength () > 0)
            str += ", ";
        str += _lastsearchspeedchecktime;
        str += " - ";
        if (_numberofsellstarts_fails == 0)
        {
            str += "Zero";
        }
        else
        {
            format.Format ("%i items", _numberofsellstarts_fails);
            str += format;
        }
        if (_max_number_of_sellstarts_fails > 0)
        {
            format.Format (" (max %i)", _max_number_of_sellstarts_fails);
            str += format;
        }
    }
    CWnd *button = GetDlgItem (IDC_TOGGLESELLSPEED);
    button->SetWindowText (str);
    button->EnableWindow (ticker ? true : false);
}
#endif


void IB_PTraderDlg::SetBuyAverageButtonText (Ticker *ticker)
{
    CWnd *avg_button = GetDlgItem (IDC_AVG_ACTION);
    if (ticker)
    {
        bool userdefined = ticker->_userdefined_avg > 0;
        CString str;
        switch (ticker->_buyaverages)
        {
            case Ticker::Always:
                if (userdefined)
                    str = "2 Bel - ";
                else
                    str = "2 Below - ";
                break;
            case Ticker::OnlyIfBelow:
                if (userdefined)
                    str = "2 Alys - ";
                else
                    str = " Always - ";
                break;
        }
        CString format;
        PortfolioPart *part = GetPortfolioPart (ticker);
        if (part)
        {
            format.Format ("IB=%0.2f", part->_averageCost);
            str += format;
        }        
        else
        {
            str += "???";
        }
        if (userdefined)
        {
            format.Format (" (U=%0.2f)", ticker->_userdefined_avg);
            str += format;
        }
        avg_button->EnableWindow (true);
        avg_button->SetWindowText (str);
    }
    else
    {
        avg_button->EnableWindow (false);
        avg_button->SetWindowText ("");
    }
}


#define FULL_WINDOWTITLE 1
#define WANT_ACCOUNT_DETAILS 1

void IB_PTraderDlg::SetWindowTitle (int numberitemsrecorded, CTime *recordtime)
{
    if (_titletextsetfirsttime)
    {
        this->GetWindowText (_orignalwindowtitletext);
        _titletextsetfirsttime = false;
    }
    CString format, format1;
    format.Format (
#if FULL_WINDOWTITLE
                   "PGEN "
#endif
                   "%s %s -"
#if WANT_ACCOUNT_DETAILS
                   " %s (%s)" 
#endif
                   " on %s" 
#if FULL_WINDOWTITLE
                   ", CLIENTID=%d, PORT=%u"
#endif
                   , 
                   (LPCTSTR) _accountselection, 
                   (LPCTSTR) _orignalwindowtitletext, 
#if WANT_ACCOUNT_DETAILS
                   _username.c_str (), 
                   _accountnumber.c_str (), 
#endif
                   (LPCSTR) _dbschema
#if FULL_WINDOWTITLE
                   , _CLIENTID
                   , _PORTID
#endif
                  );
    IB_PTraderDlg::AccountValue *currentaccountvalue = GetAccountValue ("NetLiquidationByCurrency", "BASE");
    if (currentaccountvalue)
    {
        TickerListBox::GetFormattedNumberString (currentaccountvalue->GetAdjustedCashValue (), false, format1);
        format += ", Value=£" + format1;
#if FULL_WINDOWTITLE
        TickerListBox::GetFormattedNumberString (_minaccountvaluebalance, false, format1);
        format += ", Min=£" + format1;
        TickerListBox::GetFormattedNumberString (_maxaccountvaluebalance, false, format1);
        format += ", Max=£" + format1;
#endif
    }
    currentaccountvalue = GetAccountValue ("UnrealizedPnL", "BASE");
    if (currentaccountvalue)
    {
        double currentPnL = currentaccountvalue->GetAdjustedCashValue ();
        TickerListBox::GetFormattedNumberString (_minPnL, false, format1);
        format += ", MinP=£" + format1;
        TickerListBox::GetFormattedNumberString (_maxPnL, false, format1);
        format += ", MaxP=£" + format1;
        TickerListBox::GetFormattedNumberString (currentPnL, false, format1);
        format += (currentPnL < 0 ? CString (", L") : CString (", P")) + "=£" + format1;
    }
    if (_locked)
    {
        format += " (LOCKED";
        if (_allowedorders == Stopped)
        {
            format += " and STOPPED";
        }
        format += ")";
    }
    else if (_allowedorders == Stopped)
    {
        format += " (STOPPED)";
    }
#if ! PUBLIC_BUILD
    if (recordtime && numberitemsrecorded > 0)
    {
        _extrawindowtitletext.Format (" - Recorded %d %s at %0.2d:%0.2d:%0.2d",
                                      numberitemsrecorded,
                                      numberitemsrecorded > 1 ? "prices" : "price",
                                      recordtime->GetHour (), 
                                      recordtime->GetMinute (), 
                                      recordtime->GetSecond ());
    }
#endif
    format += _extrawindowtitletext;
    this->SetWindowText ((LPCTSTR) format);
}


void IB_PTraderDlg::SetWindowTitle ()
{
    SetWindowTitle (0, 0);
}
