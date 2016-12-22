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
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



void IB_PTraderDlg::InitialiseLiquidation (std::string ticker, std::string primaryexch, std::string sectype, Ticker::LiquidateOptions liquidateption, long time)
{
    Ticker *foundticker = _tickers ? _tickers->GetTickerByContract (ticker, primaryexch, sectype) : 0;
    if (foundticker)
    {
        long hr, min;
        bool valid = GetLiquidationHrsMins (time, hr, min);
        if (valid)
        {
            CTime liquidatetime = GetLiquidationTime (hr, min);
            foundticker->_autoliquidate_on = true;
            foundticker->_autoliquidate_DONE = false;
            foundticker->_autoliquidatetime = liquidatetime;
            foundticker->_liquidationtype = liquidateption;
        }
    }
}


bool IB_PTraderDlg::GetLiquidationHrsMins (long time, long &hr, long &min)
{
    if (time >= 800 && time <= 2100)
    {
        hr = time / 100;
        min = time - (hr * 100);
        if ((hr >= 8 && hr <= 21) && (min >= 0 && min <= 59))
        {
            return true;
        }
    }
    return false;
}


CTime IB_PTraderDlg::GetLiquidationTime (long hr, long min)
{
    CTime now = CTime::GetCurrentTime ();
    return CTime (now.GetYear (), now.GetMonth (), now.GetDay (), hr, min, 0);
}


int IB_PTraderDlg::Liquidations::Count ()
{
    return (int) _liquidations.size ();
}


long IB_PTraderDlg::Liquidations::NumberOfShares ()
{
    long totalsize = 0;
    for each (BuyPricePair *buyprice in _liquidations)
    {
        totalsize += buyprice->_sellpricepair->_size;
    }
    return totalsize;
}


void IB_PTraderDlg::Liquidations::Reset ()
{
    _liquidations.clear ();
    _totalsum = 0;
    _PnL = 0;
}


void IB_PTraderDlg::Liquidations::DivideDown (double byamount)
{
    _totalsum /= byamount;
    _PnL /= byamount;
}


IB_PTraderDlg::Liquidations::Liquidations () : _totalsum (0), _PnL (0)
{
}


bool IB_PTraderDlg::LiquidationData::Empty ()
{
    return _profits.Count () == 0 && _losses.Count () == 0;
}


void IB_PTraderDlg::LiquidationData::Reset ()
{
    _profits.Reset ();
    _losses.Reset ();
}


void IB_PTraderDlg::GetSellOrdersToLiquidate (std::list <BuyPricePair *> &liquidations, SellPricePair * &singleorder, std::list <SellPricePair *> * &orderlist)
{
    for each (BuyPricePair *buyprice in liquidations)
    {
        PushOrder <SellPricePair> (buyprice->_sellpricepair, singleorder, orderlist);
    }
}


static bool SortLiquidatePrices (const SellPricePair *x, const SellPricePair *y)
{
    return x->_price < y->_price;
}


void IB_PTraderDlg::GetSellOrdersToLiquidate (Ticker::LiquidateOptions liquidateoption, LiquidationData &data, SellPricePair * &singleorder, std::list <SellPricePair *> * &orderlist)
{
    singleorder = 0;
    orderlist = 0;

    switch (liquidateoption)
    {
        case Ticker::InProfit:
            GetSellOrdersToLiquidate (data._profits._liquidations, singleorder, orderlist);
            break;
        case Ticker::JustLosses:
            GetSellOrdersToLiquidate (data._losses._liquidations, singleorder, orderlist);
            break;
        case Ticker::AllBuys:
            GetSellOrdersToLiquidate (data._profits._liquidations, singleorder, orderlist);
            GetSellOrdersToLiquidate (data._losses._liquidations, singleorder, orderlist);
            break;
    }
    if (orderlist)
    {
        orderlist->sort (SortLiquidatePrices);
    }
}


