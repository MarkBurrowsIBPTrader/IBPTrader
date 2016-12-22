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



void IB_PTraderDlg::OnLbnDblclkQuoteslist ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        double lowestsellprice = DBL_MAX, pending_lowestsellprice = DBL_MAX;
        BuyPricePair *lowestbuyprice = 0, *pending_lowestbuyprice = 0;
        CString message = "", str;
        message.Format ("TickerId=%ld", ticker->_tickerid);
        long totalnumbershares = 0;
        double totalsales = 0, profit = 0, spotprofit = 0, spotsellsum = 0;
        int numberinprofit = 0;
        long numberofsharesinprofit = 0;
        int size = ticker->_buys.GetDataCount ();
        if (ticker->_searchstartmatchindex != -1)
        {
            int hitcount = 0;
            BuyPricePair *activebuy = 0;
            int index = ticker->_searchstartmatchindex;
            while (--index >= 0)
            {
                BuyPricePair *buyprice = ticker->_buys.GetData (index);                
                if (buyprice->GetStatus () == PricePair::ACTIVE)
                {
                    activebuy = buyprice;
                    hitcount++;
                    break;
                }
            }
            if (activebuy)
            {
                str.Format (", Lower Buy %g", activebuy->_price);
                message += str;
            }
            activebuy = 0;
            index = ticker->_searchstartmatchindex;
            while (++index < size)
            {
                BuyPricePair *buyprice = ticker->_buys.GetData (index);                
                if (buyprice->GetStatus () == PricePair::ACTIVE)
                {
                    activebuy = buyprice;
                    hitcount++;
                    break;
                }
            }
            if (activebuy)
            {
                str.Format (", Higher Buy %g", activebuy->_price);
                message += str;
            }
        }
        for (int i = 0;  i < size;  i++)
        {
            BuyPricePair *buyprice = ticker->_buys.GetData (i);
            if (buyprice->GetStatus () == PricePair::BOUGHT)
            {
                if (message.GetLength () > 0)
                    message += ", ";
                totalnumbershares += buyprice->_size;
                double sellprice = buyprice->_sellpricepair->_price;
                totalsales += (sellprice * buyprice->_sellpricepair->_size);
                profit += ((sellprice - buyprice->_price) * buyprice->_sellpricepair->_size);
                if (ticker->_currentbid >= sellprice)
                {
                    spotprofit += ((ticker->_currentbid - buyprice->_price) * buyprice->_sellpricepair->_size);
                    spotsellsum += (ticker->_currentbid * buyprice->_sellpricepair->_size);
                    numberinprofit++;
                    numberofsharesinprofit += buyprice->_sellpricepair->_size;
                }
                if (buyprice->_sellpricepair->GetStatus () == PricePair::ACTIVE)
                {
                    if (sellprice < lowestsellprice)
                    {
                        lowestsellprice = sellprice;
                        lowestbuyprice = buyprice;
                    }
                }
                else if (buyprice->_sellpricepair->GetStatus () == PricePair::PENDING)
                {
                    if (sellprice < pending_lowestsellprice)
                    {
                        pending_lowestsellprice = sellprice;
                        pending_lowestbuyprice = buyprice;
                    }   
                }
                str.Format ("%lu (%s%g <%ld> to %g %s", 
                            buyprice->_dbrow->_iddeals, 
                            buyprice->_boughttoday ? "t" : "",
                            buyprice->_price,
                            buyprice->_size, 
                            sellprice,
                            PricePair::ToShortString (buyprice->_sellpricepair->GetStatus ()).c_str ());
                CString extraformat;
                if (buyprice->AvailableForLiquidation (false))
                {
                    extraformat.Format (", Fill=%g", buyprice->_lastavgfill);
                    str += extraformat;
                }
#if ! PUBLIC_BUILD
                extraformat.Format (", PIs=%d,%d", buyprice->_pairindex, buyprice->_sellpricepair->_pairindex);
                str += extraformat;
#endif
                str += ")";
                message += str;
            }
        }
        if (lowestbuyprice)
        {
            str.Format (", Lowest Sell Price %g %s", lowestsellprice, PricePair::ToShortString (lowestbuyprice->_sellpricepair->GetStatus ()).c_str ());
        }
        else
        {
            str = ", ???";
        }
        message += str;
        if (pending_lowestbuyprice)
        {
            str.Format (", Lowest PENDING Sell Price %g %s", pending_lowestsellprice, PricePair::ToShortString (pending_lowestbuyprice->_sellpricepair->GetStatus ()).c_str ());
        }
        else
        {
            str = ", No Pending";
        }
        double percentprofits = totalsales > 0 ? (profit / totalsales) * 100 : 0;
        message += str;
        str.Format (", NumberShares=%ld, SalesValue=%g, Profits=%g (%0.2f" "%%" "), *********SPOT PROFITS=%g (Total=%g, %d, %ld shares), IBOPEN=%g (%s), IBLOW=%g, IBHIGH=%g", 
                    totalnumbershares, totalsales, profit, percentprofits, spotprofit, spotsellsum, numberinprofit,
                    numberofsharesinprofit, ticker->_ib_open, ticker->_currentoffer >= ticker->_ib_open ? "Up" : "Down",
                    ticker->_ib_low, ticker->_ib_high);
        message += str;
        PortfolioPart *part = GetPortfolioPart (ticker);
        if (part && ticker->_share)
        {
            str.Format (", PortfolioValue=%0.2f (%0.2f Max), CurrentOI=%ld, (MaxOI=%ld)", 
                        part->GetMarketValue (), ticker->_share->_maxvalue_allowed, 
                        ticker->_share->_currentoi, ticker->_share->_maxoi_allowed);
        }
        else
        {
            str = ", PortfolioValue=?";
        }
        message += str;
        message += ", Vol=";
        TickerListBox::GetFormattedNumberString (ticker->_volume, false, str);
        message += str;
        message += ", AvgVol=";
        TickerListBox::GetFormattedNumberString (ticker->_avgvolume, false, str);
        message += str;
        message += ", TickBid=";
        TickerListBox::GetFormattedNumberString (ticker->_tickvolume_bid, false, str);
        message += str;
        message += ", TickOffer=";
        TickerListBox::GetFormattedNumberString (ticker->_tickvolume_offer, false, str);
        message += str;
        message += ", CombTickBid=";
        TickerListBox::GetFormattedNumberString (ticker->_tickvolume_bid + ticker->_tickdepth_bid, false, str);
        message += str;
        message += ", CombTickOffer=";
        TickerListBox::GetFormattedNumberString (ticker->_tickvolume_offer + ticker->_tickdepth_offer, false, str);
        message += str;
        message += ", MaxSpread=";
        double currentspread = ticker->_currentoffer - ticker->_currentbid;
        double allowedspreadforticker = GetSpreadForTicker (ticker);
        str.Format ("%g (Current=%g)", allowedspreadforticker, currentspread);
        message += str;

        if (part)
        {
            str.Format (", InitAvgCost=%g, (AvgCost=%g, Change %g)", part->_initial_averageCost, part->_averageCost, (part->_averageCost - part->_initial_averageCost));
            message += str;
        }

        str.Format (", TotalBuys=%0.2f (%d Buys), TotalSales=%0.2f (%d Sells)", ticker->_totalbuycost, ticker->_numberofbuys, ticker->_salestotal, ticker->_numberofsells);
        message += str;

