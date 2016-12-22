using System;
using System.Collections.Generic;
using System.Text;



namespace pshare
{
    partial class pshare
    {
        dbconnection.DBConnectionManager dbmanager = new dbconnection.DBConnectionManager ();
        dbconnection.DBConnection _mysqlconnection;

        void Go (pcmdsupport.ParseDBCMDLine dbcmdline, pcmdsupport.ParseAccountsCMDLine accountscmdline, 
                 int startindex, string [] args)
        {
            try
            {
                _mysqlconnection = dbmanager.GetDBConnection (dbcmdline.UserName, dbcmdline.Pwd, dbcmdline.Schema, dbcmdline.Hostname);
                _shareentries.Reset ();
                ReadDatabase_shares (accountscmdline);

                List <string> fields = Fields.CollateFields (startindex, true, args);
                if (fields == null || fields.Count == 0)
                {
                    return;
                }                

                // format is [ticker,displayname,exchange,maxoi,maxvalue]
                foreach (string input in fields)
                {
                    string newinput = input;
                    string [] splits = newinput.Split (',');
                    if (splits.Length == 5)
                    {
                        ShareEntry shareentry = new ShareEntry (splits [0], splits [1], splits [2], "STK", "0",
                                                                splits [3], splits [4], accountscmdline.UserName,
                                                                accountscmdline.AccountNumber);
                        bool alreadyadded = _shareentries.AddShareEntry (shareentry, true);
                        Console.WriteLine ((alreadyadded ? "Existing Ticker " : "New Ticker ") + shareentry._ticker + " has a ticker id of " + shareentry._tickerid);
                    }
                }

                Console.WriteLine ("Updating Database");
                UpdateDatabase_shares (accountscmdline);
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
                        pshare ps = new pshare ();
                        ps.Go (dbcmdline, accountscmdline, 2, args);
                        Console.Write ("Finished");
                        reporterror = false;
                    }
                }
                if (reporterror)
                {
                    Console.WriteLine ("Command Line should be:-");
                    Console.Write ("pshare " + pcmdsupport.ParseDBCMDLine.SupportedFormatString () + " " +
                                   pcmdsupport.ParseAccountsCMDLine.SupportedFormatString () + " [ticker,displayname,exchange,maxoi,maxvalue]");
                }
            }
            catch (Exception e)
            {
                Console.Write (e.Message);
            }
        }
    }
}