void IB_PTraderDlg::GetLiquidationsForTicker (Ticker *ticker, bool todayonly, LiquidationData &data)
{
    data.Reset ();
    int size = ticker->_buys.GetDataCount ();
    for (int i = 0;  i < size;  i++)
    {
        BuyPricePair *buyprice = ticker->_buys.GetData (i);
        if (buyprice->AvailableForLiquidation (todayonly))
        {
            if (ticker->_currentbid > 0)
            {
                double acquireamount = (buyprice->_size * buyprice->_lastavgfill) + buyprice->_lastbuycommission;
                double sellamount = (buyprice->_sellpricepair->_size * ticker->_currentbid) - buyprice->_lastbuycommission;
                double PnL = sellamount - acquireamount;               
                if (PnL < 0)
                {
                    data._losses._liquidations.push_back (buyprice);
                    data._losses._totalsum += sellamount;
                    data._losses._PnL += ::abs (PnL);
                }
                else
                {
                    data._profits._liquidations.push_back (buyprice);
                    data._profits._totalsum += sellamount;
                    data._profits._PnL += PnL;
                }
            }
        }
    }
    double dividedownby;
    if (ticker->_firstdbrow->_currency == "GBP")
        dividedownby = 100;
    else
        dividedownby = 1;
    data._profits.DivideDown (dividedownby);
    data._losses.DivideDown (dividedownby);
}


void IB_PTraderDlg::CheckForLiquidation (CTime &now)
{
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        Ticker *ticker = iter->second;
        if (ticker->_autoliquidate_on)
        {
            if (now >= ticker->_autoliquidatetime)
            {
                CString format;
                if (ticker->_liquidationtype == Ticker::NoLiquidation)
                {
                    _stats.IncrementStat ("CheckForLiquidation", "NoLiquidation_Set", ticker->_ticker, ticker->_firstdbrow->_currency);
                }
                else if (ticker->_autoliquidate_DONE)
                {
                    _stats.IncrementStat ("CheckForLiquidation", "AUTOLiquidation_AlreadyDone", ticker->_ticker, ticker->_firstdbrow->_currency);
                }
                else if (_allowedorders == Stopped)
                {
                    format.Format ("CheckForLiquidation for %s - Orders Stopped - price %s%g", ticker->_ticker.c_str (), ticker->GetCurrencySymbol ().c_str (), ticker->_currentbid);
                    Log (format);
                    _stats.IncrementStat ("CheckForLiquidation", "AllowedOrders_Stopped", ticker->_ticker, ticker->_firstdbrow->_currency);
                }
                else if (! ticker->SellsAllowed ())
                {
                    format.Format ("CheckForLiquidation for %s - Sells Orders Not Allowed - price %s%g", ticker->_ticker.c_str (), ticker->GetCurrencySymbol ().c_str (), ticker->_currentbid);
                    Log (format);
                    _stats.IncrementStat ("CheckForLiquidation", "Sells_Orders_NotAllowed", ticker->_ticker, ticker->_firstdbrow->_currency);
                }
                else
                {
                    LiquidationData data;
                    GetLiquidationsForTicker (ticker, false, data);

                    if (data._profits.Count () > 0 || data._losses.Count () > 0)
                    {
                        if (AllowTrade (OutStandingOrder::SELL, ticker, "CheckForLiquidation"))
                        {
                            Contract contract;
                            MakeContract (ticker, contract, ticker->_currentbid);

                            DoLiquidation (ticker->_liquidationtype, ticker, data, false, contract, true, "AUTOLIQUIDATE", "CheckForLiquidation");
                        }
                        else
                        {
                            format.Format ("CheckForLiquidation for %s - AllowTrade failed - price %s%g", ticker->_ticker.c_str (), ticker->GetCurrencySymbol ().c_str (), ticker->_currentbid);
                            Log (format);
                            _stats.IncrementStat ("CheckForLiquidation", "AllowTradeRejected", ticker->_ticker, ticker->_firstdbrow->_currency);
                        }
                    }
                    else
                    {
                        format.Format ("CheckForLiquidation for %s - NothingFoundToLiquidate - price %s%g", ticker->_ticker.c_str (), ticker->GetCurrencySymbol ().c_str (), ticker->_currentbid);
                        Log (format);
                        _stats.IncrementStat ("CheckForLiquidation", "NothingFoundToLiquidate", ticker->_ticker, ticker->_firstdbrow->_currency);
                    }
                }
                ticker->_autoliquidate_DONE = true;
            }
        }
    }
}


