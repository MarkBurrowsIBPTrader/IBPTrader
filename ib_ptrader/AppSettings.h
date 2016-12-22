/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once



class IB_PTraderDlg;


class AppSettings
{
protected:
    IB_PTraderDlg *_parent;  

public:
#if ! PUBLIC_BUILD
    virtual void InitialiseBreaks () = 0;
#endif
    virtual void InitialiseAllowEarlyBuyTickers () = 0;
    virtual void SetShareDayLimits () = 0;
    virtual void InitialiseCustomSpreads () = 0;
    virtual void InitialiseInActiveTickers () = 0;
    virtual void AddGlobalInActiveTickers () = 0;
    virtual void InitialiseTrends () = 0;
    virtual void SetBuyBelowAverages () = 0;
    virtual void InitialiseTickersTA () = 0;
    virtual void InitialiseTickersToRecord () = 0;
    virtual void InitialiseLiquidations () = 0;
    virtual void InitialiseTickersBuysOnOff () = 0;
    virtual void InitialiseGlobalCurrencyBuys () = 0;

    AppSettings (IB_PTraderDlg *parent) : _parent (parent) {}
};


class PaperSettings : public AppSettings
{
public:
#if ! PUBLIC_BUILD
    void InitialiseBreaks ();
#endif
    void InitialiseAllowEarlyBuyTickers ();
    void SetShareDayLimits ();
    void InitialiseCustomSpreads ();
    void InitialiseInActiveTickers ();
    void AddGlobalInActiveTickers ();
    void InitialiseTrends ();
    void SetBuyBelowAverages ();
    void InitialiseTickersTA ();
    void InitialiseTickersToRecord ();
    void InitialiseLiquidations ();
    void InitialiseTickersBuysOnOff ();
    void InitialiseGlobalCurrencyBuys ();

    PaperSettings (IB_PTraderDlg *parent) : AppSettings (parent) {}
};


class LiveSettings : public AppSettings
{
public:
#if ! PUBLIC_BUILD
    void InitialiseBreaks ();
#endif
    void InitialiseAllowEarlyBuyTickers ();
    void SetShareDayLimits ();
    void InitialiseCustomSpreads ();
    void InitialiseInActiveTickers ();
    void AddGlobalInActiveTickers ();
    void InitialiseTrends ();
    void SetBuyBelowAverages ();
    void InitialiseTickersTA ();
    void InitialiseTickersToRecord ();
    void InitialiseLiquidations ();
    void InitialiseTickersBuysOnOff ();
    void InitialiseGlobalCurrencyBuys ();

    LiveSettings (IB_PTraderDlg *parent) : AppSettings (parent) {}
};

