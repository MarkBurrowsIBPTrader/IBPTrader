/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "AppSettings.h"
#include "ib_ptraderdlg.h"
#include "ib_datadumpdlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif



void IB_PTraderDlg::StartUp (TickersInitialisation initialisation)
{
    LogDateTime ();

    GetDlgItem (IDOK)->EnableWindow (false);
    GetDlgItem (IDC_DB_START)->EnableWindow (false);
    GetDlgItem (IDC_XML_START)->EnableWindow (false);
    GetDlgItem (IDC_ACCOUNTTYPE)->EnableWindow (false);

    CString message;
    if (_locked)
    {
        GetDlgItem (IDC_STARTSTOP)->EnableWindow (false);
    }
    else
    {
        if (ConnectToDatabase ())
        {
            CTime starttime = CTime::GetCurrentTime ();
            InitialiseMarketDetails ();
            InitialiseZeroCommissions ();
            _appsettings->InitialiseCustomSpreads ();
            ReadSharesFromDB ();
            _appsettings->SetShareDayLimits ();
            AdjustShareBuyDayLimits ();
            _appsettings->InitialiseInActiveTickers ();
            _appsettings->InitialiseAllowEarlyBuyTickers ();
            InitialiseTickers (initialisation);
            _appsettings->AddGlobalInActiveTickers ();
            _appsettings->InitialiseTrends ();
            _appsettings->SetBuyBelowAverages ();
            SetUserDefinedTickerAverages ();
            _appsettings->InitialiseGlobalCurrencyBuys ();
            _appsettings->InitialiseTickersToRecord ();
            _appsettings->InitialiseLiquidations ();
            _appsettings->InitialiseTickersBuysOnOff ();
            _appsettings->InitialiseTickersTA ();
#if ! PUBLIC_BUILD
            _appsettings->InitialiseBreaks ();
#endif
            CTime finishtime = CTime::GetCurrentTime ();
            CTimeSpan diff = finishtime - starttime;
            message.Format ("Database load took %I64d seconds", diff.GetTotalSeconds ());
            Log (message);

            // AsIs or Alpha or ByGroup
            switch (_accounttype)
            {
                case Live:
                    UpdateTickerListBox (ByGroup);
                    break;
                case Paper:
                    UpdateTickerListBox (ByGroup);
                    break;                
            }
        }
        else
        {
            Log ("Failed to connect to MySQL database");
            _locked = true;
        }
    }

    Log ("Shares:-");
    if (_shares)
    {
        for (SharesIter iter = _shares->begin ();  iter != _shares->end ();  ++iter)
        {
            Share *share = iter->second;
            message = iter->first.c_str ();
            message += " (CurrentOI=";       
            CString str;
            str.Format ("%ld", share->_currentoi);
            message += str + ", MinOI=";
            str.Format ("%ld", share->_minoi_required);
            message += str + ", MaxOI=";
            str.Format ("%ld", share->_maxoi_allowed);
            message += str + ", MaxValue=";
            str.Format ("%g", share->_maxvalue_allowed);
            message += str + ")";
            if (share->_share_has_buy_daylimit)
            {
                str.Format (" - Buy Day Limit is %g", share->_max_allowed_purchases_today);
                message += str;
            }
            if (share->_share_has_sell_daylimit)
            {
                str.Format (" - Sell Day Limit is %g", share->_max_allowed_sales_today);
                message += str;
            }
            Log (message);
        }
    }
    else
    {
        Log ("None");
    }

    Log ("Connecting to TWS...");
    message.Format ("Connection CLIENTID=%d, PORTID=%u", _CLIENTID, _PORTID);
    Log (message);
    _pclient->eConnect (0, _PORTID, _CLIENTID);
    Log ("Connected");

    Log ("Getting Account");
    _pclient->reqAccountUpdates (true, _username.c_str ());

    std::string accountdetails = ToString (_accounttype) + ", " + _username.c_str () + ", " + _accountnumber.c_str ();
    Log (accountdetails.c_str ());

    long buyspending = 0, sellspending = 0;
    TickerStats buystats, sellstats;
    Log ("Loading Tickers...");
    if (_tickers)
    {
        for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
        {
            Ticker *ticker = iter->second;
            if (ticker->_activestate != Ticker::DISABLED)
            {
                Contract contract;
                contract.conId = ticker->_tickerid;
                contract.symbol = ticker->_ticker.c_str ();
                contract.secType = ticker->_firstdbrow->_sectype.c_str ();
                contract.expiry = _T ("");
                contract.strike = 0.0f;
                contract.right = _T ("");
                contract.multiplier = _T ("");
                contract.exchange = ticker->_firstdbrow->_exchange.c_str ();
                contract.primaryExchange = GetPrimaryExchangeForContract (ticker->_firstdbrow->_primaryexchange);
                contract.currency = ticker->_firstdbrow->_currency.c_str ();
                contract.localSymbol = ticker->_firstdbrow->_localsymbol.c_str ();
                contract.includeExpired = false;
                contract.secIdType = ticker->_firstdbrow->_secidtype.c_str ();
                contract.secId = ticker->_firstdbrow->_secid.c_str ();

                buystats.Reset ();
                ticker->GetTickerStats_Buy (buystats);
                buyspending += buystats.NumberPending ();

                sellstats.Reset ();
                ticker->GetTickerStats_Sell (sellstats);
                sellspending += sellstats.NumberPending ();

                message = CString (ticker->_ticker.c_str ()) + CString (" (") + buystats.GetLogMessage ("BUYS") + sellstats.GetLogMessage (", SELLS") + CString (") - ") + 
                          Ticker::ToString (ticker->_activestate);
                Log (message);
                if (buystats.NumberBought () > 0)
                {
                    message = "";
                    int size = ticker->_buys.GetDataCount ();
                    for (int i = 0;  i < size;  i++)
                    {
                        BuyPricePair *buyprice = ticker->_buys.GetData (i);
                        if (buyprice->GetStatus () == PricePair::BOUGHT)
                        {
                            if (message.GetLength () > 0)
                                message += ", ";
                            else
                                message = "---> ";
                            CString str;
                            str.Format ("%lu (%g)", buyprice->_dbrow->_iddeals, buyprice->_price);
                            message += str;
                            if (message.GetLength () > 80)
                            {
                                Log (message);
                                message = "";
                            }
                        }
                    }
                    if (message.GetLength () > 0)
                        Log (message);
                }

                // Request price feed from TWS
                _pclient->reqMktData (contract.conId, contract, ticker->_firstdbrow->_genericticks.c_str (), false);
            }
            else
            {
                message = CString (ticker->_ticker.c_str ()) + " is DISABLED";
                Log (message);
            }
        }
    }
    else
    {
        Log ("No tickers loaded");
    }
    message.Format ("%ld Buys pending, %ld Sells pending", buyspending, sellspending);
    Log (message);

    SetWindowTitle ();

    const UINT one_second = 1000;

    SetTimer (TWS_MESSAGES_REFRESH_TIMER, one_second, 0);
    SetTimer (CHECKSELLSTARTS_TIMER, 1 * 60 * one_second, 0);
    SetTimer (RECORDPRICES_TIMER, 1 * 30 * one_second, 0);
}

