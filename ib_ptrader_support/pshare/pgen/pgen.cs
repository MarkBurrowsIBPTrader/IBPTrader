using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;



namespace pshare
{
    partial class pshare
    {
        dbconnection.DBConnectionManager dbmanager = new dbconnection.DBConnectionManager ();
        dbconnection.DBConnection _mysqlconnection;

        void Go (pcmdsupport.ParseDBCMDLine dbcmdline, pcmdsupport.ParseAccountsCMDLine accountscmdline, int startindex, string [] args)
        {
            try
            {
                _mysqlconnection = dbmanager.GetDBConnection (dbcmdline.UserName, dbcmdline.Pwd, dbcmdline.Schema, dbcmdline.Hostname);
                _shareentries.Reset ();
                ReadDatabase_shares (accountscmdline);

                const string GENERICTICKS = "100,101,104,105,106,107,165,221,225,233,236,258";
    
                _usdmarket_isl = new IBSpecifics ("STK", "SMART", EXCHANGE_ISLAND, "USD", "", "", "", GENERICTICKS);
                _usdmarket_arca = new IBSpecifics ("STK", "SMART", EXCHANGE_ARCA, "USD", "", "", "", GENERICTICKS);
                _usdmarket_nyse = new IBSpecifics ("STK", "SMART", EXCHANGE_NYSE, "USD", "", "", "", GENERICTICKS);
                _cadmarket_tsx = new IBSpecifics ("STK", "SMART", EXCHANGE_TSE, "CAD", "", "", "", GENERICTICKS);
                _cadmarket_ven = new IBSpecifics ("STK", "SMART", EXCHANGE_VENTURE, "CAD", "", "", "", GENERICTICKS);
                _gbpmarket = new IBSpecifics ("STK", "SMART", EXCHANGE_LONDON, "GBP", "", "", "", GENERICTICKS);
                _usdmarket_nasdaq = new IBSpecifics ("STK", "SMART", EXCHANGE_NASDAQ, "USD", "", "", "", GENERICTICKS);

                Dictionary <string, bool> _donetickers = new Dictionary <string, bool> ();
                List <string> fields = Fields.CollateFields (startindex, true, args);
                if (fields == null || fields.Count == 0)
                {
                    return;
                } 

                // format is [tickerid,sharestartprice,shareendprice,gap,selloffset,numberbuyshares,pgenfilename] or [delete]
                bool deletepgenfields = false;
                foreach (string input in fields)
                {
                    string [] splits = input.Split (',');
                    if (splits.Length == 7)
                    {
                        #region "pgen"
                        string tickerid = splits [0];
                        if (! _donetickers.ContainsKey (tickerid))
                        {
                            List <string> allfields = new List <string> ();
                            _donetickers [tickerid] = true;
                            for (int i = 0;  i < fields.Count;  i++)
                            {
                                splits = fields [i].Split (',');
                                if (splits.Length == 7)
                                {
                                    if (splits [0] == tickerid)
                                        allfields.Add (fields [i]);                           
                                }
                            }
                            ShareEntry shareentry;
                            if (_shareentries._shareentries.TryGetValue (tickerid, out shareentry))
                            {
                                string xmlfile = null;
                                List <PGenEntry []> pgenentries = new List <PGenEntry []> ();
                                foreach (string allfield in allfields)
                                {
                                    try
                                    {
                                        splits = allfield.Split (',');                                
                                        IBSpecifics ibspecs = null;
                                        if (String.Compare (shareentry._primaryexchange, EXCHANGE_ISLAND, true) == 0)
                                            ibspecs = _usdmarket_isl; 
                                        else if (String.Compare (shareentry._primaryexchange, EXCHANGE_ARCA, true) == 0)
                                            ibspecs = _usdmarket_arca; 
                                        else if (String.Compare (shareentry._primaryexchange, EXCHANGE_NYSE, true) == 0)
                                            ibspecs = _usdmarket_nyse; 
                                        else if (String.Compare (shareentry._primaryexchange, EXCHANGE_TSE, true) == 0)
                                            ibspecs = _cadmarket_tsx; 
                                        else if (String.Compare (shareentry._primaryexchange, EXCHANGE_VENTURE, true) == 0)
                                            ibspecs = _cadmarket_ven; 
                                        else if (String.Compare (shareentry._primaryexchange, EXCHANGE_LONDON, true) == 0)
                                            ibspecs = _gbpmarket; 
                                        else if (String.Compare (shareentry._primaryexchange, EXCHANGE_NASDAQ, true) == 0)
                                            ibspecs = _usdmarket_nasdaq; 
                                        if (ibspecs != null)
                                        {
                                            decimal lower = decimal.Parse (splits [1]);
                                            decimal upper = decimal.Parse (splits [2]);
                                            decimal gap = decimal.Parse (splits [3]);
                                            decimal offset = decimal.Parse (splits [4]);
                                            int numbershares = int.Parse (splits [5]);
                                            PGenEntry [] pgenentry = CreatePGenSetRange (shareentry, lower, upper, gap, offset, numbershares, numbershares, ibspecs);
                                            if (pgenentry != null)
                                            {
                                                pgenentries.Add (pgenentry);
                                                if (xmlfile == null)
                                                    xmlfile = splits [6];
                                            }
                                        }
                                    }
                                    catch (Exception)
                                    {
                                    }
                                }
                                if (xmlfile != null)
                                {
                                    shareentry._pgenfilename = xmlfile;
                                    CreatePGen (accountscmdline, shareentry, xmlfile,
                                                MergePGenSets (pgenentries.ToArray ())
                                               );
                                }
                            }
                        }
                        #endregion
                    }
                    else if (splits.Length == 1)
                    {
                        #region "delete flag?"
                        if (string.Compare (splits [0], "delete", true) == 0)
                        {
                            deletepgenfields = true;
                        }
                        #endregion
                    }
                }

                UpdateDatabase_pgens (accountscmdline, deletepgenfields);
            }
            finally
            {
                dbmanager.CloseConnections ();
            }
        }

        static void Main (string [] args)
        {
            try
            {
                bool reporterror = true;
                if (args.Length >= 2)
                {
                    pcmdsupport.ParseDBCMDLine dbcmdline = new pcmdsupport.ParseDBCMDLine (args [0]);
                    pcmdsupport.ParseAccountsCMDLine accountscmdline = new pcmdsupport.ParseAccountsCMDLine (args [1]);
                    if (! dbcmdline.InError && ! accountscmdline.InError)
                    {
                        pshare pg = new pshare ();
                        pg.Go (dbcmdline, accountscmdline, 2, args);
                        Console.Write ("Finished");
                        reporterror = false;
                    }
                }
                if (reporterror)
                {
                    Console.WriteLine ("Command Line should be:-");
                    Console.Write ("pgen " + pcmdsupport.ParseDBCMDLine.SupportedFormatString () + " " +
                                   pcmdsupport.ParseAccountsCMDLine.SupportedFormatString () + 
                                   " [tickerid,sharestartprice,shareendprice,gap,selloffset,numberbuyshares,pgenfilename]");
                }
            }
            catch (Exception e)
            {
                Console.Write (e.Message);
            }
        }
    }
}
