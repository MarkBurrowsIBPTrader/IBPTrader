using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using MySql.Data.MySqlClient;
using MySql.Data.Types;



namespace pshare
{
    partial class pshare
    {
        class IBSpecifics
        {
            // POD
            public string _sectype;
            public string _exchange;
            public string _primaryexchange;
            public string _currency;
            public string _localsymbol;
            public string _secidtype;
            public string _secid;
            public string _genericticks;

            public IBSpecifics (string sectype,
                                string exchange,
                                string primaryexchange,
                                string currency,
                                string localsymbol,
                                string secidtype,
                                string secid,
                                string genericticks)
            {
                _sectype = sectype;
                _exchange = exchange;
                _primaryexchange = primaryexchange;
                _currency = currency;
                _localsymbol = localsymbol;
                _secidtype = secidtype;
                _secid = secid;
                _genericticks = genericticks;
            }
        }

        class PGenEntry
        {
            /*
                ibrokers.deals (Table)
                =======================
                iddeals: INT UNSIGNED
                tickerid: TEXT
                ticker: TEXT
                oi: TEXT
                maxoi: TEXT
                buyprice: TEXT
                numberbuyshares: TEXT
                sellprice: TEXT
                numbersellshares: TEXT
                sectype: TEXT
                exchange: TEXT
                primaryexchange: TEXT
                currency: TEXT
                localsymbol: TEXT
                secidtype: TEXT
                secid: TEXT
                genericticks: TEXT
                displayname: TEXT
            */

            // POD
            public ShareEntry _shareentry;
            public string _buyprice;
            public string _numbersharestobuy;
            public string _sellprice;
            public string _numbersharestosell;
            public string _sectype;
            public string _exchange;
            public string _primaryexchange;
            public string _currency;
            public string _localsymbol;
            public string _secidtype;
            public string _secid;
            public string _genericticks;

            override public string ToString ()
            {
                return _shareentry._ticker + " (" + _shareentry._tickerid + "), Buy=" + _buyprice + " (" + _numbersharestobuy + "), " +
                       ", Sell=" + _sellprice + " (" + _numbersharestosell + ")";
            }

            public PGenEntry (ShareEntry shareentry, 
                              string buyprice, 
                              string numbersharestobuy, 
                              string sellprice, 
                              string numbersharestosell,
                              string sectype,
                              string exchange,
                              string primaryexchange,
                              string currency,
                              string localsymbol,
                              string secidtype,
                              string secid,
                              string genericticks)
            {
                _shareentry = shareentry;
                _buyprice = buyprice;
                _numbersharestobuy = numbersharestobuy;
                _sellprice = sellprice;
                _numbersharestosell = numbersharestosell;
                _sectype = sectype;
                _exchange = exchange;
                _primaryexchange = primaryexchange;
                _currency = currency;
                _localsymbol = localsymbol;
                _secidtype = secidtype;
                _secid = secid;
                _genericticks = genericticks;
            }

            public PGenEntry (ShareEntry shareentry, 
                              string buyprice, 
                              string numbersharestobuy, 
                              string sellprice, 
                              string numbersharestosell,
                              IBSpecifics ibspecifics) : this (shareentry, buyprice, numbersharestobuy, sellprice, numbersharestosell,
                                                               ibspecifics._sectype, ibspecifics._exchange, ibspecifics._primaryexchange,
                                                               ibspecifics._currency, ibspecifics._localsymbol, ibspecifics._secidtype,
                                                               ibspecifics._secid, ibspecifics._genericticks)
            {
            }
        }

        void WritePGenAttr (StreamWriter writer, string field, string value)
        {
            writer.Write (field);
            writer.Write ("=\"");
            writer.Write (value);
            writer.Write ("\" ");
        }

