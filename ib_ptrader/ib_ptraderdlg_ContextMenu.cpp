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
#include "ib_datadumpdlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif



void IB_PTraderDlg::AddDynamicMenuItems_TidyUp (ContextMenuDynamicMaps &dynamicmenumaps)
{
    for (ContextMenuDynamicMapsIter iter = dynamicmenumaps.begin ();  iter != dynamicmenumaps.end ();  ++iter)
    {
        delete iter->second;
    }
}


void IB_PTraderDlg::AddDynamicMenuItem (CMenu &menu, std::string currency, bool somemarketsonholiday, std::string menutext, UINT stateflags, 
                                        Ticker::ActiveState newstate, Ticker::BuyAverages newbuyaverage, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    bool isallcurrencies = currency == "";
    if (isallcurrencies)
    {
        if (somemarketsonholiday)
            stateflags |= MF_GRAYED;
    }
    else
    {
        if (IsMarketOnHoliday (currency))
            stateflags |= MF_GRAYED;
    }
    menu.AppendMenuA (stateflags, currentmenuid, menutext.c_str ());
    DynamicMenuTickerStateChange *dynamenu = new DynamicMenuTickerStateChange ();
    dynamenu->_currency = currency;
    dynamenu->_newstate = newstate;
    dynamenu->_buyaverages = newbuyaverage;
    dynamicmenumaps [currentmenuid] = dynamenu;
    currentmenuid++;    
}


UINT IB_PTraderDlg::GetContextMenuFlags (Ticker *ticker, UINT inputflags, bool enableconditions, CString formattext, bool validnumber, long number, CString &outputtext)
{
    outputtext = formattext + CString (ticker->_ticker.c_str ());
    if (ticker->_activestate == Ticker::DISABLED)
    {
        outputtext += CString (" DISABLED");
        inputflags |= MF_GRAYED;
    }
    else
    {
        if (validnumber && number >= 0)
        {
            CString format;
            format.Format ("%s to %s%ld", (LPCTSTR) outputtext, ticker->GetCurrencySymbol ().c_str (), number);
            outputtext = format;
            inputflags |= MF_ENABLED;
        }
        else
        {
            outputtext += CString (" No Number");
            inputflags |= MF_GRAYED;
        }
    }
    return inputflags;
}


UINT IB_PTraderDlg::GetContextMenuFlags (Ticker *ticker, CString &outputtext)
{
    UINT flags = MF_STRING;
    if (ticker->_activestate == Ticker::DISABLED)
    {
        flags |= MF_GRAYED;
        outputtext += " DISABLED";
    }
    return flags;
}


void IB_PTraderDlg::ContextMenuChangeTickersState (Ticker::ActiveState newstate, std::string currency)
{
    if (_tickers == 0)
        return;
    CString format, tickerlist;
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        Ticker *ticker = iter->second;
        if (ticker->_activestate != Ticker::DISABLED)
        {
            if (currency == "")
                ;
            else if (ticker->_firstdbrow->_currency != currency)
                continue;
            ticker->_activestate = newstate;
            Set_DoBuys_DoSells_OnTickerStateChange (ticker);
            if (tickerlist.GetLength () > 0)
                tickerlist += ", ";
            tickerlist += ticker->_ticker.c_str ();
        }
    }
    format.Format ("OnContextMenu - %s tickers changed to %s%s", currency == "" ? "All" : currency.c_str (), (LPCTSTR) Ticker::ToString (newstate),
                   tickerlist.GetLength () > 0 ? ", tickers changed:-" : "");
    Log (format);
    if (tickerlist.GetLength () > 0)
        Log (tickerlist);
}


void IB_PTraderDlg::ContextMenuToggleEarlyBuy (Ticker *ticker, CString &tickerlist)
{
    ticker->_allow_early_buys = ! ticker->_allow_early_buys;
    Set_DoBuys_DoSells_OnTickerStateChange (ticker);
    if (tickerlist.GetLength () > 0)
        tickerlist += ", ";
    CString tickerstr = ticker->_ticker.c_str ();
    if (! ticker->_allow_early_buys)
        tickerstr = tickerstr.MakeLower ();
    tickerlist += tickerstr + CString ((ticker->_allow_early_buys ? " (T)" : " (F)"));
}


void IB_PTraderDlg::ContextMenuToggleEarlyBuys (std::string currency)
{
    if (_tickers == 0)
        return;
    CString tickerlist;
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        Ticker *ticker = iter->second;
        if (ticker->_activestate != Ticker::DISABLED)
        {
            if (currency == "")
                ;
            else if (ticker->_firstdbrow->_currency != currency)
                continue;
            ContextMenuToggleEarlyBuy (ticker, tickerlist);
        }
    }
    ContextMenuToggleEarlyBuys_Log (0, currency, tickerlist);
}


