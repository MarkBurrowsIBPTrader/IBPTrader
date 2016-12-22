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
#include <memory>



namespace
{
    const int MAX_NUMBER_PENDING_SELLS = 99;
    const bool WANT_BUYSELL_LOGGING = false;
}


#pragma region "Sell"
IB_PTraderDlg::BuySellResult IB_PTraderDlg::SendSellOrder_DB (sql::Statement *stmt, Ticker *ticker, Order &order, SellPricePair *sellprice, int batchsize)
{
    if (_ordermaps.PlaceSellOrder (stmt, order.orderId, sellprice, batchsize))
    {
        return OK;
    }
    return DB_Error;
}


void IB_PTraderDlg::SendSellOrder_InMemory (Ticker *ticker, Contract &contract, Order &order, Share *share, SellPricePair *sellprice)
{
    sellprice->SetStatus (PricePair::PENDING);
}


void IB_PTraderDlg::Place_SellOrder (Ticker *ticker, Contract &contract, Order &order, Share *share, bool trendingsellorder, 
                                     SellPricePair *singleorder, std::list <SellPricePair *> *orderlist, int batchsize)
{
    bool roundedup, stampdutyadded;
    double mincom, maxcom;
    if (! CalculateCommission (OutStandingOrder::SELL, contract, contract.strike, order.totalQuantity, mincom, maxcom, roundedup, stampdutyadded))
    {
        mincom = 0;
        maxcom = 0;
    }
    CString str;
    str.Format ("----> Selling %ld %s at %g (MinCom=%0.2f, MaxCom=%0.2f) (Orderid=%ld), batchsize=%i", order.totalQuantity, (LPCTSTR) contract.symbol, order.lmtPrice, mincom, maxcom, _currentorderid, batchsize);
    Log ("---->");
    Log (str);
    double contractvalue = contract.strike * (double) order.totalQuantity;
    str.Format ("----> Contract Value %g", contractvalue);
    Log (str);
    if (trendingsellorder)
    {
        CString header = "----> Trends";
        if (singleorder)
        {
            singleorder->FormatTrendForDisplay (str);
            Log (header + str);
        }
        else
        {
            CString message = header;
            for each (SellPricePair *sellprice in *orderlist)
            {
                sellprice->FormatTrendForDisplay (str);
                message += str;
            }
            Log (message);
        }
    }
    Log ("---->");
    PlaceOrder (contract, order);
    if (batchsize > 1)
    {
        _stats.IncrementStat ("Place_SellOrder", "CombinedOrders", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    _stats.IncrementStat ("Place_SellOrder", "Sell", ticker->_ticker, ticker->_firstdbrow->_currency);
}


IB_PTraderDlg::BuySellResult IB_PTraderDlg::SendSellOrder (Ticker *ticker, Contract &contract, double bid, long sellsize,
                                                           SellPricePair *singleorder, std::list <SellPricePair *> *orderlist, int batchsize)
{
    Share *share = GetShare (ticker);
    if (share)
    {
        if ((share->_currentoi - sellsize) >= share->_minoi_required)
        {
            bid = RoundBidOffer (ticker, bid);
            Order order;
            order.orderId = _currentorderid;
            order.clientId = _CLIENTID;
            order.action = "SELL";
            order.totalQuantity = sellsize;
            order.orderType = "LMT";
            order.lmtPrice = bid;
            order.tif = "DAY";
            order.transmit = true;

            BuySellResult result = UNKNOWN;
            {
                _dbcon->setAutoCommit (false);
                try
                {
                    std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
                    if (singleorder)
                    {
                        result = SendSellOrder_DB (stmt.get (), ticker, order, singleorder, batchsize);
                    }
                    else
                    {
                        for each (SellPricePair *sellprice in *orderlist)
                        {
                            result = SendSellOrder_DB (stmt.get (), ticker, order, sellprice, batchsize);
                            if (result != OK)
                                break;
                        }
                    }
                    if (result == OK)
                    {
                        _dbcon->commit ();
                    }
                    else
                    {
                        _stats.IncrementStat ("SendSellOrder", "rollback", ticker->_ticker, ticker->_firstdbrow->_currency);
                        _dbcon->rollback ();
                    }
                }
                catch (...)
                {
                    try
                    {
                        _stats.IncrementStat ("SendSellOrder", "rollback", ticker->_ticker, ticker->_firstdbrow->_currency);
                        _dbcon->rollback ();
                    }
                    catch (...)
                    {
                    }
                    if (result == OK)
                        result = DB_Error;
                }
                _dbcon->setAutoCommit (true);
            }
            if (result == OK)
            {
                bool trendingsellorder;
                if (singleorder)
                {
                    SendSellOrder_InMemory (ticker, contract, order, share, singleorder);
                    trendingsellorder = singleorder->_trending_hitcount > 0;
                }
                else
                {
                    trendingsellorder = false;
                    for each (SellPricePair *sellprice in *orderlist)
                    {
                        SendSellOrder_InMemory (ticker, contract, order, share, sellprice);
                        if (sellprice->_trending_hitcount > 0)
                            trendingsellorder = true;
                    }
                }
                Place_SellOrder (ticker, contract, order, share, trendingsellorder, singleorder, orderlist, batchsize);
            }
            return result;
        }
        else
        {
            _stats.IncrementStat ("SendSellOrder", "MinOIBreached", ticker->_ticker, ticker->_firstdbrow->_currency);
            return MinOIBreached;
        }
    }
    _stats.IncrementStat ("SendSellOrder", "ShareNotFound", ticker->_ticker, ticker->_firstdbrow->_currency);
    return ShareNotFound;
}


void IB_PTraderDlg::Sell_IndividualOrders (Ticker *ticker, std::list <SellPricePair *> *orderlist, double bid, int &sentordercount, Contract &contract,
                                           int &pendingbuys, int &pendingsells, BuySellResult &result)
{
    sentordercount = 0;
    Log ("Multi Sell as separate sells...", WANT_BUYSELL_LOGGING);
    for each (SellPricePair *sellprice in *orderlist)
    {
        OutStandingOrder::GetPendingBuysSells (ticker, pendingbuys, pendingsells); 
        if (pendingsells < MAX_NUMBER_PENDING_SELLS)
        {
            sentordercount++;
            result = SendSellOrder (ticker, contract, bid, sellprice->_size, sellprice, 0, 1);
            if (result != OK)
            {
                break;   
            }
        }
        else
        {
            _stats.IncrementStat ("Sell_IndividualOrders", "MaxNumberPendingSellsActive_IndividualOrder", ticker->_ticker, ticker->_firstdbrow->_currency);
            break;
        }
    }
    if (sentordercount != (int) orderlist->size ())
    {
        _stats.IncrementStat ("Sell_IndividualOrders", "SomeSell_IndividualOrders_Dropped", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
}


void IB_PTraderDlg::GetSellSearchParts (Ticker *ticker, int &startsearch, int &endsearch)
{
    bool searchtoend = ticker->_share->_share_has_sell_daylimit;
    int lowestbought_searchstart, lowestbought_searchend;
    if (ticker->_sellsstartpoint)
    {
        ticker->_sells.GetSearchStart (ticker->_sellsstartpoint->_sellpricepair->_price, 
                                       lowestbought_searchstart, lowestbought_searchend, searchtoend);
    }
    else
    {
        lowestbought_searchstart = 0;
        lowestbought_searchend = 0;
    }
    int bid_searchstart, bid_searchend;
    ticker->_sells.GetSearchStart (ticker->_currentbid, bid_searchstart, bid_searchend, searchtoend);
    ticker->_sellstartpoint_startindex = lowestbought_searchstart;
    ticker->_sellstartpoint_endindex = bid_searchend;
#if ! PUBLIC_BUILD
    if (ticker->_sellsearchspeed == Ticker::Slow)
    {
        lowestbought_searchstart = 0;
    }
#endif
    startsearch = lowestbought_searchstart;
    endsearch = bid_searchend;
}


void IB_PTraderDlg::Sell (Ticker *ticker)
{
    int startsearch, endsearch;
    GetSellSearchParts (ticker, startsearch, endsearch);
    if (_allowedorders == Stopped || ! ticker->SellsAllowed ())
    {
        return;
    }
    if (ticker->_share->_currentoi == 0)
    {
        return;
    }
    double bid = ticker->_currentbid;
    SellPricePair *singleorder = 0;
    std::list <SellPricePair *> *orderlist = 0;
    double runningselltotal = 0;
    Share *share = ticker->_share;
    double amountsellleft = share->_share_has_sell_daylimit ? share->_max_allowed_sales_today - share->_totalsales_made_today : 0;
    for (int i = startsearch;  i <= endsearch;  i++)
    {
        SellPricePair *sellprice = ticker->_sells.GetData (i);
        if (sellprice->_price <= bid)
        {
            if (sellprice->_buypricepair->GetStatus () == PricePair::BOUGHT && 
                sellprice->GetStatus () == PricePair::ACTIVE)
            {
                if (AllowTickerTrendCount (ticker, sellprice, bid))
                {
                    if (ticker->_share->_share_has_sell_daylimit)
                    {
                        double saleprice = sellprice->GetCostFromPrice (bid);
                        if ((runningselltotal + saleprice) > amountsellleft)
                        {
                            ticker->_maxdailysellcashexceeded = true;
                            if (singleorder || orderlist)
                            {
                                _stats.IncrementStat ("Sell", "BatchSell_SellsChopped", ticker->_ticker, ticker->_firstdbrow->_currency);
                            }
                            else
                            {
                                _stats.IncrementStat ("Sell", "BatchSell_SellsChopped_WithNoOrders", ticker->_ticker, ticker->_firstdbrow->_currency);
                            }
                            break;
                        }
                        runningselltotal += saleprice;
                    }
                    PushOrder <SellPricePair> (sellprice, singleorder, orderlist);
                }
            }
        }
        else
        {
            break;
        }
    }
    if (! singleorder && ! orderlist)
    {
        return;
    }
    if (! AllowTrade (OutStandingOrder::SELL, ticker, "Sell"))
    {
        return;
    }
#if ! PUBLIC_BUILD
    BreaksIter breaksiter = _sellbreaks.find (ticker->_ticker);
    if (breaksiter != _sellbreaks.end ())
    {
        ::DebugBreak ();
    }
#endif
    int pendingbuys, pendingsells;
    OutStandingOrder::GetPendingBuysSells (ticker, pendingbuys, pendingsells); 
    BuySellResult result = OK;
    Contract contract;
    MakeContract (ticker, contract, bid);
    if (singleorder)
    {
        #pragma region "Do As Single Order"
        if (pendingsells < MAX_NUMBER_PENDING_SELLS)
        {
            result = SendSellOrder (ticker, contract, bid, singleorder->_size, singleorder, orderlist, 1);
        }
        else
        {
            _stats.IncrementStat ("Sell", "MaxNumberPendingSellsActive_SingleOrder", ticker->_ticker, ticker->_firstdbrow->_currency);
            return;
        }
        #pragma endregion
    }
    else if (orderlist)
    {
        #pragma region "Determine if cheaper to send multiple orders as 1 order"
        bool useindividualorders = true;
        long totalsize = 0;
        for each (SellPricePair *sellprice in *orderlist)
        {
            totalsize += sellprice->_size;
        }
        double min_com_combined, max_com_combined;
        bool com_combined_roundedup, stampdutyadded;
        if (CalculateCommission (OutStandingOrder::SELL, contract, contract.strike, totalsize, min_com_combined, max_com_combined, com_combined_roundedup, stampdutyadded))
        {
            bool commissioncalcfailed = false;
            double min_com_runningtotal = 0, max_com_runningtotal = 0;
            for each (SellPricePair *sellprice in *orderlist)
            {
                double min_com, max_com;
                bool roundedup;
                if (! CalculateCommission (OutStandingOrder::SELL, contract, contract.strike, sellprice->_size, min_com, max_com, roundedup, stampdutyadded))
                {
                    commissioncalcfailed = true;
                    break;
                }
                min_com_runningtotal += min_com;
                max_com_runningtotal += max_com;
            }
            if (! commissioncalcfailed)
            {
                if (com_combined_roundedup || (min_com_runningtotal == 0 && max_com_runningtotal == 0))
                {
                    useindividualorders = false;
                }
            }
        }
        #pragma region "Send Sell Order"
        int sentordercount = 0;
        if (useindividualorders)
        {
            Sell_IndividualOrders (ticker, orderlist, bid, sentordercount, contract, pendingbuys, pendingsells, result);
        }
        else
        {
            Log ("Multi Sell in 1 order...", WANT_BUYSELL_LOGGING);
            if (pendingsells < MAX_NUMBER_PENDING_SELLS)
            {
                sentordercount++;
                result = SendSellOrder (ticker, contract, bid, totalsize, 0, orderlist, (int) orderlist->size ());
                if (result != OK)
                {
                    if (LogError (result))
                        Log (CString ("MULTI Sell Failed with error: ") + ToString (result) + CString (", retrying as individual orders"), WANT_BUYSELL_LOGGING);
                    useindividualorders = true;
                    Sell_IndividualOrders (ticker, orderlist, bid, sentordercount, contract, pendingbuys, pendingsells, result);
                }
            }
            else
            {
                _stats.IncrementStat ("Sell", "MaxNumberPendingSellsActive_MultiBuy", ticker->_ticker, ticker->_firstdbrow->_currency);
            }
        }
        #pragma endregion
        delete orderlist;
        if (sentordercount == 0)
            return;
        #pragma endregion
    }
    if (share->_share_has_sell_daylimit)
    {
        share->_totalsales_made_today += runningselltotal;
    }
    if (result != OK)
    {
        LogBuySellError ("Sell", result);
    }
}
#pragma endregion
