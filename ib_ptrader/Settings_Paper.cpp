#include "stdafx.h"
#include "AppSettings.h"
#include "Ticker.h"



#if ! PUBLIC_BUILD
void PaperSettings::InitialiseBreaks ()
{
    _parent->AddBuyBreak ("SLW");
    _parent->AddSellBreak ("SLW");
}
#endif


void PaperSettings::InitialiseAllowEarlyBuyTickers () // or LiveSettings::InitialiseAllowEarlyBuyTickers ()
{
#if 0
    // Allows app to buy SPY on the NYSE exchange when market status is EarlyOpen
    _parent->AddAllowEarlyBuyTicker ("SPY", "NYSE");
#endif
}


void PaperSettings::SetShareDayLimits () // or LiveSettings::SetShareDayLimits ()
{
#if 0
    // This limits how much money the app spends in a trading day on a particular share. Without
    // this specified the app has no limit
    _parent->SetShare_Buy_DayLimit ("IBM", "NYSE", 3000); // $3000 for this share

    // You can also limit the amount (in value) a share is sold in a day
    _parent->SetShare_Sell_DayLimit ("USU", "NYSE", 5000); // sell up $5000 of this share in a day
#endif
}


void PaperSettings::InitialiseCustomSpreads () // or LiveSettings::InitialiseCustomSpreads ()
{
#if 0
    // When buying a share the app checks the bid/offer spread to see if it is too wide. By default
    // if the spread is more than 1 cent then no buy order is issued.
    _parent->AddCustomSpread ("COPX", "ARCA", 0.1); // allow buy order when the spread is up to 10 cents
#endif
}


void PaperSettings::InitialiseInActiveTickers () // or LiveSettings::InitialiseInActiveTickers () 
{
#if 0
    // A share (ticker) has 5 possible states:-
    //
    // Active - can both buy and sell shares
    // InActive - no selling or buying of shares
    // SellOnly - only sell shares
    // BuyOnly - only buy shares
    // DISABLED - same as inactive but no price feed is requested and user cannot change the state in the GUI
    //
    // The user can change the state of any ticker in the GUI when it's in the first 4 states.

    // By default all tickers are Active

    _parent->AddInActiveTicker ("AAPL", "NASDAQ.NMS", Ticker::Active);
    _parent->AddInActiveTicker ("FB", "NASDAQ.NMS", Ticker::InActive);
    _parent->AddInActiveTicker ("AMZN", "NASDAQ.NMS", Ticker::SellOnly);
    _parent->AddInActiveTicker ("GOOG", "NASDAQ.NMS", Ticker::BuyOnly);
    _parent->AddInActiveTicker ("DELL", "NASDAQ.NMS", Ticker::DISABLED);
#endif
}


void PaperSettings::AddGlobalInActiveTickers () // or LiveSettings::AddGlobalInActiveTickers () 
{
#if 0
    std::list <std::string> exceps;
    exceps.push_back ("USLV");
    exceps.push_back ("DSLV");

    // This sets the ticker status for all tickers except the ones in the exceps list
    _parent->AddGlobalInActiveTickers (exceps, Ticker::SellOnly);

    // Note this method is called after InitialiseInActiveTickers and hence overrides any changes
    // made there
#endif
}


void PaperSettings::InitialiseTrends () // or LiveSettings::InitialiseTrends () 
{
#if 0
    // Normally when the app detects a bid price in excess of the sell point for a share it issues
    // a sell order. When a trend is set it waits for the bid price to change N times before issuing a sell order. 
    // The aim of this is to try and maximize profit if a share is trending upwards
    _parent->AddTickerTrend ("SLW", "NYSE", "USD", 3); // wait 3 times
#endif
}