void IB_PTraderDlg::ContextMenuToggleEarlyBuys_Log (Ticker *ticker, std::string currency, CString &tickerlist)
{
    CString format;
    if (ticker)
    {
        format.Format ("OnContextMenu - Early Buys for %s toggled:-", ticker->_ticker.c_str ());
    }
    else
    {
        format.Format ("OnContextMenu - %s early buys toggled %s", currency == "" ? "All" : currency.c_str (), 
                       tickerlist.GetLength () > 0 ? ", tickers changed:-" : "");
    }
    Log (format);
    if (tickerlist.GetLength () > 0)
        Log (tickerlist);
}


void IB_PTraderDlg::ContextMenuSetBuyAverages (DynamicMenuTickerStateChange *tickerstatechange)
{
    if (_tickers == 0)
        return;
    CString format, tickerlist;
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        Ticker *ticker = iter->second;
        if (ticker->_activestate != Ticker::DISABLED)
        {
            if (ticker->_firstdbrow->_currency != tickerstatechange->_currency)
                continue;
            ticker->_buyaverages = tickerstatechange->_buyaverages;
            if (tickerlist.GetLength () > 0)
                tickerlist += ", ";
            tickerlist += ticker->_ticker.c_str ();
        }
    }
    format.Format ("OnContextMenu - %s tickers set to Buy %s:-", tickerstatechange->_currency.c_str (), 
                   tickerstatechange->_buyaverages == Ticker::Always ? "Always" : "OnlyIfBelow");
    Log (format);
    if (tickerlist.GetLength () > 0)
        Log (tickerlist);
}


void IB_PTraderDlg::AddDynamicMenuItem_TickerStates (CMenu &menu, std::string currency, bool somemarketsonholiday, UINT stateflags, Ticker::ActiveState newstate,
                                                     int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    bool isallcurrencies = currency == "";
    std::string menutext = std::string ("Set ") + (isallcurrencies ? "All" : currency) + 
                           std::string (" Tickers ") + std::string ((LPCTSTR) Ticker::ToString (newstate));
    AddDynamicMenuItem (menu, currency, somemarketsonholiday, menutext, stateflags, newstate, (Ticker::BuyAverages) 0, currentmenuid, dynamicmenumaps);
}


void IB_PTraderDlg::AddDynamicMenuItem_TickerStates (CMenu &menu, std::string currency, int &currentmenuid, 
                                                       ContextMenuDynamicMaps &dynamicmenumaps, bool somemarketsonholiday, UINT stateflags, ...)
{
    va_list tickerstates;
    va_start (tickerstates, stateflags);

    bool fp = true;
    for (;;)
    {
        Ticker::ActiveState tickerstate = (Ticker::ActiveState) va_arg (tickerstates, Ticker::ActiveState);
        if (tickerstate == Ticker::DISABLED)
            break;
        if (fp)
            menu.AppendMenuA (MF_SEPARATOR);
        AddDynamicMenuItem_TickerStates (menu, currency, somemarketsonholiday, stateflags, tickerstate, currentmenuid, dynamicmenumaps);
        fp = false;
    }
    va_end (tickerstates);
}


void IB_PTraderDlg::AddDynamicMenuItems_TickerStates (CMenu &menu, bool somemarketsonholiday, UINT stateflags, 
                                                      int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    AddDynamicMenuItem_TickerStates (menu, "", currentmenuid, dynamicmenumaps, somemarketsonholiday, stateflags,
                                     Ticker::InActive, Ticker::SellOnly, Ticker::Active, Ticker::BuyOnly, Ticker::DISABLED);
    AddDynamicMenuItem_TickerStates (menu, "USD", currentmenuid, dynamicmenumaps, somemarketsonholiday, stateflags,
                                     Ticker::InActive, Ticker::SellOnly, Ticker::Active, Ticker::BuyOnly, Ticker::DISABLED);
    AddDynamicMenuItem_TickerStates (menu, "CAD", currentmenuid, dynamicmenumaps, somemarketsonholiday, stateflags,
                                     Ticker::InActive, Ticker::SellOnly, Ticker::Active, Ticker::BuyOnly, Ticker::DISABLED);
    AddDynamicMenuItem_TickerStates (menu, "GBP", currentmenuid, dynamicmenumaps, somemarketsonholiday, stateflags,
                                     Ticker::InActive, Ticker::SellOnly, Ticker::Active, Ticker::BuyOnly, Ticker::DISABLED);
}


void IB_PTraderDlg::AddDynamicMenuItem_ToggleEarlyBuys (CMenu &menu, std::string currency, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    bool isallcurrencies = currency == "";
    std::string menutext = std::string ("Toggle Early Buys for ") + (isallcurrencies ? "All" : currency);
    AddDynamicMenuItem (menu, currency, somemarketsonholiday, menutext, stateflags, Ticker::DISABLED, (Ticker::BuyAverages) 0, currentmenuid, dynamicmenumaps);
}


