/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include "BSearch.h"
#include "MarketStatus.h"
#include "DataTypes.h"
#include "PricePair.h"
#include "LivePriceLogger.h"
#include "TechnicalAnalysis.h"
#include <map>



class IB_PTraderDlg;
class Share;


class TickerStats
{
private:
    long _numberactive;
    long _numberpending;
    long _numberbought;

    friend class Ticker;

public:
    inline long NumberActive () { return _numberactive; }
    inline long NumberPending () { return _numberpending; }
    inline long NumberBought () { return _numberbought; }

public:
    void Reset ();
    CString GetLogMessage (CString prefix);

    TickerStats ();
};


class Ticker
{
    #pragma region "ActiveState"
public:
    enum ActiveState
    {
        Active, InActive, SellOnly, BuyOnly, DISABLED
    };

    inline bool BuysAllowed () { return _activestate == Active || _activestate == BuyOnly; }
    inline bool SellsAllowed () { return _activestate == Active || _activestate == SellOnly; }

    static CString ToString (ActiveState state)
    {
        switch (state)
        {
            case Ticker::Active: { return "Active"; }
            case Ticker::InActive: { return "InActive"; }
            case Ticker::SellOnly: { return "Sell"; }
            case Ticker::BuyOnly: { return "Buy"; }
            case Ticker::DISABLED: { return "DISABLED"; }
        }
        return "?";
    }

    static ActiveState GetNextActiveState (ActiveState state)
    {
        switch (state)
        {
            case Ticker::Active:
                return Ticker::InActive;
            case Ticker::InActive:
                return Ticker::SellOnly;
            case Ticker::SellOnly:
                return Ticker::BuyOnly;
            case Ticker::BuyOnly:
                return Ticker::Active;
            case Ticker::DISABLED:
                return Ticker::DISABLED;
        }
        return state;
    }
    #pragma endregion

    #pragma region "Fields"
private:
    std::string _ticker;
    ActiveState _activestate;
    TickerId _tickerid;
    bool _nullticker;
    bool _tickerhasnoprices;
    Share *_share;
    MarketDetails::MarketStatus _marketstatus;
    bool _allow_early_buys;

public:
    inline std::string &TickerName () { return _ticker; }
    inline TickerId TheTickerId () { return _tickerid; }

    #pragma region "Price Recording"
public:
    enum PriceRecordType
    {
#if ! PUBLIC_BUILD
        RecordToDB, RecordToFile, 
#endif
        LivePrices, 
        NoRecording
    };

    static CString ToString (PriceRecordType pricerecordtype, bool shortform)
    {
        switch (pricerecordtype)
        {
#if ! PUBLIC_BUILD
            case RecordToDB: { return shortform ? "DB" : "ToDB"; }
            
            case RecordToFile: { return shortform ? "FI" : "ToFile"; }
#endif
            case LivePrices: { return shortform ? "LP" : "LivePrices"; }
            case NoRecording: { return shortform ? "NR" : "NoRecording"; }
        }
        return "?";
    };

private:
    PriceRecordType _pricerecordtype;
    #pragma endregion

    #pragma region "Buys"
private:
    BSearch <BuyPricePair> _buys;
    BSearchStartResult _searchstartresult;
    int _searchstartmatchindex;
    CTime _lastbuytime;
    #pragma endregion

    #pragma region "Sells"
private:
    BSearch <SellPricePair> _sells;
    BuyPricePair *_sellsstartpoint;    
    int _sellstartpoint_startindex;
    int _sellstartpoint_endindex;
    #pragma endregion

private:
    void DetermineBranches ();

    PGenDatabaseRows::PGenDatabaseRow *_firstdbrow;
    int _listboxindex;

public:
    inline PGenDatabaseRows::PGenDatabaseRow *FirstDBRow () { return _firstdbrow; }

    #pragma region "Bids"
private:
    double _currentbid, _minbid, _maxbid;
    int _bidsize;
    bool _minmaxes_set_bid;

public:
    inline double CurrentBid () { return _currentbid; }
    #pragma endregion

    #pragma region "Offers"
private:
    double _currentoffer, _minoffer, _maxoffer;
    int _offersize;
    bool _minmaxes_set_offer;

public:
    inline double CurrentOffer () { return _currentoffer; }    
    #pragma endregion

    #pragma region "Min Max Set"
public:
    inline bool MinMaxSetBidOffer () { return _minmaxes_set_bid && _minmaxes_set_offer; }
    #pragma endregion

    #pragma region "Tick Volume"
private:
    int _tickvolume_bid, _tickvolume_offer;
    int _tickdepth_bid, _tickdepth_offer;
    #pragma endregion

    #pragma region "IB High/Lows"
private:
    double _ib_low;
    double _ib_high;
    double _ib_open;
    #pragma endregion

    #pragma region "Buy/Sells"
private:
    int _numberofbuys;
    double _totalbuycost;
    int _numberofsells;
    double _salestotal;
    double _netprofits;

    int _number_of_cancelledbuys;
    int _number_of_cancelledsells;
    #pragma endregion

    #pragma region "Volume"
private:
    int _volume;
    int _avgvolume;
    int _volumemultiplier;

public:
    inline int Volume () { return _volume; }
    inline int AvgVolume () { return _avgvolume; }
    #pragma endregion

    #pragma region "Limits"
private:
    bool _maxglobalcurrencycashexceeded;
    bool _maxcashexceeded;
    bool _maxoiexceeded;
    bool _maxdailybuycashexceeded;
    bool _maxdailysellcashexceeded;
    #pragma endregion