#if ! PUBLIC_BUILD
        if (ticker->_technicalindicators)
        {
            DailyStochs *ti = (DailyStochs *) ticker->_technicalindicators->GetIndicator (TechnicalIndicator::DailyStochs);
            if (ti)
            {
                if (ti->FileReadError ())
                {
                    message += ", Stochs - read error";
                }
                else
                {
                    str.Format (", Stochs are K=%g, D=%g, wK=%g, wD=%g", ti->K (), ti-> D (), ti->Weekly_K (), ti->Weekly_D ());
                    message += str;
                }
            }
        }
#endif

        CString fullmessage = message + ", CANCEL to save";
        CString title = part ? part->_longname : ticker->_firstdbrow->_displayname.c_str ();
        if (AfxGetMainWnd ()->MessageBox ((LPCTSTR) fullmessage, (LPCTSTR) title, MB_OKCANCEL) == IDCANCEL)
        {
            CString filename = "tickers_" + CString (ticker->_ticker.c_str ());
            filename = GetFileName (filename);
            CFile file (filename, CFile::modeCreate | CFile::modeReadWrite);
            LPCTSTR buf = (LPCTSTR) message;
            UINT len = lstrlen (buf);
            file.Write (buf, len);
            file.Write ("\r\n", 2);
            file.Close ();
            OnBnClickedScrollmessages ();
        }
    }
    OnBnClickedUnselectQuotes ();
}
