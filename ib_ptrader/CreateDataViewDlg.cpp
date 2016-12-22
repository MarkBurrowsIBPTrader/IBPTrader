/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "ChartViewDlg.h"



CChartViewDlg *IB_PTraderDlg::GetChartViewDlg (Ticker *ticker, CChartViewDlg::ChartKind chartkind, CChartViewDlg::ChartPeriod chartperiod, int day, int month, int year)
{
    CString findwindowtitle = CChartViewDlg::GetWindowTitle (this, ticker, chartkind, chartperiod, day, month, year);
    CWnd *wnd = CWnd::FindWindow (0, findwindowtitle);
    if (wnd)
    {
        return (CChartViewDlg *) wnd;
    }
    return 0;
}


void IB_PTraderDlg::CreateChartDataView (Ticker *ticker, CChartViewDlg::ChartKind chartkind, CChartViewDlg::ChartPeriod chartperiod, int day, int month, int year)
{
    CChartViewDlg *dlg = GetChartViewDlg (ticker, chartkind, chartperiod, day, month, year);
    if (dlg)
    {
        dlg->BringWindowToTop ();
    }
    else
    {
        dlg = new CChartViewDlg (this);
        dlg->SetChartKind (chartkind, chartperiod, ticker, day, month, year);

        switch (chartkind)
        {
            case CChartViewDlg::Live:
                dlg->SetLivePrices (ticker->_livepriceset, false, false);
                break;
            case CChartViewDlg::Historic:
                LivePricesSet *livepricesset = new LivePricesSet (this, ticker, false);
                livepricesset->LoadDataSet (day, month, year);
                dlg->SetLivePrices (livepricesset, false, true);
                break;
        }

        dlg->Create (CChartViewDlg::IDD, GetDesktopWindow ());
        dlg->ShowWindow (SW_SHOW);
    }
}
