/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "share.h"
#include "mysql_connection.h"
#include <memory>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>



std::string Share::GetHashKey (std::string ticker, std::string primaryexch, std::string sectype)
{
    return ticker + "_" + primaryexch + "_" + sectype;
}


std::string Share::GetHashKey (PGenDatabaseRows::PGenDatabaseRow *dbrow)
{
    return GetHashKey (dbrow->_ticker, dbrow->_primaryexchange, dbrow->_sectype);
}


std::string Share::GetHashKey (Share *share)
{
    return GetHashKey (share->_ticker, share->_primaryexchange, share->_sectype);
}


std::string Share::GetHashKey (Contract &contract)
{
    return GetHashKey ((LPCTSTR) contract.symbol, (LPCTSTR) contract.primaryExchange, (LPCTSTR) contract.secType);
}


Share *IB_PTraderDlg::GetShare (Ticker *ticker, std::string &hashkey)
{
    if (ticker && ticker->_share)
        return ticker->_share;
    if (_shares)
    {
        SharesIter sharesiter = _shares->find (hashkey);
        if (sharesiter != _shares->end ())
        {
            return sharesiter->second;
        }
    }
    return 0;
}


Share *IB_PTraderDlg::GetShare (Ticker *ticker, Contract &contract)
{
    if (ticker && ticker->_share)
        return ticker->_share;
    std::string hashkey = Share::GetHashKey (contract);
    return GetShare (0, hashkey);
}


void IB_PTraderDlg::IncrementShareCurrentOI (Ticker *ticker, Share *share, long oi, double buycost)
{
    if (share)
    {
        share->_currentoi += oi;
        share->_totalpurchases_made_today += buycost;
        ticker = Ticker::GetTicker (ticker, share);
        if (ticker)
        {
            IB_PTraderDlg::GlobalCurrencyBuy *maxcurrencybuys = GetGlobalCurrencyBuy (ticker->_firstdbrow->_currency);
            maxcurrencybuys->_currenttotal += buycost;
        }
    }
}


void IB_PTraderDlg::DecrementShareCurrentOI (Ticker *ticker, Share *share, long oi, double buycost, bool cancelorder)
{
    if (share)
    {
        share->_currentoi -= oi;
        if (cancelorder)
        {
            share->_totalpurchases_made_today -= buycost;
            ticker = Ticker::GetTicker (ticker, share);
            if (ticker)
            {
                IB_PTraderDlg::GlobalCurrencyBuy *maxcurrencybuys = GetGlobalCurrencyBuy (ticker->_firstdbrow->_currency);
                maxcurrencybuys->_currenttotal -= buycost;
            }
        }
    }
}


Share::Share () : _share_has_buy_daylimit (false), _max_allowed_purchases_today (0), _totalpurchases_made_today (0), 
                  _share_has_sell_daylimit (false), _max_allowed_sales_today (0), _totalsales_made_today (0), 
                  _tickerptr (0)
{
}


void IB_PTraderDlg::SetShare_Buy_DayLimit (Share *share, double daylimit)
{
    share->_share_has_buy_daylimit = true;
    share->_max_allowed_purchases_today = daylimit;
    share->_ORIG_max_allowed_purchases_today = daylimit;
    share->_totalpurchases_made_today = 0;   
}


void IB_PTraderDlg::SetShare_Buy_DayLimit (std::string ticker, std::string primaryexch, double daylimit)
{
    std::string hashkey = Share::GetHashKey (ticker, primaryexch, "STK");
    Share *share = GetShare (0, hashkey);
    if (share)
    {
        SetShare_Buy_DayLimit (share, daylimit);
    }
}


void IB_PTraderDlg::SetShare_Sell_DayLimit (std::string ticker, std::string primaryexch, double daylimit)
{
    std::string hashkey = Share::GetHashKey (ticker, primaryexch, "STK");
    Share *share = GetShare (0, hashkey);
    if (share)
    {
        share->_share_has_sell_daylimit = true;
        share->_max_allowed_sales_today = daylimit;
        share->_ORIG_max_allowed_sales_today = daylimit;
        share->_totalsales_made_today = 0;   
    }
}


