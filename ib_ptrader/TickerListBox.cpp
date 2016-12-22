/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "TickerListBox.h"
#include "TechnicalAnalysis.h"
#include "ib_ptraderdlg.h"
#include <math.h>



IMPLEMENT_DYNAMIC(TickerListBox, CListBox)

TickerListBox::TickerListBox ()
{
}


TickerListBox::~TickerListBox ()
{
}


void TickerListBox::AddEntry (Ticker *ticker)
{
    int index = AddString (ticker->_ticker.c_str ());
    ticker->_listboxindex = index;
    SetItemData (index, (DWORD_PTR) ticker);
}


BEGIN_MESSAGE_MAP(TickerListBox, CListBox)
END_MESSAGE_MAP()


void TickerListBox::MeasureItem (LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    lpMeasureItemStruct->itemHeight = 16;
}


BuyPricePair *TickerListBox::GetAppendNearestPriceMatch_BuyPrice (Ticker *ticker, int offset)
{
    if (ticker->_searchstartmatchindex == -1)
        return 0; 
    int index = ticker->_searchstartmatchindex + offset;
    if (index < 0)
        return 0;
    if (index >= ticker->_buys.GetDataCount ())
        return 0;
    BuyPricePair *buyprice = ticker->_buys.GetData (index);
    return buyprice;
}


BuyPricePair *TickerListBox::AppendNearestPriceMatch (Ticker *ticker, CString &str, int offset, bool offsetismiddle, bool fp, bool append_selloffset, bool append_size)
{
    BuyPricePair *buyprice = GetAppendNearestPriceMatch_BuyPrice (ticker, offset);
    if (buyprice && buyprice->_turnedon)
    {
        if (fp)
            str += ", ";
        else
            str += ", ";
        CString format;
        format.Format ("%0.2f (", buyprice->_price);
        if (offsetismiddle)
            str += "**";
        else
            str += " ";
        str += format;
        PricePair::Status status = buyprice->GetStatus ();
        if (status == PricePair::BOUGHT && buyprice->_boughttoday)
            str += "t";
        str += PricePair::ToShortString (status).c_str ();
        if (append_size || offsetismiddle)
        {
            format.Format ("_%ld", buyprice->_size);
            str += format;
        }
        if (append_selloffset || offsetismiddle)
        {
            double selldiff = buyprice->_sellpricepair->_price - buyprice->_price;
            format.Format ("_%0.2f", selldiff);
            str += format;
        }
        str += ")";
        return offsetismiddle ? buyprice : 0;
    }
    return 0;
}


void TickerListBox::GetFormattedNumberString (double value, bool round, CString &str)
{
    const double MILLION_DBL = 1000000;

    double comparevalue = ::abs (value);
    if (comparevalue >= MILLION_DBL)
    {
        if (round)
            str.Format ("%dM", (int) (value / MILLION_DBL));
        else
            str.Format ("%0.1fM", value / MILLION_DBL);
    }
    else if (comparevalue < 1000)
    {
        str.Format ("%d", (int) value);
    }
    else
    {
        if (round)
            str.Format ("%dK", (int) (value / 1000));
        else
            str.Format ("%0.1fK", value / 1000);
    }
}


#define BUYSSELLS_EARLY 1

bool TickerListBox::DrawLine (Ticker *ticker, CString &str, CDC &dc, CRect &textrect)
{
    ticker->_currentlistboxline = str;
    dc.DrawText ((LPCTSTR) str, str.GetLength (), textrect, DT_SINGLELINE | DT_VCENTER);
    return true;
}


