using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MySql.Data.MySqlClient;
using MySql.Data.Types;



namespace pshare
{
    partial class pshare
    {
        dbconnection.DBConnectionManager dbmanager = new dbconnection.DBConnectionManager ();
        dbconnection.DBConnection _mysqlconnection;

        bool Go (pcmdsupport.ParseDBCMDLine dbcmdline, pcmdsupport.ParseAccountsCMDLine accountscmdline, string tickeridstr)
        {
            try
            {
                int tickerid;
                if (! int.TryParse (tickeridstr, out tickerid))
                    return true;

                _mysqlconnection = dbmanager.GetDBConnection (dbcmdline.UserName, dbcmdline.Pwd, dbcmdline.Schema, dbcmdline.Hostname);

                /*
                    ibrokers.deals (Table)
                    =======================
                    iddeals             int(10) unsigned PK
                    tickerid            int(11)
                    ticker              text
                    oi                  int(11)
                    maxoi               int(11)
                    buyprice            double
                    numberbuyshares     int(11)
                    sellprice           double
                    numbersellshares    int(11)
                    sectype             text
                    exchange            text
                    primaryexchange     text
                    currency            text
                    localsymbol         text
                    secidtype           text
                    secid               text
                    genericticks        text
                    displayname         text
                    username            text
                    accountnumber       text
                    hitcount            int(11)
                    lastavgfillbuyprice double
                    lastbuycommission   double
                    lastbuydate         datetime
                    updatetimestamp     timestamp
                */
                string username = accountscmdline.UserName;
                string accountnumber = accountscmdline.AccountNumber;
                StringBuilder sb = new StringBuilder ("select iddeals,tickerid,ticker,oi from deals where tickerid = ");
                sb.Append (tickerid.ToString ());
                sb.Append (" and username = '");
                sb.Append (username);
                sb.Append ("' and accountnumber = '");
                sb.Append (accountnumber);
                sb.Append ("'");
                MySqlCommand mysqlcommand = new MySqlCommand (sb.ToString (), _mysqlconnection);
                mysqlcommand.CommandTimeout = dbconnection.DBConnectionManager.DATABASE_TIMEOUT;
                MySqlDataReader mysqlreader = mysqlcommand.ExecuteReader ();
                int numberofrows = 0;
                List <ulong> rowswith_open_oi = new List <ulong> ();
                string assumed_ticker = null;
                while (mysqlreader.Read ()) 
                {
                    ulong iddeals = mysqlreader.GetUInt64 (0);
                    int rowtickerid = mysqlreader.GetInt32 (1);
                    string rowticker = mysqlreader.GetString (2);
                    int rowoi = mysqlreader.GetInt32 (3);
                    if (rowoi > 0)
                    {
                        rowswith_open_oi.Add (iddeals);
                    }
                    if (numberofrows == 0)
                    {
                        assumed_ticker = rowticker;
                    }
                    numberofrows++;
                }
                mysqlreader.Close ();
                if (rowswith_open_oi.Count > 0)
                {
                    Console.WriteLine (assumed_ticker + " has " + rowswith_open_oi.Count.ToString () + " rows with Open Interest");
                }   
                Console.WriteLine ("Do you want to delete ticker " + assumed_ticker + "?");
                string reply = Console.ReadLine ();
                if (reply == "y" || reply == "Y")
                {
                    Console.WriteLine ("Answer is Yes, deleting...");

                    MySqlTransaction mysqltransaction = null;
                    mysqlcommand = null;
                    try
                    {
                        mysqltransaction = _mysqlconnection.TheDBConnection.BeginTransaction ();
                        mysqlcommand = _mysqlconnection.TheDBConnection.CreateCommand ();
                        mysqlcommand.Connection = _mysqlconnection.TheDBConnection;
                        mysqlcommand.Transaction = mysqltransaction;

                        sb = new StringBuilder ("delete from deals where tickerid = ");
                        sb.Append (tickerid.ToString ());
                        sb.Append (" and username = '");
                        sb.Append (username);
                        sb.Append ("' and accountnumber = '");
                        sb.Append (accountnumber);
                        sb.Append ("'");

                        mysqlcommand.CommandText = sb.ToString ();                
                        mysqlcommand.Parameters.Clear ();
                        mysqlcommand.ExecuteNonQuery ();

                        mysqltransaction.Commit ();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine ("Delete failed with " + e.Message);
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
                    Console.WriteLine ("Deleted");
                }
                else
                {
                    Console.WriteLine ("Answer is NO");
                }
            }
            finally
            {
                dbmanager.CloseConnections ();
            }
            return false;
        }

        static void Main (string [] args)
        {
            try
            {
                bool reporterror = true;
                if (args.Length == 3)
                {
                    pcmdsupport.ParseDBCMDLine dbcmdline = new pcmdsupport.ParseDBCMDLine (args [0]);
                    pcmdsupport.ParseAccountsCMDLine accountscmdline = new pcmdsupport.ParseAccountsCMDLine (args [1]);
                    if (! dbcmdline.InError && ! accountscmdline.InError)
                    {
                        pshare pg = new pshare ();
                        reporterror = pg.Go (dbcmdline, accountscmdline, args [2]);
                        Console.Write ("Finished");
                    }
                }
                if (reporterror)
                {
                    Console.WriteLine ("Command Line should be:-");
                    Console.Write ("pgendel " + pcmdsupport.ParseDBCMDLine.SupportedFormatString () + " " +
                                   pcmdsupport.ParseAccountsCMDLine.SupportedFormatString () + 
                                   " tickerid");
                }
            }
            catch (Exception e)
            {
                Console.WriteLine ("Exception Thrown:-");
                Console.Write (e.Message);
            }
        }
    }
}