    #pragma region "Trending"
private:
    int _maxtrendcount;
    #pragma endregion

    #pragma region "Buy Averages"
public:
    enum BuyAverages
    {
        OnlyIfBelow, Always
    };

private:
    BuyAverages _buyaverages;
    double _userdefined_avg;
    #pragma endregion

    CString _currentlistboxline;

    void UpdateTickerStats (TickerStats &stats, PricePair *pricepair);
    void GetTickerStats_Buy (TickerStats &stats);
    void GetTickerStats_Sell (TickerStats &stats);

public:
    BuyPricePair *AddPricePair (PGenDatabaseRows::PGenDatabaseRow *dbrow, IB_PTraderDlg *parent);
    #pragma endregion

    #pragma region "SellSearchSpeed"
private:
#if ! PUBLIC_BUILD
    enum SellSearchSpeed
    {
        Fast, Slow
    };

    SellSearchSpeed _sellsearchspeed;
#endif
    #pragma endregion

    #pragma region "Batch Buys"
private:
    bool _makebatchbuy;
    #pragma endregion

    #pragma region "Auto Liquidation"
public:
    enum LiquidateOptions
    {
        NoLiquidation, InProfit, JustLosses, AllBuys
    };

    static std::string ToDisplayString (LiquidateOptions liquidateoptions)
    {
        switch (liquidateoptions)
        {
            case NoLiquidation: { return "NoLiquidation"; }
            case InProfit: { return "InProfit"; }
            case JustLosses: { return "JustLosses"; }
            case AllBuys: { return "AllBuys"; }
        }
        return "";
    }

    static std::string ToTickerDisplayString (LiquidateOptions liquidateoptions)
    {
        switch (liquidateoptions)
        {
            case NoLiquidation: { return "N"; }
            case InProfit: { return "P"; }
            case JustLosses: { return "L"; }
            case AllBuys: { return "A"; }
        }
        return "";
    }

private:
    bool _autoliquidate_on, _autoliquidate_DONE;
    CTime _autoliquidatetime;
    LiquidateOptions _liquidationtype;
    #pragma endregion

    #pragma region "Currency Support"
private:
    bool _currencysymbolset;
    std::string _currencysymbol;

    std::string GetCurrencySymbol ()
    {
        if (! _currencysymbolset)
        {
            if (_firstdbrow->_currency == "USD")
            {
                _currencysymbol = "$";
            }
            else if (_firstdbrow->_currency == "CAD")
            {
                _currencysymbol = "c$";
            }
            else if (_firstdbrow->_currency == "GBP")
            {
                _currencysymbol = "£";
            }
            else if (_firstdbrow->_currency == "AUD")
            {
                _currencysymbol = "a$";
            }
            else if (_firstdbrow->_currency == "EUR")
            {
                _currencysymbol = "e";
            }
            else
            {
                _currencysymbol = "";
            }
            _currencysymbolset = true;
        }
        return _currencysymbol;
    }    
    #pragma endregion

    #pragma region "Turn on/off buys"
public:
    enum OnOffOptions
    {
        All_On, All_Off, Off_Even, Off_Odd, One_In_Three, One_In_Four
    };

    static std::string ToDisplayString (OnOffOptions onoffoptions)
    {
        switch (onoffoptions)
        {
            case All_On: { return "All_On"; }
            case All_Off: { return "All_Off"; }
            case Off_Even: { return "Off_Even"; }
            case Off_Odd: { return "Off_Odd"; }
            case One_In_Three: { return "One_In_Three"; }
            case One_In_Four: { return "One_In_Four"; }
        }
        return "";
    }

    void SetOnOffBuyOptions (OnOffOptions onoffoptions);
    #pragma endregion

    #pragma region "Live Prices Set"
private:
    LivePricesSet *_livepriceset;

public:
    inline LivePricesSet *GetLivePricesSet () { return _livepriceset; }
    #pragma endregion

    #pragma region "Technical Indicators"
private:
    TechnicalAnalysisSet *_technicalindicators;
    bool _ignoretechnicalindicators;
    #pragma endregion

public:
    friend class IB_PTraderDlg;
    friend class PricePair;
    friend class BuyPricePair;
    friend class SellPricePair;
    friend class TickerListBox;
    friend class Stats;

    static Ticker *GetTicker (Ticker *ticker, Share *share);

    Ticker (TickerId tickerid, std::string ticker, PGenDatabaseRows::PGenDatabaseRow *dbrow);
    ~Ticker ();
};


typedef std::map <TickerId, Ticker *>::iterator TickersIter;
typedef std::map <std::string, Ticker *>::iterator TickersByContractIter;


class Tickers
{
private:
    std::map <TickerId, Ticker *> *_tickers;
    std::map <std::string, Ticker *> *_tickers_by_contract; // <ticker+sectype+exchange>

public:
    inline std::map <TickerId, Ticker *> *GetTickers () { return _tickers; }

public:
    inline Ticker *GetTicker (TickerId tickerid)
    {
        if (_tickers)
        {
            TickersIter iter = _tickers->find (tickerid);
            if (iter != _tickers->end ())
                return iter->second;
        }
        return 0;
    }

    Ticker *GetTickerByContract (std::string hashkey);
    Ticker *GetTickerByContract (std::string ticker, std::string primaryexch, std::string sectype);

    void AddTickerByContract (Ticker *ticker);
    Ticker *AddTicker (TickerId tickerid, std::string ticker, PGenDatabaseRows::PGenDatabaseRow *dbrow);

    Tickers (PGenDatabaseRows *initialisation, IB_PTraderDlg *parent);
    ~Tickers ();
};
