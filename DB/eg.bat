pshare [demoschema,%1,%2,localhost] [%3,%4] [RGL,Royal Gold,TSE,500,100000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [0,1,200,0.5,1.25,15,c:\pgens\rgl.xml]

pshare [demoschema,%1,%2,localhost] [%3,%4] [TRI,Thomson Reuters,TSE,1000,100000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [1,1,500,0.75,4.66,10,c:\pgens\tri.xml]

rem
pshare [demoschema,%1,%2,localhost] [%3,%4] [RBS,Royal Bank of Scotland,LSE,1000,500000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [2,1,1500,10,150,55,c:\pgens\rbs.xml]

pshare [demoschema,%1,%2,localhost] [%3,%4] [TSCO,Tesco,LSE,2000,300000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [3,1,1200,0.5,125,25,c:\pgens\tesco.xml]

rem
pshare [demoschema,%1,%2,localhost] [%3,%4] [AAPL,Apple,NASDAQ.NMS,1000,250000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [4,1,1200,0.5,1.25,15,c:\pgens\apple.xml]

pshare [demoschema,%1,%2,localhost] [%3,%4] [FB,Facebook,NASDAQ.NMS,2000,25000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [5,1,200,0.5,1.25,15,c:\pgens\fb.xml]

pshare [demoschema,%1,%2,localhost] [%3,%4] [JPM,JPMorgan Chase,NYSE,500,10000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [6,1,200,0.5,1.25,15,c:\pgens\jpm.xml]

pshare [demoschema,%1,%2,localhost] [%3,%4] [LNKD,LinkedIn,NYSE,350,7000]
pgen [demoschema,%1,%2,localhost] [%3,%4] [7,1,200,0.5,1.25,15,c:\pgens\linkedin.xml]

pshare [demoschema,%1,%2,localhost] [%3,%4] [TWTR,Twitter,NYSE,1368,11500]
pgen [demoschema,%1,%2,localhost] [%3,%4] [8,1,200,0.5,1.25,15,c:\pgens\twitter.xml]
