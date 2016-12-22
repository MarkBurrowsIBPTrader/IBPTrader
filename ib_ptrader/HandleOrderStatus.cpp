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



bool IB_PTraderDlg::ResetBuyOrders (OutStandingOrder *outstandingorder, OrderId orderId, Share * &share)
{
    share = 0;
    bool commit = false;
    _dbcon->setAutoCommit (false);
    try
    {
        std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
        if (_ordermaps.RemoveOrderId_FromDB (stmt.get (), orderId))
        {
            commit = true;
            for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
            {
                if (share == 0)
                {
                    std::string sharehashkey = Share::GetHashKey (buyprice->_dbrow);
                    share = GetShare (outstandingorder->_ticker, sharehashkey);
                    if (share == 0)
                    {
                        commit = false;
                        break;
                    }
                }
                if (! UpdateOI_OnDB (stmt.get (), buyprice->_dbrow, 0, false))
                {
                    commit = false;
                    break;
                }
            }
        }
        if (commit && share)
        {
            _dbcon->commit ();
        }
        else
        {
            _stats.IncrementStat ("ResetBuyOrders", "rollback", outstandingorder->_ticker->_ticker, outstandingorder->_ticker->_firstdbrow->_currency);
            commit = false;
            _dbcon->rollback ();
        }
    }
    catch (...)
    {
        try
        {
            _stats.IncrementStat ("ResetBuyOrders", "rollback", outstandingorder->_ticker->_ticker, outstandingorder->_ticker->_firstdbrow->_currency);
            _dbcon->rollback ();
        }
        catch (...)
        {
        }
        commit = false;
    }
    _dbcon->setAutoCommit (true);
    return commit;
}


void IB_PTraderDlg::CancelBuyOrder (OutStandingOrder *outstandingorder, OrderId orderId)
{
    Share *share;
    if (ResetBuyOrders (outstandingorder, orderId, share))
    {
        double costperorder = outstandingorder->_buycost / (double) outstandingorder->_buypricepairs.size ();
        for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
        {
            buyprice->SetStatus (PricePair::CANCELLED);
            DecrementShareCurrentOI (outstandingorder->_ticker, share, buyprice->_oi, costperorder, true);
            buyprice->_oi = 0;
        }
        if (outstandingorder->_ticker)
            outstandingorder->_ticker->_number_of_cancelledbuys += outstandingorder->_batchsize;
        DisplayOrderCompleted (outstandingorder, "BUY order Cancelled processed", false);
        _ordermaps.RemoveOrderId_FromMemory (orderId);
    }
    else
    {
        DisplayOrderCompleted (outstandingorder, "BUY order CANCELLED processing FAILED", false);
    }
}


void IB_PTraderDlg::CancelSellOrder (OutStandingOrder *outstandingorder, OrderId orderId)
{
    if (_ordermaps.RemoveOrderId_FromDB (0, orderId))
    {
        for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
        {
            buyprice->_sellpricepair->SetStatus (PricePair::CANCELLED);                
        }
        if (outstandingorder->_ticker)
            outstandingorder->_ticker->_number_of_cancelledsells += outstandingorder->_batchsize;
        DisplayOrderCompleted (outstandingorder, "SELL order Cancelled processed", false);
        _ordermaps.RemoveOrderId_FromMemory (orderId);
    }
    else
    {
        DisplayOrderCompleted (outstandingorder, "SELL order CANCELLED processing FAILED", false);
    }
}


void IB_PTraderDlg::CancelOrder (OutStandingOrder *outstandingorder, OrderId orderId)
{
    switch (outstandingorder->GetOrderType ())
    {
        case OutStandingOrder::BUY:
            CancelBuyOrder (outstandingorder, orderId);
            break;
        case OutStandingOrder::SELL:
            CancelSellOrder (outstandingorder, orderId);
            break;
    }
}


void IB_PTraderDlg::DisplayOrderCompleted (OutStandingOrder *outstandingorder, LPCTSTR message, bool recalculateavg)
{
    if (outstandingorder)
    {
        CString displaymessage (message);
        if (outstandingorder->_timerecorded)
        {
            int hour = outstandingorder->_recordedtime.GetHour (); 
            int min = outstandingorder->_recordedtime.GetMinute ();
            int sec = outstandingorder->_recordedtime.GetSecond ();
            CString timeasstr;
            timeasstr.Format (" - order placed at %0.2d:%0.2d:%0.2d", hour, min, sec);
            displaymessage += timeasstr;
        }
        else
        {
            displaymessage += " - order placed yesterday";
        }
        Log (displaymessage);
        if (recalculateavg && outstandingorder->_ticker)
        {
            double oldaverage = outstandingorder->_ticker->_userdefined_avg;
            SetUserDefinedTickerAverage (outstandingorder->_ticker);
            displaymessage.Format ("Old Buy Avg %g, New Avg %g", oldaverage, outstandingorder->_ticker->_userdefined_avg);
            Log (displaymessage);
        }
    }
    else
    {
        Log (message);
    }
}


CString IB_PTraderDlg::FormatHandleOrderStatusMessage_ForFilled (OutStandingOrder::OrderType ordertype, OutStandingOrder *outstandingorder)
{
    CString format;
    CString str = "(";
    bool fp = true;
    for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
    {
        if (fp)
            fp = false;
        else
            str += ", ";
        double value;
        if (ordertype == OutStandingOrder::BUY)
            value = buyprice->_price;
        else
            value = buyprice->_sellpricepair->_price;
        format.Format ("%g", value);
        str += format;
    }
    str += ")";
    return str;
}