void IB_PTraderDlg::AdjustShareBuyDayLimits ()
{
    /*
        ibrokers.trades (Table)
        ========================
        idtrades         int(10) unsigned PK
        ticker           text
        buyorsell        text
        fillcount        int(11)
        avgfillprice     double
        lastfillprice    double
        totalvalue       double
        commission       double
        mincommission    double
        maxcommission    double
        commissioncurrency text
        buyidtradesrow   int(10) unsigned
        iddealsrowid     int(10) unsigned
        pgenbuyprice     double
        pgensellprice    double
        grossprofit      double
        netprofit        double
        sectype          text
        exchange         text
        primaryexchange  text
        currency         text
        username         text
        accountnumber    text
        createtimestamp  timestamp
        batchsize        int(11)
        mintrend         double
        maxtrend         double
        actualtrend      double
    */
    try
    {
        typedef std::map <std::string, double> TotalBuyCounts; // <ticker,total sum of buys>
        typedef std::map <std::string, double>::iterator TotalBuyCountsIter;
        TotalBuyCounts totalbuycounts; 
        CString format;
        {
            std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
            CTime now = CTime::GetCurrentTime ();
            format.Format ("%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d", now.GetYear (), now.GetMonth (), now.GetDay (), 0, 0, 1);
            std::string sqlquery = std::string (
"select idtrades,ticker,primaryexchange,totalvalue from trades where username= '" + _username + 
std::string ("' and accountnumber = '") + _accountnumber +
"' and buyorsell = '" + OutStandingOrder::ToString (OutStandingOrder::BUY) + "' and createtimestamp > '" +
std::string ((LPCTSTR) format) + "'");
            std::auto_ptr <sql::ResultSet> resultset (stmt->executeQuery (sqlquery));
            while (resultset->next ()) 
            {
                sql::SQLString ticker = resultset->getString (2);
                sql::SQLString primaryexchange = resultset->getString (3);
                double totalvalue = (double) resultset->getDouble (4);

                std::string key = Share::GetHashKey (ticker.asStdString (), primaryexchange.asStdString (), "STK");
                TotalBuyCountsIter finditer = totalbuycounts.find (key);
                if (finditer != totalbuycounts.end ())
                {
                    finditer->second += totalvalue;
                }
                else
                {
                    totalbuycounts [key] = totalvalue;
                }
            }
        }
        for (TotalBuyCountsIter iter = totalbuycounts.begin ();  iter != totalbuycounts.end ();  ++iter)
        {
            std::string key = iter->first;
            Share *share = GetShare (0, key);
            if (share)
            {       
                if (share->_share_has_buy_daylimit)
                {
                    double alreadyspent = iter->second;
                    double newamount = share->_max_allowed_purchases_today - alreadyspent;
                    if (newamount > 0)
                    {
                        format.Format ("%0.2f to %0.2f", share->_max_allowed_purchases_today, newamount);
                        Log (CString (share->_ticker.c_str ()) + " day limit changed from " + format);
                        SetShare_Buy_DayLimit (share, newamount);
                        _stats.IncrementStat ("AdjustShareBuyDayLimits", "NewBuyDayLimitSet_" + std::string ((LPCTSTR) format), share->_ticker, "");
                    }
                    else
                    {
                        Log (CString (share->_ticker.c_str ()) + " day limit reset to 0");
                        SetShare_Buy_DayLimit (share, 0);
                        _stats.IncrementStat ("AdjustShareBuyDayLimits", "NewBuyDayLimitReset_ToZero", share->_ticker, "");
                    }
                }
                else
                {
                    _stats.IncrementStat ("AdjustShareBuyDayLimits", "NoBuyDayLimitSet", share->_ticker, "");
                }
            }
            else
            {
                _stats.IncrementStat ("AdjustShareBuyDayLimits", "CouldNotFindShare_" + iter->first, "", "");
            }
        }
    }
    catch (std::exception &e) 
    {
        Log ("AdjustShareBuyDayLimits - exception thrown");  
        Log (e.what ());
        _stats.IncrementStat ("AdjustShareBuyDayLimits", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("AdjustShareBuyDayLimits - exception thrown");       
        _stats.IncrementStat ("AdjustShareBuyDayLimits", "exception ...", "", "");
    }
}


void IB_PTraderDlg::ReadSharesFromDB ()
{
    Log ("Loading from shares DB");
    _shares = new std::map <std::string, Share *> ();
    try
    {
        /*
            ibrokers.shares (Table)
            ========================
            idshares         int(10) unsigned PK
            tickerid         int(11)
            ticker           text
            displayname      text
            primaryexchange  text
            sectype          text
            minoi            int(11)
            maxoi            int(11)
            maxvalue         double
            username         text
            accountnumber    text
        */
        std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
        std::string sqlquery = std::string ("select tickerid, ticker, displayname, primaryexchange, sectype, minoi, maxoi, shares.maxvalue from shares where username= '") + _username + 
                                            std::string ("' and accountnumber = '" + _accountnumber + std::string ("'"));
        std::auto_ptr <sql::ResultSet> resultset (stmt->executeQuery (sqlquery));
        while (resultset->next ()) 
        {
            Share *share = new Share ();
            share->_tickerid = resultset->getInt (1);
            share->_ticker = resultset->getString (2);
            share->_displayname = resultset->getString (3);
            share->_primaryexchange = resultset->getString (4);
            share->_sectype = resultset->getString (5);
            share->_minoi = resultset->getInt (6);
            share->_maxoi = resultset->getInt (7);
            share->_maxvalue = (double) resultset->getDouble (8);

            share->_minoi_required = share->_minoi;
            share->_maxoi_allowed = share->_maxoi;
            share->_maxvalue_allowed = share->_maxvalue;
            share->_currentoi = 0;

            std::string hashkey = Share::GetHashKey (share);
            (*_shares) [hashkey] = share;
        }
    }
    catch (std::exception &e) 
    {
        Log ("ReadSharesFromDB - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("ReadSharesFromDB", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("ReadSharesFromDB - exception thrown");
        _stats.IncrementStat ("ReadSharesFromDB", "exception ...", "", "");
    }
}
