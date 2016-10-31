Data Driven System Article Source
=================================

The source file creature_spawning_schema.sql is a script that can be
used to recreate schema of the example Creature Spawning subsystem in
the article.

It has been tested on MS SQL Server 2000, but should also be compatible
with SQL Server version 7.  It may work on earlier versions, but
hopefully you don't have to deal with anything older than Version 7!

The script can most easily be used by running SQL Query Analyzer and
connecting to a database via a login that has the authority to create 
tables in that database.  Use the File menu and open the file.  Once
the script is loaded, run it by pressing F5 or selecting the Execute
Query menu option.

The sample stored procedure from the article will also get created
along with the schema.  It is called sp_CheckForBadSpawners.  You
are encouraged to place sample data in the various tables, and 
write your own stored procedures using the one provided as a 
launching point.

Good luck.  I look forward to hearing from you about your progress with
data driven systems!


Jay Lee

jlee@ncaustin.com