void FormatDumpLiquidString (CString &newstr, CString &str, std::list <std::string> &lines, bool log, bool append)
{
    if (log)
    {
        if (append)
        {
            std::list <std::string>::reference lastiter = lines.back ();
            std::string endstr = lastiter;    
            endstr.append (newstr);
            lines.pop_back ();
            lines.push_back (endstr);
        }
        else
        {
            lines.push_back ((LPCTSTR) newstr);
        }
    }
    else
    {
        str += newstr;
    }
}


bool IB_PTraderDlg::SortLiquidations (const BuyPricePair *d1, const BuyPricePair *d2)
{
    return d1->_lastavgfill < d2->_lastavgfill;
}


CString IB_PTraderDlg::DumpLiquidationStats (Ticker *ticker, Liquidations &liquidations, std::string text, bool log)
{
    std::list <std::string> logmessages;
    CString str, format; 
    std::string currencysymbol = ticker->GetCurrencySymbol ();
    if (liquidations.Count () > 0)
    {
        long numberofshares = liquidations.NumberOfShares ();
        format.Format ("***%s from selling %ld shares (%d pairs) for %s%0.2f for a %s of %s%0.2f", 
                       text.c_str (),
                       numberofshares, liquidations.Count (),
                       currencysymbol.c_str (), liquidations._totalsum,
                       text.c_str (),
                       currencysymbol.c_str (), liquidations._PnL);
        CString tabstr = CString (' ', 0);
        FormatDumpLiquidString (format, str, logmessages, log, false);
        const int NUMBER_PER_LINE = 3;
        int count = NUMBER_PER_LINE + 1;
        std::list <BuyPricePair *> sortedliquidations;
        for each (BuyPricePair *buyprice in liquidations._liquidations)
        {
            sortedliquidations.push_back (buyprice);
        }
        sortedliquidations.sort (SortLiquidations);
        for each (BuyPricePair *buyprice in sortedliquidations)
        {
            bool dotab = false;
            if (--count <= 0)
            {
                dotab = true;
                count = NUMBER_PER_LINE;
            }
            format.Format ("%s<FILL=%s%0.2f, %s%0.2f, Com=%s%0.2f (%s%0.2f)>", 
                           dotab ? (LPCTSTR) tabstr : ", ",
                           currencysymbol.c_str (), buyprice->_lastavgfill,
                           currencysymbol.c_str (), buyprice->_price, 
                           currencysymbol.c_str (), buyprice->_lastbuycommission,
                           currencysymbol.c_str (), buyprice->_sellpricepair->_price);
            FormatDumpLiquidString (format, str, logmessages, log, ! dotab);
        }
    }
    else
    {
        format.Format ("No %s found", text.c_str ());
        FormatDumpLiquidString (format, str, logmessages, log, false);
    }
    if (log)
    {
        for each (std::string str in logmessages)
        {
            Log (str.c_str ());
        }
    }
    return str;
}


CString IB_PTraderDlg::GetLiquidatePnLText (Ticker *ticker, CString text, 
                                            long numbershares, 
                                            double profits, double losses,
                                            double profits_totalsum, double losses_totalsum,
                                            double maxcom, double mincom, 
                                            Contract &contract, bool log)
{
    double PnL_mincom = (profits - mincom) - losses;
    double PnL_maxcom = (profits - maxcom) - losses;

    std::string currencysymbol = ticker->GetCurrencySymbol ();
    CString str;
    str.Format ("%s, NumberOfShares=%ld, SalesTotal=%s%0.2f, PnL_MaxCom=%s%0.2f (%s, MaxCom=%0.2f), PnL_MinCom=%s%0.2f (%s, MinCom=%0.2f)",
                (LPCTSTR) text,
                numbershares,
                currencysymbol.c_str (),
                profits_totalsum + losses_totalsum, 

                currencysymbol.c_str (),
                PnL_maxcom,
                PnL_maxcom < 0 ? "LOSS" : "Profit",
                maxcom,

                currencysymbol.c_str (),
                PnL_mincom,
                PnL_mincom < 0 ? "LOSS" : "Profit",
                mincom);
    if (log)
        Log (str);
    return str;
}