        void CreatePGen (pcmdsupport.ParseAccountsCMDLine account, ShareEntry shareentry, 
                         string filename,
                         params PGenEntry [] pgens)
        {
            string username = account.UserName;
            Utility.CreateDirectory (Path.GetDirectoryName (filename));

            FileStream filestream = null;
            StreamWriter writer = null;
            try
            {
                filestream = new FileStream (filename, FileMode.Create);
                writer = new StreamWriter (filestream);

                writer.WriteLine ("<orders>");

                foreach (PGenEntry pgenentry in pgens)
                {
                    pgenentry._shareentry._pgenfilename = filename;

                    writer.Write (" <order ");

                    WritePGenAttr (writer, "ticker", pgenentry._shareentry._ticker);
                    WritePGenAttr (writer, "tickerid", pgenentry._shareentry._tickerid);
                    WritePGenAttr (writer, "buyprice", pgenentry._buyprice);
                    WritePGenAttr (writer, "numberbuyshares", pgenentry._numbersharestobuy);
                    WritePGenAttr (writer, "sellprice", pgenentry._sellprice);
                    WritePGenAttr (writer, "numbersellshares", pgenentry._numbersharestosell);                   
                    WritePGenAttr (writer, "sectype", pgenentry._sectype);  
                    WritePGenAttr (writer, "exchange", pgenentry._exchange);  
                    WritePGenAttr (writer, "primaryexchange", pgenentry._primaryexchange);  
                    WritePGenAttr (writer, "currency", pgenentry._currency);  
                    WritePGenAttr (writer, "localsymbol", pgenentry._shareentry._ticker); 
                    WritePGenAttr (writer, "secidtype", pgenentry._secidtype);  
                    WritePGenAttr (writer, "secid", pgenentry._secid);  
                    WritePGenAttr (writer, "genericticks", pgenentry._genericticks);  
                    WritePGenAttr (writer, "displayname", pgenentry._shareentry._displayname);

                    writer.WriteLine ("></order>");
                }

                writer.WriteLine ("</orders>");
            }
            finally
            {
                if (writer != null)
                    writer.Close ();
                if (filestream != null)
                    filestream.Close ();
            }
        }

        bool CompareFileName (string file1, string file2)
        {
            var directory1 = Path.GetDirectoryName (file1);
            var directory2 = Path.GetDirectoryName (file2);
            var fileName1 = Path.GetFileName (file1);
            var fileName2 = Path.GetFileName (file2);

            return directory1.Equals (directory2, StringComparison.InvariantCultureIgnoreCase) &&
                   fileName1.Equals (fileName2, StringComparison.InvariantCultureIgnoreCase);
        }

        void UpdateDatabase_pgens (pcmdsupport.ParseAccountsCMDLine account, bool deletepgenfields)
        {
            /*
                ibrokers.pgens (Table)
                =======================
                idpgens: INT UNSIGNED
                pgenxml: TEXT
                username: TEXT
                accountnumber: TEXT
            */
            MySqlTransaction mysqltransaction = null;
            MySqlCommand mysqlcommand = null;
            try
            {
                string username = account.UserName;
                string accountnumber = account.AccountNumber;

                List <string> existingpgensxmls = new List <string> ();
                if (! deletepgenfields)
                {
                    StringBuilder sb = new StringBuilder ("select pgenxml from pgens where ");
                    sb.Append (" username = '");
                    sb.Append (username);
                    sb.Append ("' and accountnumber = '");
                    sb.Append (accountnumber);
                    sb.Append ("'");                    
                    mysqlcommand = new MySqlCommand (sb.ToString (), _mysqlconnection);
                    mysqlcommand.CommandTimeout = dbconnection.DBConnectionManager.DATABASE_TIMEOUT;
                    MySqlDataReader mysqlreader = mysqlcommand.ExecuteReader ();
                    while (mysqlreader.Read ()) 
                    {
                        string pgenxml = mysqlreader.GetString (0);
                        existingpgensxmls.Add (pgenxml);
                    }
                    mysqlreader.Close ();
                }

                mysqltransaction = _mysqlconnection.TheDBConnection.BeginTransaction();
                mysqlcommand = _mysqlconnection.TheDBConnection.CreateCommand();
                mysqlcommand.Connection = _mysqlconnection.TheDBConnection;
                mysqlcommand.Transaction = mysqltransaction;

                if (deletepgenfields)
                {
                    mysqlcommand.CommandText = "delete from pgens";                
                    mysqlcommand.Parameters.Clear ();
                    mysqlcommand.ExecuteNonQuery ();
                }

                foreach (ShareEntry entry in _shareentries._linearshareentries)
                {
                    if (entry._pgenfilename != null)
                    {
                        bool matchfound = false;
                        foreach (string searchfile in existingpgensxmls)
                        {
                            if (CompareFileName (entry._pgenfilename, searchfile))
                            {
                                matchfound = true;
                                break;
                            }
                        }
                        if (! matchfound)
                        {
                            mysqlcommand.CommandText = "insert into pgens (pgenxml, username, accountnumber) VALUES (?field1, ?field2, ?field3)";
                            mysqlcommand.Parameters.Clear ();
                            mysqlcommand.Parameters.AddWithValue ("?field1", entry._pgenfilename);
                            mysqlcommand.Parameters.AddWithValue ("?field2", username);
                            mysqlcommand.Parameters.AddWithValue ("?field3", accountnumber);
                            mysqlcommand.ExecuteNonQuery ();
                        }
                    }
                }

                mysqltransaction.Commit ();
            }
            catch (Exception e)
            {
                try
                {
                    if (mysqltransaction != null)
                        mysqltransaction.Rollback ();
                }
                catch
                {
                }
                if (e is System.Threading.ThreadAbortException)
                    throw;
            } 
        }

