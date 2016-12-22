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



void IB_PTraderDlg::BatchBuyClick ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        CButton *batchbuycheck = (CButton *) GetDlgItem (IDC_BATCHBUY);
        ticker->_makebatchbuy = batchbuycheck->GetCheck () > 0 ? true : false;
    }
}


void IB_PTraderDlg::OnBnClickedBatchbuycheck ()
{
    bool displayedmess = false;
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        if (ticker->_share)
        {
            CString message = "", str;
            if (ticker->_share->_share_has_buy_daylimit)
            {
                double offer = ticker->_currentoffer;
                if (offer > 0)
                {
                    std::vector <BuyPricePair *> sharesthatwouldbebought;
                    long numberofshares = 0;
                    double lastbuycost = 0;
                    double runningbuytotal = 0;

                    double maxallowed_purchasestoday = ticker->_share->_max_allowed_purchases_today;
                    bool editfieldvalid;
                    long newlimit = GetEditFieldAsNumber (IDC_ORDERID, editfieldvalid);
                    if (editfieldvalid)
                    {
                        maxallowed_purchasestoday = (double) newlimit;
                    }

                    double buylimit = maxallowed_purchasestoday - ticker->_share->_totalpurchases_made_today;
   
                    int searchstart, searchend;
                    ticker->_buys.GetSearchStart (offer, searchstart, searchend, true);
                    for (int i = searchstart;  i <= searchend;  i++)
                    {
                        BuyPricePair *buyprice = ticker->_buys.GetData (i);
                        if (buyprice->_turnedon && buyprice->_price >= offer && buyprice->GetStatus () == PricePair::ACTIVE)
                        {
                            double buycost = buyprice->GetCostFromPrice (offer);
                            lastbuycost = buycost;
                            if ((runningbuytotal + buycost) > buylimit)
                            {
                                break;
                            }
                            sharesthatwouldbebought.push_back (buyprice);
                            numberofshares += buyprice->_size;
                            runningbuytotal += buycost;
                        }
                    }
                    str.Format ("BUY %ld shares, max $%g from editfield=%s, (%d pgens) at cost of %g (overflow buy was %g)", 
                                numberofshares, maxallowed_purchasestoday, 
                                editfieldvalid ? "T" : "F",
                                sharesthatwouldbebought.size (), 
                                runningbuytotal, lastbuycost);
                    message += str;
                    if (sharesthatwouldbebought.size () > 0)
                    {
                        str.Format (", 1st buy price %g, last buy price %g", 
                                    sharesthatwouldbebought [0]->_price,
                                    sharesthatwouldbebought [sharesthatwouldbebought.size () - 1]->_price);
                        message += str;
                        PortfolioPart *part = GetPortfolioPart (ticker);
                        if (part)
                        {
                            double currentsum = (double) part->_position * part->_averageCost;
                            double newsum = (double) numberofshares * offer;
                            long newsharecount = (long) part->_position + numberofshares;
                            double newtotal = currentsum + newsum;
                            double newavg = newtotal / (double) newsharecount;
                            str.Format (", --> Old Avg %g, New Avg %g", part->_averageCost, newavg);
                            message += str;
                        }
                        message += "\n";
                        message += "PGens";
                        for each (BuyPricePair *buyprice in sharesthatwouldbebought)
                        {
                            str.Format (", %g", buyprice->_price);
                            message += str;
                        }
                    }
                }
            }
            if (ticker->_share->_share_has_sell_daylimit)
            {
                double bid = ticker->_currentbid;
                if (bid > 0)
                {
                    std::vector <SellPricePair *> sharesthatwouldbesold;
                    long numbersharessold = 0;
                    double runningselltotal = 0;
                    Share *share = ticker->_share;
                    double amountsellleft = share->_max_allowed_sales_today - share->_totalsales_made_today;
                    int startsearch, endsearch;
                    GetSellSearchParts (ticker, startsearch, endsearch);
                    for (int i = startsearch;  i <= endsearch;  i++)
                    {
                        SellPricePair *sellprice = ticker->_sells.GetData (i);
                        if (sellprice->_buypricepair->GetStatus () == PricePair::BOUGHT && 
                            sellprice->GetStatus () == PricePair::ACTIVE &&
                            sellprice->_price <= bid)
                        {
                            double saleprice = sellprice->GetCostFromPrice (bid);
                            if ((runningselltotal + saleprice) > amountsellleft)
                            {
                                break;
                            }
                            sharesthatwouldbesold.push_back (sellprice);
                            numbersharessold += sellprice->_size;
                            runningselltotal += saleprice;
                        }
                    }
                    str.Format ("SELL %ld shares (%d pgens) for %g", numbersharessold, sharesthatwouldbesold.size (), runningselltotal);
                    if (message.GetLength () > 0)
                        message += ", ";
                    message += str;
                    if (sharesthatwouldbesold.size () > 0)
                    {
                        str.Format (", 1st sell price %g, last sell price %g", 
                                    sharesthatwouldbesold [0]->_price,
                                    sharesthatwouldbesold [sharesthatwouldbesold.size () - 1]->_price);
                        message += str;
                    }
                }
            }
            if (message.GetLength () > 0)
            {
                displayedmess = true;
                AfxMessageBox ((LPCTSTR) message);
            }
        }
        if (! displayedmess)
            AfxMessageBox ("BatchBuyCheck Error occurred");
        GetDlgItem (IDC_ORDERID)->SetWindowText ("");
    }
    OnBnClickedUnselectQuotes ();
}
