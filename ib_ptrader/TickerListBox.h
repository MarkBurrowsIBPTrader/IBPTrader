/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include "afxwin.h"



class IB_PTraderDlg;


class TickerListBox : public CListBox
{
    DECLARE_DYNAMIC(TickerListBox)

private:
    IB_PTraderDlg *_parent;

public:
    inline void SetParent (IB_PTraderDlg *parent) { _parent = parent; }

public:
    TickerListBox ();
    virtual ~TickerListBox ();

    void AddEntry (Ticker *ticker);

protected:
    DECLARE_MESSAGE_MAP()

public:
    virtual void MeasureItem (LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    virtual void DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
    BuyPricePair *GetAppendNearestPriceMatch_BuyPrice (Ticker *ticker, int offset);
    BuyPricePair *AppendNearestPriceMatch (Ticker *ticker, CString &str, int offset, bool offsetismiddle, bool fp, bool append_selloffset, bool append_size);
    bool DrawLine (Ticker *ticker, CString &str, CDC &dc, CRect &textrect);
    bool DrawListBoxLine (CDC &dc, CRect textrect, Ticker *ticker);

public:
    static void GetFormattedNumberString (double value, bool round, CString &str);
};