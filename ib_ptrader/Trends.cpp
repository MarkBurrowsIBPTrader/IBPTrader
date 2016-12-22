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



std::string IB_PTraderDlg::GetTrendTickerKey (std::string ticker, std::string exchange)
{
    return ticker + "_" + exchange;
}


void IB_PTraderDlg::AddTickerTrend (std::string ticker, std::string exchange, std::string currency, int trendcount)
{
    if (trendcount > 1)
    {
        std::string key = GetTrendTickerKey (ticker, exchange);
        _trends_for_tickers [key] = trendcount;
        CString str;
        str.Format ("Trend Count %d", trendcount);
        _stats.IncrementStat ("AddTickerTrend", std::string ((LPCSTR) str), ticker, currency);
    }
}


bool IB_PTraderDlg::AllowTickerTrendCount (Ticker *ticker, SellPricePair *sellprice, double bid)
{
    int trendcount;
    std::string key = GetTrendTickerKey (ticker->_ticker, ticker->_firstdbrow->_primaryexchange);
    TrendsIter iter = _trends_for_tickers.find (key);
    if (iter != _trends_for_tickers.end ())
    {
        trendcount = iter->second;
        ticker->_maxtrendcount = trendcount;
    }
    else
    {
        return true;
    }
    double diff = bid - sellprice->_price;
    sellprice->_trending_hitcount++;
    if (sellprice->_trending_hitcount == 1)
    {
        sellprice->_min_trend_over = diff;
        sellprice->_max_trend_over = diff;
        return false;
    }
    if (diff < sellprice->_min_trend_over)
        sellprice->_min_trend_over = diff;
    if (diff > sellprice->_max_trend_over)
        sellprice->_max_trend_over = diff;
    if (sellprice->_trending_hitcount >= trendcount)
    {
        sellprice->_actual_sellgap = diff;
        return true;
    }
    return false;
}
