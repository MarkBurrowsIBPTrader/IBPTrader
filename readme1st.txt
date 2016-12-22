IB_PTrader for Interactive Brokers
----------------------------------

IB_PTrader is an automated trading application designed to work on the Interactive Brokers TWS platform.
IB_PTrader uses the concept of a "pyramid generator" of buy/sell orders as described on the
http://www.gracelandupdates.com website. This approach removes the need to try and determine the direction
of the price of a share (via technical analysis for example), but only to react to the price change.
As the price of a share falls the app buys more, as it rises the app sells the shares it bought at a lower price.

The pyramid takes its name from the buy orders, ie the lower the price the more shares are bought.

     XX          buy 1 share at $12 per share, sell at $14
    XXXX         buy 2 shares at $8 per share, sell at $10
   XXXXXX        buy 5 shares at $4 per share, sell at $6
  XXXXXXXX       buy 15 shares at $2 per share, sell at $3
 XXXXXXXXXX      buy 40 shares at $1 per share, sell at $1.5

IBs low commissions ($1 per trade on small trades) means it's possible to sell shares for a profit after a small increase in price. 
The app can be used to trade in several scenarios:-
1) trading highly volatile shares
2) shares that trade in a tight range
3) spread trade a leveraged 2x/3x Bull ETF and the corresponding Bear ETF
4) a trading position that backs the core holding of a share

The app was originally written in 2009 as a C++ MFC application using Visual Studio 2008. The use of MFC is dictated by the IB API
which uses the MFC message pump to send data to the app.

Since the app was intended for personal use the GUI lacks a polished look and changing some of the settings requires a
rebuild of the app rather than via a front end.

Interactive Brokers offers 2 types of commission pricing - Cost Plus and Flat Rate. Currently the app only
supports Flat Rate. Anyone using the Cost Plus plan will have to code this up. This is important because the
app attempts to estimate the commission part of a trade to prevent trading losses in a situation where a sell order
is too close to the buy order.