bool TickerListBox::DrawListBoxLine (CDC &dc, CRect textrect, Ticker *ticker)
{
    CString str, format;
    IB_PTraderDlg::PortfolioPart *part = 0;
    if (ticker)
    {
        if (ticker->_nullticker)    
        {
            str = "--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"
                  "--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"
                  ;
            return DrawLine (ticker, str, dc, textrect);
        }
        bool tickerdisabled = ticker->_activestate == Ticker::DISABLED;
        CString autoliquidatestr;
        if (ticker->_autoliquidate_DONE)
        {
            autoliquidatestr = "aDONE";
        }
        else if (ticker->_autoliquidate_on)
        {
            autoliquidatestr.Format ("a%s_%0.2d:%0.2d", 
                                     Ticker::ToTickerDisplayString (ticker->_liquidationtype).c_str (),
                                     ticker->_autoliquidatetime.GetHour (),
                                     ticker->_autoliquidatetime.GetMinute ());
        }
        str.Format ("%s<%s%s%s%s=%0.2f (%i), %0.2f (%i) %s", 
                    (LPCTSTR) autoliquidatestr,
                    (ticker->_buyaverages == Ticker::OnlyIfBelow && ! tickerdisabled) ? "*" : "", 
                    (ticker->_allow_early_buys && ! tickerdisabled) ? "#" : "",
                    ticker->_ignoretechnicalindicators ? "i_" : "",
                    ticker->_ticker.c_str (), 
                    ticker->_currentbid,    
                    ticker->_bidsize, 
                    ticker->_currentoffer, 
                    ticker->_offersize,
                    ticker->_pricerecordtype == Ticker::NoRecording ? "" : (LPCTSTR) Ticker::ToString (ticker->_pricerecordtype, ticker->_technicalindicators != 0)
                   );
        if (ticker->_volume > 0)
        {
            GetFormattedNumberString (ticker->_volume, true, format);
            if (ticker->_volume > ticker->_avgvolume && ticker->_avgvolume > 0)
                str+= " *";
            else
                str += " ";
            str += format;
        }

        part = _parent->GetPortfolioPart (ticker);
        if (part)
        {
            CString avgdiff;
            if (part->_averageCost > part->_initial_averageCost)
            {
                avgdiff = "U";
            }
            else if (part->_averageCost < part->_initial_averageCost)
            {
                avgdiff = "D";
            }
            else
            {
                avgdiff = "S";
            }
            format.Format (", Av%s=%0.2f", (LPCTSTR) avgdiff, part->_averageCost);
            str += format;
        }

        if (ticker->_activestate != Ticker::Active)
        {
            str += ", " + Ticker::ToString (ticker->_activestate);
        }
        if (ticker->_tickerhasnoprices)
        {
            str += "> - NO ACTIVE PRICES";
            if (ticker->_pricerecordtype != Ticker::NoRecording)
            {
                str += " - Recording " + Ticker::ToString (ticker->_pricerecordtype, false);
            }
            return DrawLine (ticker, str, dc, textrect);
        }
        if (tickerdisabled || ticker->_marketstatus == MarketDetails::OnHoliday)
        {
            CString messageextra;
            if (tickerdisabled)
            {
                messageextra = "Disabled_";
            }
            else
            {
                str += ", ";
                str += MarketDetails::ToDisplayString (ticker->_marketstatus).c_str ();
            }
            str += ">";
            if (ticker->_share)
            {
                if (ticker->_share->_currentoi != 0)
                {
                    if (part == 0)
                        part = _parent->GetPortfolioPart (ticker);
                    if (part)
                    {
                        format.Format (", %sOI=%ld (%ld)", (LPCTSTR) messageextra, ticker->_share->_currentoi, part->_position - ticker->_share->_currentoi);
                    }
                    else
                    {
                        format.Format (", %sOI=%ld", (LPCTSTR) messageextra, ticker->_share->_currentoi);
                    }
                    str += format;
                }
            }
            return DrawLine (ticker, str, dc, textrect);
        }
        if (_parent->_allowedorders == IB_PTraderDlg::Stopped)
        {
            str += ", Stopped";
        }
        else if (ticker->_marketstatus != MarketDetails::Open)
        {
            str += ", ";
            str += MarketDetails::ToDisplayString (ticker->_marketstatus).c_str ();
        }
#if BUYSSELLS_EARLY
        if (ticker->_numberofbuys > 0)
        {
            format.Format (", %d Buy", ticker->_numberofbuys);
            str += format;
        }
        if (ticker->_numberofsells > 0)
        {
            format.Format (", %d Sell", ticker->_numberofsells);
            str += format;
        }
#endif
        if (ticker->_technicalindicators)
        {
            bool defaultvalue;
            for each (TechnicalIndicator *ti in ticker->_technicalindicators->TechnicalIndicators ())
            {
                if (ti->DisplayIndicator ())
                {
                    str += ", " + TechnicalIndicator::ToDisplayString (ti->TechnicalIndicators ());
                    format.Format ("=%0.2f", ti->LatestValue (defaultvalue));
                    str += format;
                }
            }
        }
        str += ">";

        bool append_selloffset = false, append_size = false;
        if (ticker->_marketstatus != MarketDetails::AfterHours)
        {
            #pragma region "Dump Nearest PGen Prices"
            const int START_INDEX = -1;
            const int END_INDEX = 1;

            double currentvalue = 0;
            long currentsize = 0;
            for (int i = START_INDEX;  i <= END_INDEX;  i++)
            {
                BuyPricePair *buyprice = GetAppendNearestPriceMatch_BuyPrice (ticker, i);
                if (buyprice)
                {       
                    double selldiff = buyprice->_sellpricepair->_price - buyprice->_price;
                    selldiff = ::floor (selldiff * 100 + 0.5) / 100;
                    if (i == START_INDEX)
                    {
                        currentvalue = selldiff;
                        currentsize = buyprice->_size;
                    }
                    else
                    {
                        if (currentvalue != selldiff)
                        {
                            append_selloffset = true;
                        }
                        if (currentsize != buyprice->_size)
                        {
                            append_size = true;
                        }
                        if (append_selloffset && append_size)
                        {
                            break;
                        }
                    }                
                }   
                else
                {
                    append_selloffset = true;
                    append_size = true;
                    break;
                }         
            }       
            bool fp = true;
            BuyPricePair *displayedmiddleprice = 0;
            for (int i = START_INDEX;  i <= END_INDEX;  i++)
            {
                BuyPricePair *appendedbuypricepair = AppendNearestPriceMatch (ticker, str, i, i == 0, fp, append_selloffset, append_size);
                if (appendedbuypricepair)
                {
                    displayedmiddleprice = appendedbuypricepair;
                }
                fp = false;
            }
            if (displayedmiddleprice == 0)
            {
                str += ", NODISPLAY";
            }
            #pragma endregion

            #pragma region "Diagnostics"
#if ! PUBLIC_BUILD
            if (_parent->_displayquotesdiagnostics)
            {
                str += " - ";
                str += BSearch <BuyPricePair>::ToStringDisplay (ticker->_searchstartresult).c_str ();
            }
#endif
            if (ticker->_sellsstartpoint)
            {
#if ! PUBLIC_BUILD
                SellPricePair *lowersellprice = ticker->_sellstartpoint_startindex != -1 ? ticker->_sells.GetData (ticker->_sellstartpoint_startindex) : 0;
                SellPricePair *uppersellprice = ticker->_sellstartpoint_endindex != -1 ? ticker->_sells.GetData (ticker->_sellstartpoint_endindex) : 0;
#endif
                CString format1;
                if (ticker->_sellsstartpoint->_sellpricepair->_trending_hitcount > 0)
                {
                    format1.Format (" --> (LOW %g (%d, %d))", 
                                    ticker->_sellsstartpoint->_sellpricepair->_price, 
                                    ticker->_sellsstartpoint->_sellpricepair->_trending_hitcount,
                                    ticker->_maxtrendcount);
                }
                else
                {
                    format1.Format (" --> (LOW %g)", ticker->_sellsstartpoint->_sellpricepair->_price);
                }
#if ! PUBLIC_BUILD
                if (_parent->_displayquotesdiagnostics)
                {
                    if (lowersellprice && uppersellprice)
                    {
                        format.Format (" %g (%d) to %g (%d)", 
                                       lowersellprice->_price, ticker->_sellstartpoint_startindex,
                                       uppersellprice->_price, ticker->_sellstartpoint_endindex);
                    }
                    else
                    {
                        format.Format (" idx %d to %d", 
                                       ticker->_sellstartpoint_startindex, ticker->_sellstartpoint_endindex);
                    }
                }
                else
                {
#endif
                    format = "";
#if ! PUBLIC_BUILD
                }
#endif
                str += format1;
                str += format;
            }
#if ! PUBLIC_BUILD
            else if (_parent->_displayquotesdiagnostics)
            {
                str += " - FullSell";
            }
#endif
            #pragma endregion
        }

#if ! BUYSSELLS_EARLY
        format.Format (", %d BUY, %d Sell", ticker->_numberofbuys, ticker->_numberofsells);
        str += format;
#endif
        int pendingbuys, pendingsells;
        OutStandingOrder::GetPendingBuysSells (ticker, pendingbuys, pendingsells);
        if (pendingbuys != 0)
        {
            format.Format (", %d p_B", pendingbuys);
            str += format;
        }
        if (pendingsells != 0)
        {
            format.Format (", %d p_S", pendingsells);
            str += format;
        }
        if (ticker->_number_of_cancelledbuys != 0)
        {
            format.Format (", %d c_B", ticker->_number_of_cancelledbuys);
            str += format;
        }
        if (ticker->_number_of_cancelledsells != 0)
        {
            format.Format (", %d c_S", ticker->_number_of_cancelledsells);
            str += format;
        }
        if (ticker->_share)
        {
            if (part == 0)
                part = _parent->GetPortfolioPart (ticker);
            long otheroi = part ? part->_position - ticker->_share->_currentoi : 0;
            if (otheroi > 0)
            {
                format.Format (", OI=%ld (%ld)", ticker->_share->_currentoi, otheroi);
            }
            else
            {
                format.Format (", OI=%ld", ticker->_share->_currentoi);
            }
            str += format;
        }
        bool showingprofits = ticker->_numberofsells > 0;
        CString minmaxstr;
        bool showminmaxranges = (! showingprofits) && (! append_selloffset) && (! append_size);
        if (showminmaxranges)
        {
            if (ticker->_minmaxes_set_bid || ticker->_minmaxes_set_offer)
            {
                minmaxstr = " - <";
                if (ticker->_minmaxes_set_bid)
                {
                    format.Format ("%0.2f to %0.2f", ticker->_minbid, ticker->_maxbid);
                    minmaxstr += format;
                }
                else
                {
                    minmaxstr += "?";
                }
                minmaxstr += ", ";
                if (ticker->_minmaxes_set_offer)
                {
                    format.Format ("%0.2f to %0.2f", ticker->_minoffer, ticker->_maxoffer);
                    minmaxstr += format;
                }
                else
                {
                    minmaxstr += "?";
                }
                minmaxstr += ">";
            }
        }
        double divider;
        if (ticker->_firstdbrow->_currency == "GBP")
            divider = 100;
        else
            divider = 1;
        std::string currencysymbol = ticker->GetCurrencySymbol ();
        if (ticker->_share)
        {
            if (! ticker->_share->_share_has_sell_daylimit)
            {
                str += minmaxstr;
            }
            if (ticker->_share->_share_has_buy_daylimit)
            {
                format.Format (" - B=%s%0.2f (%0.2f)", currencysymbol.c_str (),
                               (ticker->_share->_totalpurchases_made_today / divider), (ticker->_share->_max_allowed_purchases_today / divider));
                str += format;
            }
            else if (ticker->_share->_totalpurchases_made_today > 0)
            {
                format.Format (" - B=%s%0.2f", currencysymbol.c_str (),
                               (ticker->_share->_totalpurchases_made_today / divider));
                str += format;
            }
            if (ticker->_share->_share_has_sell_daylimit)
            {
                format.Format (" - S=%s%0.2f (%0.2f)", currencysymbol.c_str (),
                               (ticker->_share->_totalsales_made_today / divider), (ticker->_share->_max_allowed_sales_today / divider));
                str += format;
            }
        }
        else
        {
            str += minmaxstr;
        }
        if (showingprofits)
        {
            format.Format (", P=%s%0.2f", currencysymbol.c_str (), (ticker->_netprofits / divider));   
            str += format;
        }
        if (ticker->_maxglobalcurrencycashexceeded)
        {
            str += ", MAXGLOBALCASH_";
            str += ticker->_firstdbrow->_currency.c_str ();
        }
        else
        {
            if (ticker->_maxcashexceeded)
            {
                str += ", MaxCash";
            }
            if (ticker->_maxoiexceeded)
            {
                str += ", MaxOI";
            }
            if (ticker->_maxdailybuycashexceeded)
            {
                str += ", MaxBuyDay";
            }
            if (ticker->_maxdailysellcashexceeded)
            {
                str += ", MaxSellDay";
            }
        }
    }
    else
    {
        str = "NULL ticker";
    }
    return DrawLine (ticker, str, dc, textrect);
}