CString IB_PTraderDlg::GetLiquidatePnLText (Ticker *ticker, LiquidationData &data, Contract &contract, bool log)
{
    CString str;
    long totalshares_profit = data._profits.NumberOfShares ();
    long totalshares_losses = data._losses.NumberOfShares ();

    bool roundedup, stampdutyadded;
    double mincom, maxcom;
    if (! CalculateCommission (OutStandingOrder::SELL, contract, contract.strike, totalshares_profit + totalshares_losses, mincom, maxcom, roundedup, stampdutyadded))
    {
        mincom = 0;
        maxcom = 0;
    }

    CString part1 = GetLiquidatePnLText (ticker, totalshares_losses == 0 ? "Profits" : "ProfitsAndLosses", 
                                         totalshares_profit + totalshares_losses, 
                                         data._profits._PnL, data._losses._PnL, 
                                         data._profits._totalsum, data._losses._totalsum, 
                                         maxcom, mincom,
                                         contract, log);

    CString part2;
    if (totalshares_losses != 0 && totalshares_profit != 0)
    {
        if (! CalculateCommission (OutStandingOrder::SELL, contract, contract.strike, totalshares_profit, mincom, maxcom, roundedup, stampdutyadded))
        {
            mincom = 0;
            maxcom = 0;
        }

        part2 = GetLiquidatePnLText (ticker, "Profits", 
                                     totalshares_profit, 
                                     data._profits._PnL, 0, 
                                     data._profits._totalsum, 0, 
                                     maxcom, mincom,
                                     contract, log);
    }

    return part1 + (part2.GetLength () > 0 ? "\n" : "") + part2;
}


