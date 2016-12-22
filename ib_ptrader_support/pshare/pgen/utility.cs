using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Text;
using MySql.Data.MySqlClient;
using MySql.Data.Types;



namespace pshare
{
    static public class Utility
    { 
        #region "Display"
        static public string DisplayMoney (this double value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORMONEY).ToString ();
        }

        static public string DisplayMoney (this double value, int rounding)
        {
            return Math.Round (value, rounding).ToString ();
        }

        static public string DisplayMoney (this decimal value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORMONEY).ToString ();
        }

        static public string DisplayMoney (this decimal value, int rounding)
        {
            return Math.Round (value, rounding).ToString ();
        }

        static public string DisplayWeight (this double value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORWEIGHTS).ToString ();
        }

        static public string DisplayWeight (this double value, int rounding)
        {
            return Math.Round (value, rounding).ToString ();
        }

        static public string DisplayPercentage (this double value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORPERCENTAGE).ToString ();
        }

        static public string DisplayPercentage (this decimal value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORPERCENTAGE).ToString ();
        }

        static public double RoundMoney (this double value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORMONEY);
        }

        static public decimal RoundMoney (this decimal value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORMONEY);
        }

        static public double RoundWeight (this double value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORWEIGHTS);
        }

        static public double RoundWeight (this double value, int rounding)
        {
            return Math.Round (value, rounding);
        }

        static public double RoundPercentage (this double value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORPERCENTAGE);
        }

        static public decimal RoundPercentage (this decimal value)
        {
            return Math.Round (value, Constants.ROUNDING_SIZE_FORPERCENTAGE);
        }
        #endregion

        #region "File/Directory Support"
        static public Exception CreateDirectory (string directory)
        {
            if (directory != null)
            {
                try
                {
                    if (! Directory.Exists (directory))
                        Directory.CreateDirectory (directory);
                }
                catch (System.Threading.ThreadAbortException e)
                {
                    throw e;
                }
                catch (Exception e)
                {
                    return e;
                }
            }
            return null;
        }
        #endregion
    }

    static public class Constants
    {
        #region "Constants"
        public const int ROUNDING_SIZE_FORWEIGHTS = 3;
        public const int ROUNDING_SIZE_FORMONEY = 2;
        public const int ROUNDING_SIZE_FORPERCENTAGE = 2;
        #endregion
    }

    static class Fields
    {
        #region "Collate Fields"
        static public List <string> CollateFields (int startindex, bool removesquarebracks, string [] args)
        {
            string lastline;
            List <string> fields = new List <string> ();
            for (int i = startindex;  i < args.Length;  i++)
            {
                if (args [i] [0] == '[')
                {
                    fields.Add (args [i]);
                }
                else
                {
                    if (fields.Count == 0)
                    {
                        Console.WriteLine ("Badly formed command line");
                        return null;
                    }
                    lastline = fields [fields.Count - 1];
                    if (lastline [lastline.Length - 1] == ']')
                    {
                        Console.WriteLine ("Badly formed command line");
                        return null;
                    }
                    fields [fields.Count - 1] += " " + args [i];
                }
            }
            if (fields.Count == 0)
            {
                Console.WriteLine ("Badly formed command line");
                return null;
            }                
            lastline = fields [fields.Count - 1];
            if (lastline [lastline.Length - 1] != ']')
            {
                Console.WriteLine ("Badly formed command line");
                return null;
            }
            if (removesquarebracks)
            {
                for (int i = 0;  i < fields.Count;  i++)
                {
                    string input = fields [i];
                    if (input.Length >= 2 && input [0] == '[' && input [input.Length - 1] == ']')
                    {
                        fields [i] = input.Substring (1, input.Length - 2);
                    }
                    else
                    {
                        return null;
                    }
                }
            }
            return fields;
        }
        #endregion
    }
}
