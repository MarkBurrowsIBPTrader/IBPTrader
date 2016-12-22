-- MySQL dump 10.13  Distrib 5.5.13, for Win32 (x86)
--
-- Host: localhost    Database: ibrokers_v2
-- ------------------------------------------------------
-- Server version	5.5.13

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `deals`
--

DROP TABLE IF EXISTS `deals`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `deals` (
  `iddeals` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `tickerid` int(11) NOT NULL,
  `ticker` text NOT NULL,
  `oi` int(11) NOT NULL,
  `buyprice` double NOT NULL,
  `numberbuyshares` int(11) NOT NULL,
  `sellprice` double NOT NULL,
  `numbersellshares` int(11) NOT NULL,
  `sectype` text NOT NULL,
  `exchange` text NOT NULL,
  `primaryexchange` text NOT NULL,
  `currency` text NOT NULL,
  `localsymbol` text NOT NULL,
  `secidtype` text NOT NULL,
  `secid` text NOT NULL,
  `genericticks` text NOT NULL,
  `displayname` text NOT NULL,
  `username` text NOT NULL,
  `accountnumber` text NOT NULL,
  `hitcount` int(11) NOT NULL,
  `lastavgfillbuyprice` double DEFAULT NULL,
  `lastbuycommission` double DEFAULT NULL,
  `lastbuydate` datetime DEFAULT NULL,
  `updatetimestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`iddeals`),
  UNIQUE KEY `iddeals_UNIQUE` (`iddeals`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `orders`
--

DROP TABLE IF EXISTS `orders`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `orders` (
  `idorders` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `orderid` int(11) NOT NULL,
  `iddeals` int(10) unsigned NOT NULL,
  `buyorsell` text NOT NULL,
  `ticker` text NOT NULL,
  `price` double NOT NULL,
  `username` text NOT NULL,
  `accountnumber` text NOT NULL,
  `batchsize` int(11) NOT NULL,
  `createtimestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`idorders`),
  UNIQUE KEY `idorders_UNIQUE` (`idorders`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pgens`
--

DROP TABLE IF EXISTS `pgens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pgens` (
  `idpgens` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `pgenxml` text NOT NULL,
  `username` text NOT NULL,
  `accountnumber` text,
  PRIMARY KEY (`idpgens`),
  UNIQUE KEY `idpgens_UNIQUE` (`idpgens`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `recordedprices`
--

DROP TABLE IF EXISTS `recordedprices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `recordedprices` (
  `idrecordedprices` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ticker` text NOT NULL,
  `primaryexchange` text NOT NULL,
  `currency` text NOT NULL,
  `bid` double NOT NULL,
  `offer` double NOT NULL,
  `volume` int(11) NOT NULL,
  `createtimestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`idrecordedprices`),
  UNIQUE KEY `idrecordedprices_UNIQUE` (`idrecordedprices`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `shares`
--

DROP TABLE IF EXISTS `shares`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `shares` (
  `idshares` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `tickerid` int(11) NOT NULL,
  `ticker` text NOT NULL,
  `displayname` text NOT NULL,
  `primaryexchange` text NOT NULL,
  `sectype` text NOT NULL,
  `minoi` int(11) NOT NULL,
  `maxoi` int(11) NOT NULL,
  `maxvalue` double NOT NULL,
  `username` text NOT NULL,
  `accountnumber` text NOT NULL,
  PRIMARY KEY (`idshares`),
  UNIQUE KEY `idshares_UNIQUE` (`idshares`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `trades`
--

DROP TABLE IF EXISTS `trades`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `trades` (
  `idtrades` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ticker` text NOT NULL,
  `buyorsell` text NOT NULL,
  `fillcount` int(11) NOT NULL,
  `avgfillprice` double NOT NULL,
  `lastfillprice` double NOT NULL,
  `totalvalue` double NOT NULL,
  `commission` double NOT NULL,
  `mincommission` double NOT NULL,
  `maxcommission` double NOT NULL,
  `commissioncurrency` text NOT NULL,
  `buyidtradesrow` int(10) unsigned DEFAULT NULL,
  `iddealsrowid` int(10) unsigned NOT NULL,
  `pgenbuyprice` double NOT NULL,
  `pgensellprice` double NOT NULL,
  `grossprofit` double NOT NULL,
  `netprofit` double NOT NULL,
  `sectype` text NOT NULL,
  `exchange` text NOT NULL,
  `primaryexchange` text NOT NULL,
  `currency` text NOT NULL,
  `username` text NOT NULL,
  `accountnumber` text NOT NULL,
  `createtimestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `batchsize` int(11) NOT NULL,
  `mintrend` double NOT NULL,
  `maxtrend` double NOT NULL,
  `actualtrend` double NOT NULL,
  PRIMARY KEY (`idtrades`),
  UNIQUE KEY `idtrades_UNIQUE` (`idtrades`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-04-05 18:36:52