void IB_PTraderDlg::DoLiquidation (Ticker::LiquidateOptions liquidateoption, 
                                   Ticker *ticker, 
                                   LiquidationData &data, 
                                   bool todayonly,
                                   Contract &contract, 
                                   bool silent,
                                   std::string displaytext,
                                   std::string callingfunction)
{
    CString str;
    SellPricePair *singleorder = 0;
    std::list <SellPricePair *> *orderlist = 0;
    GetSellOrdersToLiquidate (liquidateoption, data, singleorder, orderlist);
    if (singleorder || orderlist)
    {
        _stats.IncrementStat (callingfunction, "DoLiquidation_AttemptingLiquidation_" + displaytext, ticker->_ticker, ticker->_firstdbrow->_currency);
        long totalsize = 0;
        if (orderlist)
        {
            for each (SellPricePair *sellprice in *orderlist)
            {
                totalsize += sellprice->_size;
            }
        }
        else
        {
            totalsize = 1;
        }
        bool proceed = true;
        str.Format ("%ld", totalsize);
        std::string title = displaytext + std::string (" ") + Ticker::ToDisplayString (liquidateoption) + std::string (" for ") + ticker->_ticker + std::string ("? (") + std::string ((LPCTSTR) str) + std::string (" shares)");
        CString line1 = DumpLiquidationStats (ticker, data._profits, "Profit", false);
        CString line2 = DumpLiquidationStats (ticker, data._losses, "LOSS", false);
        CString line3 = GetLiquidatePnLText (ticker, data, contract, false);
        if (silent)
        {
            Log (title.c_str ());
            Log (line1);
            Log (line2);
            Log (line3);
        }
        else
        {
            CString fullmessage = line1 + "\n" + line2 + "\n" + line3;
            if (AfxGetMainWnd ()->MessageBox ((LPCTSTR) fullmessage, title.c_str (), MB_YESNO) == IDYES)
                proceed = true;
            else
                proceed = false;
        }
        if (proceed)
        {
            BuySellResult result = OK;
            if (orderlist)
            {
                result = SendSellOrder (ticker, contract, contract.strike, totalsize, 0, orderlist, (int) orderlist->size ());
                _stats.IncrementStat (callingfunction, "SendSellOrder_Multi_Liquidation", ticker->_ticker, ticker->_firstdbrow->_currency);
            }
            else if (singleorder)
            {
                result = SendSellOrder (ticker, contract, contract.strike, singleorder->_size, singleorder, 0, 1);
                _stats.IncrementStat (callingfunction, "SendSellOrder_Single_Liquidation", ticker->_ticker, ticker->_firstdbrow->_currency);
            }   
        }
        else
        {
            _stats.IncrementStat (callingfunction, "DoLiquidation_NoLiquidation_Proceed", ticker->_ticker, ticker->_firstdbrow->_currency);
        }
    }    
    else
    {
        str.Format ("%s - Nothing to liquidate for %s - price %s%g", callingfunction.c_str (), ticker->_ticker.c_str (), ticker->GetCurrencySymbol ().c_str (), ticker->_currentbid);
        Log (str);
        _stats.IncrementStat (callingfunction, "DoLiquidation_NoLiquidation_NothingFound", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    delete orderlist;
}


void IB_PTraderDlg::GoLiquidate (bool todayonly)
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        bool displaymode = true;
        Ticker::LiquidateOptions liquidateoption = Ticker::NoLiquidation;
        CString editboxtext;
        GetDlgItem (IDC_ORDERID)->GetWindowText (editboxtext);
        if (editboxtext.GetLength () == 1)
        {
            switch (editboxtext [0])
            {
                case 'P':
                case 'p':
                    liquidateoption = Ticker::InProfit;
                    break;
            }
        }
        else if (editboxtext.GetLength () == 2)
        {
            if (editboxtext == "LL" || editboxtext == "ll")
            {
                liquidateoption = Ticker::JustLosses;
            }
            else if (editboxtext == "AA" || editboxtext == "aa")
            {
                liquidateoption = Ticker::AllBuys;
            }
        }
        displaymode = liquidateoption == Ticker::NoLiquidation;
        GetDlgItem (IDC_ORDERID)->SetWindowText ("");

        Contract contract;
        MakeContract (ticker, contract, ticker->_currentbid);

        LiquidationData data;
        GetLiquidationsForTicker (ticker, todayonly, data);

        CString str;
        if (displaymode)
        {
            Log ("----------------------------------------");
            std::string currencysymbol = ticker->GetCurrencySymbol ();
            str.Format (" for %s (Price %s%0.2f) - range %s%0.2f to %s%0.2f", ticker->_ticker.c_str (), 
                        currencysymbol.c_str (), ticker->_currentbid,
                        currencysymbol.c_str (), ticker->_ib_low,
                        currencysymbol.c_str (), ticker->_ib_high
                       );
            str = (todayonly ? "TODAY liquidation" : "ALL liquidation") + str;
            Log (str);

            if (data.Empty ())
            {
                Log ("Nothing to liquidate");
            }
            else
            {
                DumpLiquidationStats (ticker, data._profits, "Profit", true);
                DumpLiquidationStats (ticker, data._losses, "LOSS", true);
                GetLiquidatePnLText (ticker, data, contract, true);
            }
        }
        else
        {
            DoLiquidation (liquidateoption, ticker, data, todayonly, contract, false, todayonly ? "TODAY Liquidate" : "ALL Liquidate", "OnBnClickedLiquidate");
        }
    }
    else
    {
        Log ("No Ticker Found");
    }
    OnBnClickedScrollmessages ();
}


void IB_PTraderDlg::OnBnClickedLiquidate ()
{
    GoLiquidate (true);
}


void IB_PTraderDlg::OnBnClickedAllliquidate ()
{
    GoLiquidate (false);
}