void IB_PTraderDlg::AddDynamicMenuItems_ToggleEarlyBuys (CMenu &menu, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    menu.AppendMenuA (MF_SEPARATOR);

    AddDynamicMenuItem_ToggleEarlyBuys (menu, "", somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
    AddDynamicMenuItem_ToggleEarlyBuys (menu, "USD", somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
    AddDynamicMenuItem_ToggleEarlyBuys (menu, "CAD", somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
    AddDynamicMenuItem_ToggleEarlyBuys (menu, "GBP", somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
}


void IB_PTraderDlg::AddDynamicMenuItem_BuyAverages (CMenu &menu, std::string currency, Ticker::BuyAverages newbuyaverage, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    std::string menutext = std::string ("Set ") + currency + std::string (" to ");
    switch (newbuyaverage)
    {
        case Ticker::OnlyIfBelow:
            menutext += " Buy OnlyIfBelow";
            break;
        case Ticker::Always:
            menutext += " Buy Always";
            break;
    }
    AddDynamicMenuItem (menu, currency, somemarketsonholiday, menutext, stateflags, (Ticker::ActiveState) 0, newbuyaverage, currentmenuid, dynamicmenumaps);
}


void IB_PTraderDlg::AddDynamicMenuItems_BuyAverages (CMenu &menu, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    menu.AppendMenuA (MF_SEPARATOR);
    AddDynamicMenuItem_BuyAverages (menu, "CAD", Ticker::OnlyIfBelow, somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
    AddDynamicMenuItem_BuyAverages (menu, "CAD", Ticker::Always, somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);

    menu.AppendMenuA (MF_SEPARATOR);
    AddDynamicMenuItem_BuyAverages (menu, "USD", Ticker::OnlyIfBelow, somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
    AddDynamicMenuItem_BuyAverages (menu, "USD", Ticker::Always, somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);

    menu.AppendMenuA (MF_SEPARATOR);
    AddDynamicMenuItem_BuyAverages (menu, "GBP", Ticker::OnlyIfBelow, somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
    AddDynamicMenuItem_BuyAverages (menu, "GBP", Ticker::Always, somemarketsonholiday, stateflags, currentmenuid, dynamicmenumaps);
}


void IB_PTraderDlg::AddDynamicMenuItems_UpdateGlobalCashLimit (std::string currency, long newlimit)
{
    if (_tickers == 0)
    {
        Log ("No Change made - no tickers");
        return;
    }
    CString format, tickerlist;
    IB_PTraderDlg::GlobalCurrencyBuy *maxcurrencybuys = GetGlobalCurrencyBuy (currency);
    maxcurrencybuys->_maxallowedbuys = (double) newlimit;
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        Ticker *ticker = iter->second;
        if (ticker->_activestate != Ticker::DISABLED)
        {
            if (currency == "")
                ;
            else if (ticker->_firstdbrow->_currency != currency)
                continue;
            ticker->_maxglobalcurrencycashexceeded = false;
            if (tickerlist.GetLength () > 0)
                tickerlist += ", ";
            tickerlist += ticker->_ticker.c_str ();
        }
    }
    format.Format ("GlobalCashLimits for %s set to %ld, tickers changed:-", currency.c_str (), newlimit);
    Log (format);
    if (tickerlist.GetLength () > 0)
        Log (tickerlist);
}


void IB_PTraderDlg::AddDynamicMenuItems_ChangeGlobalCashLimit (CMenu &menu, std::string currency, bool validnumber, long number, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps)
{
    UINT stateflags = MF_STRING;
    bool available = validnumber && IsMarketOpen (currency);
    CString outputtext = "Set GlobalCash " + CString (currency.c_str ()) + " to ";
    CString extra;
    if (available)
    {
        extra.Format ("%ld", number);
    }
    else
    {
        stateflags |= MF_GRAYED;
        extra = "No Number";
    }
    DynamicMenuTickerStateChange *dynamenu = new DynamicMenuTickerStateChange ();
    dynamenu->_currency = currency;
    dynamicmenumaps [currentmenuid] = dynamenu;
    outputtext += extra;
    menu.AppendMenuA (stateflags, currentmenuid, outputtext);
    currentmenuid++;  
}


bool IB_PTraderDlg::TidyUp_Handle_OnContextMenu (std::list <ContextMenuDynamicMaps> &dynamicmapslist)
{
    for each (ContextMenuDynamicMaps map in dynamicmapslist)
    {
        AddDynamicMenuItems_TidyUp (map);
    }
    OnBnClickedScrollmessages ();
    OnBnClickedUnselectQuotes ();   
    GetDlgItem (IDC_ORDERID)->SetWindowText ("");
    return true;
}


bool IB_PTraderDlg::Handle_OnContextMenu (CWnd *pwnd, CPoint &point)
{
    CString format, buymenutext, sellmenutext, statsmessage;
    CMenu menu;
    ContextMenuDynamicMaps dynamicmenumaps_globalcashlimits,
                           dynamicmenumaps_tickers, dynamicmenumaps_togglearlybuys,
                           dynamicmenumaps_buyaverages;
    std::list <ContextMenuDynamicMaps> dynamicmapslist;
    dynamicmapslist.push_back (dynamicmenumaps_globalcashlimits);
    dynamicmapslist.push_back (dynamicmenumaps_tickers);
    dynamicmapslist.push_back (dynamicmenumaps_togglearlybuys);
    dynamicmapslist.push_back (dynamicmenumaps_buyaverages);

    LONG origx = point.x;
    LONG origy = point.y;
    _mainlistbox.ScreenToClient (&point);
    Ticker *foundticker = 0;
    int numberitems = _mainlistbox.GetCount ();
    RECT r;
    for (int i = 0;  i < numberitems;  i++)
    {
        _mainlistbox.GetItemRect (i, &r);
        CRect rect = r;
        if (rect.PtInRect (point))
        {
            if (foundticker)
            {
                Log ("OnContextMenu - duplicate tickers found");
                return TidyUp_Handle_OnContextMenu (dynamicmapslist);
            }
            foundticker = (Ticker *) _mainlistbox.GetItemData (i);
        }
    }

    if (foundticker == 0)
    {
        Log ("OnContextMenu - No ticker found");
        return TidyUp_Handle_OnContextMenu (dynamicmapslist);
    }

    if (foundticker->_nullticker)
    {
        return TidyUp_Handle_OnContextMenu (dynamicmapslist);
    }

    if (_locked)
    {
        Log ("OnContextMenu - Trader locked");
        return TidyUp_Handle_OnContextMenu (dynamicmapslist);
    }

    if (foundticker->_tickerhasnoprices)
    {
        Log ("OnContextMenu - No prices available");
        return TidyUp_Handle_OnContextMenu (dynamicmapslist);
    }

    bool editfieldvalid;
    long newlimit = GetEditFieldAsNumber (IDC_ORDERID, editfieldvalid);

    bool buylimitavailable = foundticker->_share ? foundticker->_share->_share_has_buy_daylimit : false;
    UINT menuflagsforbuylimit = GetContextMenuFlags (foundticker, MF_STRING, buylimitavailable,
                                                     "Buy DayLimit for ", editfieldvalid, newlimit, buymenutext);

    bool selllimitavailable = foundticker->_share ? foundticker->_share->_share_has_sell_daylimit : false;
    UINT sellflagsforselllimit = GetContextMenuFlags (foundticker, MF_STRING, selllimitavailable, "Sell DayLimit for ", editfieldvalid, newlimit, sellmenutext);

    menu.CreatePopupMenu ();

    const int MENUID_TOGGLE_EARLYBUYS = 1;

    const int MENUID_SET_BUYLIMIT = 2;
    const int MENUID_SET_SELLLIMIT = 3;
    const int MENUID_RESET_BUYLIMIT = 4;
    const int MENUID_RESET_SELLLIMIT = 5;

    const int MENUID_EXIT = 6;

    const int MENUID_TOGGLE_DISPLAYDIAGNOSTICS = 7;

    const int MENUID_DISPLAYSTATS = 8;

    const int MENUID_ALLBUYS_ON = 9;
    const int MENUID_ALLBUYS_OFF = 10;
    const int MENUID_EVENBUYS_OFF = 11;
    const int MENUID_ODDBUYS_OFF = 12;
    const int MENUID_BUYS_1_IN_3 = 13;
    const int MENUID_BUYS_1_IN_4 = 14;

    const int MENUID_TOGGLE_AUTOLIQUIDATE_INPROFIT = 15;
    const int MENUID_TOGGLE_AUTOLIQUIDATE_ALLBUYS = 16;
    const int MENUID_TOGGLE_AUTOLIQUIDATE_JUSTLOSSES = 17;
    const int MENUID_TOGGLE_AUTOLIQUIDATE_NOLIQUIDATION = 18;

    const int MENUID_LIVE_CHART_2point5_MIN = 19;
    const int MENUID_LIVE_CHART_1_MIN = 20;
    const int MENUID_LIVE_CHART_30_SEC = 21;
    const int MENUID_HISTORIC_CHART = 22;

    const int MENUID_IGNORE_TIs = 23;
    const int MENUID_TIs_DUMP_MINMAXS = 24;

    const int MENUID_BASEID_FORDYNAMICITEMS = 25;

    int chart_day = 0, chart_month = 0, chart_year = 0;
    if (foundticker->_activestate != Ticker::DISABLED)
    {
        CTime now = CTime::GetCurrentTime ();
        CTimeSpan oneday = CTimeSpan (1, 0, 0, 0);
        int dayofweek;
        do
        {
            now -= oneday;
            dayofweek = now.GetDayOfWeek ();
        }
        while (dayofweek == 1 || dayofweek == 7);
        chart_day = now.GetDay ();
        chart_month = now.GetMonth ();
        chart_year = now.GetYear ();

        format.Format ("Historic Chart for %d/%d/%d", chart_day, chart_month, chart_year);
        menu.AppendMenuA (MF_STRING, MENUID_HISTORIC_CHART, (LPCTSTR) format);
        menu.AppendMenuA (MF_SEPARATOR);

        if (foundticker->_livepriceset)
        {
            menu.AppendMenuA (MF_STRING, MENUID_LIVE_CHART_2point5_MIN, "Live Chart 2.5 Min");
            menu.AppendMenuA (MF_STRING, MENUID_LIVE_CHART_1_MIN, "Live Chart 1 Min");
            menu.AppendMenuA (MF_STRING, MENUID_LIVE_CHART_30_SEC, "Live Chart 30 Sec");
            menu.AppendMenuA (MF_SEPARATOR);
        }
    }
        
    if (! IsMarketOnHoliday (foundticker->_firstdbrow->_currency))
    {
        format = foundticker->_allow_early_buys ? "Stop Early Buys for " : "Allow Early Buys for ";
        format += foundticker->_ticker.c_str ();
        menu.AppendMenuA (GetContextMenuFlags (foundticker, format), MENUID_TOGGLE_EARLYBUYS, format);

        menu.AppendMenuA (MF_SEPARATOR);
        menu.AppendMenuA (menuflagsforbuylimit, MENUID_SET_BUYLIMIT, buymenutext);
        menu.AppendMenuA (sellflagsforselllimit, MENUID_SET_SELLLIMIT, sellmenutext);

        int sepcount = 0;
        if (buylimitavailable)
        {
            menu.AppendMenuA (MF_SEPARATOR);
            sepcount++;
            format.Format ("Reset Buy Limit for %s to %g", foundticker->_ticker.c_str (), foundticker->_share->_ORIG_max_allowed_purchases_today);
            menu.AppendMenuA (GetContextMenuFlags (foundticker, format), MENUID_RESET_BUYLIMIT, format);            
        }
        if (selllimitavailable)
        {
            if (sepcount == 0)
                menu.AppendMenuA (MF_SEPARATOR);
            sepcount++;
            format.Format ("Reset Sell Limit for %s to %g", foundticker->_ticker.c_str (), foundticker->_share->_ORIG_max_allowed_sales_today);
            menu.AppendMenuA (GetContextMenuFlags (foundticker, format), MENUID_RESET_SELLLIMIT, format);
        }

        menu.AppendMenuA (MF_SEPARATOR);
    }
    menu.AppendMenuA (MF_STRING, MENUID_EXIT, "Exit");

    menu.AppendMenuA (MF_SEPARATOR);
    statsmessage = CString ("Stats for ") + foundticker->_ticker.c_str ();
    menu.AppendMenuA (MF_STRING, MENUID_DISPLAYSTATS, statsmessage);

    menu.AppendMenuA (MF_SEPARATOR);
    menu.AppendMenuA (MF_STRING, MENUID_ALLBUYS_ON, "All Buys On");
    menu.AppendMenuA (MF_STRING, MENUID_ALLBUYS_OFF, "All Buys Off");
    menu.AppendMenuA (MF_STRING, MENUID_EVENBUYS_OFF, "Even Buys Off");
    menu.AppendMenuA (MF_STRING, MENUID_ODDBUYS_OFF, "Odd Buys Off");
    menu.AppendMenuA (MF_STRING, MENUID_BUYS_1_IN_3, "Buys 1 in 3");
    menu.AppendMenuA (MF_STRING, MENUID_BUYS_1_IN_4, "Buys 1 in 4");

    menu.AppendMenuA (MF_SEPARATOR);

    UINT autoflags = MF_STRING;
    bool disableautoflag = false;
    CTime autoflagtime = CTime::GetCurrentTime ();
    format = "";
    if (! foundticker->_autoliquidate_on)
    {
        disableautoflag = true;
        long hr, min;
        if (editfieldvalid && GetLiquidationHrsMins (newlimit, hr, min))
        {
            autoflagtime = GetLiquidationTime (hr, min);
            format.Format (" at %0.2d:%0.2d", autoflagtime.GetHour (), autoflagtime.GetMinute ());
            disableautoflag = false;
        }
    }
    if (disableautoflag)
        autoflags |= MF_GRAYED; 
    menu.AppendMenuA (autoflags, MENUID_TOGGLE_AUTOLIQUIDATE_INPROFIT, "Auto Liquidate to " + CString ((foundticker->_autoliquidate_on ? "Off" : Ticker::ToDisplayString (Ticker::InProfit).c_str ())) + format);
    menu.AppendMenuA (autoflags, MENUID_TOGGLE_AUTOLIQUIDATE_ALLBUYS, "Auto Liquidate to " + CString ((foundticker->_autoliquidate_on ? "Off" : Ticker::ToDisplayString (Ticker::AllBuys).c_str ())) + format);
    menu.AppendMenuA (autoflags, MENUID_TOGGLE_AUTOLIQUIDATE_JUSTLOSSES, "Auto Liquidate to " + CString ((foundticker->_autoliquidate_on ? "Off" : Ticker::ToDisplayString (Ticker::JustLosses).c_str ())) + format);
    menu.AppendMenuA (autoflags, MENUID_TOGGLE_AUTOLIQUIDATE_NOLIQUIDATION, "Auto Liquidate to " + CString ((foundticker->_autoliquidate_on ? "Off" : Ticker::ToDisplayString (Ticker::NoLiquidation).c_str ())) + format);

    int currentmenuid = MENUID_BASEID_FORDYNAMICITEMS;

    menu.AppendMenuA (MF_SEPARATOR);
    AddDynamicMenuItems_ChangeGlobalCashLimit (menu, "CAD", editfieldvalid, newlimit, currentmenuid, dynamicmenumaps_globalcashlimits);
    AddDynamicMenuItems_ChangeGlobalCashLimit (menu, "GBP", editfieldvalid, newlimit, currentmenuid, dynamicmenumaps_globalcashlimits);
    AddDynamicMenuItems_ChangeGlobalCashLimit (menu, "USD", editfieldvalid, newlimit, currentmenuid, dynamicmenumaps_globalcashlimits);

    menu.AppendMenuA (MF_SEPARATOR);
    menu.AppendMenuA (MF_STRING
#if PUBLIC_BUILD
                      | MF_GRAYED
#endif
                      ,
                      MENUID_TOGGLE_DISPLAYDIAGNOSTICS, 
#if PUBLIC_BUILD
                      "N/A"
#else
                      "Set DisplayDiag to " + CString ((_displayquotesdiagnostics ? "Off" : "On"))
#endif
                     );
    menu.AppendMenuA (MF_STRING, MENUID_DISPLAYSTATS, statsmessage);

    if (foundticker->_technicalindicators)
    {
        menu.AppendMenuA (MF_SEPARATOR);
        menu.AppendMenuA (MF_STRING, MENUID_TIs_DUMP_MINMAXS, "Dump TI Ranges");
        menu.AppendMenuA (MF_STRING, MENUID_IGNORE_TIs, foundticker->_ignoretechnicalindicators ? "Use TIs" : "Ignore TIs");
    }

    UINT tickerstateflags = MF_STRING;
    if (_tickers == 0 || _tickers->GetTickers () == 0)
    {
        tickerstateflags |= MF_GRAYED;
    }
    bool somemarketsonholiday = IsMarketOnHoliday ("USD") || IsMarketOnHoliday ("GBP") || IsMarketOnHoliday ("AUD") || IsMarketOnHoliday ("CAD");

    AddDynamicMenuItems_TickerStates (menu, somemarketsonholiday, tickerstateflags, currentmenuid, dynamicmenumaps_tickers);

    AddDynamicMenuItems_ToggleEarlyBuys (menu, somemarketsonholiday, tickerstateflags, currentmenuid, dynamicmenumaps_togglearlybuys);

    AddDynamicMenuItems_BuyAverages (menu, somemarketsonholiday, tickerstateflags, currentmenuid, dynamicmenumaps_buyaverages);

    menu.AppendMenuA (MF_SEPARATOR);
    menu.AppendMenuA (MF_STRING, MENUID_EXIT, "Exit");
    int result = menu.TrackPopupMenu (TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NOANIMATION,
                                      origx, origy, this);
    menu.DestroyMenu ();

    switch (result)
    {
        case MENUID_TOGGLE_EARLYBUYS:
        {
            CString tickerlist;
            ContextMenuToggleEarlyBuy (foundticker, tickerlist);
            ContextMenuToggleEarlyBuys_Log (foundticker, "", tickerlist);
            break;
        }
        case MENUID_SET_BUYLIMIT:
        {
            if (buylimitavailable)
            {
                double oldlimit = foundticker->_share->_max_allowed_purchases_today;
                foundticker->_share->_max_allowed_purchases_today = (double) newlimit;
                format.Format ("Buy Day Limit for %s changed from %g to %g", foundticker->_ticker.c_str (), oldlimit, foundticker->_share->_max_allowed_purchases_today);
            }
            else
            {
                foundticker->_share->_share_has_buy_daylimit = true;
                foundticker->_share->_max_allowed_purchases_today = (double) newlimit;
                foundticker->_share->_ORIG_max_allowed_purchases_today = (double) newlimit;
                format.Format ("Buy Day Limit for %s set 1st time to %g", foundticker->_ticker.c_str (), foundticker->_share->_max_allowed_purchases_today);
            }
            Log (format);
            if (foundticker->_share->_max_allowed_purchases_today == 0)
            {
                foundticker->_share->_share_has_buy_daylimit = false;
                format.Format ("Buy Day Limit for %s removed", foundticker->_ticker.c_str ());
                Log (format);
            }
            foundticker->_maxdailybuycashexceeded = false;
            Set_DoBuys_DoSells_OnTickerStateChange (foundticker);
            break;
        }
        case MENUID_SET_SELLLIMIT:
        {
            if (selllimitavailable)
            {
                double oldlimit = foundticker->_share->_max_allowed_sales_today;
                foundticker->_share->_max_allowed_sales_today = (double) newlimit;
                format.Format ("Sell Day Limit for %s changed from %g to %g", foundticker->_ticker.c_str (), oldlimit, foundticker->_share->_max_allowed_sales_today);
            }
            else
            {
                foundticker->_share->_share_has_sell_daylimit = true;
                foundticker->_share->_max_allowed_sales_today = (double) newlimit;
                foundticker->_share->_ORIG_max_allowed_sales_today = (double) newlimit;
                format.Format ("Sell Day Limit for %s set 1st time to %g", foundticker->_ticker.c_str (), foundticker->_share->_max_allowed_sales_today);
            }
            Log (format);
            if (foundticker->_share->_max_allowed_sales_today == 0)
            {
                foundticker->_share->_share_has_sell_daylimit = false;
                format.Format ("Sell Day Limit for %s removed", foundticker->_ticker.c_str ());
                Log (format);
            }
            foundticker->_maxdailysellcashexceeded = false;
            Set_DoBuys_DoSells_OnTickerStateChange (foundticker);
            break;
        }
        case MENUID_RESET_BUYLIMIT:
        {
            foundticker->_share->_max_allowed_purchases_today = foundticker->_share->_ORIG_max_allowed_purchases_today;
            format.Format ("Buy Limit for %s reset to %g", foundticker->_ticker.c_str (), foundticker->_share->_max_allowed_purchases_today);
            Log (format);
            foundticker->_maxdailybuycashexceeded = false;
            Set_DoBuys_DoSells_OnTickerStateChange (foundticker);
            break;
        }
        case MENUID_RESET_SELLLIMIT:
        {
            foundticker->_share->_max_allowed_sales_today = foundticker->_share->_ORIG_max_allowed_sales_today;
            format.Format ("Sell Limit for %s reset to %g", foundticker->_ticker.c_str (), foundticker->_share->_max_allowed_sales_today);
            Log (format);
            foundticker->_maxdailysellcashexceeded = false;
            Set_DoBuys_DoSells_OnTickerStateChange (foundticker);
            break;
        }
        case MENUID_TOGGLE_DISPLAYDIAGNOSTICS:
        {
#if ! PUBLIC_BUILD
            _displayquotesdiagnostics = ! _displayquotesdiagnostics;
            format.Format ("Display Diagnostics set to %s", _displayquotesdiagnostics ? "On" : "Off");
            Log (format);
#endif
            break;
        }
        case MENUID_DISPLAYSTATS:
        {
            _stats.DumpStatsForTicker (foundticker, 0);
            break;
        }
        case MENUID_ALLBUYS_ON:
        {
            SetTickersBuyOnOff (foundticker, Ticker::All_On, true);
            break;
        }
        case MENUID_ALLBUYS_OFF:
        {
            SetTickersBuyOnOff (foundticker, Ticker::All_Off, true);
            break;
        }
        case MENUID_EVENBUYS_OFF:
        {
            SetTickersBuyOnOff (foundticker, Ticker::Off_Even, true);
            break;
        }
        case MENUID_ODDBUYS_OFF:
        {
            SetTickersBuyOnOff (foundticker, Ticker::Off_Odd, true);
            break;
        }
        case MENUID_BUYS_1_IN_3:
        {
            SetTickersBuyOnOff (foundticker, Ticker::One_In_Three, true);
            break;
        }
        case MENUID_BUYS_1_IN_4:
        {
            SetTickersBuyOnOff (foundticker, Ticker::One_In_Four, true);
            break;
        }
        case MENUID_TOGGLE_AUTOLIQUIDATE_INPROFIT:
        case MENUID_TOGGLE_AUTOLIQUIDATE_ALLBUYS:
        case MENUID_TOGGLE_AUTOLIQUIDATE_JUSTLOSSES:
        case MENUID_TOGGLE_AUTOLIQUIDATE_NOLIQUIDATION:
        {
            Ticker::LiquidateOptions liquidateoption;
            switch (result)
            {
                case MENUID_TOGGLE_AUTOLIQUIDATE_INPROFIT:
                    liquidateoption = Ticker::InProfit;
                    break;
                case MENUID_TOGGLE_AUTOLIQUIDATE_ALLBUYS:
                    liquidateoption = Ticker::AllBuys;
                    break;
                case MENUID_TOGGLE_AUTOLIQUIDATE_JUSTLOSSES:
                    liquidateoption = Ticker::JustLosses;
                    break;
                case MENUID_TOGGLE_AUTOLIQUIDATE_NOLIQUIDATION:
                default:
                    liquidateoption = Ticker::NoLiquidation;
                    break;
            }
            foundticker->_autoliquidate_on = ! foundticker->_autoliquidate_on;
            foundticker->_liquidationtype = liquidateoption; 
            format.Format ("AutoLiquidate set to %s for %s", (foundticker->_autoliquidate_on ? Ticker::ToDisplayString (foundticker->_liquidationtype).c_str () : "Off"), foundticker->_ticker.c_str ());
            Log (format);
            if (foundticker->_autoliquidate_on)
            {
                foundticker->_autoliquidatetime = autoflagtime;
                format.Format ("AutoLiquidate Time is %0.2d:%0.2d:%0.2d", 
                               foundticker->_autoliquidatetime.GetHour (), 
                               foundticker->_autoliquidatetime.GetMinute (), 
                               foundticker->_autoliquidatetime.GetSecond ());
                Log (format);
            }
            break;
        }
        case MENUID_LIVE_CHART_2point5_MIN:
        {
            CreateChartDataView (foundticker, CChartViewDlg::Live, CChartViewDlg::Min2point5);
            break;
        }
        case MENUID_LIVE_CHART_1_MIN:
        {
            CreateChartDataView (foundticker, CChartViewDlg::Live, CChartViewDlg::Min1);
            break;
        }
        case MENUID_LIVE_CHART_30_SEC:
        {
            CreateChartDataView (foundticker, CChartViewDlg::Live, CChartViewDlg::Sec30);
            break;
        }
        case MENUID_HISTORIC_CHART:
        {
            CreateChartDataView (foundticker, CChartViewDlg::Historic, CChartViewDlg::Min1, chart_day, chart_month, chart_year);
            break;
        }
        case MENUID_IGNORE_TIs:
        {
            foundticker->_ignoretechnicalindicators = ! foundticker->_ignoretechnicalindicators;
            format.Format ("IgnoreTechnicalIndicators now %s for %s", foundticker->_ignoretechnicalindicators ? "True" : "False", foundticker->_ticker.c_str ());
            Log (format);
            Set_DoBuys_DoSells_OnTickerStateChange (foundticker);
            break;
        }
        case MENUID_TIs_DUMP_MINMAXS:
        {
            format.Format ("TI min maxs for %s", foundticker->_ticker.c_str ());
            Log (format);
            for each (TechnicalIndicator *ti in foundticker->_technicalindicators->TechnicalIndicators ())
            {
                format.Format ("%s, Min=%s, Max=%s", ti->Name ().c_str (), ti->GetMinValue ().c_str (), ti->GetMaxValue ().c_str ());
                Log (format);
            }
            break;
        }
        default:
        {
            ContextMenuDynamicMapsIter searchiter = dynamicmenumaps_globalcashlimits.find (result);
            if (searchiter != dynamicmenumaps_globalcashlimits.end ())
            {
                AddDynamicMenuItems_UpdateGlobalCashLimit (searchiter->second->_currency, newlimit);
                break;
            }
            searchiter = dynamicmenumaps_tickers.find (result);
            if (searchiter != dynamicmenumaps_tickers.end ())
            {
                ContextMenuChangeTickersState (searchiter->second->_newstate, searchiter->second->_currency);
                break;
            }
            searchiter = dynamicmenumaps_togglearlybuys.find (result);
            if (searchiter != dynamicmenumaps_togglearlybuys.end ())
            {
                ContextMenuToggleEarlyBuys (searchiter->second->_currency);
                break;
            }
            searchiter = dynamicmenumaps_buyaverages.find (result);
            if (searchiter != dynamicmenumaps_buyaverages.end ())
            {
                ContextMenuSetBuyAverages (searchiter->second);
                break;
            }
            Log ("OnContextMenu - No change made");
            break;
        }
    }
    return TidyUp_Handle_OnContextMenu (dynamicmapslist);
}


void IB_PTraderDlg::OnContextMenu (CWnd *pwnd, CPoint point)
{
    if (pwnd == &_mainlistbox)
    {
        Handle_OnContextMenu (pwnd, point);
    }
}
