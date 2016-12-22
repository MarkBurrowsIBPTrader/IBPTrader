/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once



class Ticker;


class Share
{
    /*
        ibrokers.shares (Table)
        ========================
        idshares         int(10) unsigned PK
        tickerid         int(11)
        ticker           text
        displayname      text
        primaryexchange  text
        sectype          text
        minoi            int(11)
        maxoi            int(11)
        maxvalue         double
        username         text
        accountnumber    text
    */

    // POD. Easier to keep public, rather than unnecessary get/set methods.
    // Each instance of this class represents 1 row in the shares table.
    // Everyone else needs easy access to this data.
public:
    TickerId _tickerid;      
    std::string _ticker;  
    std::string _displayname;  
    std::string _primaryexchange;  
    std::string _sectype;  
    long _minoi;
    long _maxoi;  
    double _maxvalue; 

    long _minoi_required;
    long _maxoi_allowed;
    double _maxvalue_allowed;

    long _currentoi;

    bool _share_has_buy_daylimit;
    double _max_allowed_purchases_today;
    double _ORIG_max_allowed_purchases_today;
    double _totalpurchases_made_today;

    bool _share_has_sell_daylimit;
    double _max_allowed_sales_today;
    double _ORIG_max_allowed_sales_today;
    double _totalsales_made_today;

    Ticker *_tickerptr;

    static std::string GetHashKey (std::string ticker, std::string primaryexch, std::string sectype);
    static std::string GetHashKey (PGenDatabaseRows::PGenDatabaseRow *dbrow);
    static std::string GetHashKey (Share *share);
    static std::string GetHashKey (Contract &contract);

    Share ();
};