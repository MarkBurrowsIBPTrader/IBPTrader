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



void IB_PTraderDlg::OnTimer (UINT_PTR nIDEvent)    
{
    switch (nIDEvent)
    {
        case TWS_MESSAGES_REFRESH_TIMER:
        {
            _pclient->checkMessages ();
            InvalidateTickerDisplay (0);
            break;
        }
        case CHECKSELLSTARTS_TIMER:
        {
            CTime now = CTime::GetCurrentTime ();
            // CheckForLiquidation
            CheckForLiquidation (now);
#if ! PUBLIC_BUILD
            _numberofsellstarts_fails = DumpSellStarts (0, false);
            if (_numberofsellstarts_fails > _max_number_of_sellstarts_fails)
            {
                _max_number_of_sellstarts_fails = _numberofsellstarts_fails;
            }            
            _lastsearchspeedchecktime.Format ("%0.2d:%0.2d:%0.2d", now.GetHour (), now.GetMinute (), now.GetSecond ());
            SetSellStartsButtonText ();
#endif
            break;
        }
        case RECORDPRICES_TIMER:
        {
            CTime now = CTime::GetCurrentTime ();
            RecordPrices (now);
            break;
        }
    }
    __super::OnTimer (nIDEvent);
}
