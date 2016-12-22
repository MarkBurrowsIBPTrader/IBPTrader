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
#include "Ticker.h"



IB_PTraderDlg::GlobalCurrencyBuy *IB_PTraderDlg::GetGlobalCurrencyBuy (std::string currency)
{
    IB_PTraderDlg::MaxBuysPerCurrencyIter iter = _maxbuyspercurrency.find (currency);
    if (iter != _maxbuyspercurrency.end ())
        return iter->second;
    IB_PTraderDlg::GlobalCurrencyBuy *result = new IB_PTraderDlg::GlobalCurrencyBuy ();
    _maxbuyspercurrency [currency] = result;
    return result;
}