void TickerListBox::DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (lpDrawItemStruct->itemID == (UINT) -1) 
        return;

    Ticker *ticker = (Ticker *) GetItemData ((int) lpDrawItemStruct->itemID);

    CDC dc;
    dc.Attach (lpDrawItemStruct->hDC);

    // Save these value to restore them when done drawing.
    COLORREF crOldTextColor = dc.GetTextColor ();
    COLORREF crOldBkColor = dc.GetBkColor ();

    COLORREF fgColor = crOldTextColor;
    COLORREF bgColor = crOldBkColor;

    // If this item is selected, set the background color 
    // and the text color to appropriate values. Also, erase
    // rect by filling it with the background color.
    if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
    {
        dc.SetTextColor (bgColor);
        dc.SetBkColor (fgColor);
        dc.FillSolidRect (&lpDrawItemStruct->rcItem, fgColor);
    }
    else
    {
        dc.SetTextColor (fgColor);
        dc.SetBkColor (bgColor);
        dc.FillSolidRect (&lpDrawItemStruct->rcItem, bgColor);
    }
    if (lpDrawItemStruct->itemAction & ODA_FOCUS)
    {
        dc.DrawFocusRect (&lpDrawItemStruct->rcItem);
    }

    lpDrawItemStruct->rcItem.left += 5;

    // Draw the text
    DrawListBoxLine (dc,  
                     lpDrawItemStruct->rcItem,
                     ticker);

    dc.SetTextColor (crOldTextColor);
    dc.SetBkColor (crOldBkColor);

    dc.Detach (); 
}