void PaperSettings::SetBuyBelowAverages () // or LiveSettings::SetBuyBelowAverages ()
{
#if 0
    // IB stores an average price for a shareholding. Normally the app will ignore this
    // when buying a share. If a ticker is configured to OnlyIfBelow the share is only
    // bought if the offer price is less the current share average point.

    _parent->SetBuyBelowAverage ("USLV", "NASDAQ.NMS", Ticker::OnlyIfBelow);
    _parent->SetBuyBelowAverage ("DSLV", "NASDAQ.NMS", Ticker::Always); 

    // Can set to only below average for tickers with a particular currency
    _parent->SetAllBuyBelowAverage ("USD");
    _parent->SetAllBuyBelowAverage ("CAD");
    _parent->SetAllBuyBelowAverage ("GBP");
#endif
}


void PaperSettings::InitialiseTickersTA () // or LiveSettings::InitialiseTickersTA ()
{
#if 0
    // All these technical indicators are calculated using only the current trading day.
    // The app will buy if these TI's are oversold on a particular share

    _parent->SetTickersTA ("USU", "NYSE", "STK", TechnicalIndicator::RSI_14);            // must < 30 to buy
    _parent->SetTickersTA ("USU", "NYSE", "STK", TechnicalIndicator::StochRSI14);        // < 0.2 to buy
    _parent->SetTickersTA ("USU", "NYSE", "STK", TechnicalIndicator::TSI);               // < 0 to buy
    _parent->SetTickersTA ("USU", "NYSE", "STK", TechnicalIndicator::Stochs14_3_3);      // K and D < 20
    _parent->SetTickersTA ("USU", "NYSE", "STK", TechnicalIndicator::Williams_45);       // <= -80 to buy

    // If no TIs are set for a share then the app just buys

    // Note for the TIs to operate for a particular share then LivePrices recording must be set for
    // it as specified in the method below
#endif
}


void PaperSettings::InitialiseTickersToRecord () // or LiveSettings::InitialiseTickersToRecord ()
{
#if 0
    // All changes of the offer price are recorded
    // This is needed to be set so a live chart price for the ticker can be displayed
    _parent->AddTickerToRecord ("USU", "ARCA", "STK", Ticker::LivePrices);

    // You will note a text file is created in the apps working directory for each share/trading day
    // These can be deleted at the end of a trading day or periodically.
#endif
}


void PaperSettings::InitialiseLiquidations () // or LiveSettings::InitialiseLiquidations ()
{
#if 0
    // With a 3 day settlement of cash on selling it is important to sell as many shares each day as
    // possible. Liquidations (when set) will sell any shares that have not hit their sell points but
    // are still in profit if they were sold.

    // Sell any bought positions of SLW if in profit at 19:50pm (in UK time)
    _parent->InitialiseLiquidation ("SLW", "NYSE", "STK", Ticker::InProfit, 1950); 

    // The 3 possible values are
    //
    // InProfit - just shares in profit
    // JustLosses - just shares that are in loss (if you like losing money)
    // AllBuys - all shares whether in profit or loss
#endif
}


void PaperSettings::InitialiseTickersBuysOnOff () // or LiveSettings::InitialiseTickersBuysOnOff ()
{
#if 0
    // Let's say the app has these buy points on a share 5 5.1 5.2 5.3 5.4 5.5 5.6
    // The below have the following effect:-
    //    
    // All_On - all buy points are available
    // All_Off - no buy points are available - app will never make a buy
    // Off_Even - buy points are 5.1 5.3 5.5
    // Off_Odd - buy points are 5 5.2 5.4 5.6
    // One_In_Three - buy points are 5 5.3 5.6
    // One_In_Four - buy points are 5 5.4

    // By reducing the buy points the available capital can be spread more widely

    _parent->SetTickersBuyOnOff ("UNG", "ARCA", Ticker::Off_Even);
    _parent->SetTickersBuyOnOff ("SLW", "NYSE", Ticker::Off_Odd);
#endif
}


void PaperSettings::InitialiseGlobalCurrencyBuys () // or LiveSettings::InitialiseGlobalCurrencyBuys ()
{
#if 0
    // This limits the value of the shares the app can buy per day on a currency basis
    _parent->GetGlobalCurrencyBuy ("CAD")->_maxallowedbuys = 400;  // only ca$400 per day
    _parent->GetGlobalCurrencyBuy ("USD")->_maxallowedbuys = 15000; // only $15000 per day
#endif
}

