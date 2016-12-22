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
    const int MAX_NUMBER_PENDING_BUYS = 99;
    const bool WANT_BUYSELL_LOGGING = false;
}


#pragma region "Buy"
IB_PTraderDlg::BuySellResult IB_PTraderDlg::SendBuyOrder_DB (sql::Statement *stmt, Ticker *ticker, Order &order, BuyPricePair *buyprice, int batchsize, double buycost)
{
    if (UpdateOI_OnDB (stmt, buyprice->_dbrow, buyprice->_size, true))
    {
        if (_ordermaps.PlaceBuyOrder (stmt, order.orderId, buyprice, batchsize, buycost))
        {
            return OK;
        }
    }
    return DB_Error;
}


void IB_PTraderDlg::SendBuyOrder_InMemory (Ticker *ticker, Contract &contract, Order &order, Share *share, BuyPricePair *buyprice, double buycost)
{
    buyprice->SetStatus (PricePair::PENDING);
    buyprice->_oi = buyprice->_size;
    IncrementShareCurrentOI (ticker, share, buyprice->_oi, buycost);
}


void IB_PTraderDlg::Place_BuyOrder (Ticker *ticker, Contract &contract, Order &order, Share *share, BuyPricePair *buyprice, int batchsize)
{
    bool roundedup, stampdutyadded;
    double mincom, maxcom;
    if (! CalculateCommission (OutStandingOrder::BUY, contract, contract.strike, order.totalQuantity, mincom, maxcom, roundedup, stampdutyadded))
    {
        mincom = 0;
        maxcom = 0;
    }
    CString str;
    str.Format ("----> Buying %ld %s at %g (MinCom=%0.2f, MaxCom=%0.2f), sell for %g (Orderid=%ld), batchsize=%i, currentspread=%g", order.totalQuantity, (LPCTSTR) contract.symbol, 
                order.lmtPrice, mincom, maxcom,
                buyprice->_sellpricepair->_price, _currentorderid, batchsize, ticker->_currentoffer - ticker->_currentbid);
    Log ("---->");
    Log (str);
    double contractvalue = contract.strike * (double) order.totalQuantity;
    str.Format ("----> Contract Value %g", contractvalue);
    Log (str);
    Log ("---->");
    PlaceOrder (contract, order);
    if (batchsize > 1)
    {
        _stats.IncrementStat ("Place_BuyOrder", "CombinedOrders", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    ticker->_lastbuytime = CTime::GetCurrentTime ();
    _stats.IncrementStat ("Place_BuyOrder", "Buy", ticker->_ticker, ticker->_firstdbrow->_currency);
}


IB_PTraderDlg::BuySellResult IB_PTraderDlg::SendBuyOrder (Ticker *ticker, Contract &contract, double offer, long buysize,
                                                          BuyPricePair *singleorder, std::list <BuyPricePair *> *orderlist, int batchsize)
{
    Share *share = GetShare (ticker);
    if (share)
    {
        if ((share->_currentoi + buysize) <= share->_maxoi_allowed)
        {
            offer = RoundBidOffer (ticker, offer);
            double estimatedcost = offer * buysize;
            if (share->_share_has_buy_daylimit)
            {
                if ((estimatedcost + share->_totalpurchases_made_today) > share->_max_allowed_purchases_today)
                {
                    _stats.IncrementStat ("SendBuyOrder", "MaxCashDayLimitExceeded", ticker->_ticker, ticker->_firstdbrow->_currency);
                    ticker->_maxdailybuycashexceeded = true;
                    return MaxCashDayLimitExceeded;
                }
            }
            IB_PTraderDlg::GlobalCurrencyBuy *maxcurrencybuys = GetGlobalCurrencyBuy (ticker->_firstdbrow->_currency);
            if ((estimatedcost + maxcurrencybuys->_currenttotal) > maxcurrencybuys->_maxallowedbuys)
            {
                _stats.IncrementStat ("SendBuyOrder", "MaxGlobalCurrencyCashExceeded", ticker->_ticker, ticker->_firstdbrow->_currency);
                _stats.IncrementStat ("SendBuyOrder", std::string ("MaxGlobalCurrencyCashExceeded_") + ticker->_firstdbrow->_currency, ticker->_ticker, ticker->_firstdbrow->_currency);
                ticker->_maxglobalcurrencycashexceeded = true;
                return MaxGlobalCurrencyCashExceeded;
            }
            PortfolioPart *part = GetPortfolioPart (contract);
            if (part)
            {
                double currentholding = part->GetAdjustedMarketValue ();
                if (estimatedcost + currentholding <= share->_maxvalue_allowed)
                    ;
                else
                {
                    _stats.IncrementStat ("SendBuyOrder", "MaxCashExceeded", ticker->_ticker, ticker->_firstdbrow->_currency);
                    ticker->_maxcashexceeded = true;
                    return MaxCashExceeded;
                }
            }
            else
            {
                _stats.IncrementStat ("SendBuyOrder", "PortfolioPart_NotFound", ticker->_ticker, ticker->_firstdbrow->_currency);
            }
            AccountValue *cashbalance = GetCashBalance (ticker->_firstdbrow->_currency);
            if (cashbalance)
            {
                double currentvalue = cashbalance->GetAdjustedCashValue ();
                if (estimatedcost < currentvalue)
                {
                    Order order;
                    order.orderId = _currentorderid;
                    order.clientId = _CLIENTID;
                    order.action = "BUY";
                    order.totalQuantity = buysize;
                    order.orderType = "LMT";
                    order.lmtPrice = offer;
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
                                result = SendBuyOrder_DB (stmt.get (), ticker, order, singleorder, batchsize, estimatedcost);
                            }
                            else
                            {
                                for each (BuyPricePair *buyprice in *orderlist)
                                {
                                    result = SendBuyOrder_DB (stmt.get (), ticker, order, buyprice, batchsize, estimatedcost);
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
                                _stats.IncrementStat ("SendBuyOrder", "rollback", ticker->_ticker, ticker->_firstdbrow->_currency);
                                _dbcon->rollback ();
                            }
                        }
                        catch (...)
                        {
                            try
                            {
                                _stats.IncrementStat ("SendBuyOrder", "rollback", ticker->_ticker, ticker->_firstdbrow->_currency);
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
                        if (singleorder)
                        {
                            SendBuyOrder_InMemory (ticker, contract, order, share, singleorder, estimatedcost);
                            Place_BuyOrder (ticker, contract, order, share, singleorder, batchsize);
                        }
                        else
                        {
                            BuyPricePair *lowestsellprice = 0;
                            double costperorder = estimatedcost / (double) (*orderlist).size ();
                            for each (BuyPricePair *buyprice in *orderlist)
                            {
                                SendBuyOrder_InMemory (ticker, contract, order, share, buyprice, costperorder);
                                if (lowestsellprice)
                                {
                                    if (buyprice->_sellpricepair->_price < lowestsellprice->_sellpricepair->_price)
                                        lowestsellprice = buyprice;
                                }
                                else
                                {
                                    lowestsellprice = buyprice;
                                }
                            }
                            Place_BuyOrder (ticker, contract, order, share, lowestsellprice, batchsize);
                        }
                    }  
                    return result;
                }
                else
                {
                    _stats.IncrementStat ("SendBuyOrder", "LackOfFunds", ticker->_ticker, ticker->_firstdbrow->_currency);
                    return LackOfFunds;
                }
            }
            else
            {
                _stats.IncrementStat ("SendBuyOrder", "NoCashBalanceFound", ticker->_ticker, ticker->_firstdbrow->_currency);
                return NoCashBalanceFound;
            }
        }
        else
        {
            _stats.IncrementStat ("SendBuyOrder", "MaxOIExceeded", ticker->_ticker, ticker->_firstdbrow->_currency);
            ticker->_maxoiexceeded = true;
            return MaxOIExceeded;
        }
    }
    _stats.IncrementStat ("SendBuyOrder", "ShareNotFound", ticker->_ticker, ticker->_firstdbrow->_currency);
    return ShareNotFound;
}


bool IB_PTraderDlg::TradeWouldBeALoss (BuyPricePair *buyprice, Contract &contract, int batchsize, bool &stampdutyadded)
{
    bool roundedup;
    double buy_mincom, buy_maxcom;
    if (CalculateCommission (OutStandingOrder::BUY, contract, contract.strike, buyprice->_sellpricepair->_size, buy_mincom, buy_maxcom, roundedup, stampdutyadded))
    {
        double sell_mincom, sell_maxcom;
        if (CalculateCommission (OutStandingOrder::SELL, contract, buyprice->_sellpricepair->_price, buyprice->_sellpricepair->_size, sell_mincom, sell_maxcom, roundedup, stampdutyadded))
        {
            double com1 = buy_maxcom / (double) batchsize;
            double com2 = sell_maxcom / (double) batchsize;
            double buyamount = buyprice->GetCost (buyprice->_sellpricepair->_size);
            double sellamount = buyprice->_sellpricepair->GetCost ();
            double profit = (sellamount - buyamount) - com1 - com2;
            return profit <= 0;
        }
    }
    return true;
}


bool IB_PTraderDlg::Allow_CheckForBelowAverageBuy (Ticker *ticker, double offer)
{
    if (ticker->_buyaverages == Ticker::Always)
        return true;
    PortfolioPart *part = GetPortfolioPart (ticker);
    if (part)
    {
        if (part->_position == 0)
        {
            _stats.IncrementStat ("Allow_CheckForBelowAverageBuy", "PortfolioPosition_Zero", ticker->_ticker, ticker->_firstdbrow->_currency);
            return true;
        }
        if (ticker->_firstdbrow->_currency == "GBP")
            offer /= 100;
        if (offer < part->_averageCost)
            return true;
        _stats.IncrementStat ("Allow_CheckForBelowAverageBuy", "BuyAboveAverage", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    else
    {
        _stats.IncrementStat ("Allow_CheckForBelowAverageBuy", "PortfolioPartNotFound_BuyAllowed", ticker->_ticker, ticker->_firstdbrow->_currency);
        return true;
    }
    return false;
}


int IB_PTraderDlg::CheckTechnicalIndicators_OK (Ticker *ticker, std::list <TechnicalIndicator::Indicators> &indicators)
{
    int count = 0;
    for each (TechnicalIndicator::Indicators indicator in indicators)
    {
        TechnicalIndicator *ti = ticker->_technicalindicators->GetIndicator (indicator);
        if (ti)
        {
            TechnicalIndicator::BuyIndicator buyindicator = ti->BuyAction ();
            _stats.IncrementStat ("CheckTechnicalIndicators_OK", "TechIndi_" + TechnicalIndicator::ToString (indicator) + "_" + TechnicalIndicator::ToString (buyindicator), 
                                  ticker->_ticker, ticker->_firstdbrow->_currency);
            if (buyindicator == TechnicalIndicator::Buy)
                count++;
        }
        else
        {
            _stats.IncrementStat ("CheckTechnicalIndicators_OK", "TechIndi_" + TechnicalIndicator::ToString (indicator) + "_NotFound", ticker->_ticker, ticker->_firstdbrow->_currency);
        }
    }
    return count;
}


void IB_PTraderDlg::Buy_IndividualOrders (Ticker *ticker, std::list <BuyPricePair *> *orderlist, double offer, int &sentordercount, Contract &contract,
                                          int &pendingbuys, int &pendingsells, BuySellResult &result)
{
    Log ("Multi Buy as separate buys...", WANT_BUYSELL_LOGGING);
    sentordercount = 0;
    for each (BuyPricePair *buyprice in *orderlist)
    {
        bool stampdutyadded;
        if (TradeWouldBeALoss (buyprice, contract, 1, stampdutyadded))
        {
            _stats.IncrementStat ("Buy_IndividualOrders", "TradeWouldBeALoss_IndividualOrder" + std::string ((stampdutyadded ? "_StampDuty" : "")), ticker->_ticker, ticker->_firstdbrow->_currency);
        }
        else
        {
            OutStandingOrder::GetPendingBuysSells (ticker, pendingbuys, pendingsells); 
            if (pendingbuys < MAX_NUMBER_PENDING_BUYS)
            {
                sentordercount++;
                result = SendBuyOrder (ticker, contract, offer, buyprice->_size, buyprice, 0, 1);
                if (result != OK)
                {
                    break;   
                }
            }
            else
            {
                _stats.IncrementStat ("Buy_IndividualOrders", "MaxNumberPendingBuysActive_IndividualOrder", ticker->_ticker, ticker->_firstdbrow->_currency);
                break;
            }
        }
    }
    if (sentordercount != (int) orderlist->size ())
    {
        _stats.IncrementStat ("Buy_IndividualOrders", "SomeBuy_IndividualOrders_Dropped", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
}


void IB_PTraderDlg::Buy (Ticker *ticker)
{ 
    double offer = ticker->_currentoffer;
    BuyPricePair *singleorder = 0;
    std::list <BuyPricePair *> *orderlist = 0;
    BuyPricePair *previousbuyprice = 0;
    int searchstart, searchend;
    ticker->_searchstartresult = ticker->_buys.GetSearchStart (offer, searchstart, searchend, ticker->_makebatchbuy);
    ticker->_searchstartmatchindex = -1;
    bool donotcheckprices = _allowedorders == Stopped || ! ticker->BuysAllowed ();
    double runningbuytotal = 0;
    double buylimit = ticker->_makebatchbuy ? ticker->_share->_max_allowed_purchases_today - ticker->_share->_totalpurchases_made_today : -1;
    for (int i = searchstart;  i <= searchend;  i++)
    {
        BuyPricePair *buyprice = ticker->_buys.GetData (i);
        if (buyprice->_price >= offer)
        {
            if (ticker->_searchstartmatchindex == -1)
            {
                ticker->_searchstartmatchindex = i;
                if (donotcheckprices)
                    return;
            }
            if (buyprice->_turnedon && buyprice->GetStatus () == PricePair::ACTIVE)
            {
                if (previousbuyprice && previousbuyprice->_price >= offer && ! ticker->_makebatchbuy)
                {
                    break;
                }
                else
                {
                    if (ticker->_makebatchbuy)
                    {
                        double buycost = buyprice->GetCostFromPrice (offer);
                        if ((runningbuytotal + buycost) > buylimit)
                        {
                            if (singleorder || orderlist)
                                _stats.IncrementStat ("Buy", "BatchBuy_BuysChopped", ticker->_ticker, ticker->_firstdbrow->_currency);
                            else
                                _stats.IncrementStat ("Buy", "BatchBuy_BuysChopped_WithNoOrders", ticker->_ticker, ticker->_firstdbrow->_currency);
                            break;
                        }
                        runningbuytotal += buycost;
                    }
                    PushOrder <BuyPricePair> (buyprice, singleorder, orderlist);
                }   
            }
        }
        previousbuyprice = buyprice;
    }
    if (! singleorder && ! orderlist)
    {
        return;
    }
    if (! AllowTrade (OutStandingOrder::BUY, ticker, "Buy"))
    {
        return;
    }
    if (! Allow_CheckForBelowAverageBuy (ticker, offer))
    {
        return;
    }
    CString str, tis_message;
    if (ticker->_technicalindicators)
    {
        if (ticker->_ignoretechnicalindicators)
        {
            _stats.IncrementStat ("Buy", "Ignore_TIs", ticker->_ticker, ticker->_firstdbrow->_currency);
        }
        else
        {
            std::list <TechnicalIndicator::Indicators> indicators;
            for each (TechnicalIndicator *ti in ticker->_technicalindicators->TechnicalIndicators ())
            {
                indicators.push_back (ti->TechnicalIndicators ());
            }
            int indicatorallowcount = (int) indicators.size ();
            switch (_accounttype)
            {
                case Live:
                    break;
                case Paper:
                    if (indicatorallowcount > 2)
                        indicatorallowcount--;
                    break;
                default:
                    break;
            } 
            int count = CheckTechnicalIndicators_OK (ticker, indicators);
            str.Format ("%d", count);
            if (count < indicatorallowcount)
            {
                _stats.IncrementStat ("Buy", "Insufficient_TIs_Pass", ticker->_ticker, ticker->_firstdbrow->_currency);
                _stats.IncrementStat ("Buy", "Insufficient_TIs_Pass_Count_" + std::string ((LPCSTR) str), ticker->_ticker, ticker->_firstdbrow->_currency);
                return;
            }
            _stats.IncrementStat ("Buy", "Allowed_TIs_Pass_Count_" + std::string ((LPCSTR) str), ticker->_ticker, ticker->_firstdbrow->_currency);
            tis_message = "TIValues";
            for each (TechnicalIndicator::Indicators indicator in indicators)
            {
                TechnicalIndicator *ti = ticker->_technicalindicators->GetIndicator (indicator);
                if (ti)
                {
                    bool defaultvalue;
                    str.Format ("_%s_%0.2f", TechnicalIndicator::ToString (indicator).c_str (), ti->LatestValue (defaultvalue));
                    tis_message += str;
                }
                else
                {
                    tis_message += "_???";
                }
            }
        }
    }
#if ! PUBLIC_BUILD
    BreaksIter breaksiter = _buybreaks.find (ticker->_ticker);
    if (breaksiter != _buybreaks.end ())
    {
        ::DebugBreak ();
    }
#endif
    double currentspread = offer - ticker->_currentbid;
    if (! SpreadAllowed (ticker, currentspread))
    {
        unsigned long count = _stats.IncrementStat ("Buy", "BuyOrderRejected_Spread_TooLarge", ticker->_ticker, ticker->_firstdbrow->_currency);
        if (count == 1)
        {
            str.Format ("%g", currentspread);
            _stats.IncrementStat ("Buy", std::string ("BuyOrderRejected_Spread_TooLarge_") + std::string ((LPCTSTR) str), ticker->_ticker, ticker->_firstdbrow->_currency);
        }
        return;
    }
    int pendingbuys, pendingsells;
    OutStandingOrder::GetPendingBuysSells (ticker, pendingbuys, pendingsells);    
    BuySellResult result = OK;
    Contract contract;
    MakeContract (ticker, contract, offer);
    if (singleorder)
    {
        #pragma region "Do As Single Order"
        bool stampdutyadded;
        if (TradeWouldBeALoss (singleorder, contract, 1, stampdutyadded))
        {
            _stats.IncrementStat ("Buy", "TradeWouldBeALoss_SingleOrder" + std::string ((stampdutyadded ? "_StampDuty" : "")), ticker->_ticker, ticker->_firstdbrow->_currency);
            return;
        }
        else if (pendingbuys < MAX_NUMBER_PENDING_BUYS)
        {
            result = SendBuyOrder (ticker, contract, offer, singleorder->_size, singleorder, orderlist, 1);
        }
        else
        {
            _stats.IncrementStat ("Buy", "MaxNumberPendingBuysActive_SingleOrder", ticker->_ticker, ticker->_firstdbrow->_currency);
            return;
        }
        #pragma endregion
    }
    else if (orderlist)
    {   
        #pragma region "Determine if cheaper to send multiple orders as 1 order"
        bool useindividualorders = true;
        long totalsize = 0;
        for each (BuyPricePair *buyprice in *orderlist)
        {
            totalsize += buyprice->_size;
        }
        double min_com_combined, max_com_combined;
        bool com_combined_roundedup, stampdutyadded;
        if (CalculateCommission (OutStandingOrder::BUY, contract, contract.strike, totalsize, min_com_combined, max_com_combined, com_combined_roundedup, stampdutyadded))
        {
            bool commissioncalcfailed = false;
            double min_com_runningtotal = 0, max_com_runningtotal = 0;
            for each (BuyPricePair *buyprice in *orderlist)
            {
                double min_com, max_com;
                bool roundedup;
                if (! CalculateCommission (OutStandingOrder::BUY, contract, contract.strike, buyprice->_size, min_com, max_com, roundedup, stampdutyadded))
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
        #pragma region "Send Buy Order"
        int sentordercount = 0;
        if (useindividualorders)
        {
            Buy_IndividualOrders (ticker, orderlist, offer, sentordercount, contract, pendingbuys, pendingsells, result);            
        }
        else
        {
            Log ("Multi Buy in 1 order...", WANT_BUYSELL_LOGGING);
            if (pendingbuys < MAX_NUMBER_PENDING_BUYS)
            {
                for each (BuyPricePair *buyprice in *orderlist)
                {
                    if (TradeWouldBeALoss (buyprice, contract, (int) orderlist->size (), stampdutyadded))
                    {
                        _stats.IncrementStat ("Buy", "TradeWouldBeALoss_MultiBuyOrder" + std::string ((stampdutyadded ? "_StampDuty" : "")), ticker->_ticker, ticker->_firstdbrow->_currency);
                        useindividualorders = true;
                        break;
                    }
                }
                if (useindividualorders)
                {
                    Buy_IndividualOrders (ticker, orderlist, offer, sentordercount, contract, pendingbuys, pendingsells, result);            
                }
                else
                {
                    sentordercount++;
                    result = SendBuyOrder (ticker, contract, offer, totalsize, 0, orderlist, (int) orderlist->size ());
                    if (result != OK)
                    {
                        if (LogError (result))
                            Log (CString ("MULTI Buy Failed with error: ") + ToString (result) + CString (", retrying as individual orders"), WANT_BUYSELL_LOGGING);
                        Buy_IndividualOrders (ticker, orderlist, offer, sentordercount, contract, pendingbuys, pendingsells, result);
                    }
                }
            }
            else        
            {
                _stats.IncrementStat ("Buy", "MaxNumberPendingBuysActive_MultiBuy", ticker->_ticker, ticker->_firstdbrow->_currency);
            }
        }
        #pragma endregion
        delete orderlist;
        if (sentordercount == 0)
            return;
        #pragma endregion
    }
    if (tis_message.GetLength () > 0)
    {
        _stats.IncrementStat ("Buy", std::string ((LPCSTR) tis_message), ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    if (ticker->_makebatchbuy)
    {
        _stats.IncrementStat ("Buy", "BatchBuyMade", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    if (result != OK)
    {
        LogBuySellError ("Buy", result);
    }
}
#pragma endregion
