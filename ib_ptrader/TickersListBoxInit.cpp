/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"



int IB_PTraderDlg::AssignCurrencyWeighting (std::string currency)
{
    if (currency == "CAD")
        return 0;
    else if (currency == "GBP")
        return 1;
    return 2;
}


bool IB_PTraderDlg::CompareAlphaTickers (const Ticker *a, const Ticker *b)
{
    int left_currweight = AssignCurrencyWeighting (a->_firstdbrow->_currency);
    int right_currweight = AssignCurrencyWeighting (b->_firstdbrow->_currency);
    if (left_currweight == right_currweight)
        return a->_ticker < b->_ticker;
    return left_currweight < right_currweight;
}


int IB_PTraderDlg::AssignTickerStateWeighting (Ticker::ActiveState state)
{
    switch (state)
    {
        case Ticker::Active:
            return 0;
        case Ticker::BuyOnly:
            return 1;
        case Ticker::SellOnly:
            return 2;
        case Ticker::InActive:
            return 3;
        case Ticker::DISABLED:
        default:
            return 4;
    }
}


bool IB_PTraderDlg::CompareGroupByTickers (const Ticker *a, const Ticker *b)
{
    int left_currweight = AssignCurrencyWeighting (a->_firstdbrow->_currency);
    int right_currweight = AssignCurrencyWeighting (b->_firstdbrow->_currency);
    if (left_currweight != right_currweight)
    {
        if (left_currweight < right_currweight)
            return true;
        return false;
    }
    int left_stateweight = AssignTickerStateWeighting (a->_activestate);
    int right_stateweight = AssignTickerStateWeighting (b->_activestate);
    if (left_stateweight != right_stateweight)
    {
        if (left_stateweight < right_stateweight)
            return true;
        return false;
    }
    return CompareAlphaTickers (a, b);
}


void IB_PTraderDlg::UpdateTickerListBox (TickerListBoxSort sortorder)
{
    std::list <Ticker *> unsortedtickers;
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        unsortedtickers.push_back (iter->second);
    }
    switch (sortorder)
    {
        case Alpha:
            unsortedtickers.sort (CompareAlphaTickers);
            break;
        case ByGroup:
            unsortedtickers.sort (CompareGroupByTickers);
            break;
    }
    std::string currentcurrency;
    for each (Ticker *ticker in unsortedtickers)
    {
        if (currentcurrency == "")
            ;
        else if (currentcurrency != ticker->_firstdbrow->_currency)
        {
            Ticker *tempticker = new Ticker (-1, "", 0);
            tempticker->_nullticker = true;
            _mainlistbox.AddEntry (tempticker);
        }
        _mainlistbox.AddEntry (ticker);
        currentcurrency = ticker->_firstdbrow->_currency;
    }
}