        PGenEntry [] CreatePGenSet (ShareEntry entry, decimal start, decimal decrement, int number, decimal selloffset, int numbershares_buy, int numbershares_sell, IBSpecifics ibspecifics)
        {
            List <PGenEntry> pgens = new List <PGenEntry> ();
            decimal current = start;
            for (int i = 0;  i < number;  i++)
            {
                PGenEntry pgen = new PGenEntry (entry, current.DisplayMoney (), numbershares_buy.ToString (), (current + selloffset).DisplayMoney (), numbershares_sell.ToString (), ibspecifics);
                pgens.Add (pgen);
                current += decrement;
            }
            return pgens.ToArray ();
        }

        PGenEntry [] CreatePGenSetRange (ShareEntry entry, decimal lower, decimal upper, decimal gap, decimal selloffset, 
                                         int numbershares_buy, int numbershares_sell, IBSpecifics ibspecifics)
        {
            List <PGenEntry> pgens = new List <PGenEntry> ();
            decimal current = lower;
            while (current <= upper)
            {
                PGenEntry pgen = new PGenEntry (entry, current.DisplayMoney (), numbershares_buy.ToString (), (current + selloffset).DisplayMoney (), numbershares_sell.ToString (), ibspecifics);
                pgens.Add (pgen);
                if (gap == 0)
                    break;
                current += gap;
            }
            return pgens.ToArray ();
        }

        private class PGenEntrySorter : IComparer <PGenEntry>
        {
            public int Compare (PGenEntry x1, PGenEntry x2)
            {
                double x1_price = double.Parse (x1._buyprice);
                double x2_price = double.Parse (x2._buyprice);
                if (x1_price < x2_price)
                    return -1;
                else if (x1_price > x2_price)
                    return 1;
                return 0;
            }
        }

        PGenEntry [] MergePGenSets (params PGenEntry [] [] sets)
        {
            List <PGenEntry> list = new List <PGenEntry> ();
            foreach (PGenEntry [] set in sets)
            {
                list.AddRange (set);
            }
            list.Sort (new PGenEntrySorter ());
            return list.ToArray ();
        }

        const string EXCHANGE_ISLAND = "ISLAND";
        const string EXCHANGE_ARCA = "ARCA";
        const string EXCHANGE_NYSE = "NYSE";
        const string EXCHANGE_LONDON = "LSE";
        const string EXCHANGE_TSE = "TSE";
        const string EXCHANGE_VENTURE = "VENTURE";
        const string EXCHANGE_NASDAQ = "NASDAQ.NMS";

        IBSpecifics _usdmarket_isl;
        IBSpecifics _usdmarket_arca;
        IBSpecifics _usdmarket_nyse;
        IBSpecifics _cadmarket_tsx;
        IBSpecifics _cadmarket_ven;
        IBSpecifics _gbpmarket;
        IBSpecifics _usdmarket_nasdaq;
    }
}