void IB_PTraderDlg::HandleOrderStatus (OrderStatusKind statuskind, OutStandingOrder *outstandingorder, OrderId orderId, const IBString &status, 
                                       int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice,
                                       int clientId, const IBString &whyHeld)
{
    if (outstandingorder->GetOrderType () == OutStandingOrder::BUY)
    {
        #pragma region "Buy"
        if (statuskind == Filled)
        {
            #pragma region "Filled Buy"
            if (outstandingorder->ValidComissionReceived ())
            {
                CString pricesformat = FormatHandleOrderStatusMessage_ForFilled (OutStandingOrder::BUY, outstandingorder);
                CString str;
                str.Format ("%s %s Processing BUY order...", outstandingorder->_ticker->_ticker.c_str (), (LPCTSTR) pricesformat);
                Log (str);
                if (_ordermaps.RemoveOrderId_FromDB (0, orderId))
                {
                    if (outstandingorder->_ticker)
                    {
                        outstandingorder->_ticker->_numberofbuys += outstandingorder->_batchsize;
                        outstandingorder->_ticker->_totalbuycost += (filled * avgFillPrice);
                    }
                    Record_BuyTrade (outstandingorder, orderId, status, filled, remaining, avgFillPrice, permId, 
                                     parentId, lastFillPrice, clientId, whyHeld);

                    int numberoftrades = (int) outstandingorder->_buypricepairs.size ();
                    double avgcommission = GetCommissionPerTrade (outstandingorder->_orderstate.commission, numberoftrades);
                    bool commit = true;
                    {
                        _dbcon->setAutoCommit (false);
                        try
                        {
                            std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
                            for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
                            {
                                buyprice->_lastbuysset = true;
                                buyprice->_boughttoday = true;
                                buyprice->_lastavgfill = avgFillPrice;
                                buyprice->_lastbuycommission = avgcommission;
                                buyprice->SetStatus (PricePair::BOUGHT);
                                buyprice->_sellpricepair->SetStatus (PricePair::ACTIVE);
                                if (commit)
                                {
                                    if (! Update_AvgFillPrice_LastBuyDate_OnDB (stmt.get (), buyprice->_dbrow, avgFillPrice, avgcommission))
                                        commit = false;
                                }
                            }
                            if (commit)
                            {
                                _dbcon->commit ();
                            }
                            else
                            {
                                _stats.IncrementStat ("HandleOrderStatus", "rollback", outstandingorder->_ticker->_ticker, outstandingorder->_ticker->_firstdbrow->_currency);
                                commit = false;
                                _dbcon->rollback ();
                            }
                        }
                        catch (...)
                        {
                            try
                            {
                                _stats.IncrementStat ("HandleOrderStatus", "rollback", outstandingorder->_ticker->_ticker, outstandingorder->_ticker->_firstdbrow->_currency);
                                _dbcon->rollback ();
                            }
                            catch (...)
                            {
                            }
                            commit = false;
                        }
                        _dbcon->setAutoCommit (true);
                    }

                    DisplayOrderCompleted (outstandingorder, "BUY order processed", true);
                    _ordermaps.RemoveOrderId_FromMemory (orderId);
                }
                else
                {
                    DisplayOrderCompleted (outstandingorder, "BUY order FILLED processing FAILED", false);
                }
            }
            else
            {
                Log ("BUY order Waiting for Valid commission...");
            }
            #pragma endregion
        }
        else if (statuskind == Cancelled)
        {
            #pragma region "Cancelled Buy"
            CancelBuyOrder (outstandingorder, orderId);
            #pragma endregion
        }
        #pragma endregion
    }
    else if (outstandingorder->GetOrderType () == OutStandingOrder::SELL)
    {
        #pragma region "Sell"
        if (statuskind == Filled)
        {
            #pragma region "Filled Sell"
            if (outstandingorder->ValidComissionReceived ())
            {
                CString pricesformat = FormatHandleOrderStatusMessage_ForFilled (OutStandingOrder::SELL, outstandingorder);
                CString str;
                str.Format ("%s %s Processing SELL order...", outstandingorder->_ticker->_ticker.c_str (), (LPCTSTR) pricesformat);
                Log (str);
                Share *share;
                if (ResetBuyOrders (outstandingorder, orderId , share))
                {
                    if (outstandingorder->_ticker)
                    {
                        outstandingorder->_ticker->_numberofsells += outstandingorder->_batchsize;
                        outstandingorder->_ticker->_salestotal += (filled * avgFillPrice);
                    }
                    Record_SellTrade (outstandingorder, orderId, status, filled, remaining, avgFillPrice, permId, 
                                      parentId, lastFillPrice, clientId, whyHeld);
                    for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
                    {
                        buyprice->_sellpricepair->SetStatus (PricePair::BOUGHT);
                        buyprice->SetStatus (PricePair::ACTIVE);
                        DecrementShareCurrentOI (outstandingorder->_ticker, share, buyprice->_oi, 0, false);
                        buyprice->_oi = 0;
                    }
                    DisplayOrderCompleted (outstandingorder, "SELL order processed", true);
                    outstandingorder->_ticker->_maxcashexceeded = false;
                    outstandingorder->_ticker->_maxoiexceeded = false;
                    _ordermaps.RemoveOrderId_FromMemory (orderId);
                }
                else
                {
                    DisplayOrderCompleted (outstandingorder, "SELL order FILLED processing FAILED", false);
                }
            }
            else
            {
                Log ("SELL order Waiting for Valid commission...");
            }
            #pragma endregion
        }
        else if (statuskind == Cancelled)
        {
            #pragma region "Cancelled Sell"
            CancelSellOrder (outstandingorder, orderId);
            #pragma endregion
        }
        #pragma endregion
    }
}

